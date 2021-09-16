// Copyright (c) 2021 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

#include "brave/browser/brave_ads/brave_ads_host_android.h"

#include <utility>

#include "base/check.h"
#include "brave/browser/brave_ads/ads_service_factory.h"
#include "brave/browser/brave_ads/android/jni_headers/BraveAdsHostAndroid_jni.h"
#include "brave/browser/brave_rewards/rewards_service_factory.h"
#include "brave/components/brave_ads/browser/ads_service.h"
#include "brave/components/brave_rewards/browser/rewards_service.h"
#include "chrome/browser/profiles/profile.h"
#include "content/public/browser/web_contents.h"

using base::android::AttachCurrentThread;
using base::android::JavaParamRef;

namespace brave_ads {

BraveAdsHostAndroid::BraveAdsHostAndroid(content::WebContents* web_contents) {
  DCHECK(web_contents);
  profile_ = Profile::FromBrowserContext(web_contents->GetBrowserContext());
  DCHECK(profile_);

  java_object_.Reset(Java_BraveAdsHostAndroid_create(
      AttachCurrentThread(), reinterpret_cast<intptr_t>(this)));
}

BraveAdsHostAndroid::~BraveAdsHostAndroid() {
  Java_BraveAdsHostAndroid_closeBraveTalkOptInPopup(AttachCurrentThread(),
                                                    java_object_);
  Java_BraveAdsHostAndroid_destroy(AttachCurrentThread(), java_object_);
}

void BraveAdsHostAndroid::RequestAdsEnabled(
    RequestAdsEnabledCallback callback) {
  if (callback_) {
    std::move(callback).Run(false);
    return;
  }

  const AdsService* ads_service = AdsServiceFactory::GetForProfile(profile_);
  brave_rewards::RewardsService* rewards_service =
      brave_rewards::RewardsServiceFactory::GetForProfile(profile_);
  if (!rewards_service || !ads_service || !ads_service->IsSupportedLocale()) {
    std::move(callback).Run(false);
    return;
  }

  if (ads_service->IsEnabled()) {
    std::move(callback).Run(true);
    return;
  }

  callback_ = std::move(callback);
  rewards_service_observation_.Observe(rewards_service);

  Java_BraveAdsHostAndroid_openBraveTalkOptInPopup(AttachCurrentThread(),
                                                   java_object_);
}

void BraveAdsHostAndroid::OnAdsEnabled(
    brave_rewards::RewardsService* rewards_service,
    bool ads_enabled) {
  DCHECK(rewards_service);

  RunCallbackAndReset(ads_enabled);
}

void BraveAdsHostAndroid::NotifyAdsEnabled(JNIEnv* env,
                                           const JavaParamRef<jobject>& obj) {
  DCHECK(callback_);

  brave_rewards::RewardsService* rewards_service =
      brave_rewards::RewardsServiceFactory::GetForProfile(profile_);
  if (!rewards_service) {
    RunCallbackAndReset(false);
    return;
  }

  rewards_service->EnableRewards();
  enable_ads_pressed_ = true;
}

void BraveAdsHostAndroid::NotifyPopupClosed(JNIEnv* env,
                                            const JavaParamRef<jobject>& obj) {
  if (!callback_) {
    return;
  }

  // If ads enable was pressed then do nothing and wait for ads enabled event.
  if (!enable_ads_pressed_) {
    RunCallbackAndReset(false);
  }
}

void BraveAdsHostAndroid::RunCallbackAndReset(bool ads_enabled) {
  DCHECK(callback_);

  enable_ads_pressed_ = false;
  rewards_service_observation_.Reset();

  std::move(callback_).Run(ads_enabled);
}

}  // namespace brave_ads
