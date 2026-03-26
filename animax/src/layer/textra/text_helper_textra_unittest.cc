// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include "src/layer/textra/text_helper_textra.h"

#include "gtest/gtest.h"
#include "src/layer/text_layer_animations.h"
#include "src/layer/textra/font_asset_manager_textra.h"
#include "src/layer/textra/text_content_textra.h"

using namespace lynx::animax;

TEST(TextHelperTextraTest, CreateTextContentReturnsTextraInstance) {
  TextLayerAnimations animations;
  FontAssetManagerTextra font_manager;
  TextContentDataSource data_source(animations, font_manager);
  TextHelperTextra helper;
  auto content = helper.CreateTextContent(data_source);
  content->Init(nullptr);
  ASSERT_NE(content, nullptr);
}

TEST(TextHelperTextraTest, CreateFontAssetManagerReturnsTextraInstance) {
  TextHelperTextra helper;
  auto font_mgr = helper.CreateFontAssetManager();
  ASSERT_NE(font_mgr, nullptr);
}
