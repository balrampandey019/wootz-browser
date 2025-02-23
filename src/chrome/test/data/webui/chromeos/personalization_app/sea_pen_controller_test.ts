// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import {beginLoadRecentSeaPenImagesAction, beginLoadSelectedImageAction, beginLoadSelectedRecentSeaPenImageAction, beginSearchSeaPenThumbnailsAction, beginSelectRecentSeaPenImageAction, beginSelectSeaPenThumbnailAction, endSelectRecentSeaPenImageAction, endSelectSeaPenThumbnailAction, getRecentSeaPenImages, getSeaPenStore, SeaPenState, SeaPenStoreAdapter, SeaPenStoreInterface, searchSeaPenThumbnails, selectRecentSeaPenImage, selectSeaPenWallpaper, setCurrentSeaPenQueryAction, setRecentSeaPenImagesAction, setSeaPenThumbnailsAction, setSelectedRecentSeaPenImageAction, setThumbnailResponseStatusCodeAction, WallpaperLayout, WallpaperType} from 'chrome://personalization/js/personalization_app.js';
import {MantaStatusCode} from 'chrome://resources/ash/common/sea_pen/sea_pen.mojom-webui.js';
import {loadTimeData} from 'chrome://resources/js/load_time_data.js';
import {assertDeepEquals, assertEquals, assertFalse, assertTrue} from 'chrome://webui-test/chai_assert.js';

import {filterAndFlattenState, typeCheck} from './personalization_app_test_utils.js';
import {TestPersonalizationStore} from './test_personalization_store.js';
import {TestSeaPenProvider} from './test_sea_pen_interface_provider.js';

