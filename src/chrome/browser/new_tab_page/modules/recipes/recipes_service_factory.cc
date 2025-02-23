// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/new_tab_page/modules/recipes/recipes_service_factory.h"

#include "chrome/browser/browser_process.h"
#include "chrome/browser/content_settings/cookie_settings_factory.h"
#include "chrome/browser/new_tab_page/modules/recipes/recipes_service.h"
#include "chrome/browser/profiles/profile.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/storage_partition.h"

// static
RecipesService* RecipesServiceFactory::GetForProfile(Profile* profile) {
  return static_cast<RecipesService*>(
      GetInstance()->GetServiceForBrowserContext(profile, true));
}

// static
RecipesServiceFactory* RecipesServiceFactory::GetInstance() {
  static base::NoDestructor<RecipesServiceFactory> instance;
  return instance.get();
}

RecipesServiceFactory::RecipesServiceFactory()
    : ProfileKeyedServiceFactory(
          "RecipesService",
          ProfileSelections::Builder()
              .WithRegular(ProfileSelection::kOriginalOnly)
              // TODO(crbug.com/40257657): Check if this service is needed in
              // Guest mode.
              .WithGuest(ProfileSelection::kOriginalOnly)
              .Build()) {
  DependsOn(CookieSettingsFactory::GetInstance());
}

RecipesServiceFactory::~RecipesServiceFactory() = default;

std::unique_ptr<KeyedService>
RecipesServiceFactory::BuildServiceInstanceForBrowserContext(
    content::BrowserContext* context) const {
  auto url_loader_factory = context->GetDefaultStoragePartition()
                                ->GetURLLoaderFactoryForBrowserProcess();
  return std::make_unique<RecipesService>(
      url_loader_factory, Profile::FromBrowserContext(context),
      g_browser_process->GetApplicationLocale());
}
