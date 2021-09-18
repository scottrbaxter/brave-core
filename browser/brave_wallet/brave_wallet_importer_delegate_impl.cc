/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/brave_wallet/brave_wallet_importer_delegate_impl.h"

#include <utility>

#include "base/base64.h"
#include "base/bind_post_task.h"
#include "base/json/json_reader.h"
#include "base/threading/sequenced_task_runner_handle.h"
#include "brave/browser/ethereum_remote_client/ethereum_remote_client_constants.h"
#include "brave/browser/ethereum_remote_client/ethereum_remote_client_service.h"
#include "brave/browser/ethereum_remote_client/ethereum_remote_client_service_factory.h"
#include "brave/components/brave_wallet/browser/password_encryptor.h"
#include "content/public/browser/browser_thread.h"
#include "extensions/browser/api/storage/backend_task_runner.h"
#include "extensions/browser/api/storage/storage_frontend.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/browser/extension_system.h"
#include "extensions/browser/value_store/value_store.h"
#include "extensions/common/extension.h"
#include "extensions/common/mojom/manifest.mojom.h"

using content::BrowserThread;
using extensions::Extension;
using extensions::ExtensionRegistry;
using extensions::IsOnBackendSequence;
using extensions::StorageFrontend;
using extensions::mojom::ManifestLocation;

namespace brave_wallet {

namespace {
void OnRunWithStorage(
    base::OnceCallback<void(std::unique_ptr<base::DictionaryValue>)> callback,
    ValueStore* storage) {
  DCHECK(IsOnBackendSequence());
  DCHECK(storage);
  ValueStore::ReadResult result = storage->Get();
  std::move(callback).Run(result.PassSettings());
}

static base::span<const uint8_t> ToSpan(base::StringPiece sp) {
  return base::as_bytes(base::make_span(sp));
}

}  // namespace

BraveWalletImporterDelegateImpl::BraveWalletImporterDelegateImpl(
    content::BrowserContext* context)
    : context_(context), weak_ptr_factory_(this) {}

BraveWalletImporterDelegateImpl::~BraveWalletImporterDelegateImpl() = default;

void BraveWalletImporterDelegateImpl::ImportFromBraveCryptoWallet(
    const std::string& password,
    ImportFromBraveCryptoWalletCallback callback) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  DCHECK(context_);
  extension_registry_observer_.Observe(ExtensionRegistry::Get(context_));

  EthereumRemoteClientService* service =
      EthereumRemoteClientServiceFactory::GetInstance()->GetForContext(
          context_);
  service->MaybeLoadCryptoWalletsExtension(base::DoNothing());
  callback_ = std::move(callback);
  password_ = password;
}
void BraveWalletImporterDelegateImpl::ImportFromMetamask(
    const std::string& password,
    ImportFromMetamaskCallback callback) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  DCHECK(context_);

  ExtensionRegistry* registry = ExtensionRegistry::Get(context_);
  const Extension* extension =
      registry->GetInstalledExtension(metamask_extension_id);
  if (!extension) {
    std::move(callback).Run(false);
    return;
  }

  GetLocalStorage(extension, std::move(callback));
  password_ = password;
}

void BraveWalletImporterDelegateImpl::OnExtensionLoaded(
    content::BrowserContext* browser_context,
    const extensions::Extension* extension) {
  // Wait for reinstall as normal extension
  if (extension->location() == ManifestLocation::kComponent ||
      extension->id() != ethereum_remote_client_extension_id) {
    return;
  }
  extension_registry_observer_.Reset();

  GetLocalStorage(extension, std::move(callback_));

  EthereumRemoteClientService* service =
      EthereumRemoteClientServiceFactory::GetInstance()->GetForContext(
          context_);
  DCHECK(service);
  service->RemoveCryptoWalletExtension();
}

void BraveWalletImporterDelegateImpl::GetLocalStorage(
    const extensions::Extension* extension,
    base::OnceCallback<void(bool)> callback) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);

  std::string error;
  extension_ = Extension::Create(
      extension->path(), ManifestLocation::kExternalPref,
      *extension->manifest()->value(), extension->creation_flags(), &error);

  StorageFrontend* frontend = StorageFrontend::Get(context_);
  if (!frontend) {
    std::move(callback).Run(false);
    return;
  }

  // Passing the result back using BindPostTask because OnRunWithStorage will
  // run on backend thread
  frontend->RunWithStorage(
      extension_, extensions::settings_namespace::LOCAL,
      base::BindOnce(
          &OnRunWithStorage,
          base::BindPostTask(
              base::SequencedTaskRunnerHandle::Get(),
              base::BindOnce(
                  &BraveWalletImporterDelegateImpl ::OnGetLocalStorage,
                  weak_ptr_factory_.GetWeakPtr(), std::move(callback)))));
}

void BraveWalletImporterDelegateImpl::OnGetLocalStorage(
    base::OnceCallback<void(bool)> callback,
    std::unique_ptr<base::DictionaryValue> dict) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  const std::string* vault_str =
      dict->FindStringPath("data.KeyringController.vault");
  if (!vault_str) {
    VLOG(0) << "cannot find data.KeyringController.vault";
    std::move(callback).Run(false);
    return;
  }
  auto vault = base::JSONReader::Read(*vault_str);
  if (!vault) {
    std::move(callback).Run(false);
    return;
  }
  auto* data_str = vault->FindStringKey("data");
  auto* iv_str = vault->FindStringKey("iv");
  auto* salt_str = vault->FindStringKey("salt");
  if (!data_str || !iv_str || !salt_str) {
    std::move(callback).Run(false);
    return;
  }

  std::string salt_decoded;
  if (!base::Base64Decode(*salt_str, &salt_decoded)) {
    std::move(callback).Run(false);
    return;
  }
  std::string iv_decoded;
  if (!base::Base64Decode(*iv_str, &iv_decoded)) {
    std::move(callback).Run(false);
    return;
  }
  std::string data_decoded;
  if (!base::Base64Decode(*data_str, &data_decoded)) {
    std::move(callback).Run(false);
    return;
  }

  std::unique_ptr<PasswordEncryptor> encryptor =
      PasswordEncryptor::DeriveKeyFromPasswordUsingPbkdf2(
          password_, ToSpan(salt_decoded), 10000, 256);
  DCHECK(encryptor);

  std::vector<uint8_t> decrypted_keyring;
  if (!encryptor->DecryptForImporter(ToSpan(data_decoded), ToSpan(iv_decoded),
                                     &decrypted_keyring)) {
    VLOG(1) << "Importer decryption failed";
    std::move(callback).Run(false);
    return;
  }

  LOG(ERROR) << std::string(decrypted_keyring.begin(), decrypted_keyring.end());

  std::move(callback).Run(true);
}

}  // namespace brave_wallet
