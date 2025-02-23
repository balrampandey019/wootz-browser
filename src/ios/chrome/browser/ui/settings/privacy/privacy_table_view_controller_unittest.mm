// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/chrome/browser/ui/settings/privacy/privacy_table_view_controller.h"

#import <LocalAuthentication/LAContext.h>
#import <memory>

#import "base/apple/foundation_util.h"
#import "base/memory/ptr_util.h"
#import "base/memory/raw_ptr.h"
#import "base/strings/sys_string_conversions.h"
#import "base/test/scoped_feature_list.h"
#import "components/content_settings/core/common/features.h"
#import "components/handoff/pref_names_ios.h"
#import "components/policy/core/common/policy_pref_names.h"
#import "components/prefs/pref_service.h"
#import "components/safe_browsing/core/common/features.h"
#import "components/safe_browsing/core/common/safe_browsing_prefs.h"
#import "components/signin/public/identity_manager/account_info.h"
#import "components/strings/grit/components_strings.h"
#import "components/sync/test/mock_sync_service.h"
#import "components/sync_preferences/pref_service_mock_factory.h"
#import "components/sync_preferences/pref_service_syncable.h"
#import "components/sync_preferences/testing_pref_service_syncable.h"
#import "ios/chrome/browser/policy/model/policy_util.h"
#import "ios/chrome/browser/shared/model/browser/test/test_browser.h"
#import "ios/chrome/browser/shared/model/browser_state/test_chrome_browser_state.h"
#import "ios/chrome/browser/shared/model/prefs/browser_prefs.h"
#import "ios/chrome/browser/shared/model/prefs/pref_names.h"
#import "ios/chrome/browser/shared/public/features/features.h"
#import "ios/chrome/browser/shared/public/features/system_flags.h"
#import "ios/chrome/browser/shared/ui/table_view/legacy_chrome_table_view_controller_test.h"
#import "ios/chrome/browser/sync/model/mock_sync_service_utils.h"
#import "ios/chrome/browser/sync/model/sync_service_factory.h"
#import "ios/chrome/browser/ui/settings/privacy/privacy_guide/features.h"
#import "ios/chrome/browser/web/model/features.h"
#import "ios/chrome/grit/ios_branded_strings.h"
#import "ios/chrome/grit/ios_strings.h"
#import "ios/chrome/test/ios_chrome_scoped_testing_local_state.h"
#import "ios/components/security_interstitials/https_only_mode/feature.h"
#import "ios/web/public/test/web_task_environment.h"
#import "testing/gtest/include/gtest/gtest.h"
#import "ui/base/l10n/l10n_util.h"

using ::testing::Return;

namespace {

NSString* const kSpdyProxyEnabled = @"SpdyProxyEnabled";

// Checks if the device has Passcode, Face ID, or Touch ID set up.
BOOL DeviceSupportsAuthentication() {
  LAContext* context = [[LAContext alloc] init];
  return [context canEvaluatePolicy:LAPolicyDeviceOwnerAuthentication
                              error:nil];
}

struct PrivacyTableViewControllerTestConfig {
  // Available of Incognito mode tests should run with.
  IncognitoModePrefs incognitoModeAvailability;
};

class PrivacyTableViewControllerTest
    : public LegacyChromeTableViewControllerTest,
      public testing::WithParamInterface<PrivacyTableViewControllerTestConfig> {
 protected:
  PrivacyTableViewControllerTest() {}

  void SetUp() override {
    LegacyChromeTableViewControllerTest::SetUp();

    TestChromeBrowserState::Builder test_cbs_builder;
    test_cbs_builder.AddTestingFactory(
        SyncServiceFactory::GetInstance(),
        base::BindRepeating(&CreateMockSyncService));
    chrome_browser_state_ = test_cbs_builder.Build();

    browser_ = std::make_unique<TestBrowser>(chrome_browser_state_.get());

    NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];
    initialValueForSpdyProxyEnabled_ =
        [[defaults valueForKey:kSpdyProxyEnabled] copy];
    [defaults setValue:@"Disabled" forKey:kSpdyProxyEnabled];

    // Set Incognito Mode availability depending on test config.
    chrome_browser_state_->GetTestingPrefService()->SetManagedPref(
        policy::policy_prefs::kIncognitoModeAvailability,
        std::make_unique<base::Value>(
            static_cast<int>(GetParam().incognitoModeAvailability)));

    feature_list_.InitAndEnableFeature(kPrivacyGuideIos);
  }

  void TearDown() override {
    if (initialValueForSpdyProxyEnabled_) {
      [[NSUserDefaults standardUserDefaults]
          setObject:initialValueForSpdyProxyEnabled_
             forKey:kSpdyProxyEnabled];
    } else {
      [[NSUserDefaults standardUserDefaults]
          removeObjectForKey:kSpdyProxyEnabled];
    }
    [base::apple::ObjCCastStrict<PrivacyTableViewController>(controller())
        settingsWillBeDismissed];
    LegacyChromeTableViewControllerTest::TearDown();
  }

