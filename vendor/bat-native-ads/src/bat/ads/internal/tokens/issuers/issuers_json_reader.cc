/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/tokens/issuers/issuers_json_reader.h"

#include "base/json/json_reader.h"
#include "base/values.h"
#include "bat/ads/internal/tokens/issuers/issuer_info_aliases.h"
#include "bat/ads/internal/tokens/issuers/issuers_info.h"
#include "bat/ads/internal/tokens/issuers/issuers_json_reader_util.h"
#include "third_party/abseil-cpp/absl/types/optional.h"

namespace ads {
namespace JSONReader {

absl::optional<IssuersInfo> ReadIssuers(const std::string& json) {
  const absl::optional<base::Value> value = base::JSONReader::Read(json);
  if (!value || !value->is_dict()) {
    return absl::nullopt;
  }

  const absl::optional<int> ping = ParsePing(*value);
  if (!ping) {
    return absl::nullopt;
  }

  const absl::optional<IssuerList> issuers = ParseIssuers(*value);
  if (!issuers) {
    return absl::nullopt;
  }

  IssuersInfo new_issuers;
  new_issuers.ping = ping.value();
  new_issuers.issuers = issuers.value();

  return new_issuers;
}

}  // namespace JSONReader
}  // namespace ads
