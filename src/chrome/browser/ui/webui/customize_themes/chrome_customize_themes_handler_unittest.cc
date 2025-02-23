// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/customize_themes/chrome_customize_themes_handler.h"

#include <optional>
#include <string>

#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/json/json_reader.h"
#include "base/memory/raw_ptr.h"
#include "base/path_service.h"
#include "base/run_loop.h"
#include "base/test/bind.h"
#include "base/test/metrics/histogram_tester.h"
#include "base/test/values_test_util.h"
#include "base/values.h"
#include "chrome/browser/extensions/test_extension_environment.h"
#include "chrome/browser/new_tab_page/chrome_colors/generated_colors_info.h"
#include "chrome/browser/new_tab_page/chrome_colors/selected_colors_info.h"
#include "chrome/browser/themes/test/theme_service_changed_waiter.h"
#include "chrome/browser/themes/theme_service.h"
#include "chrome/browser/themes/theme_service_factory.h"
#include "chrome/common/chrome_paths.h"
#include "chrome/common/themes/autogenerated_theme_util.h"
#include "chrome/test/base/testing_profile.h"
#include "components/keyed_service/core/keyed_service.h"
#include "content/public/browser/web_contents.h"
#include "content/public/test/browser_task_environment.h"
#include "content/public/test/test_utils.h"
#include "content/public/test/test_web_contents_factory.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/pending_remote.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/skia/include/core/SkColor.h"
#include "ui/webui/resources/cr_components/customize_themes/customize_themes.mojom.h"

using testing::_;
using testing::NiceMock;
using testing::Pointwise;

namespace {

constexpr char kThemeExtensionName[] = "minimal";
// Extension id must have a valid format.
constexpr char kThemeExtensionId[] = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";

class MockCustomizeThemesClient
    : public customize_themes::mojom::CustomizeThemesClient {
 public:
  MockCustomizeThemesClient() = default;
  ~MockCustomizeThemesClient() override = default;

  mojo::PendingRemote<customize_themes::mojom::CustomizeThemesClient>
  BindAndGetRemote() {
    DCHECK(!receiver_.is_bound());
    return receiver_.BindNewPipeAndPassRemote();
  }

  MOCK_METHOD(void, SetTheme, (customize_themes::mojom::ThemePtr), (override));

  mojo::Receiver<customize_themes::mojom::CustomizeThemesClient> receiver_{
      this};
};

SkColor GetChromeThemeColorById(int chrome_theme_id) {
  for (const auto& color_info : chrome_colors::kGeneratedColorsInfo) {
    if (color_info.id == chrome_theme_id)
      return color_info.color;
  }

  ADD_FAILURE() << "Couldn't find ColorInfo with id=" << chrome_theme_id;
  return SK_ColorTRANSPARENT;
}

// Matches a customize_themes::mojom::ThemePtr argument with an autogenerated
// theme. We do not check the colors themselves and assume the color APIs
// will return the correct color.
MATCHER_P(MatchesAutogeneratedTheme, color, "") {
  return arg->type == customize_themes::mojom::ThemeType::kAutogenerated &&
         arg->info->is_autogenerated_theme_colors();
}

// Matches a customize_themes::mojom::ThemePtr argument with the default theme.
MATCHER(MatchesDefaultTheme, "") {
  return arg->type == customize_themes::mojom::ThemeType::kDefault;
}

// Matches a customize_themes::mojom::ThemePtr argument with a Chrome theme with
// `theme_id`.
MATCHER_P(MatchesChromeTheme, theme_id, "") {
  return arg->type == customize_themes::mojom::ThemeType::kChrome &&
         arg->info->is_chrome_theme_id() &&
         arg->info->get_chrome_theme_id() == theme_id;
}

// Matches a customize_themes::mojom::ThemePtr argument with a third party theme
// with `extension_id` and `name`.
MATCHER_P2(MatchesThirdPartyTheme, extension_id, name, "") {
  return arg->type == customize_themes::mojom::ThemeType::kThirdParty &&
         arg->info->is_third_party_theme_info() &&
         arg->info->get_third_party_theme_info()->id == extension_id &&
         arg->info->get_third_party_theme_info()->name == name;
}

// Matches the id of two elements in
// std::tuple<customize_themes::mojom::ChromeThemePtr,
// chrome_colors::ColorInfo>. We do not check the colors themselves and assume
// the color APIs will return the correct color. Useful for matching two
// containers with testing::Pointwise().
MATCHER(MatchesColorInfo, "") {
  const customize_themes::mojom::ChromeThemePtr& chrome_theme =
      std::get<0>(arg);
  const chrome_colors::ColorInfo& color_info = std::get<1>(arg);
  return chrome_theme->id == color_info.id;
}

}  // namespace