  LegacyChromeTableViewController* InstantiateController() override {
    return [[PrivacyTableViewController alloc] initWithBrowser:browser_.get()
                                        reauthenticationModule:nil];
  }

  syncer::MockSyncService* mock_sync_service() {
    return static_cast<syncer::MockSyncService*>(
        SyncServiceFactory::GetForBrowserState(chrome_browser_state_.get()));
  }

  // Returns the proper detail text for the safe browsing item depending on the
  // safe browsing and enhanced protection preference values.
  NSString* SafeBrowsingDetailText() {
    PrefService* prefService = chrome_browser_state_->GetPrefs();
    if (safe_browsing::IsEnhancedProtectionEnabled(*prefService)) {
      return l10n_util::GetNSString(
          IDS_IOS_SAFE_BROWSING_ENHANCED_PROTECTION_TITLE);
    } else if (safe_browsing::IsSafeBrowsingEnabled(*prefService)) {
      return l10n_util::GetNSString(
          IDS_IOS_PRIVACY_SAFE_BROWSING_STANDARD_PROTECTION_TITLE);
    }
    return l10n_util::GetNSString(
        IDS_IOS_PRIVACY_SAFE_BROWSING_NO_PROTECTION_DETAIL_TITLE);
  }

  web::WebTaskEnvironment task_environment_;
  IOSChromeScopedTestingLocalState local_state_;
  std::unique_ptr<TestChromeBrowserState> chrome_browser_state_;
  std::unique_ptr<Browser> browser_;
  NSString* initialValueForSpdyProxyEnabled_;
  base::test::ScopedFeatureList feature_list_;
};

// Tests PrivacyTableViewController is set up with all appropriate items
// and sections.
TEST_P(PrivacyTableViewControllerTest, TestModel) {
  PrefService* prefService = chrome_browser_state_->GetPrefs();
  CreateController();
  CheckController();

  int expectedNumberOfSections = 6;
  if (base::FeatureList::IsEnabled(
          security_interstitials::features::kHttpsOnlyMode)) {
    expectedNumberOfSections++;
  }

  // IncognitoInterstitial section.
  expectedNumberOfSections++;
  EXPECT_EQ(expectedNumberOfSections, NumberOfSections());

  int currentSection = 0;
  // PrivacyContent section.
  EXPECT_EQ(1, NumberOfItemsInSection(currentSection));
  CheckTextCellTextAndDetailText(
      l10n_util::GetNSString(IDS_IOS_CLEAR_BROWSING_DATA_TITLE), nil,
      currentSection, 0);

  // PrivacyGuide section.
  currentSection++;
  EXPECT_EQ(1, NumberOfItemsInSection(currentSection));
  CheckTextCellTextAndDetailText(
      l10n_util::GetNSString(IDS_IOS_PRIVACY_GUIDE_TITLE), nil, currentSection,
      0);

  // SafeBrowsing section.
  currentSection++;
  EXPECT_EQ(1, NumberOfItemsInSection(currentSection));
  CheckTextCellTextAndDetailText(
      l10n_util::GetNSString(IDS_IOS_PRIVACY_SAFE_BROWSING_TITLE),
      SafeBrowsingDetailText(), currentSection, 0);

  // HTTPS-Only Mode section.
  if (base::FeatureList::IsEnabled(
          security_interstitials::features::kHttpsOnlyMode)) {
    currentSection++;
    EXPECT_EQ(1, NumberOfItemsInSection(currentSection));
    CheckSwitchCellStateAndTextWithId(
        NO, IDS_IOS_SETTINGS_HTTPS_ONLY_MODE_TITLE, currentSection, 0);
  }

  // WebServices section.
  currentSection++;
  EXPECT_EQ(1, NumberOfItemsInSection(currentSection));
  NSString* handoffSubtitle = chrome_browser_state_->GetPrefs()->GetBoolean(
                                  prefs::kIosHandoffToOtherDevices)
                                  ? l10n_util::GetNSString(IDS_IOS_SETTING_ON)
                                  : l10n_util::GetNSString(IDS_IOS_SETTING_OFF);
  CheckTextCellTextAndDetailText(
      l10n_util::GetNSString(IDS_IOS_OPTIONS_ENABLE_HANDOFF_TO_OTHER_DEVICES),
      handoffSubtitle, currentSection, 0);

  // IncognitoAuth section.
  currentSection++;
  EXPECT_EQ(1, NumberOfItemsInSection(currentSection));
  if ((IsIncognitoModeDisabled(prefService) ||
       !DeviceSupportsAuthentication())) {
    // Disabled version of Incognito auth item is expected in this case.
    CheckInfoButtonCellStatusWithIdAndTextWithId(
        IDS_IOS_SETTING_OFF, IDS_IOS_INCOGNITO_REAUTH_SETTING_NAME,
        currentSection, 0);
  } else {
    CheckSwitchCellStateAndTextWithId(NO, IDS_IOS_INCOGNITO_REAUTH_SETTING_NAME,
                                      currentSection, 0);
  }

  // IncognitoInterstitial section.
  currentSection++;
  EXPECT_EQ(1, NumberOfItemsInSection(currentSection));
  if ((IsIncognitoModeDisabled(prefService) ||
       IsIncognitoModeForced(prefService))) {
    // Disabled version of Incognito interstitial item is expected in this
    // case.
    CheckInfoButtonCellStatusWithIdAndTextWithId(
        IDS_IOS_SETTING_OFF, IDS_IOS_OPTIONS_ENABLE_INCOGNITO_INTERSTITIAL,
        currentSection, 0);
  } else {
    CheckSwitchCellStateAndTextWithId(
        NO, IDS_IOS_OPTIONS_ENABLE_INCOGNITO_INTERSTITIAL, currentSection, 0);
  }

  // Lockdown Mode section.
  currentSection++;
  EXPECT_EQ(1, NumberOfItemsInSection(currentSection));
  CheckTextCellTextAndDetailText(
      l10n_util::GetNSString(IDS_IOS_LOCKDOWN_MODE_TITLE),
      l10n_util::GetNSString(IDS_IOS_SETTING_OFF), currentSection, 0);

  // Testing section index and text of the privacy footer.
  if (base::FeatureList::IsEnabled(kLinkAccountSettingsToPrivacyFooter)) {
    CheckSectionFooter(
        l10n_util::GetNSString(IDS_IOS_PRIVACY_SIGNED_OUT_FOOTER),
        /* section= */ expectedNumberOfSections - 1);
  } else {
    CheckSectionFooter(
        l10n_util::GetNSString(IDS_IOS_PRIVACY_GOOGLE_SERVICES_FOOTER),
        /* section= */ expectedNumberOfSections - 1);
  }
}

