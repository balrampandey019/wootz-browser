// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.tasks.tab_management;

import static org.mockito.Mockito.spy;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import androidx.test.filters.SmallTest;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TestRule;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;
import org.robolectric.RuntimeEnvironment;
import org.robolectric.annotation.Config;

import org.chromium.base.test.BaseRobolectricTestRunner;
import org.chromium.base.test.util.CallbackHelper;
import org.chromium.base.test.util.Features;
import org.chromium.chrome.browser.profiles.Profile;
import org.chromium.chrome.browser.tab.Tab;
import org.chromium.chrome.browser.tasks.tab_groups.TabGroupModelFilter;
import org.chromium.chrome.browser.tasks.tab_management.TabListEditorAction.ActionDelegate;
import org.chromium.chrome.browser.tasks.tab_management.TabListEditorAction.ActionObserver;
import org.chromium.chrome.browser.tasks.tab_management.TabListEditorAction.ButtonType;
import org.chromium.chrome.browser.tasks.tab_management.TabListEditorAction.IconPosition;
import org.chromium.chrome.browser.tasks.tab_management.TabListEditorAction.ShowMode;
import org.chromium.chrome.browser.tasks.tab_management.TabListEditorActionUnitTestHelper.TabIdGroup;
import org.chromium.chrome.browser.tasks.tab_management.TabListEditorActionUnitTestHelper.TabListHolder;
import org.chromium.chrome.tab_ui.R;
import org.chromium.chrome.test.util.browser.tabmodel.MockTabModel;
import org.chromium.components.browser_ui.widget.selectable_list.SelectionDelegate;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Set;

/** Unit tests for {@link TabListEditorCloseAction}. */
@RunWith(BaseRobolectricTestRunner.class)
@Config(manifest = Config.NONE)
public class TabListEditorCloseActionUnitTest {
    @Rule public TestRule mProcessor = new Features.JUnitProcessor();

    @Mock private TabGroupModelFilter mGroupFilter;
    @Mock private SelectionDelegate<Integer> mSelectionDelegate;
    @Mock private ActionDelegate mDelegate;
    @Mock private Profile mProfile;
    private MockTabModel mTabModel;
    private TabListEditorAction mAction;

    @Before
    public void setUp() {
        MockitoAnnotations.initMocks(this);
        mAction =
                TabListEditorCloseAction.createAction(
                        RuntimeEnvironment.application,
                        ShowMode.MENU_ONLY,
                        ButtonType.TEXT,
                        IconPosition.START);
        mTabModel = spy(new MockTabModel(mProfile, null));
        when(mGroupFilter.getTabModel()).thenReturn(mTabModel);
    }

    private void configure(boolean actionOnRelatedTabs) {
        mAction.configure(() -> mGroupFilter, mSelectionDelegate, mDelegate, actionOnRelatedTabs);
    }

    @Test
    @SmallTest
    public void testInherentActionProperties() {
        Assert.assertEquals(
                R.id.tab_list_editor_close_menu_item,
                mAction.getPropertyModel().get(TabListEditorActionProperties.MENU_ITEM_ID));
        Assert.assertEquals(
                R.plurals.tab_selection_editor_close_tabs,
                mAction.getPropertyModel()
                        .get(TabListEditorActionProperties.TITLE_RESOURCE_ID));
        Assert.assertEquals(
                true,
                mAction.getPropertyModel().get(TabListEditorActionProperties.TITLE_IS_PLURAL));
        Assert.assertEquals(
                R.plurals.accessibility_tab_selection_editor_close_tabs,
                mAction.getPropertyModel()
                        .get(TabListEditorActionProperties.CONTENT_DESCRIPTION_RESOURCE_ID)
                        .intValue());
        Assert.assertNotNull(
                mAction.getPropertyModel().get(TabListEditorActionProperties.ICON));
    }

    @Test
    @SmallTest
    public void testCloseActionNoTabs() {
        configure(false);
        mAction.onSelectionStateChange(new ArrayList<Integer>());
        Assert.assertEquals(
                false, mAction.getPropertyModel().get(TabListEditorActionProperties.ENABLED));
        Assert.assertEquals(
                0, mAction.getPropertyModel().get(TabListEditorActionProperties.ITEM_COUNT));
    }

    @Test
    @SmallTest
    public void testCloseActionWithOneTab() throws Exception {
        configure(false);
        List<Integer> tabIds = new ArrayList<>();
        tabIds.add(5);
        tabIds.add(3);
        tabIds.add(7);
        List<Tab> tabs = new ArrayList<>();
        for (int id : tabIds) {
            tabs.add(mTabModel.addTab(id));
        }
        Set<Integer> tabIdsSet = new LinkedHashSet<>();
        tabIdsSet.add(3);
        when(mSelectionDelegate.getSelectedItems()).thenReturn(tabIdsSet);

        mAction.onSelectionStateChange(Arrays.asList(3));
        Assert.assertEquals(
                true, mAction.getPropertyModel().get(TabListEditorActionProperties.ENABLED));
        Assert.assertEquals(
                1, mAction.getPropertyModel().get(TabListEditorActionProperties.ITEM_COUNT));

        Assert.assertTrue(mAction.perform());
        verify(mGroupFilter)
                .closeMultipleTabs(
                        List.of(tabs.get(1)), /* canUndo= */ true, /* hideTabGroups= */ false);
        verify(mDelegate).hideByAction();
    }

