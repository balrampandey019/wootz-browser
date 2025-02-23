// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "components/autofill/ios/browser/form_suggestion.h"

@implementation FormSuggestion

- (instancetype)initWithValue:(NSString*)value
                    minorValue:(NSString*)minorValue
            displayDescription:(NSString*)displayDescription
                          icon:(UIImage*)icon
                   popupItemId:(autofill::SuggestionType)popupItemId
             backendIdentifier:(NSString*)backendIdentifier
                requiresReauth:(BOOL)requiresReauth
    acceptanceA11yAnnouncement:(NSString*)acceptanceA11yAnnouncement
                      metadata:(FormSuggestionMetadata)metadata {
  self = [super init];
  if (self) {
    _value = [value copy];
    _minorValue = [minorValue copy];
    _displayDescription = [displayDescription copy];
    _icon = [icon copy];
    _popupItemId = popupItemId;
    _backendIdentifier = backendIdentifier;
    _requiresReauth = requiresReauth;
    _acceptanceA11yAnnouncement = [acceptanceA11yAnnouncement copy];
    _metadata = metadata;
  }
  return self;
}

+ (FormSuggestion*)suggestionWithValue:(NSString*)value
                    displayDescription:(NSString*)displayDescription
                                  icon:(UIImage*)icon
                           popupItemId:(autofill::SuggestionType)popupItemId
                     backendIdentifier:(NSString*)backendIdentifier
                        requiresReauth:(BOOL)requiresReauth
            acceptanceA11yAnnouncement:(NSString*)acceptanceA11yAnnouncement
                              metadata:(FormSuggestionMetadata)metadata {
  return [[FormSuggestion alloc] initWithValue:value
                                    minorValue:nil
                            displayDescription:displayDescription
                                          icon:icon
                                   popupItemId:popupItemId
                             backendIdentifier:backendIdentifier
                                requiresReauth:requiresReauth
                    acceptanceA11yAnnouncement:acceptanceA11yAnnouncement
                                      metadata:metadata];
}

+ (FormSuggestion*)suggestionWithValue:(NSString*)value
                            minorValue:(NSString*)minorValue
                    displayDescription:(NSString*)displayDescription
                                  icon:(UIImage*)icon
                           popupItemId:(autofill::SuggestionType)popupItemId
                     backendIdentifier:(NSString*)backendIdentifier
                        requiresReauth:(BOOL)requiresReauth
            acceptanceA11yAnnouncement:(NSString*)acceptanceA11yAnnouncement {
  return [[FormSuggestion alloc] initWithValue:value
                                    minorValue:minorValue
                            displayDescription:displayDescription
                                          icon:icon
                                   popupItemId:popupItemId
                             backendIdentifier:backendIdentifier
                                requiresReauth:requiresReauth
                    acceptanceA11yAnnouncement:acceptanceA11yAnnouncement
                                      metadata:FormSuggestionMetadata()];
}

+ (FormSuggestion*)suggestionWithValue:(NSString*)value
                    displayDescription:(NSString*)displayDescription
                                  icon:(UIImage*)icon
                           popupItemId:(autofill::SuggestionType)popupItemId
                     backendIdentifier:(NSString*)backendIdentifier
                        requiresReauth:(BOOL)requiresReauth {
  return [[FormSuggestion alloc] initWithValue:value
                                    minorValue:nil
                            displayDescription:displayDescription
                                          icon:icon
                                   popupItemId:popupItemId
                             backendIdentifier:backendIdentifier
                                requiresReauth:requiresReauth
                    acceptanceA11yAnnouncement:nil
                                      metadata:FormSuggestionMetadata()];
}

@end
