// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "survey_config.h"

#include <optional>

#include "base/feature_list.h"
#include "base/features.h"
#include "components/autofill/core/common/autofill_features.h"
#include "components/permissions/features.h"
#include "components/permissions/permission_hats_trigger_helper.h"

#if !BUILDFLAG(IS_ANDROID)
#include "chrome/browser/download/download_warning_desktop_hats_utils.h"
#include "chrome/common/chrome_features.h"
#include "components/password_manager/core/browser/features/password_features.h"  // nogncheck
#include "components/performance_manager/public/features.h"         // nogncheck
#include "components/permissions/constants.h"                       // nogncheck
#include "components/safe_browsing/core/common/features.h"          // nogncheck
#include "components/safe_browsing/core/common/safebrowsing_constants.h"  // nogncheck
#else
#include "chrome/browser/flags/android/chrome_feature_list.h"
#endif  // #if !BUILDFLAG(IS_ANDROID)

#if !BUILDFLAG(IS_ANDROID)
constexpr char kHatsSurveyTriggerAutofillAddress[] = "autofill-address";
constexpr char kHatsSurveyTriggerAutofillAddressUserPerception[] =
    "autofill-address-users-perception";
constexpr char kHatsSurveyTriggerAutofillCreditCardUserPerception[] =
    "autofill-credit-card-users-perception";
constexpr char kHatsSurveyTriggerAutofillCard[] = "autofill-card";
constexpr char kHatsSurveyTriggerAutofillPassword[] = "autofill-password";
constexpr char kHatsSurveyTriggerDownloadWarningBubbleBypass[] =
    "download-warning-bubble-bypass";
constexpr char kHatsSurveyTriggerDownloadWarningBubbleHeed[] =
    "download-warning-bubble-heed";
constexpr char kHatsSurveyTriggerDownloadWarningBubbleIgnore[] =
    "download-warning-bubble-ignore";
constexpr char kHatsSurveyTriggerDownloadWarningPageBypass[] =
    "download-warning-page-bypass";
constexpr char kHatsSurveyTriggerDownloadWarningPageHeed[] =
    "download-warning-page-heed";
constexpr char kHatsSurveyTriggerDownloadWarningPageIgnore[] =
    "download-warning-page-ignore";
#if BUILDFLAG(GOOGLE_CHROME_BRANDING)
constexpr char kHatsSurveyTriggerGetMostChrome[] = "get-most-chrome";
#endif
constexpr char kHatsSurveyTriggerM1AdPrivacyPage[] = "m1-ad-privacy-page";
constexpr char kHatsSurveyTriggerM1TopicsSubpage[] = "m1-topics-subpage";
constexpr char kHatsSurveyTriggerM1FledgeSubpage[] = "m1-fledge-subpage";
constexpr char kHatsSurveyTriggerM1AdMeasurementSubpage[] =
    "m1-ad-measurement-subpage";
constexpr char kHatsSurveyTriggerNtpModules[] = "ntp-modules";
constexpr char kHatsSurveyTriggerNtpPhotosModuleOptOut[] =
    "ntp-photos-module-opt-out";
constexpr char kHatsSurveyTriggerPerformanceControlsPerformance[] =
    "performance-general";
constexpr char kHatsSurveyTriggerPerformanceControlsBatteryPerformance[] =
    "performance-battery";
constexpr char kHatsSurveyTriggerPerformanceControlsMemorySaverOptOut[] =
    "performance-high-efficiency-opt-out";
constexpr char kHatsSurveyTriggerPerformanceControlsBatterySaverOptOut[] =
    "performance-battery-saver-opt-out";
