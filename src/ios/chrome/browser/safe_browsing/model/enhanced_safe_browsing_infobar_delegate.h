// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_SAFE_BROWSING_MODEL_ENHANCED_SAFE_BROWSING_INFOBAR_DELEGATE_H_
#define IOS_CHROME_BROWSER_SAFE_BROWSING_MODEL_ENHANCED_SAFE_BROWSING_INFOBAR_DELEGATE_H_

#import "components/infobars/core/confirm_infobar_delegate.h"

@protocol SettingsCommands;
namespace web {
class WebState;
}

// Delegate for infobar that prompts users to learn more about Enhanced Safe
// Browsing and navgiates them to the Enhanced Safe Browsing settings page. when
// the package(s) are tracked or untracked.
class EnhancedSafeBrowsingInfobarDelegate : public ConfirmInfoBarDelegate {
 public:
  EnhancedSafeBrowsingInfobarDelegate(
      web::WebState* web_state,
      id<SettingsCommands> settings_commands_handler);

  ~EnhancedSafeBrowsingInfobarDelegate() override;

  // Navigates the user to the Safe Browsing settings menu page.
  void ShowSafeBrowsingSettings();

  // ConfirmInfoBarDelegate implementation.
  InfoBarIdentifier GetIdentifier() const override;
  std::u16string GetMessageText() const override;
  bool EqualsDelegate(infobars::InfoBarDelegate* delegate) const override;

 private:
  raw_ptr<web::WebState> web_state_ = nullptr;
  id<SettingsCommands> settings_commands_handler_;
};

#endif  // IOS_CHROME_BROWSER_SAFE_BROWSING_MODEL_ENHANCED_SAFE_BROWSING_INFOBAR_DELEGATE_H_
