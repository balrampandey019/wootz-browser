// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/ranges/algorithm.h"
#include "base/uuid.h"
#include "chrome/browser/sync/test/integration/saved_tab_groups_helper.h"
#include "chrome/browser/sync/test/integration/sync_service_impl_harness.h"
#include "chrome/browser/sync/test/integration/sync_test.h"
#include "chrome/browser/ui/tabs/saved_tab_groups/saved_tab_group_keyed_service.h"
#include "chrome/browser/ui/tabs/saved_tab_groups/saved_tab_group_service_factory.h"
#include "components/saved_tab_groups/features.h"
#include "components/sync/base/features.h"
#include "components/sync/base/model_type.h"
#include "components/sync/protocol/saved_tab_group_specifics.pb.h"
#include "components/sync/protocol/sync.pb.h"
#include "components/sync/protocol/sync_entity.pb.h"
#include "components/sync/test/fake_server.h"
#include "content/public/test/browser_test.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace tab_groups {
namespace {

sync_pb::SavedTabGroupSpecifics CreateSavedTabGroupSpecific(base::Uuid guid,
                                                            bool ui_v2,
                                                            int position) {
  sync_pb::SavedTabGroupSpecifics pb_specific;
  pb_specific.set_guid(guid.AsLowercaseString());
  pb_specific.set_creation_time_windows_epoch_micros(10);
  pb_specific.set_update_time_windows_epoch_micros(10);
  sync_pb::SavedTabGroup* pb_group = pb_specific.mutable_group();
  pb_group->set_color(sync_pb::SavedTabGroup::SAVED_TAB_GROUP_COLOR_GREY);
  pb_group->set_title("Test");
  if (ui_v2) {
    pb_group->set_pinned_position(position);
  } else {
    pb_group->set_position(position);
  }
  return pb_specific;
}

class SingleClientSavedTabGroupsSyncTest
    : public SyncTest,
      public ::testing::WithParamInterface<bool> {
 public:
  SingleClientSavedTabGroupsSyncTest() : SyncTest(SINGLE_CLIENT) {
    if (IsV2UIEnabled()) {
      features_.InitWithFeatures({tab_groups::kTabGroupsSaveUIUpdate}, {});
    } else {
      features_.InitWithFeatures({}, {tab_groups::kTabGroupsSaveUIUpdate});
    }
  }
  ~SingleClientSavedTabGroupsSyncTest() override = default;
  SingleClientSavedTabGroupsSyncTest(
      const SingleClientSavedTabGroupsSyncTest&) = delete;
  SingleClientSavedTabGroupsSyncTest& operator=(
      const SingleClientSavedTabGroupsSyncTest&) = delete;

  bool IsV2UIEnabled() const { return GetParam(); }

  void AddDataToFakeServer(const sync_pb::SavedTabGroupSpecifics& specifics) {
    sync_pb::EntitySpecifics group_entity_specifics;
    sync_pb::SavedTabGroupSpecifics* group_specifics =
        group_entity_specifics.mutable_saved_tab_group();
    group_specifics->CopyFrom(specifics);

    std::string client_tag = group_specifics->guid();
    int64_t creation_time =
        group_specifics->creation_time_windows_epoch_micros();
    int64_t update_time = group_specifics->update_time_windows_epoch_micros();

    fake_server_->InjectEntity(
        syncer::PersistentUniqueClientEntity::CreateFromSpecificsForTesting(
            "non_unique_name", client_tag, group_entity_specifics,
            /*creation_time=*/creation_time,
            /*last_modified_time=*/update_time));
  }

  void RemoveDataFromFakeServer(base::Uuid uuid) {
    std::vector<sync_pb::SyncEntity> server_tabs_and_groups =
        GetFakeServer()->GetSyncEntitiesByModelType(syncer::SAVED_TAB_GROUP);

    // Remove the entity with a matching `uuid`.
    for (const sync_pb::SyncEntity& tab_or_group : server_tabs_and_groups) {
      const sync_pb::SavedTabGroupSpecifics actual_specifics =
          tab_or_group.specifics().saved_tab_group();
      if (base::Uuid::ParseCaseInsensitive(actual_specifics.guid()) == uuid) {
        // Replace it with a tombstone to remove it from sync.
        GetFakeServer()->InjectEntity(
            syncer::PersistentTombstoneEntity::CreateFromEntity(tab_or_group));
        return;
      }
    }

    NOTREACHED_IN_MIGRATION();
  }

  bool ContainsUuidInFakeServer(base::Uuid uuid) {
    const std::vector<sync_pb::SyncEntity> server_tabs_and_groups =
        GetFakeServer()->GetSyncEntitiesByModelType(syncer::SAVED_TAB_GROUP);

    const std::string& uuid_string = uuid.AsLowercaseString();

    auto it = base::ranges::find_if(
        server_tabs_and_groups,
        [uuid_string](const sync_pb::SyncEntity entity) {
          return entity.specifics().saved_tab_group().guid() == uuid_string;
        });

    return it != server_tabs_and_groups.end();
  }

 private:
  base::test::ScopedFeatureList features_;
};

// Save a group with two tabs and validate they are added to the model.
IN_PROC_BROWSER_TEST_P(SingleClientSavedTabGroupsSyncTest,
                       DownloadsGroupAndTabs) {
  SavedTabGroup group1(u"Group 1", tab_groups::TabGroupColorId::kGrey, {},
                       /*position=*/0);
  SavedTabGroupTab tab1(GURL("about:blank"), u"about:blank",
                        group1.saved_guid(), /*position=*/0);
  SavedTabGroupTab tab2(GURL("about:blank"), u"about:blank",
                        group1.saved_guid(), /*position=*/1);

  // Add a group with two tabs to sync.
  AddDataToFakeServer(*group1.ToSpecifics());
  AddDataToFakeServer(*tab1.ToSpecifics());
  AddDataToFakeServer(*tab2.ToSpecifics());

  ASSERT_TRUE(SetupSync()) << "SetupSync() failed.";
  ASSERT_TRUE(
      GetSyncService(0)->GetActiveDataTypes().Has(syncer::SAVED_TAB_GROUP));

  SavedTabGroupKeyedService* const service =
      SavedTabGroupServiceFactory::GetForProfile(GetProfile(0));

  // Verify they are added to the model.
  EXPECT_TRUE(
      tab_groups::SavedTabOrGroupExistsChecker(service, group1.saved_guid())
          .Wait());

  EXPECT_TRUE(
      tab_groups::SavedTabOrGroupExistsChecker(service, tab1.saved_tab_guid())
          .Wait());

  EXPECT_TRUE(
      tab_groups::SavedTabOrGroupExistsChecker(service, tab2.saved_tab_guid())
          .Wait());
}

// Save a group with no tabs and validate it is added to the model.
IN_PROC_BROWSER_TEST_P(SingleClientSavedTabGroupsSyncTest,
                       DownloadsGroupWithNoTabs) {
  SavedTabGroup group1(u"Group 1", tab_groups::TabGroupColorId::kGrey, {},
                       /*position=*/0);
  SavedTabGroupTab tab1(GURL("about:blank"), u"about:blank",
                        group1.saved_guid(), /*position=*/0);

  // Add a group with no tabs from sync.
  AddDataToFakeServer(*group1.ToSpecifics());

  ASSERT_TRUE(SetupSync()) << "SetupSync() failed.";
  ASSERT_TRUE(
      GetSyncService(0)->GetActiveDataTypes().Has(syncer::SAVED_TAB_GROUP));

  SavedTabGroupKeyedService* const service =
      SavedTabGroupServiceFactory::GetForProfile(GetProfile(0));

  // Verify the group is added to the model but not the tab.
  EXPECT_TRUE(
      tab_groups::SavedTabOrGroupExistsChecker(service, group1.saved_guid())
          .Wait());

  EXPECT_TRUE(service->model()->Contains(group1.saved_guid()));
  EXPECT_TRUE(service->model()->Get(group1.saved_guid())->saved_tabs().empty());
}

// Save a tab with no group and validate it is added to the model.
IN_PROC_BROWSER_TEST_P(SingleClientSavedTabGroupsSyncTest,
                       DownloadsTabWithNoGroup) {
  SavedTabGroup group1(u"Group 1", tab_groups::TabGroupColorId::kGrey, {},
                       /*position=*/0);
  SavedTabGroupTab tab1(GURL("about:blank"), u"about:blank",
                        group1.saved_guid(), /*position=*/0);

  // Add a group with no tabs from sync.
  AddDataToFakeServer(*tab1.ToSpecifics());

  ASSERT_TRUE(SetupSync()) << "SetupSync() failed.";
  ASSERT_TRUE(
      GetSyncService(0)->GetActiveDataTypes().Has(syncer::SAVED_TAB_GROUP));

  SavedTabGroupKeyedService* const service =
      SavedTabGroupServiceFactory::GetForProfile(GetProfile(0));

  // TODO(crbug.com/40912573): Verify that the orphaned tab exists but isn't
  // linked to any group.

  // Verify adding the corresponding group adds the orphaned tab to the model.
  AddDataToFakeServer(*group1.ToSpecifics());

  EXPECT_TRUE(
      tab_groups::SavedTabOrGroupExistsChecker(service, group1.saved_guid())
          .Wait());

  EXPECT_TRUE(
      tab_groups::SavedTabOrGroupExistsChecker(service, tab1.saved_tab_guid())
          .Wait());
}

// Add a tab to an existing group.
IN_PROC_BROWSER_TEST_P(SingleClientSavedTabGroupsSyncTest, AddToExistingGroup) {
  SavedTabGroup group1(u"Group 1", tab_groups::TabGroupColorId::kGrey, {},
                       /*position=*/0);
  SavedTabGroupTab tab1(GURL("about:blank"), u"about:blank",
                        group1.saved_guid(), /*position=*/0);
  SavedTabGroupTab tab2(GURL("about:blank"), u"about:blank",
                        group1.saved_guid(), /*position=*/1);

  // Add a group with two tabs to sync.
  AddDataToFakeServer(*group1.ToSpecifics());
  AddDataToFakeServer(*tab1.ToSpecifics());
  AddDataToFakeServer(*tab2.ToSpecifics());

  ASSERT_TRUE(SetupSync()) << "SetupSync() failed.";
  ASSERT_TRUE(
      GetSyncService(0)->GetActiveDataTypes().Has(syncer::SAVED_TAB_GROUP));

  SavedTabGroupKeyedService* const service =
      SavedTabGroupServiceFactory::GetForProfile(GetProfile(0));

  // Verify they are added to the model.
  ASSERT_TRUE(
      tab_groups::SavedTabOrGroupExistsChecker(service, group1.saved_guid())
          .Wait());
  ASSERT_TRUE(
      tab_groups::SavedTabOrGroupExistsChecker(service, tab1.saved_tab_guid())
          .Wait());
  ASSERT_TRUE(
      tab_groups::SavedTabOrGroupExistsChecker(service, tab2.saved_tab_guid())
          .Wait());

  // Add another tab to `group1`.
  SavedTabGroupTab tab3(GURL("about:blank"), u"about:blank",
                        group1.saved_guid(), /*position=*/2);
  AddDataToFakeServer(*tab3.ToSpecifics());

  // Verify the group is updated with the additional tab.
  EXPECT_TRUE(
      tab_groups::SavedTabOrGroupExistsChecker(service, tab3.saved_tab_guid())
          .Wait());
}

// Remove one tab from a group with two tabs.
IN_PROC_BROWSER_TEST_P(SingleClientSavedTabGroupsSyncTest, RemoveTabFromGroup) {
  SavedTabGroup group1(u"Group 1", tab_groups::TabGroupColorId::kGrey, {},
                       /*position=*/0);
  SavedTabGroupTab tab1(GURL("about:blank"), u"about:blank",
                        group1.saved_guid(), /*position=*/0);
  SavedTabGroupTab tab2(GURL("about:blank"), u"about:blank",
                        group1.saved_guid(), /*position=*/1);

  // Add a group with two tabs to sync.
  AddDataToFakeServer(*group1.ToSpecifics());
  AddDataToFakeServer(*tab1.ToSpecifics());
  AddDataToFakeServer(*tab2.ToSpecifics());

  ASSERT_TRUE(SetupSync()) << "SetupSync() failed.";
  ASSERT_TRUE(
      GetSyncService(0)->GetActiveDataTypes().Has(syncer::SAVED_TAB_GROUP));

  SavedTabGroupKeyedService* const service =
      SavedTabGroupServiceFactory::GetForProfile(GetProfile(0));

  // Verify they are added to the model.
  ASSERT_TRUE(
      tab_groups::SavedTabOrGroupExistsChecker(service, group1.saved_guid())
          .Wait());

  ASSERT_TRUE(
      tab_groups::SavedTabOrGroupExistsChecker(service, tab1.saved_tab_guid())
          .Wait());

  ASSERT_TRUE(
      tab_groups::SavedTabOrGroupExistsChecker(service, tab2.saved_tab_guid())
          .Wait());

  // Remove tab2.
  RemoveDataFromFakeServer(tab2.saved_tab_guid());

  // Verify it was removed from the model.
  EXPECT_TRUE(tab_groups::SavedTabOrGroupDoesNotExistChecker(
                  service, tab2.saved_tab_guid())
                  .Wait());
}

// Remove a saved group from the model.
IN_PROC_BROWSER_TEST_P(SingleClientSavedTabGroupsSyncTest, RemoveGroup) {
  SavedTabGroup group1(u"Group 1", tab_groups::TabGroupColorId::kGrey, {},
                       /*position=*/0);
  SavedTabGroupTab tab1(GURL("about:blank"), u"about:blank",
                        group1.saved_guid(), /*position=*/0);
  SavedTabGroupTab tab2(GURL("about:blank"), u"about:blank",
                        group1.saved_guid(), /*position=*/1);

  // Add a group with two tabs to sync.
  AddDataToFakeServer(*group1.ToSpecifics());
  AddDataToFakeServer(*tab1.ToSpecifics());
  AddDataToFakeServer(*tab2.ToSpecifics());

  ASSERT_TRUE(SetupSync()) << "SetupSync() failed.";
  ASSERT_TRUE(
      GetSyncService(0)->GetActiveDataTypes().Has(syncer::SAVED_TAB_GROUP));

  SavedTabGroupKeyedService* const service =
      SavedTabGroupServiceFactory::GetForProfile(GetProfile(0));

  // Verify they are added to the model.
  ASSERT_TRUE(
      tab_groups::SavedTabOrGroupExistsChecker(service, group1.saved_guid())
          .Wait());
  ASSERT_TRUE(
      tab_groups::SavedTabOrGroupExistsChecker(service, tab1.saved_tab_guid())
          .Wait());
  ASSERT_TRUE(
      tab_groups::SavedTabOrGroupExistsChecker(service, tab2.saved_tab_guid())
          .Wait());

  // Simulate that the group was deleted on another device, which
  // corresponds to deleting the group, but not the tab entities.
  RemoveDataFromFakeServer(group1.saved_guid());

  // Verify the group and its tabs were removed from the model.
  EXPECT_TRUE(tab_groups::SavedTabOrGroupDoesNotExistChecker(
                  service, group1.saved_guid())
                  .Wait());
  EXPECT_TRUE(tab_groups::SavedTabOrGroupDoesNotExistChecker(
                  service, tab1.saved_tab_guid())
                  .Wait());
  EXPECT_TRUE(tab_groups::SavedTabOrGroupDoesNotExistChecker(
                  service, tab2.saved_tab_guid())
                  .Wait());

  // Verify only the tabs are stored in the server.
  EXPECT_FALSE(ContainsUuidInFakeServer(group1.saved_guid()));
  EXPECT_TRUE(ContainsUuidInFakeServer(tab1.saved_tab_guid()));
  EXPECT_TRUE(ContainsUuidInFakeServer(tab2.saved_tab_guid()));
}

// Update the metadata of a saved group already in the model.
IN_PROC_BROWSER_TEST_P(SingleClientSavedTabGroupsSyncTest,
                       UpdateGroupMetadata) {
  SavedTabGroup group1(u"Group 1", tab_groups::TabGroupColorId::kGrey,
                       /*urls=*/{},
                       /*position=*/0);

  // Add a group with two tabs to sync.
  AddDataToFakeServer(*group1.ToSpecifics());

  ASSERT_TRUE(SetupSync()) << "SetupSync() failed.";
  ASSERT_TRUE(
      GetSyncService(0)->GetActiveDataTypes().Has(syncer::SAVED_TAB_GROUP));

  SavedTabGroupKeyedService* const service =
      SavedTabGroupServiceFactory::GetForProfile(GetProfile(0));

  // Verify they are added to the model.
  ASSERT_TRUE(
      tab_groups::SavedTabOrGroupExistsChecker(service, group1.saved_guid())
          .Wait());

  // Update metadata for group1in the server.
  group1.SetTitle(u"Updated Title");
  group1.SetColor(tab_groups::TabGroupColorId::kOrange);
  AddDataToFakeServer(*group1.ToSpecifics());

  // Verify the group's metadata is updated locally.
  EXPECT_TRUE(tab_groups::SavedTabGroupMatchesChecker(service, group1).Wait());
}

// Update the URL and title of a saved tab already in the model.
IN_PROC_BROWSER_TEST_P(SingleClientSavedTabGroupsSyncTest, UpdatedTabData) {
  SavedTabGroup group1(u"Group 1", tab_groups::TabGroupColorId::kGrey,
                       /*urls=*/{},
                       /*position=*/0);
  SavedTabGroupTab tab1(GURL("about:blank"), u"about:blank",
                        group1.saved_guid(), /*position=*/0);

  // Add a group with a tab to sync.
  AddDataToFakeServer(*group1.ToSpecifics());
  AddDataToFakeServer(*tab1.ToSpecifics());

  ASSERT_TRUE(SetupSync()) << "SetupSync() failed.";
  ASSERT_TRUE(
      GetSyncService(0)->GetActiveDataTypes().Has(syncer::SAVED_TAB_GROUP));

  SavedTabGroupKeyedService* const service =
      SavedTabGroupServiceFactory::GetForProfile(GetProfile(0));

  // Verify they are added to the model.
  ASSERT_TRUE(
      tab_groups::SavedTabOrGroupExistsChecker(service, group1.saved_guid())
          .Wait());
  ASSERT_TRUE(
      tab_groups::SavedTabOrGroupExistsChecker(service, tab1.saved_tab_guid())
          .Wait());

  // Update url and title for tab1 in the server.
  tab1.SetURL(GURL("https://new.url"));
  tab1.SetTitle(u"Updated Title");
  AddDataToFakeServer(*tab1.ToSpecifics());

  // Verify the tab is updated locally to match.
  EXPECT_TRUE(tab_groups::SavedTabMatchesChecker(service, tab1).Wait());
}

// Reorder groups already saved in the model.
IN_PROC_BROWSER_TEST_P(SingleClientSavedTabGroupsSyncTest, ReorderGroups) {
  SavedTabGroup group1(u"Group 1", tab_groups::TabGroupColorId::kGrey,
                       /*urls=*/{},
                       /*position=*/0);
  SavedTabGroup group2(u"Group 2", tab_groups::TabGroupColorId::kOrange,
                       /*urls=*/{},
                       /*position=*/1);

  // Add a group with a tab to sync.
  AddDataToFakeServer(*group1.ToSpecifics());
  AddDataToFakeServer(*group2.ToSpecifics());

  ASSERT_TRUE(SetupSync()) << "SetupSync() failed.";
  ASSERT_TRUE(
      GetSyncService(0)->GetActiveDataTypes().Has(syncer::SAVED_TAB_GROUP));

  SavedTabGroupKeyedService* const service =
      SavedTabGroupServiceFactory::GetForProfile(GetProfile(0));

  // Verify they are added to the model.
  ASSERT_TRUE(
      tab_groups::SavedTabOrGroupExistsChecker(service, group1.saved_guid())
          .Wait());
  ASSERT_TRUE(
      tab_groups::SavedTabOrGroupExistsChecker(service, group2.saved_guid())
          .Wait());

  // Update the positions of the groups in the server.
  group1.SetPosition(1);
  group2.SetPosition(0);
  AddDataToFakeServer(*group1.ToSpecifics());
  AddDataToFakeServer(*group2.ToSpecifics());

  // Verify the group positions are updated in the local model as well.
  EXPECT_TRUE(tab_groups::GroupOrderChecker(
                  service, {group2.saved_guid(), group1.saved_guid()})
                  .Wait());
}

// Reorder tabs in a group.
IN_PROC_BROWSER_TEST_P(SingleClientSavedTabGroupsSyncTest, ReorderTabs) {
  SavedTabGroup group1(u"Group 1", tab_groups::TabGroupColorId::kGrey, {}, 0);
  SavedTabGroupTab tab1(GURL("about:blank"), u"about:blank",
                        group1.saved_guid(), /*position=*/0);
  SavedTabGroupTab tab2(GURL("about:blank"), u"about:blank",
                        group1.saved_guid(), /*position=*/1);

  // Add a group with two tabs to sync.
  AddDataToFakeServer(*group1.ToSpecifics());
  AddDataToFakeServer(*tab1.ToSpecifics());
  AddDataToFakeServer(*tab2.ToSpecifics());

  ASSERT_TRUE(SetupSync()) << "SetupSync() failed.";
  ASSERT_TRUE(
      GetSyncService(0)->GetActiveDataTypes().Has(syncer::SAVED_TAB_GROUP));

  SavedTabGroupKeyedService* const service =
      SavedTabGroupServiceFactory::GetForProfile(GetProfile(0));

  // Verify they are added to the model.
  ASSERT_TRUE(
      tab_groups::SavedTabOrGroupExistsChecker(service, group1.saved_guid())
          .Wait());
  ASSERT_TRUE(
      tab_groups::SavedTabOrGroupExistsChecker(service, tab1.saved_tab_guid())
          .Wait());
  ASSERT_TRUE(
      tab_groups::SavedTabOrGroupExistsChecker(service, tab2.saved_tab_guid())
          .Wait());

  // Reorder the tabs in group 1 on the server.
  tab1.SetPosition(1);
  tab2.SetPosition(0);
  AddDataToFakeServer(*tab1.ToSpecifics());
  AddDataToFakeServer(*tab2.ToSpecifics());

  // Verify the tab order was updated in the model.
  EXPECT_TRUE(tab_groups::TabOrderChecker(
                  service, group1.saved_guid(),
                  {tab2.saved_tab_guid(), tab1.saved_tab_guid()})
                  .Wait());
}

IN_PROC_BROWSER_TEST_P(SingleClientSavedTabGroupsSyncTest,
                       V1BrowserWithV2Proto) {
  if (IsV2UIEnabled()) {
    GTEST_SKIP() << "N/A for V2";
  }

  auto guid1 = base::Uuid::GenerateRandomV4();
  AddDataToFakeServer(CreateSavedTabGroupSpecific(guid1, /*ui_v2=*/true, 0));

  ASSERT_TRUE(SetupSync()) << "SetupSync() failed.";
  SavedTabGroupKeyedService* const service =
      SavedTabGroupServiceFactory::GetForProfile(GetProfile(0));

  // Verify guid1 is added to the model.
  ASSERT_TRUE(tab_groups::SavedTabOrGroupExistsChecker(service, guid1).Wait());

  // Verify guid1 has position even the position in the proto is
  // not set.
  EXPECT_EQ(0, service->model()->Get(guid1)->position());

  auto guid2 = base::Uuid::GenerateRandomV4();
  AddDataToFakeServer(CreateSavedTabGroupSpecific(guid2, /*ui_v2=*/true, 1));

  // Verify guid2 is added to the model.
  ASSERT_TRUE(tab_groups::SavedTabOrGroupExistsChecker(service, guid2).Wait());

  // Verify guid2 has position even the position in the proto is
  // not set.
  EXPECT_EQ(0, service->model()->Get(guid2)->position());
}

IN_PROC_BROWSER_TEST_P(SingleClientSavedTabGroupsSyncTest,
                       V2BrowserWithV1Proto) {
  if (!IsV2UIEnabled()) {
    GTEST_SKIP() << "N/A for V1";
  }

  auto guid1 = base::Uuid::GenerateRandomV4();
  AddDataToFakeServer(CreateSavedTabGroupSpecific(guid1, /*ui_v2=*/false, 0));

  ASSERT_TRUE(SetupSync()) << "SetupSync() failed.";
  SavedTabGroupKeyedService* const service =
      SavedTabGroupServiceFactory::GetForProfile(GetProfile(0));

  // Verify guid1 is added to the model.
  ASSERT_TRUE(tab_groups::SavedTabOrGroupExistsChecker(service, guid1).Wait());

  // Verify guid1 has no position set even the position in the proto is set.
  EXPECT_EQ(std::nullopt, service->model()->Get(guid1)->position());

  auto guid2 = base::Uuid::GenerateRandomV4();
  AddDataToFakeServer(CreateSavedTabGroupSpecific(guid2, /*ui_v2=*/false, 1));

  // Verify guid2 is added to the model.
  ASSERT_TRUE(tab_groups::SavedTabOrGroupExistsChecker(service, guid2).Wait());

  // Verify guid2 has no position set even the position in the proto is set.
  EXPECT_EQ(std::nullopt, service->model()->Get(guid2)->position());
}

INSTANTIATE_TEST_SUITE_P(SavedTabGroup,
                         SingleClientSavedTabGroupsSyncTest,
                         testing::Bool());

}  // namespace
}  // namespace tab_groups