// The permission prompt trigger permits configuring multiple triggers
// simultaneously. Each trigger increments a counter at the end -->
// "permission-prompt0", "permission-prompt1", ...
constexpr char kHatsSurveyTriggerPrivacyGuide[] = "privacy-guide";
constexpr char kHatsSurveyTriggerRedWarning[] = "red-warning";
constexpr char kHatsSurveyTriggerSettings[] = "settings";
constexpr char kHatsSurveyTriggerSettingsPrivacy[] = "settings-privacy";
constexpr char kHatsSurveyTriggerSettingsSecurity[] = "settings-security";
constexpr char kHatsSurveyTriggerExtensions[] = "extensions";
constexpr char kHatsSurveyTriggerSuggestedPasswordsExperiment[] =
    "suggested-passwords-experiment";
constexpr char kHatsSurveyTriggerTrustSafetyPrivacySandbox4ConsentAccept[] =
    "ts-ps4-consent-accept";
constexpr char kHatsSurveyTriggerTrustSafetyPrivacySandbox4ConsentDecline[] =
    "ts-ps4-consent-decline";
constexpr char kHatsSurveyTriggerTrustSafetyPrivacySandbox4NoticeOk[] =
    "ts-ps4-notice-ok";
constexpr char kHatsSurveyTriggerTrustSafetyPrivacySandbox4NoticeSettings[] =
    "ts-ps4-notice-settings";
constexpr char kHatsSurveyTriggerTrustSafetyPrivacySettings[] =
    "ts-privacy-settings";
constexpr char kHatsSurveyTriggerTrustSafetyTrustedSurface[] =
    "ts-trusted-surface";
constexpr char kHatsSurveyTriggerTrustSafetyTransactions[] = "ts-transactions";
constexpr char kHatsSurveyTriggerWhatsNew[] = "whats-new";
constexpr char kHatsSurveyTriggerTrustSafetyV2BrowsingData[] =
    "ts-v2-browsing-data";
constexpr char kHatsSurveyTriggerTrustSafetyV2ControlGroup[] =
    "ts-v2-control-group";
constexpr char kHatsSurveyTriggerTrustSafetyV2DownloadWarningUI[] =
    "ts-v2-download-warning-ui";
constexpr char kHatsSurveyTriggerTrustSafetyV2PasswordCheck[] =
    "ts-v2-password-check";
constexpr char kHatsSurveyTriggerTrustSafetyV2PasswordProtectionUI[] =
    "ts-v2-password-protection-ui";
constexpr char kHatsSurveyTriggerTrustSafetyV2SafetyCheck[] =
    "ts-v2-safety-check";
constexpr char kHatsSurveyTriggerTrustSafetyV2TrustedSurface[] =
    "ts-v2-trusted-surface";
constexpr char kHatsSurveyTriggerTrustSafetyV2PrivacyGuide[] =
    "ts-v2-privacy-guide";
constexpr char kHatsSurveyTriggerTrustSafetyV2PrivacySandbox4ConsentAccept[] =
    "ts-v2-ps4-consent-accept";
constexpr char kHatsSurveyTriggerTrustSafetyV2PrivacySandbox4ConsentDecline[] =
    "ts-v2-ps4-consent-decline";
constexpr char kHatsSurveyTriggerTrustSafetyV2PrivacySandbox4NoticeOk[] =
    "ts-v2-ps4-notice-ok";
constexpr char kHatsSurveyTriggerTrustSafetyV2PrivacySandbox4NoticeSettings[] =
    "ts-v2-ps4-notice-settings";
constexpr char kHatsSurveyTriggerTrustSafetyV2SafeBrowsingInterstitial[] =
    "ts-v2-safe-browsing-interstitial";
constexpr char kHatsSurveyTriggerWallpaperSearch[] = "wallpaper-search";
#else   // BUILDFLAG(IS_ANDROID)
constexpr char kHatsSurveyTriggerAndroidStartupSurvey[] = "startup_survey";
#endif  // #if !BUILDFLAG(IS_ANDROID)

constexpr char kHatsSurveyTriggerTesting[] = "testing";
constexpr char kHatsNextSurveyTriggerIDTesting[] =
    "HLpeYy5Av0ugnJ3q1cK0XzzA8UHv";