suite('SeaPen reducers', () => {
  let seaPenProvider: TestSeaPenProvider;
  let personalizationStore: TestPersonalizationStore;
  let seaPenStore: SeaPenStoreInterface;

  setup(() => {
    loadTimeData.overrideValues({isSeaPenEnabled: true});
    seaPenProvider = new TestSeaPenProvider();
    personalizationStore = new TestPersonalizationStore({});
    personalizationStore.setReducersEnabled(true);
    personalizationStore.replaceSingleton();
    SeaPenStoreAdapter.initSeaPenStore();
    seaPenStore = getSeaPenStore();
  });

  test('sets recent sea pen images in store', async () => {
    await getRecentSeaPenImages(seaPenProvider, seaPenStore);

    assertDeepEquals(
        [
          beginLoadRecentSeaPenImagesAction(),
          setRecentSeaPenImagesAction(seaPenProvider.recentImageIds),
        ],
        personalizationStore.actions, 'recent images action');

    assertDeepEquals(
        seaPenProvider.recentImageIds,
        personalizationStore.data.wallpaper.seaPen.recentImages,
        'recent images set in store');
  });

  test('sets sea pen thumbnails in store', async () => {
    const query = {textQuery: 'test_query'};
    await searchSeaPenThumbnails(query, seaPenProvider, seaPenStore);
    assertDeepEquals(
        [
          beginSearchSeaPenThumbnailsAction(query),
          setCurrentSeaPenQueryAction(query),
          setThumbnailResponseStatusCodeAction(MantaStatusCode.kOk),
          setSeaPenThumbnailsAction(query, seaPenProvider.images),
        ],
        personalizationStore.actions, 'expected actions match');

    assertDeepEquals(
        [
          {
            'wallpaper.seaPen': typeCheck<SeaPenState>({
              loading: {
                recentImageData: {},
                recentImages: false,
                thumbnails: true,
                currentSelected: false,
                setImage: 0,
              },
              recentImageData: {},
              recentImages: null,
              thumbnailResponseStatusCode: null,
              thumbnails: null,
              currentSeaPenQuery: null,
              pendingSelected: null,
              currentSelected: null,
              shouldShowSeaPenIntroductionDialog: false,
              error: null,
            }),
          },
          {
            'wallpaper.seaPen': typeCheck<SeaPenState>({
              loading: {
                recentImageData: {},
                recentImages: false,
                thumbnails: true,
                currentSelected: false,
                setImage: 0,
              },
              recentImageData: {},
              recentImages: null,
              thumbnailResponseStatusCode: null,
              thumbnails: null,
              currentSeaPenQuery: query,
              pendingSelected: null,
              currentSelected: null,
              shouldShowSeaPenIntroductionDialog: false,
              error: null,
            }),
          },
          {
            'wallpaper.seaPen': typeCheck<SeaPenState>({
              loading: {
                recentImageData: {},
                recentImages: false,
                thumbnails: true,
                currentSelected: false,
                setImage: 0,
              },
              recentImageData: {},
              recentImages: null,
              thumbnailResponseStatusCode: MantaStatusCode.kOk,
              thumbnails: null,
              currentSeaPenQuery: query,
              pendingSelected: null,
              currentSelected: null,
              shouldShowSeaPenIntroductionDialog: false,
              error: null,
            }),
          },
          {
            'wallpaper.seaPen': typeCheck<SeaPenState>({
              loading: {
                recentImageData: {},
                recentImages: false,
                thumbnails: false,
                currentSelected: false,
                setImage: 0,
              },
              recentImageData: {},
              recentImages: null,
              thumbnailResponseStatusCode: MantaStatusCode.kOk,
              thumbnails: seaPenProvider.images,
              currentSeaPenQuery: query,
              pendingSelected: null,
              currentSelected: null,
              shouldShowSeaPenIntroductionDialog: false,
              error: null,
            }),
          },
        ],
        personalizationStore.states.map(
            filterAndFlattenState(['wallpaper.seaPen'])),
        'expected states match');
  });

  test('resets loading state after select thumbnail failure', async () => {
    seaPenProvider.selectSeaPenThumbnailResponse =
        Promise.resolve({success: false});
    personalizationStore.data.wallpaper.currentSelected = {
      type: WallpaperType.kSeaPen,
      key: '123',
      layout: WallpaperLayout.kCenterCropped,
      descriptionContent: '',
      descriptionTitle: '',
    };
    personalizationStore.data.wallpaper.seaPen.currentSelected = 123;

    const promise = selectSeaPenWallpaper(
        {image: {url: ''}, id: 456}, seaPenProvider, seaPenStore);

    assertDeepEquals(
        {image: true, attribution: true},
        personalizationStore.data.wallpaper.loading.selected,
        'image and attribution are loading');
    assertTrue(
        personalizationStore.data.wallpaper.seaPen.loading.currentSelected,
        'seaPen.loading.currentSelected is true');

    await promise;

    assertDeepEquals(
        {image: false, attribution: false},
        personalizationStore.data.wallpaper.loading.selected,
        'image and attribution are not loading');
    assertFalse(
        personalizationStore.data.wallpaper.seaPen.loading.currentSelected,
        'seaPen.loading.currentSelected is false');

    assertDeepEquals(
        [
          beginLoadSelectedImageAction(),
          beginSelectSeaPenThumbnailAction({image: {url: ''}, id: 456}),
          endSelectSeaPenThumbnailAction({image: {url: ''}, id: 456}, false),
          setSelectedRecentSeaPenImageAction(123),
        ],
        personalizationStore.actions,
        'resets to original selected id after failure');
  });

  test('resets loading state after select recent image failure', async () => {
    seaPenProvider.selectSeaPenRecentImageResponse =
        Promise.resolve({success: false});
    personalizationStore.data.wallpaper.currentSelected = {
      type: WallpaperType.kSeaPen,
      key: '123',
      layout: WallpaperLayout.kCenterCropped,
      descriptionContent: '',
      descriptionTitle: '',
    };
    personalizationStore.data.wallpaper.seaPen.currentSelected = 123;

    const promise = selectRecentSeaPenImage(456, seaPenProvider, seaPenStore);

    assertDeepEquals(
        {image: true, attribution: true},
        personalizationStore.data.wallpaper.loading.selected,
        'image and attribution are loading');
    assertTrue(
        personalizationStore.data.wallpaper.seaPen.loading.currentSelected,
        'seaPen.loading.currentSelected is true');

    await promise;

    assertDeepEquals(
        {image: false, attribution: false},
        personalizationStore.data.wallpaper.loading.selected,
        'image and attribution are not loading');
    assertFalse(
        personalizationStore.data.wallpaper.seaPen.loading.currentSelected,
        'seaPen.loading.currentSelected is false');

    assertDeepEquals(
        [
          beginSelectRecentSeaPenImageAction(456),
          beginLoadSelectedImageAction(),
          beginLoadSelectedRecentSeaPenImageAction(),
          endSelectRecentSeaPenImageAction(456, false),
          setSelectedRecentSeaPenImageAction(123),
        ],
        personalizationStore.actions,
        'resets to original selected id after failure');
  });

  test('sets error after select thumbnail failure', async () => {
    seaPenProvider.selectSeaPenThumbnailResponse =
        Promise.resolve({success: false});

    const thumbnail = {image: {url: ''}, id: 456};
    await selectSeaPenWallpaper(thumbnail, seaPenProvider, seaPenStore);

    assertDeepEquals(
        [
          beginLoadSelectedImageAction(),
          beginSelectSeaPenThumbnailAction(thumbnail),
          endSelectSeaPenThumbnailAction(thumbnail, false),
          setSelectedRecentSeaPenImageAction(null),
        ],
        personalizationStore.actions, 'fails selecting the thumbnail');

    assertDeepEquals(
        [
          null,
          null,
          loadTimeData.getString('seaPenErrorGeneric'),
          loadTimeData.getString('seaPenErrorGeneric'),
        ],
        personalizationStore.states.map(state => state.wallpaper.seaPen.error),
        'sets expected error state');

    // Try and fail again.
    const promise =
        selectSeaPenWallpaper(thumbnail, seaPenProvider, seaPenStore);

    // Error reset to null while attempting to select again.
    assertEquals(null, personalizationStore.data.wallpaper.seaPen.error);

    await promise;

    // Error is visible again.
    assertEquals(
        loadTimeData.getString('seaPenErrorGeneric'),
        personalizationStore.data.wallpaper.seaPen.error);
  });

  test('sets error after select recent image failure', async () => {
    seaPenProvider.selectSeaPenRecentImageResponse =
        Promise.resolve({success: false});

    await selectRecentSeaPenImage(456, seaPenProvider, seaPenStore);

    assertDeepEquals(
        [
          beginSelectRecentSeaPenImageAction(456),
          beginLoadSelectedImageAction(),
          beginLoadSelectedRecentSeaPenImageAction(),
          endSelectRecentSeaPenImageAction(456, false),
          setSelectedRecentSeaPenImageAction(null),
        ],
        personalizationStore.actions,
        'expected actions when failing to select recent image');

    assertDeepEquals(
        [
          null,
          null,
          null,
          loadTimeData.getString('seaPenErrorGeneric'),
          loadTimeData.getString('seaPenErrorGeneric'),
        ],
        personalizationStore.states.map(state => state.wallpaper.seaPen.error),
        'expected error states set when failing to select recent image');

    // Try and fail again.
    const promise = selectRecentSeaPenImage(789, seaPenProvider, seaPenStore);

    // Error reset to null while attempting to select again.
    assertEquals(null, personalizationStore.data.wallpaper.seaPen.error);

    await promise;

    // Error is visible again.
    assertEquals(
        loadTimeData.getString('seaPenErrorGeneric'),
        personalizationStore.data.wallpaper.seaPen.error);
  });
});