class ChromeCustomizeThemesHandlerTest : public testing::Test {
 public:
  ChromeCustomizeThemesHandlerTest()
      : web_contents_(factory_.CreateWebContents(profile())),
        handler_(std::make_unique<ChromeCustomizeThemesHandler>(
            mock_client_.BindAndGetRemote(),
            mojo::PendingReceiver<
                customize_themes::mojom::CustomizeThemesHandler>(),
            web_contents_,
            profile())) {}

  void TearDown() override {
    if (handler_) {
      // Confirm all pending changes to not trigger a revert on destruction.
      handler_->ConfirmThemeChanges();
    }
  }

  void ResetHandler() { handler_.reset(); }

  extensions::TestExtensionEnvironment* env() {
    return &extension_environment_;
  }

  Profile* profile() { return extension_environment_.profile(); }

  ChromeCustomizeThemesHandler* handler() { return handler_.get(); }

  MockCustomizeThemesClient* mock_client() { return &mock_client_; }

  ThemeService* theme_service() {
    return ThemeServiceFactory::GetForProfile(profile());
  }

 private:
  extensions::TestExtensionEnvironment extension_environment_;
  NiceMock<MockCustomizeThemesClient> mock_client_;
  content::TestWebContentsFactory factory_;
  raw_ptr<content::WebContents> web_contents_;  // Weak. Owned by factory_.
  std::unique_ptr<ChromeCustomizeThemesHandler> handler_;
};

TEST_F(ChromeCustomizeThemesHandlerTest, ApplyAutogeneratedTheme) {
  constexpr SkColor kAutogeneratedThemeColor = SK_ColorBLUE;
  EXPECT_CALL(*mock_client(),
              SetTheme(MatchesAutogeneratedTheme(kAutogeneratedThemeColor)));
  handler()->ApplyAutogeneratedTheme(kAutogeneratedThemeColor);
  EXPECT_TRUE(theme_service()->UsingAutogeneratedTheme());
  EXPECT_EQ(kAutogeneratedThemeColor,
            theme_service()->GetAutogeneratedThemeColor());
}

TEST_F(ChromeCustomizeThemesHandlerTest, ApplyDefaultTheme) {
  EXPECT_CALL(*mock_client(), SetTheme(MatchesDefaultTheme()));
  handler()->ApplyDefaultTheme();
  EXPECT_TRUE(theme_service()->UsingDefaultTheme());
}

TEST_F(ChromeCustomizeThemesHandlerTest, ApplyChromeTheme) {
  constexpr int kChromeThemeId = 4;
  EXPECT_CALL(*mock_client(), SetTheme(MatchesChromeTheme(kChromeThemeId)));
  handler()->ApplyChromeTheme(kChromeThemeId);
  EXPECT_TRUE(theme_service()->UsingAutogeneratedTheme());
  EXPECT_EQ(GetChromeThemeColorById(kChromeThemeId),
            theme_service()->GetAutogeneratedThemeColor());
}

TEST_F(ChromeCustomizeThemesHandlerTest, InitializeTheme) {
  EXPECT_CALL(*mock_client(), SetTheme(_));
  handler()->InitializeTheme();
}

TEST_F(ChromeCustomizeThemesHandlerTest, GetChromeThemes) {
  base::RunLoop run_loop;
  std::vector<customize_themes::mojom::ChromeThemePtr> chrome_themes;

  handler()->GetChromeThemes(base::BindLambdaForTesting(
      [&](std::vector<customize_themes::mojom::ChromeThemePtr> themes) {
        chrome_themes = std::move(themes);
        run_loop.Quit();
      }));
  run_loop.Run();

  std::vector<chrome_colors::ColorInfo> generated_colors_info(
      std::begin(chrome_colors::kGeneratedColorsInfo),
      std::end(chrome_colors::kGeneratedColorsInfo));
  EXPECT_THAT(chrome_themes,
              Pointwise(MatchesColorInfo(), generated_colors_info));
}

