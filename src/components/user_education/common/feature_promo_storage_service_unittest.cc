// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/user_education/common/feature_promo_storage_service.h"

#include <memory>

#include "base/feature_list.h"
#include "components/user_education/test/test_feature_promo_storage_service.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace user_education {

namespace {
BASE_FEATURE(kTestIPHFeature,
             "TestIPHFeature",
             base::FEATURE_ENABLED_BY_DEFAULT);
BASE_FEATURE(kTestIPHFeature2,
             "TestIPHFeature2",
             base::FEATURE_ENABLED_BY_DEFAULT);
}  // namespace

TEST(FeaturePromoStorageServiceTest, GetSnoozeCount) {
  test::TestFeaturePromoStorageService service;
  EXPECT_EQ(0, service.GetSnoozeCount(kTestIPHFeature));
  EXPECT_EQ(0, service.GetSnoozeCount(kTestIPHFeature2));
  FeaturePromoData data;
  data.snooze_count = 3;
  service.SavePromoData(kTestIPHFeature, data);
  EXPECT_EQ(3, service.GetSnoozeCount(kTestIPHFeature));
  EXPECT_EQ(0, service.GetSnoozeCount(kTestIPHFeature2));
  service.Reset(kTestIPHFeature);
  EXPECT_EQ(0, service.GetSnoozeCount(kTestIPHFeature));
  EXPECT_EQ(0, service.GetSnoozeCount(kTestIPHFeature2));
}

TEST(FeaturePromoStorageServiceTest, GetShownForKeys) {
  static constexpr char kAppName1[] = "App1";
  static constexpr char kAppName2[] = "App2";

  test::TestFeaturePromoStorageService service;
  EXPECT_THAT(service.GetShownForKeys(kTestIPHFeature), testing::IsEmpty());
  EXPECT_THAT(service.GetShownForKeys(kTestIPHFeature2), testing::IsEmpty());
  FeaturePromoData data;
  data.shown_for_keys.insert(kAppName1);
  data.shown_for_keys.insert(kAppName2);
  service.SavePromoData(kTestIPHFeature, data);
  EXPECT_THAT(service.GetShownForKeys(kTestIPHFeature),
              testing::UnorderedElementsAre(kAppName1, kAppName2));
  EXPECT_THAT(service.GetShownForKeys(kTestIPHFeature2), testing::IsEmpty());
  service.Reset(kTestIPHFeature);
  EXPECT_THAT(service.GetShownForKeys(kTestIPHFeature), testing::IsEmpty());
  EXPECT_THAT(service.GetShownForKeys(kTestIPHFeature2), testing::IsEmpty());
}

}  // namespace user_education
