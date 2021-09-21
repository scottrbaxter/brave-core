/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/common/webui_url_constants.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "content/public/browser/web_contents.h"
#include "url/gurl.h"

namespace chrome {

bool IsBravePanel(content::WebContents* content) {
  return content->GetURL().EqualsIgnoringRef(GURL(kBraveUIWalletPanelURL));
}

Browser* BraveFindBrowserIfWalletPanel(content::WebContents* contents) {
  if (IsBravePanel(contents))
    return chrome::FindBrowserWithProfile(
        Profile::FromBrowserContext(contents->GetBrowserContext()));
  return chrome::FindBrowserWithWebContents(contents);
}

}  // namespace chrome

#define FindBrowserWithWebContents BraveFindBrowserIfWalletPanel
#define GetActiveWebContents                           \
  GetActiveWebContents() && !IsBravePanel(contents) && \
      browser->tab_strip_model()->GetActiveWebContents

#include "../../../../../../../chrome/browser/ui/views/permission_bubble/chooser_bubble_ui.cc"
#undef GetActiveWebContents
#undef FindBrowserWithWebContents
