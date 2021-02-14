/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/server/via_header_util.h"

#include <vector>

#include "base/feature_list.h"
#include "base/strings/stringprintf.h"
#include "base/test/scoped_feature_list.h"
#include "bat/ads/internal/features/user_activity/user_activity_features.h"
#include "bat/ads/internal/unittest_base.h"
#include "bat/ads/internal/unittest_util.h"

// npm run test -- brave_unit_tests --filter=BatAds*

namespace ads {

namespace {

struct PermutationInfo {
  bool is_uncertain_future;
  bool is_inactive_user;
  int version;
};

const PermutationInfo kTests[] = {{false, false, 0},
                                  {false, true, 1},
                                  {true, false, 2},
                                  {true, true, 3}};

}  // namespace

class BatAdsViaHeaderUtilTest
    : public UnitTestBase,
      public ::testing::WithParamInterface<PermutationInfo> {
 protected:
  BatAdsViaHeaderUtilTest() = default;

  ~BatAdsViaHeaderUtilTest() override = default;
};

TEST_P(BatAdsViaHeaderUtilTest, BuildViaHeader) {
  // Arrange
  base::FieldTrialParams parameters;
  const char kTriggersParameter[] = "triggers";
  parameters[kTriggersParameter] = "0D=1.0";
  const char kTimeWindowParameter[] = "time_window";
  parameters[kTimeWindowParameter] = "1h";
  const char kThresholdParameter[] = "threshold";
  parameters[kThresholdParameter] = "1.0";
  std::vector<base::test::ScopedFeatureList::FeatureAndParams> enabled_features;
  enabled_features.push_back({features::user_activity::kFeature, parameters});

  const std::vector<base::Feature> disabled_features;

  base::test::ScopedFeatureList scoped_feature_list;
  scoped_feature_list.InitWithFeaturesAndParameters(enabled_features,
                                                    disabled_features);

  const PermutationInfo info = GetParam();

  SysInfo sys_info;
  sys_info.is_uncertain_future = info.is_uncertain_future;
  SetSysInfo(sys_info);

  if (!info.is_inactive_user) {
    UserActivity::Get()->RecordEvent(UserActivityEventType::kOpenedNewTab);
  }

  // Act
  const std::string via_header = server::BuildViaHeader();

  // Assert
  const std::string expect_via_header = base::StringPrintf(
      "Via: 1.%d brave, 1.1 ads-serve.brave.com (Apache/1.2)", info.version);

  EXPECT_EQ(expect_via_header, via_header);
}

static std::string GetTestCaseName(
    ::testing::TestParamInfo<PermutationInfo> info) {
  const std::string is_uncertain_future =
      info.param.is_uncertain_future ? "GuestOS" : "HostOS";

  const std::string is_inactive_user =
      info.param.is_inactive_user ? "InactiveUser" : "ActiveUser";

  return base::StringPrintf("For%sAnd%s", is_uncertain_future.c_str(),
                            is_inactive_user.c_str());
}

INSTANTIATE_TEST_SUITE_P(BuildViaHeaders,
                         BatAdsViaHeaderUtilTest,
                         ::testing::ValuesIn(kTests),
                         GetTestCaseName);

}  // namespace ads
