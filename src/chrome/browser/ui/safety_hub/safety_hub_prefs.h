// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_SAFETY_HUB_SAFETY_HUB_PREFS_H_
#define CHROME_BROWSER_UI_SAFETY_HUB_SAFETY_HUB_PREFS_H_

class PrefRegistrySimple;

namespace safety_hub_prefs {

// Dictionary that determines the next time SafetyHub will trigger a background
// password check.
inline constexpr char kBackgroundPasswordCheckTimeAndInterval[] =
    "profile.background_password_check";

// Keys used inside the `kBackgroundPasswordCheckTimeAndInterval` pref dict.
inline constexpr char kNextPasswordCheckTimeKey[] = "next_check_time";
inline constexpr char kPasswordCheckIntervalKey[] = "check_interval";
inline constexpr char kPasswordCheckMonWeight[] = "check_mon_weight";
inline constexpr char kPasswordCheckTueWeight[] = "check_tue_weight";
inline constexpr char kPasswordCheckWedWeight[] = "check_wed_weight";
inline constexpr char kPasswordCheckThuWeight[] = "check_thu_weight";
inline constexpr char kPasswordCheckFriWeight[] = "check_fri_weight";
inline constexpr char kPasswordCheckSatWeight[] = "check_sat_weight";
inline constexpr char kPasswordCheckSunWeight[] = "check_sun_weight";

// Dictionary that holds the notifications in the three-dot menu and their
// associated results.
inline const char kMenuNotificationsPrefsKey[] =
    "profile.safety_hub_menu_notifications";

}  // namespace safety_hub_prefs

void RegisterSafetyHubProfilePrefs(PrefRegistrySimple* registry);

#endif  // CHROME_BROWSER_UI_SAFETY_HUB_SAFETY_HUB_PREFS_H_
