// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/performance_manager/test_support/test_user_performance_tuning_manager_environment.h"

#include "base/power_monitor/battery_state_sampler.h"
#include "base/test/power_monitor_test_utils.h"
#include "chrome/browser/performance_manager/test_support/fake_frame_throttling_delegate.h"
#include "chrome/browser/performance_manager/test_support/fake_memory_saver_mode_delegate.h"
#include "chrome/browser/performance_manager/test_support/fake_power_monitor_source.h"
#include "chrome/browser/performance_manager/test_support/fake_render_tuning_delegate.h"
#include "components/prefs/pref_service.h"

namespace performance_manager::user_tuning {

TestUserPerformanceTuningManagerEnvironment::
    TestUserPerformanceTuningManagerEnvironment() = default;

TestUserPerformanceTuningManagerEnvironment::
    ~TestUserPerformanceTuningManagerEnvironment() {
  DCHECK(!user_performance_tuning_manager_)
      << "TearDown must be invoked before destruction";
  DCHECK(!battery_saver_mode_manager_)
      << "TearDown must be invoked before destruction";
  DCHECK(!battery_sampler_) << "TearDown must be invoked before destruction";
}

void TestUserPerformanceTuningManagerEnvironment::SetUp(
    PrefService* local_state) {
  auto source = std::make_unique<FakePowerMonitorSource>();
  power_monitor_source_ = source.get();
  base::PowerMonitor::Initialize(std::move(source));

  auto test_sampling_event_source =
      std::make_unique<base::test::TestSamplingEventSource>();
  auto test_battery_level_provider =
      std::make_unique<base::test::TestBatteryLevelProvider>();

  sampling_source_ = test_sampling_event_source.get();
  battery_level_provider_ = test_battery_level_provider.get();

  battery_sampler_ = std::make_unique<base::BatteryStateSampler>(
      std::move(test_sampling_event_source),
      std::move(test_battery_level_provider));

  user_performance_tuning_manager_.reset(
      new user_tuning::UserPerformanceTuningManager(
          local_state, nullptr,
          std::make_unique<FakeMemorySaverModeDelegate>()));
  battery_saver_mode_manager_.reset(new user_tuning::BatterySaverModeManager(
      local_state,
      std::make_unique<FakeFrameThrottlingDelegate>(&throttling_enabled_),
      std::make_unique<FakeRenderTuningDelegate>(&render_tuning_enabled_)));
  user_performance_tuning_manager_->Start();
  battery_saver_mode_manager_->Start();
}

void TestUserPerformanceTuningManagerEnvironment::TearDown() {
  sampling_source_ = nullptr;
  battery_level_provider_ = nullptr;
  user_performance_tuning_manager_.reset();
  battery_saver_mode_manager_.reset();
  battery_sampler_.reset();
  base::PowerMonitor::ShutdownForTesting();
}

base::test::TestSamplingEventSource*
TestUserPerformanceTuningManagerEnvironment::sampling_source() {
  return sampling_source_;
}

base::test::TestBatteryLevelProvider*
TestUserPerformanceTuningManagerEnvironment::battery_level_provider() {
  return battery_level_provider_;
}

FakePowerMonitorSource*
TestUserPerformanceTuningManagerEnvironment::power_monitor_source() {
  return power_monitor_source_;
}

}  // namespace performance_manager::user_tuning