    @Test
    @SmallTest
    public void testCloseActionWithTabs() throws Exception {
        configure(false);
        List<Integer> tabIds = new ArrayList<>();
        tabIds.add(5);
        tabIds.add(3);
        tabIds.add(7);
        List<Tab> tabs = new ArrayList<>();
        for (int id : tabIds) {
            tabs.add(mTabModel.addTab(id));
        }
        Set<Integer> tabIdsSet = new LinkedHashSet<>(tabIds);
        when(mSelectionDelegate.getSelectedItems()).thenReturn(tabIdsSet);

        mAction.onSelectionStateChange(tabIds);
        Assert.assertEquals(
                true, mAction.getPropertyModel().get(TabListEditorActionProperties.ENABLED));
        Assert.assertEquals(
                3, mAction.getPropertyModel().get(TabListEditorActionProperties.ITEM_COUNT));

        final CallbackHelper helper = new CallbackHelper();
        ActionObserver observer =
                new ActionObserver() {
                    @Override
                    public void preProcessSelectedTabs(List<Tab> tabs) {
                        helper.notifyCalled();
                    }
                };
        mAction.addActionObserver(observer);

        Assert.assertTrue(mAction.perform());
        verify(mGroupFilter)
                .closeMultipleTabs(tabs, /* canUndo= */ true, /* hideTabGroups= */ false);
        verify(mDelegate).hideByAction();

        helper.waitForFirst();
        mAction.removeActionObserver(observer);

        Assert.assertTrue(mAction.perform());
        verify(mGroupFilter, times(2))
                .closeMultipleTabs(tabs, /* canUndo= */ true, /* hideTabGroups= */ false);
        verify(mDelegate, times(2)).hideByAction();
        Assert.assertEquals(1, helper.getCallCount());
    }

    @Test
    @SmallTest
    public void testCloseActionWithTabGroups_ActionOnRelatedTabs() throws Exception {
        final boolean actionOnRelatedTabs = true;
        configure(actionOnRelatedTabs);
        List<TabIdGroup> tabIdGroups = new ArrayList<>();
        tabIdGroups.add(new TabIdGroup(new int[] {0}, false));
        tabIdGroups.add(new TabIdGroup(new int[] {5, 3}, true));
        tabIdGroups.add(new TabIdGroup(new int[] {4}, false));
        tabIdGroups.add(new TabIdGroup(new int[] {8, 7, 6}, true));
        tabIdGroups.add(new TabIdGroup(new int[] {1}, true));
        TabListHolder holder =
                TabListEditorActionUnitTestHelper.configureTabs(
                        mTabModel, mGroupFilter, mSelectionDelegate, tabIdGroups, true);

        Assert.assertEquals(3, holder.getSelectedTabs().size());
        Assert.assertEquals(5, holder.getSelectedTabs().get(0).getId());
        Assert.assertEquals(8, holder.getSelectedTabs().get(1).getId());
        Assert.assertEquals(1, holder.getSelectedTabs().get(2).getId());
        mAction.onSelectionStateChange(holder.getSelectedTabIds());
        Assert.assertEquals(
                true, mAction.getPropertyModel().get(TabListEditorActionProperties.ENABLED));
        Assert.assertEquals(
                6, mAction.getPropertyModel().get(TabListEditorActionProperties.ITEM_COUNT));

        Assert.assertEquals(6, holder.getSelectedAndRelatedTabs().size());
        Assert.assertEquals(5, holder.getSelectedAndRelatedTabs().get(0).getId());
        Assert.assertEquals(3, holder.getSelectedAndRelatedTabs().get(1).getId());
        Assert.assertEquals(8, holder.getSelectedAndRelatedTabs().get(2).getId());
        Assert.assertEquals(7, holder.getSelectedAndRelatedTabs().get(3).getId());
        Assert.assertEquals(6, holder.getSelectedAndRelatedTabs().get(4).getId());
        Assert.assertEquals(1, holder.getSelectedAndRelatedTabs().get(5).getId());
        Assert.assertTrue(mAction.perform());
        verify(mGroupFilter)
                .closeMultipleTabs(
                        holder.getSelectedAndRelatedTabs(),
                        /* canUndo= */ true,
                        /* hideTabGroups= */ true);
        verify(mDelegate).hideByAction();
    }

    @Test
    @SmallTest
    public void testCloseActionWithTabGroups_NoActionOnRelatedTabs() throws Exception {
        final boolean actionOnRelatedTabs = false;
        configure(actionOnRelatedTabs);
        List<TabIdGroup> tabIdGroups = new ArrayList<>();
        tabIdGroups.add(new TabIdGroup(new int[] {0}, false));
        tabIdGroups.add(new TabIdGroup(new int[] {5, 3}, true));
        tabIdGroups.add(new TabIdGroup(new int[] {4}, false));
        tabIdGroups.add(new TabIdGroup(new int[] {8, 7, 6}, true));
        tabIdGroups.add(new TabIdGroup(new int[] {1}, true));
        TabListHolder holder =
                TabListEditorActionUnitTestHelper.configureTabs(
                        mTabModel, mGroupFilter, mSelectionDelegate, tabIdGroups, true);

        Assert.assertEquals(3, holder.getSelectedTabs().size());
        Assert.assertEquals(5, holder.getSelectedTabs().get(0).getId());
        Assert.assertEquals(8, holder.getSelectedTabs().get(1).getId());
        Assert.assertEquals(1, holder.getSelectedTabs().get(2).getId());
        mAction.onSelectionStateChange(holder.getSelectedTabIds());
        Assert.assertEquals(
                true, mAction.getPropertyModel().get(TabListEditorActionProperties.ENABLED));
        Assert.assertEquals(
                3, mAction.getPropertyModel().get(TabListEditorActionProperties.ITEM_COUNT));

        Assert.assertTrue(mAction.perform());
        verify(mGroupFilter)
                .closeMultipleTabs(
                        holder.getSelectedTabs(), /* canUndo= */ true, /* hideTabGroups= */ false);
        verify(mDelegate).hideByAction();
    }
}
