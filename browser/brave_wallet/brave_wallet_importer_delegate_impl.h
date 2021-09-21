/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_BROWSER_BRAVE_WALLET_BRAVE_WALLET_IMPORTER_DELEGATE_IMPL_H_
#define BRAVE_BROWSER_BRAVE_WALLET_BRAVE_WALLET_IMPORTER_DELEGATE_IMPL_H_

#include <memory>
#include <string>
#include <vector>

#include "base/memory/scoped_refptr.h"
#include "base/memory/weak_ptr.h"
#include "base/scoped_observation.h"
#include "base/values.h"
#include "brave/components/brave_wallet/browser/brave_wallet_importer_delegate.h"
#include "brave/components/brave_wallet/common/brave_wallet.mojom.h"
#include "mojo/public/cpp/bindings/remote.h"

class ValueStore;

namespace content {
class BrowserContext;
}

namespace extensions {
class Extension;
}

namespace brave_wallet {

class BraveWalletImporterDelegateImpl : public BraveWalletImporterDelegate {
 public:
  explicit BraveWalletImporterDelegateImpl(content::BrowserContext* context);
  BraveWalletImporterDelegateImpl(const BraveWalletImporterDelegateImpl&) =
      delete;
  BraveWalletImporterDelegateImpl& operator=(
      const BraveWalletImporterDelegateImpl&) = delete;
  ~BraveWalletImporterDelegateImpl() override;

  void IsBraveCryptoWalletInstalled(
      IsBraveCryptoWalletInstalledCallback callback) override;
  void IsMetamaskInstalled(IsMetamaskInstalledCallback callback) override;
  void ImportFromBraveCryptoWallet(
      const std::string& password,
      const std::string& new_password,
      ImportFromBraveCryptoWalletCallback callback) override;
  void ImportFromMetamask(const std::string& password,
                          const std::string& new_password,
                          ImportFromMetamaskCallback callback) override;

 private:
  void OnCryptoWalletLoaded(ImportFromMetamaskCallback callback,
                            bool should_unload);

  void GetLocalStorage(const extensions::Extension* extension,
                       ImportFromBraveCryptoWalletCallback callback);
  void OnGetLocalStorage(ImportFromBraveCryptoWalletCallback callback,
                         std::unique_ptr<base::DictionaryValue> dict);

  bool IsLegacyBraveCryptoWallet() const;
  bool IsBraveCryptoWalletInstalledInternal() const;
  const extensions::Extension* GetBraveCryptoWallet();
  const extensions::Extension* GetMetamask();

  void EnsureConnected();
  void OnConnectionError();

  mojo::Remote<brave_wallet::mojom::KeyringController> keyring_controller_;
  content::BrowserContext* context_;
  std::string password_;
  std::string new_password_;
  scoped_refptr<extensions::Extension> extension_;
  base::WeakPtrFactory<BraveWalletImporterDelegateImpl> weak_ptr_factory_;
};

}  // namespace brave_wallet

#endif  // BRAVE_BROWSER_BRAVE_WALLET_BRAVE_WALLET_IMPORTER_DELEGATE_IMPL_H_
