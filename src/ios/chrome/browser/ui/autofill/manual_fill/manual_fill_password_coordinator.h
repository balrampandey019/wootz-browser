// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_UI_AUTOFILL_MANUAL_FILL_MANUAL_FILL_PASSWORD_COORDINATOR_H_
#define IOS_CHROME_BROWSER_UI_AUTOFILL_MANUAL_FILL_MANUAL_FILL_PASSWORD_COORDINATOR_H_

#import <string>

#import "ios/chrome/browser/ui/autofill/manual_fill/fallback_coordinator.h"

class GURL;

namespace autofill {
class FormRendererId;
}

namespace password_manager {
struct CredentialUIEntry;
}

// Delegate for the coordinator actions.
@protocol PasswordCoordinatorDelegate <FallbackCoordinatorDelegate>

// Opens the password manager.
- (void)openPasswordManager;

// Opens the passwords settings.
- (void)openPasswordSettings;

// Opens the all passwords picker, used for manual fallback.
- (void)openAllPasswordsPicker;

// Opens password suggestion confirmation alert.
- (void)openPasswordSuggestion;

// Opens the details of the given credential.
- (void)openPasswordDetailsForCredential:
    (password_manager::CredentialUIEntry)credential;

@end

// Creates and manages a view controller to present passwords to the user. It
// will filter the passwords based on the passed URL when instantiating it. Any
// selected password will be sent to the current field in the active web state.
@interface ManualFillPasswordCoordinator : FallbackCoordinator

// The delegate for this coordinator. Delegate protocol conforms to
// FallbackCoordinatorDelegate, and replaces the superclass delegate.
@property(nonatomic, weak) id<PasswordCoordinatorDelegate> delegate;

// Creates a coordinator that uses a `viewController`, `browser`,
// `URL`, an `injectionHandler` and relevant information related to the current
// form.
- (instancetype)
    initWithBaseViewController:(UIViewController*)viewController
                       browser:(Browser*)browser
                           URL:(const GURL&)URL
              injectionHandler:(ManualFillInjectionHandler*)injectionHandler
      invokedOnObfuscatedField:(BOOL)invokedOnObfuscatedField
                        formID:(const autofill::FormRendererId)formID
                       frameID:(const std::string&)frameID
    NS_DESIGNATED_INITIALIZER;

// Unavailable, use
// -initWithBaseViewController:browser:URL:injectionHandler:.
- (instancetype)initWithBaseViewController:(UIViewController*)viewController
                                   browser:(Browser*)browser
                          injectionHandler:
                              (ManualFillInjectionHandler*)injectionHandler
    NS_UNAVAILABLE;

// Presents the password view controller as a popover from the passed button.
- (void)presentFromButton:(UIButton*)button;

@end

#endif  // IOS_CHROME_BROWSER_UI_AUTOFILL_MANUAL_FILL_MANUAL_FILL_PASSWORD_COORDINATOR_H_
