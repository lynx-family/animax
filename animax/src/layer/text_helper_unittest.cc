// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include "src/layer/text_helper.h"

#include "gtest/gtest.h"
#include "src/layer/font_asset_manager.h"
#include "src/layer/text_content_default.h"
#include "src/layer/text_layer_animations.h"

using namespace lynx::animax;

class CustomTextHelper : public TextHelper {
 public:
  CustomTextHelper() : TextHelper() {}
  // Override CreateTextContent to provide custom behavior (or call base)
  std::unique_ptr<TextContent> CreateTextContent(
      const TextContentDataSource& data_source) override {
    return std::make_unique<TextContentDefault>(data_source);
  }
  // Override CreateFontAssetManager to provide custom behavior (or call base)
  std::unique_ptr<FontAssetManager> CreateFontAssetManager() override {
    return std::make_unique<FontAssetManager>();
  }
};
TEST(TextHelperTest, DefaultImplReturnsDefaultInstance) {
  // Ensure no custom implementation is registered
  TextHelper::RegisterImpl(nullptr);
  TextHelper& impl = TextHelper::Impl();
  TextLayerAnimations animations;
  FontAssetManager font_asset_manager;
  // Check that impl returns a valid default implementation
  auto content = impl.CreateTextContent(
      TextContentDataSource{animations, font_asset_manager});
  content->Init(nullptr);
  ASSERT_NE(content, nullptr);
  auto font_mgr = impl.CreateFontAssetManager();
  ASSERT_NE(font_mgr, nullptr);
}
TEST(TextHelperTest, RegisterImplReturnsCustomInstance) {
  CustomTextHelper custom_impl;
  // Register custom implementation
  TextHelper::RegisterImpl(&custom_impl);
  TextHelper& impl = TextHelper::Impl();
  // Verify that Impl() returns the registered custom implementation
  ASSERT_EQ(&impl, &custom_impl);
  TextLayerAnimations animations;
  FontAssetManager font_asset_manager;
  auto content = impl.CreateTextContent(
      TextContentDataSource{animations, font_asset_manager});
  ASSERT_NE(content, nullptr);
  auto font_mgr = impl.CreateFontAssetManager();
  ASSERT_NE(font_mgr, nullptr);
  // Reset to default implementation to avoid side effects on other tests
  TextHelper::RegisterImpl(nullptr);
}
