/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/ui/webui/brave_adblock_ui.h"

#include <memory>
#include <utility>

#include "base/scoped_observation.h"
#include "brave/browser/brave_browser_process.h"
#include "brave/browser/ui/webui/brave_webui_source.h"
#include "brave/common/webui_url_constants.h"
#include "brave/components/brave_adblock/resources/grit/brave_adblock_generated_map.h"
#include "brave/components/brave_shields/browser/ad_block_custom_filters_service.h"
#include "brave/components/brave_shields/browser/ad_block_regional_service_manager.h"
#include "brave/components/brave_shields/browser/ad_block_service.h"
#include "brave/components/brave_shields/browser/ad_block_service_helper.h"
#include "brave/components/brave_shields/browser/ad_block_subscription_service_manager.h"
#include "brave/components/brave_shields/browser/ad_block_subscription_service_manager_observer.h"
#include "components/grit/brave_components_resources.h"
#include "content/public/browser/web_ui_message_handler.h"

#if defined(OS_ANDROID)
#include "chrome/browser/android/tab_web_contents_delegate_android.h"
#include "content/public/browser/web_contents.h"
#else
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/browser_navigator_params.h"
#include "chrome/browser/ui/singleton_tabs.h"
#endif

namespace {

class AdblockDOMHandler
    : public content::WebUIMessageHandler,
      public brave_shields::AdBlockSubscriptionServiceManagerObserver {
 public:
  AdblockDOMHandler();
  ~AdblockDOMHandler() override;

  // WebUIMessageHandler implementation.
  void RegisterMessages() override;

  // brave_shields::AdblockSubscriptionServiceManagerObserver overrides:
  void OnServiceUpdateEvent() override;

  void OnJavascriptAllowed() override;
  void OnJavascriptDisallowed() override;

 private:
  void HandleEnableFilterList(const base::ListValue* args);
  void HandleGetCustomFilters(const base::ListValue* args);
  void HandleGetRegionalLists(const base::ListValue* args);
  void HandleGetListSubscriptions(const base::ListValue* args);
  void HandleUpdateCustomFilters(const base::ListValue* args);
  void HandleSubmitNewSubscription(const base::ListValue* args);
  void HandleSetSubscriptionEnabled(const base::ListValue* args);
  void HandleDeleteSubscription(const base::ListValue* args);
  void HandleRefreshSubscription(const base::ListValue* args);
  void HandleViewSubscriptionSource(const base::ListValue* args);

  void RefreshSubscriptionsList();

  base::ScopedObservation<
      brave_shields::AdBlockSubscriptionServiceManager,
      brave_shields::AdBlockSubscriptionServiceManagerObserver>
      service_observer_{this};

  DISALLOW_COPY_AND_ASSIGN(AdblockDOMHandler);
};

AdblockDOMHandler::AdblockDOMHandler() {}

AdblockDOMHandler::~AdblockDOMHandler() {}

void AdblockDOMHandler::RegisterMessages() {
  web_ui()->RegisterMessageCallback(
      "brave_adblock.enableFilterList",
      base::BindRepeating(&AdblockDOMHandler::HandleEnableFilterList,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "brave_adblock.getCustomFilters",
      base::BindRepeating(&AdblockDOMHandler::HandleGetCustomFilters,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "brave_adblock.getRegionalLists",
      base::BindRepeating(&AdblockDOMHandler::HandleGetRegionalLists,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "brave_adblock.getListSubscriptions",
      base::BindRepeating(&AdblockDOMHandler::HandleGetListSubscriptions,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "brave_adblock.updateCustomFilters",
      base::BindRepeating(&AdblockDOMHandler::HandleUpdateCustomFilters,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "brave_adblock.submitNewSubscription",
      base::BindRepeating(&AdblockDOMHandler::HandleSubmitNewSubscription,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "brave_adblock.setSubscriptionEnabled",
      base::BindRepeating(&AdblockDOMHandler::HandleSetSubscriptionEnabled,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "brave_adblock.deleteSubscription",
      base::BindRepeating(&AdblockDOMHandler::HandleDeleteSubscription,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "brave_adblock.refreshSubscription",
      base::BindRepeating(&AdblockDOMHandler::HandleRefreshSubscription,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "brave_adblock.viewSubscriptionSource",
      base::BindRepeating(&AdblockDOMHandler::HandleViewSubscriptionSource,
                          base::Unretained(this)));
}

void AdblockDOMHandler::OnJavascriptAllowed() {
  service_observer_.Observe(g_brave_browser_process->ad_block_service()
                                ->subscription_service_manager());
}

void AdblockDOMHandler::OnJavascriptDisallowed() {
  service_observer_.Reset();
}

void AdblockDOMHandler::OnServiceUpdateEvent() {
  if (!IsJavascriptAllowed()) {
    return;
  }
  RefreshSubscriptionsList();
}

void AdblockDOMHandler::HandleEnableFilterList(const base::ListValue* args) {
  DCHECK_EQ(args->GetSize(), 2U);
  std::string uuid;
  if (!args->GetString(0, &uuid))
    return;
  bool enabled;
  if (!args->GetBoolean(1, &enabled))
    return;
  g_brave_browser_process->ad_block_regional_service_manager()
      ->EnableFilterList(uuid, enabled);
}

void AdblockDOMHandler::HandleGetCustomFilters(const base::ListValue* args) {
  DCHECK_EQ(args->GetSize(), 0U);
  AllowJavascript();
  const std::string custom_filters =
      g_brave_browser_process->ad_block_custom_filters_service()
          ->GetCustomFilters();
  CallJavascriptFunction("brave_adblock.onGetCustomFilters",
                         base::Value(custom_filters));
}

void AdblockDOMHandler::HandleGetRegionalLists(const base::ListValue* args) {
  DCHECK_EQ(args->GetSize(), 0U);
  AllowJavascript();
  std::unique_ptr<base::ListValue> regional_lists =
      g_brave_browser_process->ad_block_regional_service_manager()
          ->GetRegionalLists();
  CallJavascriptFunction("brave_adblock.onGetRegionalLists", *regional_lists);
}

void AdblockDOMHandler::HandleGetListSubscriptions(
    const base::ListValue* args) {
  DCHECK_EQ(args->GetSize(), 0U);
  AllowJavascript();
  RefreshSubscriptionsList();
}

void AdblockDOMHandler::HandleUpdateCustomFilters(const base::ListValue* args) {
  DCHECK_EQ(args->GetSize(), 1U);
  std::string custom_filters;
  if (!args->GetString(0, &custom_filters))
    return;

  g_brave_browser_process->ad_block_custom_filters_service()
      ->UpdateCustomFilters(custom_filters);
}

void AdblockDOMHandler::HandleSubmitNewSubscription(
    const base::ListValue* args) {
  DCHECK_EQ(args->GetSize(), 1U);
  AllowJavascript();
  std::string subscription_url_string;
  if (!args->GetString(0, &subscription_url_string)) {
    return;
  }
  const GURL subscription_url = GURL(subscription_url_string);
  if (!subscription_url.is_valid()) {
    return;
  }

  g_brave_browser_process->ad_block_service()
      ->subscription_service_manager()
      ->CreateSubscription(subscription_url);
  RefreshSubscriptionsList();
}

void AdblockDOMHandler::HandleSetSubscriptionEnabled(
    const base::ListValue* args) {
  DCHECK_EQ(args->GetSize(), 2U);
  AllowJavascript();
  std::string subscription_url_string;
  if (!args->GetString(0, &subscription_url_string)) {
    return;
  }
  bool enabled;
  if (!args->GetBoolean(1, &enabled)) {
    return;
  }
  const GURL subscription_url = GURL(subscription_url_string);
  if (!subscription_url.is_valid()) {
    return;
  }
  g_brave_browser_process->ad_block_service()
      ->subscription_service_manager()
      ->EnableSubscription(subscription_url, enabled);
  RefreshSubscriptionsList();
}

void AdblockDOMHandler::HandleDeleteSubscription(const base::ListValue* args) {
  DCHECK_EQ(args->GetSize(), 1U);
  AllowJavascript();
  std::string subscription_url_string;
  if (!args->GetString(0, &subscription_url_string)) {
    return;
  }
  const GURL subscription_url = GURL(subscription_url_string);
  if (!subscription_url.is_valid()) {
    return;
  }
  g_brave_browser_process->ad_block_service()
      ->subscription_service_manager()
      ->DeleteSubscription(subscription_url);
  RefreshSubscriptionsList();
}

void AdblockDOMHandler::HandleRefreshSubscription(const base::ListValue* args) {
  DCHECK_EQ(args->GetSize(), 1U);
  // This handler does not call Javascript directly, but refreshing the
  // subscription will trigger the observer later, which will require it.
  AllowJavascript();
  std::string subscription_url_string;
  if (!args->GetString(0, &subscription_url_string)) {
    return;
  }
  const GURL subscription_url = GURL(subscription_url_string);
  if (!subscription_url.is_valid()) {
    return;
  }
  g_brave_browser_process->ad_block_service()
      ->subscription_service_manager()
      ->RefreshSubscription(subscription_url, true);
}

void AdblockDOMHandler::HandleViewSubscriptionSource(
    const base::ListValue* args) {
  DCHECK_EQ(args->GetSize(), 1U);
  std::string subscription_url_string;
  if (!args->GetString(0, &subscription_url_string)) {
    return;
  }
  const GURL subscription_url = GURL(subscription_url_string);
  if (!subscription_url.is_valid()) {
    return;
  }

  const GURL file_url = g_brave_browser_process->ad_block_service()
                            ->subscription_service_manager()
                            ->GetListTextFileUrl(subscription_url);

#if defined(OS_ANDROID)
  web_ui()->GetWebContents()->GetDelegate()->OpenURLFromTab(
      web_ui()->GetWebContents(),
      content::OpenURLParams(file_url, content::Referrer(),
                             WindowOpenDisposition::NEW_FOREGROUND_TAB,
                             ui::PAGE_TRANSITION_AUTO_TOPLEVEL, false));
#else
  auto* browser =
      chrome::FindBrowserWithWebContents(web_ui()->GetWebContents());
  NavigateParams params(GetSingletonTabNavigateParams(browser, file_url));
  ShowSingletonTabOverwritingNTP(browser, &params);
#endif
}

// Convenience method to push updated subscription information to the UI.
void AdblockDOMHandler::RefreshSubscriptionsList() {
  DCHECK(IsJavascriptAllowed());
  auto list_subscriptions = g_brave_browser_process->ad_block_service()
                                ->subscription_service_manager()
                                ->GetSubscriptions();
  auto list_value = std::make_unique<base::ListValue>();
  for (const auto& subscription : list_subscriptions) {
    auto dict = std::make_unique<base::DictionaryValue>();
    dict->SetStringKey("subscription_url",
                       subscription.subscription_url.spec());
    dict->SetBoolKey("enabled", subscription.enabled);
    dict->SetDoubleKey("last_update_attempt",
                       subscription.last_update_attempt.ToJsTime());
    dict->SetDoubleKey("last_successful_update_attempt",
                       subscription.last_successful_update_attempt.ToJsTime());
    list_value->Append(std::move(dict));
  }
  CallJavascriptFunction("brave_adblock.onGetListSubscriptions", *list_value);
}

}  // namespace

BraveAdblockUI::BraveAdblockUI(content::WebUI* web_ui, const std::string& name)
    : WebUIController(web_ui) {
  CreateAndAddWebUIDataSource(web_ui, name, kBraveAdblockGenerated,
                              kBraveAdblockGeneratedSize,
                              IDR_BRAVE_ADBLOCK_HTML);
  web_ui->AddMessageHandler(std::make_unique<AdblockDOMHandler>());
}

BraveAdblockUI::~BraveAdblockUI() = default;
