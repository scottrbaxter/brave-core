/* Copyright 2016 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_GREASELION_BROWSER_GREASELION_SERVICE_IMPL_H_
#define BRAVE_COMPONENTS_GREASELION_BROWSER_GREASELION_SERVICE_IMPL_H_

#include <map>
#include <string>
#include <vector>

#include "base/files/file_path.h"
#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "brave/components/greaselion/browser/greaselion_service.h"
#include "extensions/common/extension_id.h"
#include "url/gurl.h"

namespace base {
class SequencedTaskRunner;
}

namespace extensions {
class Extension;
class ExtensionRegistry;
class ExtensionService;
class ExtensionSystem;
}  // namespace extensions

namespace greaselion {

class GreaselionDownloadService;

class GreaselionServiceImpl : public GreaselionService {
 public:
  explicit GreaselionServiceImpl(
      GreaselionDownloadService* download_service,
      const base::FilePath& install_directory,
      extensions::ExtensionSystem* extension_system,
      extensions::ExtensionRegistry* extension_registry,
      scoped_refptr<base::SequencedTaskRunner> task_runner);
  ~GreaselionServiceImpl() override;

  // GreaselionService overrides
  void SetFeatureEnabled(GreaselionFeature feature, bool enabled) override;
  void UpdateInstalledExtensions() override;
  bool IsGreaselionExtension(const std::string& id) override;
  bool ready() override;
  void AddObserver(Observer* observer) override;
  void RemoveObserver(Observer* observer) override;

  // ExtensionRegistryObserver overrides
  void OnExtensionReady(content::BrowserContext* browser_context,
                        const extensions::Extension* extension) override;
  void OnExtensionUnloaded(content::BrowserContext* browser_context,
                           const extensions::Extension* extension,
                           extensions::UnloadedExtensionReason reason) override;

 private:
  void CreateAndInstallExtensions();
  void PostConvert(scoped_refptr<extensions::Extension> extension);
  void Install(scoped_refptr<extensions::Extension> extension);
  void MaybeNotifyObservers();

  GreaselionDownloadService* download_service_;  // NOT OWNED
  GreaselionFeatures state_;
  const base::FilePath install_directory_;
  extensions::ExtensionSystem* extension_system_;      // NOT OWNED
  extensions::ExtensionService* extension_service_;    // NOT OWNED
  extensions::ExtensionRegistry* extension_registry_;  // NOT OWNED
  bool all_rules_installed_successfully_;
  bool update_in_progress_;
  int pending_installs_;
  scoped_refptr<base::SequencedTaskRunner> task_runner_;
  base::ObserverList<Observer> observers_;
  std::vector<extensions::ExtensionId> greaselion_extensions_;
  base::WeakPtrFactory<GreaselionServiceImpl> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(GreaselionServiceImpl);
};

}  // namespace greaselion

#endif  // BRAVE_COMPONENTS_GREASELION_BROWSER_GREASELION_SERVICE_IMPL_H_
