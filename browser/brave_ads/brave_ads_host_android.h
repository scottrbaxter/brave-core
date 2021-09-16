// Copyright (c) 2021 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef BRAVE_BROWSER_BRAVE_ADS_BRAVE_ADS_HOST_ANDROID_H_
#define BRAVE_BROWSER_BRAVE_ADS_BRAVE_ADS_HOST_ANDROID_H_

#include "base/android/jni_android.h"
#include "base/scoped_observation.h"
#include "brave/components/brave_ads/common/brave_ads_host.mojom.h"
#include "brave/components/brave_rewards/browser/rewards_service.h"
#include "brave/components/brave_rewards/browser/rewards_service_observer.h"

class Profile;

namespace content {
class WebContents;
}

namespace brave_ads {

// The class handles chrome.braveRequestAdsEnabled() js api call for Android
// platform. The js api asks the user for permission to enable ads.
class BraveAdsHostAndroid : public brave_ads::mojom::BraveAdsHost,
                            public brave_rewards::RewardsServiceObserver {
 public:
  explicit BraveAdsHostAndroid(content::WebContents* web_contents);
  BraveAdsHostAndroid(const BraveAdsHostAndroid&) = delete;
  BraveAdsHostAndroid& operator=(const BraveAdsHostAndroid&) = delete;
  ~BraveAdsHostAndroid() override;

  // brave_ads::mojom::BraveAdsHost
  void RequestAdsEnabled(RequestAdsEnabledCallback callback) override;

  // brave_rewards::RewardsServiceObserver
  void OnAdsEnabled(brave_rewards::RewardsService* rewards_service,
                    bool ads_enabled) override;

  void NotifyAdsEnabled(JNIEnv* env,
                        const base::android::JavaParamRef<jobject>& obj);
  void NotifyPopupClosed(JNIEnv* env,
                         const base::android::JavaParamRef<jobject>& obj);

 private:
  void RunCallbackAndReset(bool result);

  Profile* profile_ = nullptr;
  base::android::ScopedJavaGlobalRef<jobject> java_object_;
  RequestAdsEnabledCallback callback_;
  base::ScopedObservation<brave_rewards::RewardsService,
                          brave_rewards::RewardsServiceObserver>
      rewards_service_observation_{this};
  bool enable_ads_pressed_ = false;
};

}  // namespace brave_ads

#endif  // BRAVE_BROWSER_BRAVE_ADS_BRAVE_ADS_HOST_ANDROID_H_