TEST_F(ChromeCustomizeThemesHandlerTest, ObserveThemeChanges) {
  constexpr SkColor kAutogeneratedThemeColor = SK_ColorBLUE;
  EXPECT_CALL(*mock_client(),
              SetTheme(MatchesAutogeneratedTheme(kAutogeneratedThemeColor)));
  theme_service()->BuildAutogeneratedThemeFromColor(kAutogeneratedThemeColor);
}

TEST_F(ChromeCustomizeThemesHandlerTest, InstallThirdPartyTheme) {
  // Prevent "Cached Theme.pak" from being created in the current directory by
  // this test.
  ThemeService::DisableThemePackForTesting();

  // Read and parse an extension theme manifest from a test file.
  base::FilePath test_data_dir;
  ASSERT_TRUE(base::PathService::Get(chrome::DIR_TEST_DATA, &test_data_dir));
  base::FilePath manifest_path =
      test_data_dir.AppendASCII("extensions/theme_minimal/manifest.json");
  std::string config_contents;
  ASSERT_TRUE(base::ReadFileToString(manifest_path, &config_contents));
  std::optional<base::Value::Dict> manifest =
      base::test::ParseJsonDict(config_contents);
  ASSERT_TRUE(manifest.has_value());

  test::ThemeServiceChangedWaiter waiter(theme_service());
  EXPECT_CALL(*mock_client(), SetTheme(MatchesThirdPartyTheme(
                                  kThemeExtensionId, kThemeExtensionName)));
  env()->MakeExtension(manifest.value(), kThemeExtensionId);
  waiter.WaitForThemeChanged();
}

TEST_F(ChromeCustomizeThemesHandlerTest, RevertThemeChanges) {
  constexpr SkColor kAutogeneratedThemeColor = SK_ColorBLUE;
  theme_service()->BuildAutogeneratedThemeFromColor(kAutogeneratedThemeColor);

  handler()->ApplyDefaultTheme();
  EXPECT_TRUE(theme_service()->UsingDefaultTheme());
  // Cleans all pending SetTheme() calls.
  base::RunLoop().RunUntilIdle();

  EXPECT_CALL(*mock_client(),
              SetTheme(MatchesAutogeneratedTheme(kAutogeneratedThemeColor)));
  handler()->RevertThemeChanges();
  EXPECT_TRUE(theme_service()->UsingAutogeneratedTheme());
  EXPECT_EQ(kAutogeneratedThemeColor,
            theme_service()->GetAutogeneratedThemeColor());
}

TEST_F(ChromeCustomizeThemesHandlerTest, ConfirmThemeChanges) {
  constexpr SkColor kAutogeneratedThemeColor = SK_ColorBLUE;
  theme_service()->BuildAutogeneratedThemeFromColor(kAutogeneratedThemeColor);

  handler()->ApplyDefaultTheme();
  EXPECT_TRUE(theme_service()->UsingDefaultTheme());
  // Cleans all pending SetTheme() calls.
  base::RunLoop().RunUntilIdle();

  EXPECT_CALL(*mock_client(), SetTheme(_)).Times(0);
  handler()->ConfirmThemeChanges();
  // Revert should have no effect.
  handler()->RevertThemeChanges();
  EXPECT_TRUE(theme_service()->UsingDefaultTheme());
}

TEST_F(ChromeCustomizeThemesHandlerTest, ResetHandler) {
  constexpr SkColor kAutogeneratedThemeColor = SK_ColorBLUE;
  theme_service()->BuildAutogeneratedThemeFromColor(kAutogeneratedThemeColor);

  handler()->ApplyDefaultTheme();
  EXPECT_TRUE(theme_service()->UsingDefaultTheme());
  // Cleans all pending SetTheme() calls.
  base::RunLoop().RunUntilIdle();

  // Handler should revert theme changes when it's destructed.
  ResetHandler();
  EXPECT_TRUE(theme_service()->UsingAutogeneratedTheme());
  EXPECT_EQ(kAutogeneratedThemeColor,
            theme_service()->GetAutogeneratedThemeColor());
}
