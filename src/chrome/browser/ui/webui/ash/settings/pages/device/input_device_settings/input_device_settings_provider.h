// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_ASH_SETTINGS_PAGES_DEVICE_INPUT_DEVICE_SETTINGS_INPUT_DEVICE_SETTINGS_PROVIDER_H_
#define CHROME_BROWSER_UI_WEBUI_ASH_SETTINGS_PAGES_DEVICE_INPUT_DEVICE_SETTINGS_INPUT_DEVICE_SETTINGS_PROVIDER_H_

#include "ash/public/cpp/input_device_settings_controller.h"
#include "ash/public/mojom/input_device_settings.mojom.h"
#include "ash/shell.h"
#include "ash/shell_observer.h"
#include "ash/system/keyboard_brightness_control_delegate.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/scoped_observation.h"
#include "chrome/browser/ui/webui/ash/settings/pages/device/input_device_settings/input_device_settings_provider.mojom.h"
#include "chromeos/dbus/power/power_manager_client.h"
#include "content/public/browser/web_ui.h"
#include "mojo/public/cpp/bindings/pending_remote.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "mojo/public/cpp/bindings/remote_set.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/views/widget/widget.h"
#include "ui/views/widget/widget_observer.h"

namespace ash::settings {

class InputDeviceSettingsProvider
    : public mojom::InputDeviceSettingsProvider,
      public InputDeviceSettingsController::Observer,
      public views::WidgetObserver,
      public chromeos::PowerManagerClient::Observer,
      public ash::ShellObserver {
 public:
  InputDeviceSettingsProvider();
  InputDeviceSettingsProvider(const InputDeviceSettingsProvider& other) =
      delete;
  InputDeviceSettingsProvider& operator=(
      const InputDeviceSettingsProvider& other) = delete;
  ~InputDeviceSettingsProvider() override;

  void Initialize(content::WebUI* web_ui);

  void BindInterface(
      mojo::PendingReceiver<mojom::InputDeviceSettingsProvider> receiver);

  // mojom::InputDeviceSettingsProvider:
  void ObserveKeyboardSettings(
      mojo::PendingRemote<mojom::KeyboardSettingsObserver> observer) override;
  void ObserveTouchpadSettings(
      mojo::PendingRemote<mojom::TouchpadSettingsObserver> observer) override;
  void ObservePointingStickSettings(
      mojo::PendingRemote<mojom::PointingStickSettingsObserver> observer)
      override;
  void ObserveMouseSettings(
      mojo::PendingRemote<mojom::MouseSettingsObserver> observer) override;
  void ObserveGraphicsTabletSettings(
      mojo::PendingRemote<mojom::GraphicsTabletSettingsObserver> observer)
      override;
  void ObserveButtonPresses(
      mojo::PendingRemote<mojom::ButtonPressObserver> observer) override;
  void ObserveKeyboardBrightness(
      mojo::PendingRemote<mojom::KeyboardBrightnessObserver> observer) override;

  void RestoreDefaultKeyboardRemappings(uint32_t device_id) override;
  void SetKeyboardSettings(uint32_t device_id,
                           ::ash::mojom::KeyboardSettingsPtr settings) override;
  void SetPointingStickSettings(
      uint32_t device_id,
      ::ash::mojom::PointingStickSettingsPtr settings) override;
  void SetMouseSettings(uint32_t device_id,
                        ::ash::mojom::MouseSettingsPtr settings) override;
  void SetTouchpadSettings(uint32_t device_id,
                           ::ash::mojom::TouchpadSettingsPtr settings) override;
  void SetGraphicsTabletSettings(
      uint32_t device_id,
      ::ash::mojom::GraphicsTabletSettingsPtr settings) override;
  void SetKeyboardBrightness(double percent) override;
  void SetKeyboardAmbientLightSensorEnabled(bool enabled) override;

  // InputDeviceSettingsController::Observer:
  void OnKeyboardConnected(const ::ash::mojom::Keyboard& keyboard) override;
  void OnKeyboardDisconnected(const ::ash::mojom::Keyboard& keyboard) override;
  void OnKeyboardSettingsUpdated(
      const ::ash::mojom::Keyboard& keyboard) override;
  void OnKeyboardPoliciesUpdated(
      const ::ash::mojom::KeyboardPolicies& keyboard_policies) override;
  void OnTouchpadConnected(const ::ash::mojom::Touchpad& touchpad) override;
  void OnTouchpadDisconnected(const ::ash::mojom::Touchpad& touchpad) override;
  void OnTouchpadSettingsUpdated(
      const ::ash::mojom::Touchpad& touchpad) override;
  void OnPointingStickConnected(
      const ::ash::mojom::PointingStick& pointing_stick) override;
  void OnPointingStickDisconnected(
      const ::ash::mojom::PointingStick& pointing_stick) override;
  void OnPointingStickSettingsUpdated(
      const ::ash::mojom::PointingStick& pointing_stick) override;
  void OnMouseConnected(const ::ash::mojom::Mouse& mouse) override;
  void OnMouseDisconnected(const ::ash::mojom::Mouse& mouse) override;
  void OnMouseSettingsUpdated(const ::ash::mojom::Mouse& mouse) override;
  void OnMousePoliciesUpdated(
      const ::ash::mojom::MousePolicies& mouse_policies) override;
  void OnGraphicsTabletConnected(
      const ::ash::mojom::GraphicsTablet& graphics_tablet) override;
  void OnGraphicsTabletDisconnected(
      const ::ash::mojom::GraphicsTablet& graphics_tablet) override;
  void OnGraphicsTabletSettingsUpdated(
      const ::ash::mojom::GraphicsTablet& graphics_tablet) override;
  void OnCustomizableMouseButtonPressed(
      const ::ash::mojom::Mouse& mouse,
      const ::ash::mojom::Button& button) override;
  void OnCustomizablePenButtonPressed(
      const ::ash::mojom::GraphicsTablet& graphics_tablet,
      const ::ash::mojom::Button& button) override;
  void OnCustomizableTabletButtonPressed(
      const ::ash::mojom::GraphicsTablet& graphics_tablet,
      const ::ash::mojom::Button& button) override;
  void OnKeyboardBatteryInfoChanged(
      const ::ash::mojom::Keyboard& keyboard) override;
  void OnGraphicsTabletBatteryInfoChanged(
      const ::ash::mojom::GraphicsTablet& graphics_tablet) override;
  void OnMouseBatteryInfoChanged(const ::ash::mojom::Mouse& mouse) override;
  void OnTouchpadBatteryInfoChanged(
      const ::ash::mojom::Touchpad& touchpad) override;

  void StartObserving(uint32_t device_id) override;
  void StopObserving() override;
  void GetActionsForMouseButtonCustomization(
      GetActionsForMouseButtonCustomizationCallback callback) override;
  void GetActionsForGraphicsTabletButtonCustomization(
      GetActionsForGraphicsTabletButtonCustomizationCallback callback) override;

  // chromeos::PowerManagerClient observer:
  void KeyboardBrightnessChanged(
      const power_manager::BacklightBrightnessChange& change) override;

  // ash::ShellObserver:
  void OnShellDestroying() override;

  // views::WidgetObserver:
  void OnWidgetVisibilityChanged(views::Widget* widget, bool visible) override;
  void OnWidgetActivationChanged(views::Widget* widget, bool active) override;
  void OnWidgetDestroyed(views::Widget* widget) override;

  void SetWidgetForTesting(views::Widget* widget);
  void HasLauncherButton(HasLauncherButtonCallback callback) override;
  void HasKeyboardBacklight(HasKeyboardBacklightCallback callback) override;
  void HasAmbientLightSensor(HasAmbientLightSensorCallback callback) override;
  void IsRgbKeyboardSupported(IsRgbKeyboardSupportedCallback callback) override;
  void RecordKeyboardColorLinkClicked() override;
  void RecordKeyboardBrightnessChangeFromSlider(double percent) override;

  void SetKeyboardBrightnessControlDelegateForTesting(
      raw_ptr<KeyboardBrightnessControlDelegate> delegate) {
    keyboard_brightness_control_delegate_ = delegate;
  }

 private:
  void NotifyKeyboardsUpdated();
  void NotifyTouchpadsUpdated();
  void NotifyPointingSticksUpdated();
  void NotifyMiceUpdated();
  void NotifyGraphicsTabletUpdated();

  void HandleObserving();

  void OnReceiveHasKeyboardBacklight(HasKeyboardBacklightCallback callback,
                                     std::optional<bool> has_backlight);

  void OnReceiveHasAmbientLightSensor(HasAmbientLightSensorCallback callback,
                                      std::optional<bool> has_sensor);

  void OnReceiveKeyboardBrightness(std::optional<double> brightness_percent);

  // Denotes whether button observing should be paused due to the settings app
  // being out of focus or minimized. Default to true to require a valid widget
  // to observe devices.
  bool observing_paused_ = true;
  // The list of device ids to observe when the settings app is focused or in
  // use by the user.
  base::flat_set<uint32_t> observing_devices_;

  mojo::RemoteSet<mojom::KeyboardSettingsObserver> keyboard_settings_observers_;
  mojo::RemoteSet<mojom::TouchpadSettingsObserver> touchpad_settings_observers_;
  mojo::RemoteSet<mojom::PointingStickSettingsObserver>
      pointing_stick_settings_observers_;
  mojo::RemoteSet<mojom::MouseSettingsObserver> mouse_settings_observers_;
  mojo::RemoteSet<mojom::GraphicsTabletSettingsObserver>
      graphics_tablet_settings_observers_;
  mojo::RemoteSet<mojom::ButtonPressObserver> button_press_observers_;
  mojo::Remote<mojom::KeyboardBrightnessObserver> keyboard_brightness_observer_;

  raw_ptr<views::Widget> widget_ = nullptr;

  raw_ptr<KeyboardBrightnessControlDelegate>
      keyboard_brightness_control_delegate_ = nullptr;

  mojo::Receiver<mojom::InputDeviceSettingsProvider> receiver_{this};

  // The observation on `ash::Shell`.
  base::ScopedObservation<ash::Shell, ash::ShellObserver> shell_observation_{
      this};

  base::WeakPtrFactory<InputDeviceSettingsProvider> weak_ptr_factory_{this};
};

}  // namespace ash::settings

#endif  // CHROME_BROWSER_UI_WEBUI_ASH_SETTINGS_PAGES_DEVICE_INPUT_DEVICE_SETTINGS_INPUT_DEVICE_SETTINGS_PROVIDER_H_