constexpr char kHatsSurveyTriggerPermissionsPrompt[] = "permissions-prompt";

namespace {

constexpr char kHatsSurveyProbability[] = "probability";
constexpr char kHatsSurveyEnSiteID[] = "en_site_id";
constexpr double kHatsSurveyProbabilityDefault = 0;

// Survey configs must always be hardcoded here, so that they require review
// from HaTS owners. Do not move this method out of the anonymous namespace or
// change its signature to work around this.
std::vector<hats::SurveyConfig> GetAllSurveyConfigs() {
  std::vector<hats::SurveyConfig> survey_configs;

  // Always add the default survey.
  hats::SurveyConfig default_survey;
  default_survey.enabled = true;
  default_survey.probability = 1.0f;
  default_survey.trigger = kHatsSurveyTriggerTesting;
  default_survey.trigger_id = kHatsNextSurveyTriggerIDTesting;
  default_survey.product_specific_bits_data_fields = {"Test Field 1",
                                                      "Test Field 2"};
  default_survey.product_specific_string_data_fields = {"Test Field 3"};
  survey_configs.emplace_back(default_survey);

  // Permission prompt survey
  survey_configs.emplace_back(
      &permissions::features::kPermissionsPromptSurvey,
      kHatsSurveyTriggerPermissionsPrompt,
      /*presupplied_trigger_id=*/std::nullopt,
      std::vector<std::string>{
          permissions::kPermissionsPromptSurveyHadGestureKey},
      std::vector<std::string>{
          permissions::kPermissionsPromptSurveyPromptDispositionKey,
          permissions::kPermissionsPromptSurveyPromptDispositionReasonKey,
          permissions::kPermissionsPromptSurveyActionKey,
          permissions::kPermissionsPromptSurveyRequestTypeKey,
          permissions::kPermissionsPromptSurveyReleaseChannelKey,
          permissions::kPermissionsPromptSurveyDisplayTimeKey,
          permissions::kPermissionPromptSurveyOneTimePromptsDecidedBucketKey,
          permissions::kPermissionPromptSurveyUrlKey});

#if !BUILDFLAG(IS_ANDROID)
  // Dev tools surveys.
  survey_configs.emplace_back(&features::kHaTSDesktopDevToolsIssuesCOEP,
                              "devtools-issues-coep",
                              "1DbEs89FS0ugnJ3q1cK0Nx6T99yT");
  survey_configs.emplace_back(&features::kHaTSDesktopDevToolsIssuesMixedContent,
                              "devtools-issues-mixed-content",
                              "BhCYpUmyf0ugnJ3q1cK0VtxCftzo");
  survey_configs.emplace_back(
      &features::
          kHappinessTrackingSurveysForDesktopDevToolsIssuesCookiesSameSite,
      "devtools-issues-cookies-samesite", "w9JqqpmEr0ugnJ3q1cK0NezVg4iK");
  survey_configs.emplace_back(&features::kHaTSDesktopDevToolsIssuesHeavyAd,
                              "devtools-issues-heavy-ad",
                              "bAeiT5J4P0ugnJ3q1cK0Ra6jg7s8");
  survey_configs.emplace_back(&features::kHaTSDesktopDevToolsIssuesCSP,
                              "devtools-issues-csp",
                              "c9fjDmwjb0ugnJ3q1cK0USeAJJ9C");

  // Settings surveys.
  survey_configs.emplace_back(
      &features::kHappinessTrackingSurveysForDesktopSettings,
      kHatsSurveyTriggerSettings);
  survey_configs.emplace_back(
      &features::kHappinessTrackingSurveysForDesktopSettingsPrivacy,
      kHatsSurveyTriggerSettingsPrivacy,
      /*presupplied_trigger_id=*/std::nullopt,
      std::vector<std::string>{"3P cookies blocked"});
  survey_configs.emplace_back(
      &features::kHappinessTrackingSurveysForSecurityPage,
      kHatsSurveyTriggerSettingsSecurity,
      /*presupplied_trigger_id=*/
      features::kHappinessTrackingSurveysForSecurityPageTriggerId.Get(),
      std::vector<std::string>{},
      std::vector<std::string>{
          "Security Page User Action", "Safe Browsing Setting Before Trigger",
          "Safe Browsing Setting After Trigger", "Client Channel",
          "Time On Page", "Friendlier Safe Browsing Settings"});
  survey_configs.emplace_back(
      &features::kHappinessTrackingSurveysForDesktopPrivacyGuide,
      kHatsSurveyTriggerPrivacyGuide);

#if BUILDFLAG(GOOGLE_CHROME_BRANDING)
  survey_configs.emplace_back(&features::kHappinessTrackingSurveysGetMostChrome,
                              kHatsSurveyTriggerGetMostChrome);
#endif

  const auto ad_privacy_product_specific_bits_data =
      std::vector<std::string>{"3P cookies blocked", "Topics enabled",
                               "Fledge enabled", "Ad Measurement enabled"};
  survey_configs.emplace_back(
      &features::kHappinessTrackingSurveysForDesktopM1AdPrivacyPage,
      kHatsSurveyTriggerM1AdPrivacyPage,
      /*presupplied_trigger_id=*/std::nullopt,
      ad_privacy_product_specific_bits_data);
  survey_configs.emplace_back(
      &features::kHappinessTrackingSurveysForDesktopM1TopicsSubpage,
      kHatsSurveyTriggerM1TopicsSubpage,
      /*presupplied_trigger_id=*/std::nullopt,
      ad_privacy_product_specific_bits_data);
  survey_configs.emplace_back(
      &features::kHappinessTrackingSurveysForDesktopM1FledgeSubpage,
      kHatsSurveyTriggerM1FledgeSubpage,
      /*presupplied_trigger_id=*/std::nullopt,
      ad_privacy_product_specific_bits_data);
  survey_configs.emplace_back(
      &features::kHappinessTrackingSurveysForDesktopM1AdMeasurementSubpage,
      kHatsSurveyTriggerM1AdMeasurementSubpage,
      /*presupplied_trigger_id=*/std::nullopt,
      ad_privacy_product_specific_bits_data);

  // NTP modules survey.
  survey_configs.emplace_back(
      &features::kHappinessTrackingSurveysForDesktopNtpModules,
      kHatsSurveyTriggerNtpModules);

  // NTP Photos module opt-out survey.
  survey_configs.emplace_back(
      &features::kHappinessTrackingSurveysForNtpPhotosOptOut,
      kHatsSurveyTriggerNtpPhotosModuleOptOut);

  // Trust & Safety Sentiment surveys.
  survey_configs.emplace_back(
      &features::kTrustSafetySentimentSurvey,
      kHatsSurveyTriggerTrustSafetyPrivacySettings,
      features::kTrustSafetySentimentSurveyPrivacySettingsTriggerId.Get(),
      std::vector<std::string>{"Non default setting", "Ran safety check"});
  survey_configs.emplace_back(
      &features::kTrustSafetySentimentSurvey,
      kHatsSurveyTriggerTrustSafetyTrustedSurface,
      features::kTrustSafetySentimentSurveyTrustedSurfaceTriggerId.Get(),
      std::vector<std::string>{"Interacted with Page Info"});
  survey_configs.emplace_back(
      &features::kTrustSafetySentimentSurvey,
      kHatsSurveyTriggerTrustSafetyTransactions,
      features::kTrustSafetySentimentSurveyTransactionsTriggerId.Get(),
      std::vector<std::string>{"Saved password"});
  survey_configs.emplace_back(
      &features::kTrustSafetySentimentSurvey,
      kHatsSurveyTriggerTrustSafetyPrivacySandbox4ConsentAccept,
      features::kTrustSafetySentimentSurveyPrivacySandbox4ConsentAcceptTriggerId
          .Get());
  survey_configs.emplace_back(
      &features::kTrustSafetySentimentSurvey,
      kHatsSurveyTriggerTrustSafetyPrivacySandbox4ConsentDecline,
      features::
          kTrustSafetySentimentSurveyPrivacySandbox4ConsentDeclineTriggerId
              .Get());
  survey_configs.emplace_back(
      &features::kTrustSafetySentimentSurvey,
      kHatsSurveyTriggerTrustSafetyPrivacySandbox4NoticeOk,
      features::kTrustSafetySentimentSurveyPrivacySandbox4NoticeOkTriggerId
          .Get());
  survey_configs.emplace_back(
      &features::kTrustSafetySentimentSurvey,
      kHatsSurveyTriggerTrustSafetyPrivacySandbox4NoticeSettings,
      features::
          kTrustSafetySentimentSurveyPrivacySandbox4NoticeSettingsTriggerId
              .Get());

  // Trust & Safety Sentiment surveys - Version 2.
  survey_configs.emplace_back(
      &features::kTrustSafetySentimentSurveyV2,
      kHatsSurveyTriggerTrustSafetyV2BrowsingData,
      features::kTrustSafetySentimentSurveyV2BrowsingDataTriggerId.Get(),
      std::vector<std::string>{"Deleted history", "Deleted downloads",
                               "Deleted autofill form data"});
  survey_configs.emplace_back(
      &features::kTrustSafetySentimentSurveyV2,
      kHatsSurveyTriggerTrustSafetyV2ControlGroup,
      features::kTrustSafetySentimentSurveyV2ControlGroupTriggerId.Get());
  survey_configs.emplace_back(
      &features::kTrustSafetySentimentSurveyV2,
      kHatsSurveyTriggerTrustSafetyV2DownloadWarningUI,
      features::kTrustSafetySentimentSurveyV2DownloadWarningUITriggerId.Get(),
      std::vector<std::string>{"Enhanced protection enabled", "Is mainpage UI",
                               "Is subpage UI", "Is downloads page UI",
                               "Is download prompt UI",
                               "User proceeded past warning"});
  survey_configs.emplace_back(
      &features::kTrustSafetySentimentSurveyV2,
      kHatsSurveyTriggerTrustSafetyV2PasswordCheck,
      features::kTrustSafetySentimentSurveyV2PasswordCheckTriggerId.Get());
  survey_configs.emplace_back(
      &features::kTrustSafetySentimentSurveyV2,
      kHatsSurveyTriggerTrustSafetyV2PasswordProtectionUI,
      features::kTrustSafetySentimentSurveyV2PasswordProtectionUITriggerId
          .Get(),
      std::vector<std::string>{
          "Enhanced protection enabled", "Is page info UI",
          "Is modal dialog UI", "Is interstitial UI",
          "User completed password change", "User clicked change password",
          "User ignored warning", "User marked as legitimate"});
  survey_configs.emplace_back(
      &features::kTrustSafetySentimentSurveyV2,
      kHatsSurveyTriggerTrustSafetyV2SafetyCheck,
      features::kTrustSafetySentimentSurveyV2SafetyCheckTriggerId.Get());
  survey_configs.emplace_back(
      &features::kTrustSafetySentimentSurveyV2,
      kHatsSurveyTriggerTrustSafetyV2TrustedSurface,
      features::kTrustSafetySentimentSurveyV2TrustedSurfaceTriggerId.Get(),
      std::vector<std::string>{"Interacted with Page Info"});
  survey_configs.emplace_back(
      &features::kTrustSafetySentimentSurveyV2,
      kHatsSurveyTriggerTrustSafetyV2PrivacyGuide,
      features::kTrustSafetySentimentSurveyV2PrivacyGuideTriggerId.Get());
  survey_configs.emplace_back(
      &features::kTrustSafetySentimentSurveyV2,
      kHatsSurveyTriggerTrustSafetyV2PrivacySandbox4ConsentAccept,
      features::
          kTrustSafetySentimentSurveyV2PrivacySandbox4ConsentAcceptTriggerId
              .Get());
  survey_configs.emplace_back(
      &features::kTrustSafetySentimentSurveyV2,
      kHatsSurveyTriggerTrustSafetyV2PrivacySandbox4ConsentDecline,
      features::
          kTrustSafetySentimentSurveyV2PrivacySandbox4ConsentDeclineTriggerId
              .Get());
  survey_configs.emplace_back(
      &features::kTrustSafetySentimentSurveyV2,
      kHatsSurveyTriggerTrustSafetyV2PrivacySandbox4NoticeOk,
      features::kTrustSafetySentimentSurveyV2PrivacySandbox4NoticeOkTriggerId
          .Get());
  survey_configs.emplace_back(
      &features::kTrustSafetySentimentSurveyV2,
      kHatsSurveyTriggerTrustSafetyV2PrivacySandbox4NoticeSettings,
      features::
          kTrustSafetySentimentSurveyV2PrivacySandbox4NoticeSettingsTriggerId
              .Get());
  survey_configs.emplace_back(
      &features::kTrustSafetySentimentSurveyV2,
      kHatsSurveyTriggerTrustSafetyV2SafeBrowsingInterstitial,
      features::kTrustSafetySentimentSurveyV2SafeBrowsingInterstitialTriggerId
          .Get(),
      std::vector<std::string>{
          "User proceeded past interstitial", "Enhanced protection enabled",
          "Threat is phishing", "Threat is malware",
          "Threat is unwanted software", "Threat is billing"});
  survey_configs.emplace_back(
      &features::kHappinessTrackingSurveysExtensionsSafetyHub,
      kHatsSurveyTriggerExtensions,
      features::kHappinessTrackingSurveysExtensionsSafetyHubTriggerId.Get(),
      std::vector<std::string>{},
      std::vector<std::string>{
          "Average extension age in days", "Age of profile in days",
          "Time since last extension was installed in days",
          "Number of extensions installed", "Time on extension page in seconds",
          "Extension review panel shown", "Number of extensions removed",
          "Number of extensions kept",
          "Number of non-trigger extensions removed", "Client Channel"});

  // Autofill surveys.
  survey_configs.emplace_back(
      &::autofill::features::kAutofillAddressUserPerceptionSurvey,
      kHatsSurveyTriggerAutofillAddressUserPerception, std::nullopt,
      std::vector<std::string>{"granular filling available"},
      std::vector<std::string>{
          "Accepted fields", "Corrected to same type",
          "Corrected to a different type", "Corrected to an unknown type",
          "Corrected to empty", "Manually filled to same type",
          "Manually filled to a different type",
          "Manually filled to an unknown type", "Total corrected",
          "Total filled", "Total unfilled", "Total manually filled",
          "Total number of fields"});

  survey_configs.emplace_back(
      &::autofill::features::kAutofillCreditCardUserPerceptionSurvey,
      kHatsSurveyTriggerAutofillCreditCardUserPerception, std::nullopt,
      std::vector<std::string>{},
      std::vector<std::string>{
          "Accepted fields", "Corrected to same type",
          "Corrected to a different type", "Corrected to an unknown type",
          "Corrected to empty", "Manually filled to same type",
          "Manually filled to a different type",
          "Manually filled to an unknown type", "Total corrected",
          "Total filled", "Total unfilled", "Total manually filled",
          "Total number of fields"});

  survey_configs.emplace_back(&features::kAutofillAddressSurvey,
                              kHatsSurveyTriggerAutofillAddress);
  survey_configs.emplace_back(&features::kAutofillCardSurvey,
                              kHatsSurveyTriggerAutofillCard);
  survey_configs.emplace_back(&features::kAutofillPasswordSurvey,
                              kHatsSurveyTriggerAutofillPassword);

  // Wallpaper Search survey.
  survey_configs.emplace_back(
      &features::kHappinessTrackingSurveysForWallpaperSearch,
      kHatsSurveyTriggerWallpaperSearch);

  // What's New survey.
  survey_configs.emplace_back(
      &features::kHappinessTrackingSurveysForDesktopWhatsNew,
      kHatsSurveyTriggerWhatsNew);

  // Performance Controls surveys.
  survey_configs.emplace_back(
      &performance_manager::features::kPerformanceControlsPerformanceSurvey,
      kHatsSurveyTriggerPerformanceControlsPerformance,
      /*presupplied_trigger_id=*/std::nullopt,
      std::vector<std::string>{"high_efficiency_mode", "battery_saver_mode"},
      std::vector<std::string>{});
  survey_configs.emplace_back(
      &performance_manager::features::
          kPerformanceControlsBatteryPerformanceSurvey,
      kHatsSurveyTriggerPerformanceControlsBatteryPerformance,
      /*presupplied_trigger_id=*/std::nullopt,
      std::vector<std::string>{"high_efficiency_mode", "battery_saver_mode"},
      std::vector<std::string>{});
  survey_configs.emplace_back(
      &performance_manager::features::
          kPerformanceControlsMemorySaverOptOutSurvey,
      kHatsSurveyTriggerPerformanceControlsMemorySaverOptOut);
  survey_configs.emplace_back(
      &performance_manager::features::
          kPerformanceControlsBatterySaverOptOutSurvey,
      kHatsSurveyTriggerPerformanceControlsBatterySaverOptOut);

  // Red Warning surveys.
  survey_configs.emplace_back(
      &safe_browsing::kRedWarningSurvey, kHatsSurveyTriggerRedWarning,
      safe_browsing::kRedWarningSurveyTriggerId.Get(),
      std::vector<std::string>{},
      std::vector<std::string>{
          safe_browsing::kFlaggedUrl, safe_browsing::kMainFrameUrl,
          safe_browsing::kReferrerUrl, safe_browsing::kUserActivityWithUrls});

  // Suggested passwords experiment surveys.
  survey_configs.emplace_back(
      &password_manager::features::kPasswordGenerationExperiment,
      kHatsSurveyTriggerSuggestedPasswordsExperiment,
      password_manager::features::kPasswordGenerationExperimentSurveyTriggerId
          .Get(),
      std::vector<std::string>{"Suggested password accepted"});

  // Desktop download warning surveys.
  survey_configs.emplace_back(
      &safe_browsing::kDownloadWarningSurvey,
      kHatsSurveyTriggerDownloadWarningBubbleBypass,
      /*presupplied_trigger_id=*/std::nullopt,
      DownloadWarningHatsProductSpecificData::GetBitsDataFields(
          DownloadWarningHatsType::kDownloadBubbleBypass),
      DownloadWarningHatsProductSpecificData::GetStringDataFields(
          DownloadWarningHatsType::kDownloadBubbleBypass));
  survey_configs.emplace_back(
      &safe_browsing::kDownloadWarningSurvey,
      kHatsSurveyTriggerDownloadWarningBubbleHeed,
      /*presupplied_trigger_id=*/std::nullopt,
      DownloadWarningHatsProductSpecificData::GetBitsDataFields(
          DownloadWarningHatsType::kDownloadBubbleHeed),
      DownloadWarningHatsProductSpecificData::GetStringDataFields(
          DownloadWarningHatsType::kDownloadBubbleHeed));
  survey_configs.emplace_back(
      &safe_browsing::kDownloadWarningSurvey,
      kHatsSurveyTriggerDownloadWarningBubbleIgnore,
      /*presupplied_trigger_id=*/std::nullopt,
      DownloadWarningHatsProductSpecificData::GetBitsDataFields(
          DownloadWarningHatsType::kDownloadBubbleIgnore),
      DownloadWarningHatsProductSpecificData::GetStringDataFields(
          DownloadWarningHatsType::kDownloadBubbleIgnore));
  survey_configs.emplace_back(
      &safe_browsing::kDownloadWarningSurvey,
      kHatsSurveyTriggerDownloadWarningPageBypass,
      /*presupplied_trigger_id=*/std::nullopt,
      DownloadWarningHatsProductSpecificData::GetBitsDataFields(
          DownloadWarningHatsType::kDownloadsPageBypass),
      DownloadWarningHatsProductSpecificData::GetStringDataFields(
          DownloadWarningHatsType::kDownloadsPageBypass));
  survey_configs.emplace_back(
      &safe_browsing::kDownloadWarningSurvey,
      kHatsSurveyTriggerDownloadWarningPageHeed,
      /*presupplied_trigger_id=*/std::nullopt,
      DownloadWarningHatsProductSpecificData::GetBitsDataFields(
          DownloadWarningHatsType::kDownloadsPageHeed),
      DownloadWarningHatsProductSpecificData::GetStringDataFields(
          DownloadWarningHatsType::kDownloadsPageHeed));
  survey_configs.emplace_back(
      &safe_browsing::kDownloadWarningSurvey,
      kHatsSurveyTriggerDownloadWarningPageIgnore,
      /*presupplied_trigger_id=*/std::nullopt,
      DownloadWarningHatsProductSpecificData::GetBitsDataFields(
          DownloadWarningHatsType::kDownloadsPageIgnore),
      DownloadWarningHatsProductSpecificData::GetStringDataFields(
          DownloadWarningHatsType::kDownloadsPageIgnore));

#else
  survey_configs.emplace_back(&chrome::android::kChromeSurveyNextAndroid,
                              kHatsSurveyTriggerAndroidStartupSurvey);

#endif  // #if !BUILDFLAG(IS_ANDROID)

  return survey_configs;
}

}  // namespace

