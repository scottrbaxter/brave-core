/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_FREQUENCY_CAPPING_PERMISSION_RULES_INLINE_CONTENT_ADS_PER_DAY_FREQUENCY_CAP_H_
#define BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_FREQUENCY_CAPPING_PERMISSION_RULES_INLINE_CONTENT_ADS_PER_DAY_FREQUENCY_CAP_H_

#include <cstdint>
#include <deque>
#include <string>

#include "bat/ads/internal/frequency_capping/permission_rules/permission_rule.h"

namespace ads {

class InlineContentAdsPerDayFrequencyCap final : public PermissionRule {
 public:
  InlineContentAdsPerDayFrequencyCap();
  ~InlineContentAdsPerDayFrequencyCap() override;

  bool ShouldAllow() override;

  std::string GetLastMessage() const override;

 private:
  std::string last_message_;

  bool DoesRespectCap(const std::deque<uint64_t>& history);

  InlineContentAdsPerDayFrequencyCap(
      const InlineContentAdsPerDayFrequencyCap&) = delete;
  InlineContentAdsPerDayFrequencyCap& operator=(
      const InlineContentAdsPerDayFrequencyCap&) = delete;
};

}  // namespace ads

#endif  // BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_FREQUENCY_CAPPING_PERMISSION_RULES_INLINE_CONTENT_ADS_PER_DAY_FREQUENCY_CAP_H_
