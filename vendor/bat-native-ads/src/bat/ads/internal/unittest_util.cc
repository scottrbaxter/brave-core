/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/unittest_util.h"

#include <stdint.h>

#include <limits>
#include <vector>

#include "base/base_paths.h"
#include "base/containers/flat_map.h"
#include "base/files/file_util.h"
#include "base/path_service.h"
#include "base/strings/string16.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/time/time.h"
#include "bat/ads/internal/logging.h"
#include "net/base/registry_controlled_domains/registry_controlled_domain.h"
#include "net/http/http_status_code.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "third_party/re2/src/re2/re2.h"
#include "url/gurl.h"
#include "url/url_constants.h"
#include "bat/ads/internal/ads_client_mock.h"
#include "bat/ads/internal/url_util.h"
#include "bat/ads/pref_names.h"

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

namespace ads {

namespace {

static std::map<std::string, uint16_t> g_url_endpoint_indexes;

static std::map<std::string, std::string> g_prefs;

const char kNowTagValue[] = "now";
const char kDistantPastTagValue[] = "distant_past";
const char kDistantFutureTagValue[] = "distant_future";
const char kFromSecondsTagValue[] = "seconds";
const char kFromMinutesTagValue[] = "minutes";
const char kFromHoursTagValue[] = "hours";
const char kFromDaysTagValue[] = "days";

bool ParseTimeDelta(
    const std::string& value,
    base::TimeDelta* time_delta) {
  DCHECK(time_delta);

  const std::vector<std::string> components = base::SplitString(value,
      " ", base::TRIM_WHITESPACE, base::SPLIT_WANT_ALL);

  int offset;
  if (!base::StringToInt(components.at(0), &offset)) {
    return false;
  }

  const std::string period = components.at(1);
  if (period == kFromSecondsTagValue) {
    *time_delta = base::TimeDelta::FromSeconds(offset);
  } else if (period == kFromMinutesTagValue) {
    *time_delta = base::TimeDelta::FromMinutes(offset);
  } else if (period == kFromHoursTagValue) {
    *time_delta = base::TimeDelta::FromHours(offset);
  } else if (period == kFromDaysTagValue) {
    *time_delta = base::TimeDelta::FromDays(offset);
  } else {
    return false;
  }

  return true;
}

bool ParseTimeTag(
    std::string* value) {
  DCHECK(value);

  base::Time time;

  if (*value == kNowTagValue) {
    time = base::Time::Now();
  } else if (*value == kDistantPastTagValue) {
    time = base::Time::FromDoubleT(DistantPast());
  } else if (*value == kDistantFutureTagValue) {
    time = base::Time::FromDoubleT(DistantFuture());
  } else if (re2::RE2::FullMatch(*value,
      "[-+]?[0-9]*.*(seconds|minutes|hours|days)")) {
    base::TimeDelta time_delta;

    if (!ParseTimeDelta(*value, &time_delta)) {
      return false;
    }

    time = base::Time::Now() + time_delta;
  } else {
    return false;
  }

  base::Time::Exploded exploded;
  time.UTCExplode(&exploded);

  *value = base::StringPrintf("%04d-%02d-%02dT%02d:%02d:%02dZ",
      exploded.year, exploded.month, exploded.day_of_month, exploded.hour,
          exploded.minute, exploded.second);

  return true;
}

std::vector<std::string> ParseTagsForText(
    std::string* text) {
  DCHECK(text);

  re2::StringPiece text_string_piece(*text);
  RE2 r("<(.*)>");

  std::vector<std::string> tags;

  std::string tag;
  while (RE2::FindAndConsume(&text_string_piece, r, &tag)) {
    tag = base::ToLowerASCII(tag);
    tags.push_back(tag);
  }

  return tags;
}

void ReplaceTagsForText(
    std::string* text,
    const std::vector<std::string>& tags) {
  DCHECK(text);

  for (const auto& tag : tags) {
    const std::vector<std::string> components = base::SplitString(tag, ":",
        base::TRIM_WHITESPACE, base::SPLIT_WANT_ALL);

    if (components.size() != 2) {
      FAIL() << "Invalid tag: " << tag;
      return;
    }

    const std::string key = components.at(0);
    std::string value = components.at(1);

    if (key == "time") {
      if (!ParseTimeTag(&value)) {
        FAIL() << "Invalid tag: " << tag;
        return;
      }
    } else {
      FAIL() << "Unknown tag: " << tag;
      return;
    }

    const std::string enclosed_tag = base::StringPrintf("<%s>", tag.c_str());
    const std::string escaped_enclosed_tag = RE2::QuoteMeta(enclosed_tag);

    RE2::Replace(text, escaped_enclosed_tag, value);
  }
}

void ParseAndReplaceTagsForText(
    std::string* text) {
  DCHECK(text);

  const std::vector<std::string> tags = ParseTagsForText(text);
  ReplaceTagsForText(text, tags);
}

std::string GetUuid(
    const std::string& name) {
  const ::testing::TestInfo* const test_info =
      ::testing::UnitTest::GetInstance()->current_test_info();

  return base::StringPrintf("%s:%s.%s", name.c_str(),
      test_info->test_suite_name(), test_info->name());
}

URLEndpointResponses GetUrlEndpointResponsesForPath(
    const URLEndpoints& endpoints,
    const std::string& path) {
  const auto iter = endpoints.find(path);
  if (iter == endpoints.end()) {
    return {};
  }

  return iter->second;
}

bool GetNextUrlEndpointResponse(
    const std::string& url,
    const URLEndpoints& endpoints,
    URLEndpointResponse* url_endpoint_response) {
  DCHECK(!url.empty()) << "Empty URL";
  DCHECK(!endpoints.empty()) << "Missing endpoints";
  DCHECK(url_endpoint_response);

  const std::string path = GURL(url).PathForRequest();

  const URLEndpointResponses url_endpoint_responses =
      GetUrlEndpointResponsesForPath(endpoints, path);
  if (url_endpoint_responses.empty()) {
    // URL endpoint responses not found for given path
    return false;
  }

  uint16_t url_endpoint_response_index = 0;

  const std::string uuid = GetUuid(path);
  const auto url_endpoint_response_indexes_iter =
      g_url_endpoint_indexes.find(uuid);

  if (url_endpoint_response_indexes_iter == g_url_endpoint_indexes.end()) {
    // uuid does not exist so insert a new index set to 0 for the endpoint
    g_url_endpoint_indexes.insert({uuid, url_endpoint_response_index});
  } else {
    if (url_endpoint_response_indexes_iter->second ==
        url_endpoint_responses.size() - 1) {
      NOTREACHED() << "Missing MockUrlRequest endpoint response for " << url;
      return false;
    }

    url_endpoint_response_indexes_iter->second++;

    url_endpoint_response_index = url_endpoint_response_indexes_iter->second;
  }

  DCHECK_GE(url_endpoint_response_index, 0);
  DCHECK_LT(url_endpoint_response_index, url_endpoint_responses.size());

  *url_endpoint_response =
      url_endpoint_responses.at(url_endpoint_response_index);

  return true;
}

base::flat_map<std::string, std::string> UrlRequestHeadersToMap(
    const std::vector<std::string>& headers) {
  base::flat_map<std::string, std::string> normalized_headers;

  for (const auto& header : headers) {
    const std::vector<std::string> components = base::SplitString(header,
        ":", base::TRIM_WHITESPACE, base::SPLIT_WANT_ALL);

    if (components.size() != 2) {
      NOTREACHED();
      continue;
    }

    const std::string key = components.at(0);
    const std::string value = components.at(1);

    normalized_headers[key] = value;
  }

  return normalized_headers;
}

void MockGetBooleanPref(
    const std::unique_ptr<AdsClientMock>& mock) {
  ON_CALL(*mock, GetBooleanPref(_))
      .WillByDefault(Invoke([](
          const std::string& path) -> bool {
        const std::string pref_path = GetUuid(path);
        const std::string value = g_prefs[pref_path];

        DCHECK(!value.empty());

        int value_as_int;
        base::StringToInt(value, &value_as_int);
        return static_cast<bool>(value_as_int);
      }));
}

void MockSetBooleanPref(
    const std::unique_ptr<AdsClientMock>& mock) {
  ON_CALL(*mock, SetBooleanPref(_, _))
      .WillByDefault(Invoke([](
          const std::string& path,
          const bool value) {
        const std::string pref_path = GetUuid(path);
        g_prefs[pref_path] = base::NumberToString(static_cast<int>(value));
      }));
}

void MockGetIntegerPref(
    const std::unique_ptr<AdsClientMock>& mock) {
  ON_CALL(*mock, GetIntegerPref(_))
      .WillByDefault(Invoke([](
          const std::string& path) -> int {
        const std::string pref_path = GetUuid(path);
        const std::string value = g_prefs[pref_path];
        DCHECK(!value.empty());

        int value_as_int;
        base::StringToInt(value, &value_as_int);
        return value_as_int;
      }));
}

void MockSetIntegerPref(
    const std::unique_ptr<AdsClientMock>& mock) {
  ON_CALL(*mock, SetIntegerPref(_, _))
      .WillByDefault(Invoke([](
          const std::string& path,
          const int value) {
        const std::string pref_path = GetUuid(path);
        g_prefs[pref_path] = base::NumberToString(value);
      }));
}

void MockGetDoublePref(
    const std::unique_ptr<AdsClientMock>& mock) {
  ON_CALL(*mock, GetDoublePref(_))
      .WillByDefault(Invoke([](
          const std::string& path) -> double {
        const std::string pref_path = GetUuid(path);
        const std::string value = g_prefs[pref_path];
        DCHECK(!value.empty());

        double value_as_double;
        base::StringToDouble(value, &value_as_double);
        return value_as_double;
      }));
}

void MockSetDoublePref(
    const std::unique_ptr<AdsClientMock>& mock) {
  ON_CALL(*mock, SetDoublePref(_, _))
      .WillByDefault(Invoke([](
          const std::string& path,
          const double value) {
        const std::string pref_path = GetUuid(path);
        g_prefs[pref_path] = base::NumberToString(value);
      }));
}

void MockGetStringPref(
    const std::unique_ptr<AdsClientMock>& mock) {
  ON_CALL(*mock, GetStringPref(_))
      .WillByDefault(Invoke([](
          const std::string& path) -> std::string {
        const std::string pref_path = GetUuid(path);
        const std::string value = g_prefs[pref_path];
        return value;
      }));
}

void MockSetStringPref(
    const std::unique_ptr<AdsClientMock>& mock) {
  ON_CALL(*mock, SetStringPref(_, _))
      .WillByDefault(Invoke([](
          const std::string& path,
          const std::string& value) {
        const std::string pref_path = GetUuid(path);
        g_prefs[pref_path] = value;
      }));
}

void MockGetInt64Pref(
    const std::unique_ptr<AdsClientMock>& mock) {
  ON_CALL(*mock, GetInt64Pref(_))
      .WillByDefault(Invoke([](
          const std::string& path) -> int64_t {
        const std::string pref_path = GetUuid(path);
        const std::string value = g_prefs[pref_path];
        DCHECK(!value.empty());

        int64_t value_as_int64;
        base::StringToInt64(value, &value_as_int64);
        return value_as_int64;
      }));
}

void MockSetInt64Pref(
    const std::unique_ptr<AdsClientMock>& mock) {
  ON_CALL(*mock, SetInt64Pref(_, _))
      .WillByDefault(Invoke([](
          const std::string& path,
          const int64_t value) {
        const std::string pref_path = GetUuid(path);
        g_prefs[pref_path] = base::NumberToString(value);
      }));
}

void MockGetUint64Pref(
    const std::unique_ptr<AdsClientMock>& mock) {
  ON_CALL(*mock, GetUint64Pref(_))
      .WillByDefault(Invoke([](
          const std::string& path) -> uint64_t {
        const std::string pref_path = GetUuid(path);
        const std::string value = g_prefs[pref_path];
        DCHECK(!value.empty());

        uint64_t value_as_uint64;
        base::StringToUint64(value, &value_as_uint64);
        return value_as_uint64;
      }));
}

void MockSetUint64Pref(
    const std::unique_ptr<AdsClientMock>& mock) {
  ON_CALL(*mock, SetUint64Pref(_, _))
      .WillByDefault(Invoke([](
          const std::string& path,
          const uint64_t value) {
        const std::string pref_path = GetUuid(path);
        g_prefs[pref_path] = base::NumberToString(value);
      }));
}

void MockClearPref(
    const std::unique_ptr<AdsClientMock>& mock) {
  ON_CALL(*mock, ClearPref(_))
      .WillByDefault(Invoke([](
          const std::string& path) {
        g_prefs.erase(path);
      }));
}

void MockDefaultPrefs(
    const std::unique_ptr<AdsClientMock>& mock) {
  mock->SetBooleanPref(prefs::kEnabled, true);

  mock->SetUint64Pref(prefs::kAdsPerHour, 0);
  mock->SetUint64Pref(prefs::kAdsPerDay, 0);

  mock->SetIntegerPref(prefs::kIdleThreshold, 15);

  mock->SetBooleanPref(prefs::kShouldAllowConversionTracking, true);

  mock->SetBooleanPref(prefs::kShouldAllowAdsSubdivisionTargeting, false);
  mock->SetStringPref(prefs::kAdsSubdivisionTargetingCode, "AUTO");
  mock->SetStringPref(prefs::kAutoDetectedAdsSubdivisionTargetingCode, "");

  mock->SetStringPref(prefs::kCatalogId, "");
  mock->SetIntegerPref(prefs::kCatalogVersion, 1);
  mock->SetInt64Pref(prefs::kCatalogPing, 7200000);
  mock->SetInt64Pref(prefs::kCatalogLastUpdated, DistantPast());
}

}  // namespace

base::FilePath GetDataPath() {
  base::FilePath path;
  base::PathService::Get(base::DIR_SOURCE_ROOT, &path);
  path = path.AppendASCII("brave");
  path = path.AppendASCII("vendor");
  path = path.AppendASCII("bat-native-ads");
  path = path.AppendASCII("data");
  return path;
}

base::FilePath GetTestPath() {
  base::FilePath path = GetDataPath();
  path = path.AppendASCII("test");
  return path;
}

base::FilePath GetResourcesPath() {
  base::FilePath path = GetDataPath();
  path = path.AppendASCII("resources");
  return path;
}

void SetEnvironment(
    const Environment environment) {
  g_environment = environment;
}

void SetSysInfo(
    const SysInfo& sys_info) {
  g_sys_info.manufacturer = sys_info.manufacturer;
  g_sys_info.model = sys_info.model;
}

void SetBuildChannel(
    const bool is_release,
    const std::string& name) {
  g_build_channel.is_release = is_release;
  g_build_channel.name = name;
}

void MockLocaleHelper(
    const std::unique_ptr<brave_l10n::LocaleHelperMock>& mock,
    const std::string& locale) {
  ON_CALL(*mock, GetLocale())
      .WillByDefault(Return(locale));
}

void MockPlatformHelper(
    const std::unique_ptr<PlatformHelperMock>& mock,
    const PlatformType platform_type) {
  bool is_mobile;
  std::string platform_name;

  switch (platform_type) {
    case PlatformType::kUnknown: {
      is_mobile = false;
      platform_name = "unknown";
      break;
    }

    case PlatformType::kAndroid: {
      is_mobile = true;
      platform_name = "android";
      break;
    }

    case PlatformType::kIOS: {
      is_mobile = true;
      platform_name = "ios";
      break;
    }

    case PlatformType::kLinux: {
      is_mobile = false;
      platform_name = "linux";
      break;
    }

    case PlatformType::kMacOS: {
      is_mobile = false;
      platform_name = "macos";
      break;
    }

    case PlatformType::kWindows: {
      is_mobile = false;
      platform_name = "windows";
      break;
    }
  }

  ON_CALL(*mock, IsMobile())
      .WillByDefault(Return(is_mobile));

  ON_CALL(*mock, GetPlatformName())
      .WillByDefault(Return(platform_name));

  ON_CALL(*mock, GetPlatform())
      .WillByDefault(Return(platform_type));
}

void MockIsNetworkConnectionAvailable(
    const std::unique_ptr<AdsClientMock>& mock,
    const bool is_available) {
  ON_CALL(*mock, IsNetworkConnectionAvailable())
      .WillByDefault(Return(is_available));
}

void MockIsForeground(
    const std::unique_ptr<AdsClientMock>& mock,
    const bool is_foreground) {
  ON_CALL(*mock, IsForeground())
      .WillByDefault(Return(is_foreground));
}

void MockShouldShowNotifications(
    const std::unique_ptr<AdsClientMock>& mock,
    const bool should_show) {
  ON_CALL(*mock, ShouldShowNotifications())
      .WillByDefault(Return(should_show));
}

void MockShowNotification(
    const std::unique_ptr<AdsClientMock>& mock) {
  ON_CALL(*mock, ShowNotification(_))
      .WillByDefault(Invoke([](
          const AdNotificationInfo& ad_notification) {
      }));
}

void MockCloseNotification(
    const std::unique_ptr<AdsClientMock>& mock) {
  ON_CALL(*mock, CloseNotification(_))
      .WillByDefault(Invoke([](
          const std::string& uuid) {
      }));
}

void MockSave(
    const std::unique_ptr<AdsClientMock>& mock) {
  ON_CALL(*mock, Save(_, _, _))
      .WillByDefault(Invoke([](
          const std::string& name,
          const std::string& value,
          ResultCallback callback) {
        callback(SUCCESS);
      }));
}

void MockLoad(
    const std::unique_ptr<AdsClientMock>& mock) {
  ON_CALL(*mock, Load(_, _))
      .WillByDefault(Invoke([](
          const std::string& name,
          LoadCallback callback) {
        base::FilePath path = GetTestPath();
        path = path.AppendASCII(name);

        std::string value;
        if (!base::ReadFileToString(path, &value)) {
          callback(FAILED, value);
          return;
        }

        callback(SUCCESS, value);
      }));
}

void MockLoadUserModelForId(
    const std::unique_ptr<AdsClientMock>& mock) {
  ON_CALL(*mock, LoadUserModelForId(_, _))
      .WillByDefault(Invoke([](
          const std::string& id,
          LoadCallback callback) {
        base::FilePath path = GetTestPath();
        path = path.AppendASCII("user_models");
        path = path.AppendASCII(id);

        std::string value;
        if (!base::ReadFileToString(path, &value)) {
          callback(FAILED, value);
          return;
        }

        callback(SUCCESS, value);
      }));
}

void MockLoadResourceForId(
    const std::unique_ptr<AdsClientMock>& mock) {
  ON_CALL(*mock, LoadResourceForId(_))
      .WillByDefault(Invoke([](
          const std::string& id) -> std::string {
        base::FilePath path = GetResourcesPath();
        path = path.AppendASCII(id);

        std::string value;
        base::ReadFileToString(path, &value);

        return value;
      }));
}

void MockUrlRequest(
    const std::unique_ptr<AdsClientMock>& mock,
    const URLEndpoints& endpoints) {
  ON_CALL(*mock, UrlRequest(_, _))
      .WillByDefault(Invoke([&endpoints](
          const UrlRequestPtr& url_request,
          UrlRequestCallback callback) {
        int status_code = -1;

        std::string body;

        const base::flat_map<std::string, std::string> headers_as_map =
            UrlRequestHeadersToMap(url_request->headers);

        URLEndpointResponse url_endpoint_response;
        if (GetNextUrlEndpointResponse(url_request->url, endpoints,
            &url_endpoint_response)) {
          status_code = url_endpoint_response.first;
          if (status_code / 100 == 2) {
            body = url_endpoint_response.second;

            if (base::StartsWith(body, "/",
                base::CompareCase::INSENSITIVE_ASCII)) {
              const base::StringPiece filename = base::TrimString(body, "//",
                  base::TrimPositions::TRIM_LEADING);

              const base::FilePath path = GetTestPath().AppendASCII(filename);
              ASSERT_TRUE(base::ReadFileToString(path, &body));
            }

            ParseAndReplaceTagsForText(&body);
          }
        }

        UrlResponse url_response;
        url_response.url = url_request->url;
        url_response.status_code = status_code;
        url_response.body = body;
        url_response.headers = headers_as_map;
        callback(url_response);
      }));
}

void MockRunDBTransaction(
    const std::unique_ptr<AdsClientMock>& mock,
    const std::unique_ptr<Database>& database) {
  ON_CALL(*mock, RunDBTransaction(_, _))
      .WillByDefault(Invoke([&database](
          DBTransactionPtr transaction,
          RunDBTransactionCallback callback) {
        DBCommandResponsePtr response = DBCommandResponse::New();

        if (!database) {
          response->status = DBCommandResponse::Status::RESPONSE_ERROR;
        } else {
          database->RunTransaction(std::move(transaction), response.get());
        }

        callback(std::move(response));
      }));
}

void MockPrefs(
    const std::unique_ptr<AdsClientMock>& mock) {
  MockGetBooleanPref(mock);
  MockSetBooleanPref(mock);

  MockGetIntegerPref(mock);
  MockSetIntegerPref(mock);

  MockGetDoublePref(mock);
  MockSetDoublePref(mock);

  MockGetStringPref(mock);
  MockSetStringPref(mock);

  MockGetInt64Pref(mock);
  MockSetInt64Pref(mock);

  MockGetUint64Pref(mock);
  MockSetUint64Pref(mock);

  MockClearPref(mock);

  MockDefaultPrefs(mock);
}

base::Time TimeFromDateString(
    const std::string& date) {
  const std::string utc_date = date + " 23:59:59.999 +00:00";

  base::Time time;
  if (!base::Time::FromString(utc_date.c_str(), &time)) {
    return base::Time();
  }

  return time;
}

int64_t DistantPast() {
  return 0;  // Thursday, 1 January 1970 00:00:00 UTC
}

int64_t Now() {
  return static_cast<int64_t>(base::Time::Now().ToDoubleT());
}

int64_t DistantFuture() {
  return 4102444799;  // Thursday, December 31 2099 23:59:59 UTC
}

}  // namespace ads
