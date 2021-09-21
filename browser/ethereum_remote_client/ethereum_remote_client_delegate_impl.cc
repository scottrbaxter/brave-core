/* Copyright (c) 2021 The Brave Authors. All rights reserved.
+ * This Source Code Form is subject to the terms of the Mozilla Public
+ * License, v. 2.0. If a copy of the MPL was not distributed with this file,
+ * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/ethereum_remote_client/ethereum_remote_client_delegate_impl.h"
#include "extensions/buildflags/buildflags.h"

#if BUILDFLAG(ENABLE_EXTENSIONS)
#include "brave/browser/extensions/brave_component_loader.h"
#include "chrome/browser/extensions/extension_service.h"
#include "content/public/browser/browser_context.h"
#include "extensions/browser/extension_system.h"
#endif  // #if BUILDFLAG(ENABLE_EXTENSIONS)

EthereumRemoteClientDelegateImpl::~EthereumRemoteClientDelegateImpl() {}

void EthereumRemoteClientDelegateImpl::MaybeLoadCryptoWalletsExtension(
    content::BrowserContext* context) {
#if BUILDFLAG(ENABLE_EXTENSIONS)
  extensions::ExtensionService* service =
      extensions::ExtensionSystem::Get(context)->extension_service();
  if (service) {
    extensions::ComponentLoader* loader = service->component_loader();
    static_cast<extensions::BraveComponentLoader*>(loader)
        ->AddEthereumRemoteClientExtension();
  }
#endif  // #if BUILDFLAG(ENABLE_EXTENSIONS)
}

void EthereumRemoteClientDelegateImpl::RemoveCryptoWalletExtension(
    content::BrowserContext* context) {
#if BUILDFLAG(ENABLE_EXTENSIONS)
  extensions::ExtensionService* service =
      extensions::ExtensionSystem::Get(context)->extension_service();
  if (service) {
    extensions::ComponentLoader* loader = service->component_loader();
    static_cast<extensions::BraveComponentLoader*>(loader)
        ->RemoveEthereumRemoteClientExtension();
  }
#endif  // #if BUILDFLAG(ENABLE_EXTENSIONS)
}
