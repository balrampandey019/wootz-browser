// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/quick_answers/ui/quick_answers_view.h"

#include <memory>

#include "chrome/browser/ui/quick_answers/quick_answers_controller_impl.h"
#include "chrome/browser/ui/quick_answers/quick_answers_ui_controller.h"
#include "chrome/browser/ui/quick_answers/test/chrome_quick_answers_test_base.h"
#include "ui/views/controls/menu/menu_controller.h"

namespace {

constexpr int kMarginDip = 10;
constexpr int kSmallTop = 30;
constexpr gfx::Rect kDefaultAnchorBoundsInScreen =
    gfx::Rect(gfx::Point(500, 250), gfx::Size(80, 140));

}  // namespace

class QuickAnswersViewsTest : public ChromeQuickAnswersTestBase {
 protected:
  QuickAnswersViewsTest() = default;
  QuickAnswersViewsTest(const QuickAnswersViewsTest&) = delete;
  QuickAnswersViewsTest& operator=(const QuickAnswersViewsTest&) = delete;
  ~QuickAnswersViewsTest() override = default;

  // ChromeQuickAnswersTestBase:
  void SetUp() override {
    ChromeQuickAnswersTestBase::SetUp();

    anchor_bounds_ = kDefaultAnchorBoundsInScreen;
    GetUiController()->GetReadWriteCardsUiController().SetContextMenuBounds(
        anchor_bounds_);
  }

  void TearDown() override { ChromeQuickAnswersTestBase::TearDown(); }

  // Currently instantiated QuickAnswersView instance.
  views::View* GetQuickAnswersView() {
    return GetUiController()->quick_answers_view();
  }

  // Needed to poll the current bounds of the mock anchor.
  const gfx::Rect& GetAnchorBounds() { return anchor_bounds_; }

  QuickAnswersUiController* GetUiController() {
    return static_cast<QuickAnswersControllerImpl*>(
               QuickAnswersController::Get())
        ->quick_answers_ui_controller();
  }

  // Create a QuickAnswersView instance with custom anchor-bounds.
  void CreateQuickAnswersView(const gfx::Rect anchor_bounds) {
    // Set up a companion menu before creating the QuickAnswersView.
    CreateAndShowBasicMenu();

    anchor_bounds_ = anchor_bounds;
    GetUiController()->GetReadWriteCardsUiController().SetContextMenuBounds(
        anchor_bounds_);

    // TODO(b/222422130): Rewrite QuickAnswersViewsTest to expand coverage.
    GetUiController()->CreateQuickAnswersView(GetProfile(), "title", "query",
                                              /*is_internal=*/false);
  }

 private:
  chromeos::ReadWriteCardsUiController controller_;
  gfx::Rect anchor_bounds_;
};

TEST_F(QuickAnswersViewsTest, DefaultLayoutAroundAnchor) {
  gfx::Rect anchor_bounds = GetAnchorBounds();
  CreateQuickAnswersView(anchor_bounds);
  gfx::Rect view_bounds = GetQuickAnswersView()->GetBoundsInScreen();

  // Vertically aligned with anchor.
  EXPECT_EQ(view_bounds.x(), anchor_bounds.x());
  EXPECT_EQ(view_bounds.right(), anchor_bounds.right());

  // View is positioned above the anchor.
  EXPECT_EQ(view_bounds.bottom() + kMarginDip, anchor_bounds.y());
}

TEST_F(QuickAnswersViewsTest, PositionedBelowAnchorIfLessSpaceAbove) {
  gfx::Rect anchor_bounds = GetAnchorBounds();
  // Update anchor-bounds' position so that it does not leave enough vertical
  // space above it to show the QuickAnswersView.
  anchor_bounds.set_y(kSmallTop);

  CreateQuickAnswersView(anchor_bounds);
  gfx::Rect view_bounds = GetQuickAnswersView()->GetBoundsInScreen();

  // Anchor is positioned above the view.
  EXPECT_EQ(anchor_bounds.bottom() + kMarginDip, view_bounds.y());
}

TEST_F(QuickAnswersViewsTest, FocusProperties) {
  CreateQuickAnswersView(GetAnchorBounds());
  CHECK(views::MenuController::GetActiveInstance() &&
        views::MenuController::GetActiveInstance()->owner());

  // Gains focus only upon request, if an owned menu was active when the view
  // was created.
  CHECK(views::MenuController::GetActiveInstance() != nullptr);
  EXPECT_FALSE(GetQuickAnswersView()->HasFocus());
  GetQuickAnswersView()->RequestFocus();
  EXPECT_TRUE(GetQuickAnswersView()->HasFocus());
}