// Tests PrivacyTableViewController sets the correct privacy footer for a
// non-syncing user.
TEST_P(PrivacyTableViewControllerTest, TestModelFooterWithSyncDisabled) {
  base::test::ScopedFeatureList feature_list;
  feature_list.InitAndDisableFeature(kLinkAccountSettingsToPrivacyFooter);

  ON_CALL(*mock_sync_service()->GetMockUserSettings(),
          IsInitialSyncFeatureSetupComplete())
      .WillByDefault(Return(false));

  CreateController();
  CheckController();

  int expectedNumberOfSections = 6;
  if (base::FeatureList::IsEnabled(
          security_interstitials::features::kHttpsOnlyMode)) {
    expectedNumberOfSections++;
  }

  // IncognitoInterstitial section.
  expectedNumberOfSections++;
  EXPECT_EQ(expectedNumberOfSections, NumberOfSections());

  // Testing section index and text of the privacy footer.
  CheckSectionFooter(
      l10n_util::GetNSString(IDS_IOS_PRIVACY_GOOGLE_SERVICES_FOOTER),
      /* section= */ expectedNumberOfSections - 1);
}

// Tests PrivacyTableViewController sets the correct privacy footer for a
// syncing user.
TEST_P(PrivacyTableViewControllerTest, TestModelFooterWithSyncEnabled) {
  ON_CALL(*mock_sync_service()->GetMockUserSettings(),
          IsInitialSyncFeatureSetupComplete())
      .WillByDefault(Return(true));
  ON_CALL(*mock_sync_service(), HasSyncConsent()).WillByDefault(Return(true));

  CreateController();
  CheckController();

  int expectedNumberOfSections = 6;
  if (base::FeatureList::IsEnabled(
          security_interstitials::features::kHttpsOnlyMode)) {
    expectedNumberOfSections++;
  }

  // IncognitoInterstitial section.
  expectedNumberOfSections++;
  EXPECT_EQ(expectedNumberOfSections, NumberOfSections());

  // Testing section index and text of the privacy footer.
  CheckSectionFooter(
      l10n_util::GetNSString(IDS_IOS_PRIVACY_SYNC_AND_GOOGLE_SERVICES_FOOTER),
      /* section= */ expectedNumberOfSections - 1);
}

INSTANTIATE_TEST_SUITE_P(
    PrivacyTableViewControllerTestAllConfigs,
    PrivacyTableViewControllerTest,
    testing::Values(
        PrivacyTableViewControllerTestConfig{
            /* incognitoModeAvailability= */ IncognitoModePrefs::kEnabled},
        PrivacyTableViewControllerTestConfig{
            /* incognitoModeAvailability= */ IncognitoModePrefs::kDisabled},
        PrivacyTableViewControllerTestConfig{
            /* incognitoModeAvailability= */ IncognitoModePrefs::kForced}));

}  // namespace
