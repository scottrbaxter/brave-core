// Copyright (c) 2021 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

#include "brave/components/skus/browser/skus_sdk_impl.h"

#include <iostream>
#include <vector>

//#include "brave/components/skus/browser/br-rs/brave-rewards-cxx/src/wrapper.hpp"
#include "components/prefs/pref_service.h"

// using namespace std;
// using namespace rust::cxxbridge1;
// using namespace brave_rewards;

// namespace rust {
// inline namespace cxxbridge1 {

// Str::Str(const Str &) = default;

// } // namespace cxxbridge1
// } // namespace rust

namespace brave_rewards {

// RefreshOrderCallbackState::RefreshOrderCallbackState() = default;
// RefreshOrderCallbackState::~RefreshOrderCallbackState() = default;

// HttpRequest::HttpRequest() = default;
// HttpRequest::~HttpRequest() = default;

// void shim_executeRequest(
//     const brave_rewards::HttpRequest& req,
//     rust::cxxbridge1::Fn<void(rust::cxxbridge1::Box<brave_rewards::HttpRoundtripContext>, brave_rewards::HttpResponse)> callback,
//     rust::cxxbridge1::Box<brave_rewards::HttpRoundtripContext> ctx
//   ) {
//   cout<<"url: "<<req.url<<"\n";

//   std::string body = "{\"id\":\"b788a168-1136-411f-9546-43a372a2e3ed\",\"createdAt\":\"2021-08-17T21:35:48.658623Z\",\"currency\":\"USD\",\"updatedAt\":\"2021-08-17T21:36:06.174288Z\",\"totalPrice\":\"7\",\"merchantId\":\"brave.com\",\"location\":\"talk.brave.software\",\"status\":\"paid\",\"items\":[{\"id\":\"eac1b09f-2228-4f91-a970-a22b229bb994\",\"orderId\":\"b788a168-1136-411f-9546-43a372a2e3ed\",\"sku\":\"brave-talk-premium\",\"createdAt\":\"2021-08-17T21:35:48.658623Z\",\"updatedAt\":\"2021-08-17T21:35:48.658623Z\",\"currency\":\"USD\",\"quantity\":1,\"price\":\"7\",\"subtotal\":\"7\",\"location\":\"talk.brave.software\",\"description\":\"Premium access to Brave Talk\",\"credentialType\":\"time-limited\",\"validFor\":null,\"metadata\":{\"stripe_cancel_uri\":\"https://account.brave.software/plans/?intent=checkout\",\"stripe_item_id\":\"price_1J84oMHof20bphG6NBAT2vor\",\"stripe_product_id\":\"prod_Jlc224hFvAMvEp\",\"stripe_success_uri\":\"https://account.brave.software/account/?intent=provision\"}}],\"allowedPaymentMethods\":[\"stripe\"],\"metadata\":{\"stripeSubscriptionId\":\"sub_K3hLyRFkjj3mYs\"},\"lastPaidAt\":\"2021-08-17T21:36:06.174938Z\",\"expiresAt\":\"2021-09-17T08:05:09.176138Z\",\"validFor\":2629743001200000}";
//   std::vector<uint8_t> body_bytes (body.begin(), body.end());

//   brave_rewards::HttpResponse resp = {
//     RewardsResult::Ok,
//     200,
//     {"foo:bar"},
//     body_bytes,
//   };

//   callback(std::move(ctx), resp);
// }

// void RefreshOrderCallbackState::cb(RewardsResult result,
//                                    rust::cxxbridge1::Str order) {
//   if (CbFunc) {
//     CbFunc(result, order);
//   }
// }

SkusSdkImpl::SkusSdkImpl(PrefService* prefs) {
  // TODO: fill me in...
}

void SkusSdkImpl::StartRefreshOrder(uint32_t order_id) {
  LOG(ERROR) << "BSC]] LOL INSIDE THE BROWSER PROCESS! ORDERID=" << order_id;
}

void SkusSdkImpl::StartFetchOrderCredentials(uint32_t order_id) {

}

} // namespace brave_rewards