namespace hats {

SurveyConfig::SurveyConfig() = default;
SurveyConfig::SurveyConfig(const SurveyConfig&) = default;
SurveyConfig::~SurveyConfig() = default;

SurveyConfig::SurveyConfig(
    const base::Feature* feature,
    const std::string& trigger,
    const std::optional<std::string>& presupplied_trigger_id,
    const std::vector<std::string>& product_specific_bits_data_fields,
    const std::vector<std::string>& product_specific_string_data_fields)
    : trigger(trigger),
      product_specific_bits_data_fields(product_specific_bits_data_fields),
      product_specific_string_data_fields(product_specific_string_data_fields) {
  enabled = base::FeatureList::IsEnabled(*feature);
  if (!enabled) {
    return;
  }

  probability = base::FeatureParam<double>(feature, kHatsSurveyProbability,
                                           kHatsSurveyProbabilityDefault)
                    .Get();

  // The trigger_id may be provided through the associated feature parameter or
  // may have been included in the source code. The latter is required to enable
  // multiple surveys with a single finch group, as a limitation with finch
  // prevents duplicate param names even for different features within a group.
  // The feature parameter name is "en_site_id" for legacy reasons, as this
  // was the HaTS v1 equivalent of a trigger ID in HaTS Next.
  trigger_id = presupplied_trigger_id ? *presupplied_trigger_id
                                      : base::FeatureParam<std::string>(
                                            feature, kHatsSurveyEnSiteID, "")
                                            .Get();

  user_prompted =
      base::FeatureParam<bool>(feature, "user_prompted", false).Get();
}

void GetActiveSurveyConfigs(SurveyConfigs& survey_configs_by_triggers_) {
  auto surveys = GetAllSurveyConfigs();

  // Filter down to active surveys configs and store them in a map for faster
  // access. Triggers within the browser may attempt to show surveys regardless
  // of whether the feature is enabled, so checking whether a particular survey
  // is enabled should be fast.
  for (const SurveyConfig& survey : surveys) {
    if (!survey.enabled) {
      continue;
    }

    survey_configs_by_triggers_.emplace(survey.trigger, survey);
  }
}

}  // namespace hats
