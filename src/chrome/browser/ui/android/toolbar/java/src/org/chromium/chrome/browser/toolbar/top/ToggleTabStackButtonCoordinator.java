// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.toolbar.top;

import android.content.Context;

import androidx.annotation.VisibleForTesting;

import org.chromium.base.Callback;
import org.chromium.base.CallbackController;
import org.chromium.base.supplier.ObservableSupplier;
import org.chromium.base.supplier.OneshotSupplier;
import org.chromium.chrome.browser.layouts.LayoutStateProvider;
import org.chromium.chrome.browser.layouts.LayoutType;
import org.chromium.chrome.browser.tab.CurrentTabObserver;
import org.chromium.chrome.browser.tab.EmptyTabObserver;
import org.chromium.chrome.browser.tab.Tab;
import org.chromium.chrome.browser.toolbar.R;
import org.chromium.chrome.browser.user_education.IPHCommandBuilder;
import org.chromium.chrome.browser.user_education.UserEducationHelper;
import org.chromium.components.browser_ui.widget.highlight.ViewHighlighter.HighlightParams;
import org.chromium.components.browser_ui.widget.highlight.ViewHighlighter.HighlightShape;
import org.chromium.components.feature_engagement.FeatureConstants;
import org.chromium.url.GURL;

import java.util.function.BooleanSupplier;

/**
 * Root component for the tab switcher button on the toolbar. Intended to own the {@link
 * ToggleTabStackButton}, but currently it only manages some signals around the tab switcher button.
 * TODO(crbug.com/40588354): Finish converting HomeButton to MVC and move more logic into this
 * class.
 */
public class ToggleTabStackButtonCoordinator {
    private final CallbackController mCallbackController = new CallbackController();
    private final Context mContext;
    private final ToggleTabStackButton mToggleTabStackButton;
    private final UserEducationHelper mUserEducationHelper;
    private final BooleanSupplier mIsIncognitoSupplier;
    private final OneshotSupplier<Boolean> mPromoShownOneshotSupplier;
    private final Callback<Boolean> mSetNewTabButtonHighlightCallback;
    private final CurrentTabObserver mPageLoadObserver;

    private LayoutStateProvider mLayoutStateProvider;
    private LayoutStateProvider.LayoutStateObserver mLayoutStateObserver;
    @VisibleForTesting boolean mIphBeingShown;

    /**
     * @param context The Android context used for various view operations.
     * @param toggleTabStackButton The concrete {@link ToggleTabStackButton} class for this MVC
     *         component.
     * @param userEducationHelper Helper class for showing in-product help text bubbles.
     * @param isIncognitoSupplier Supplier for whether the current tab is incognito.
     * @param promoShownOneshotSupplier Potentially delayed information about if a promo was shown.
     * @param layoutStateProviderSupplier Allows observing layout state.
     * @param setNewTabButtonHighlightCallback Delegate to highlight the new tab button.
     * @param activityTabSupplier Supplier of the activity tab.
     */
    public ToggleTabStackButtonCoordinator(
            Context context,
            ToggleTabStackButton toggleTabStackButton,
            UserEducationHelper userEducationHelper,
            BooleanSupplier isIncognitoSupplier,
            OneshotSupplier<Boolean> promoShownOneshotSupplier,
            OneshotSupplier<LayoutStateProvider> layoutStateProviderSupplier,
            Callback<Boolean> setNewTabButtonHighlightCallback,
            ObservableSupplier<Tab> activityTabSupplier) {
        mContext = context;
        mToggleTabStackButton = toggleTabStackButton;
        mUserEducationHelper = userEducationHelper;
        mIsIncognitoSupplier = isIncognitoSupplier;
        mPromoShownOneshotSupplier = promoShownOneshotSupplier;
        mSetNewTabButtonHighlightCallback = setNewTabButtonHighlightCallback;

        layoutStateProviderSupplier.onAvailable(
                mCallbackController.makeCancelable(this::setLayoutStateProvider));
        mPageLoadObserver =
                new CurrentTabObserver(
                        activityTabSupplier,
                        new EmptyTabObserver() {
                            @Override
                            public void onPageLoadFinished(Tab tab, GURL url) {
                                handlePageLoadFinished();
                            }
                        },
                        /* swapCallback= */ null);
    }

    /** Cleans up callbacks and observers. */
    public void destroy() {
        mCallbackController.destroy();

        mPageLoadObserver.destroy();

        if (mLayoutStateProvider != null) {
            mLayoutStateProvider.removeObserver(mLayoutStateObserver);
            mLayoutStateProvider = null;
            mLayoutStateObserver = null;
        }
    }

    private void setLayoutStateProvider(LayoutStateProvider layoutStateProvider) {
        assert layoutStateProvider != null;
        assert mLayoutStateProvider == null : "the mLayoutStateProvider should set at most once.";

        mLayoutStateProvider = layoutStateProvider;
        mLayoutStateObserver =
                new LayoutStateProvider.LayoutStateObserver() {
                    private boolean mHighlightedNewTabPageButton;

                    @Override
                    public void onStartedShowing(@LayoutType int layoutType) {
                        if (layoutType == LayoutType.TAB_SWITCHER && mIphBeingShown) {
                            mSetNewTabButtonHighlightCallback.onResult(true);
                            mHighlightedNewTabPageButton = true;
                        }
                    }

                    @Override
                    public void onStartedHiding(@LayoutType int layoutType) {
                        if (layoutType == LayoutType.TAB_SWITCHER && mHighlightedNewTabPageButton) {
                            mSetNewTabButtonHighlightCallback.onResult(false);
                            mHighlightedNewTabPageButton = false;
                        }
                    }
                };
        mLayoutStateProvider.addObserver(mLayoutStateObserver);
    }

    @VisibleForTesting
    void handlePageLoadFinished() {
        if (mToggleTabStackButton == null || !mToggleTabStackButton.isShown()) return;
        if (mIsIncognitoSupplier.getAsBoolean()) return;
        if (mPromoShownOneshotSupplier.get() == null || mPromoShownOneshotSupplier.get()) return;

        HighlightParams params = new HighlightParams(HighlightShape.CIRCLE);
        params.setBoundsRespectPadding(true);
        mUserEducationHelper.requestShowIPH(
                new IPHCommandBuilder(
                                mContext.getResources(),
                                FeatureConstants.TAB_SWITCHER_BUTTON_FEATURE,
                                R.string.iph_tab_switcher_text,
                                R.string.iph_tab_switcher_accessibility_text)
                        .setAnchorView(mToggleTabStackButton)
                        .setOnShowCallback(this::handleShowCallback)
                        .setOnDismissCallback(this::handleDismissCallback)
                        .setHighlightParams(params)
                        .build());
    }

    private void handleShowCallback() {
        assert mToggleTabStackButton != null;
        mIphBeingShown = true;
    }

    private void handleDismissCallback() {
        assert mToggleTabStackButton != null;
        mIphBeingShown = false;
    }
}
