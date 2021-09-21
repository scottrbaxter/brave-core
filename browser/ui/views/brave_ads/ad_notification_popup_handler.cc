/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/ui/brave_ads/ad_notification_popup_handler.h"

#include "brave/browser/ui/brave_ads/ad_notification.h"
#include "brave/browser/ui/brave_ads/ad_notification_delegate.h"
#include "brave/browser/ui/views/brave_ads/ad_notification_popup.h"
#include "brave/browser/ui/views/brave_ads/ad_notification_popup_collection.h"

namespace brave_ads {

AdNotificationPopupHandler::AdNotificationPopupHandler() = default;

AdNotificationPopupHandler::~AdNotificationPopupHandler() = default;

// static
void AdNotificationPopupHandler::Show(Profile* profile,
                                      const AdNotification& ad_notification) {
  DCHECK(profile);

  const std::string& id = ad_notification.id();

  AdNotificationPopup* popup =
      new AdNotificationPopup(profile, ad_notification);
  AdNotificationPopupCollection::Add(id, popup);

  AdNotificationDelegate* delegate = ad_notification.delegate();
  if (delegate) {
    delegate->OnShow();
  }
}

// static
void AdNotificationPopupHandler::Close(const std::string& notification_id) {
  DCHECK(!notification_id.empty());

  AdNotificationPopup* popup =
      AdNotificationPopupCollection::Get(notification_id);
  if (!popup) {
    return;
  }

  const AdNotification ad_notification = popup->GetAdNotification();
  AdNotificationDelegate* delegate = ad_notification.delegate();
  if (delegate) {
    delegate->OnClose(/* by_user */ false);
  }

  popup->ClosePopup();
}

}  // namespace brave_ads
