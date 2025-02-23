// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import '../module_header.js';
import 'chrome://resources/cr_elements/cr_auto_img/cr_auto_img.js';
import 'chrome://resources/cr_elements/cr_lazy_render/cr_lazy_render.js';

import type {CrLazyRenderElement} from 'chrome://resources/cr_elements/cr_lazy_render/cr_lazy_render.js';
import type {DomRepeat, DomRepeatEvent} from 'chrome://resources/polymer/v3_0/polymer/polymer_bundled.min.js';
import {PolymerElement} from 'chrome://resources/polymer/v3_0/polymer/polymer_bundled.min.js';

import type {Article} from '../../feed.mojom-webui.js';
import {I18nMixin} from '../../i18n_setup.js';
import type {InfoDialogElement} from '../info_dialog.js';
import {ModuleDescriptor} from '../module_descriptor.js';

import {FeedProxy} from './feed_module_proxy.js';
import {getTemplate} from './module.html.js';

export interface FeedModuleElement {
  $: {
    articleRepeat: DomRepeat,
    articles: HTMLElement,
    infoDialogRender: CrLazyRenderElement<InfoDialogElement>,
  };
}

/** The Feed module, which shows users following feed articles.  */
export class FeedModuleElement extends I18nMixin
(PolymerElement) {
  static get is() {
    return 'feed-ntp-module';
  }

  static get properties() {
    return {
      articles: Array,
    };
  }

  articles: Article[];

  private onArticleClick_(_: DomRepeatEvent<Article>) {
    FeedProxy.getHandler().articleOpened();
  }

  static get template() {
    return getTemplate();
  }
}

customElements.define(FeedModuleElement.is, FeedModuleElement);

async function createFeedElement(): Promise<HTMLElement> {
  const {articles} = await FeedProxy.getHandler().getFollowingFeedArticles();
  const element = new FeedModuleElement();
  element.articles = articles;
  return element;
}

export const feedDescriptor: ModuleDescriptor = new ModuleDescriptor(
    /*id=*/ 'feed', createFeedElement);
