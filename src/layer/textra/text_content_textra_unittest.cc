// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/layer/textra/text_content_textra.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <memory>
#include <vector>

#include "gtest/gtest.h"
#include "skity/graphic/bitmap.hpp"
#include "skity/render/canvas.hpp"
#include "src/animation/text_keyframe_animation.h"
#include "src/base/thread/thread_assert.h"
#include "src/layer/text_content_data_source.h"
#include "src/layer/text_layer_animations.h"
#include "src/layer/textra/font_asset_manager_textra.h"
#include "src/model/value/document_data.h"
#include "src/resource/asset/font_asset.h"
#include "src/testing/font_test_data.h"

namespace lynx {
namespace animax {
namespace {

constexpr char kFontName[] = "StaticTestFont";
constexpr int32_t kWidth = 800;
constexpr int32_t kHeight = 160;

class FixedTextKeyframeAnimation : public TextKeyframeAnimation {
 public:
  explicit FixedTextKeyframeAnimation(DocumentData& document_data)
      : TextKeyframeAnimation(std::make_shared<KeyframeModelList>()),
        document_data_(document_data) {}

  DocumentData& GetValue() override { return document_data_; }

 private:
  DocumentData& document_data_;
};

struct StaticTextRun {
  explicit StaticTextRun(TestFontVariant variant)
      : document_data("Wi.mixed-width 0123456789", kFontName, 40.f,
                      DocumentJustification::kLeftAlign, 0, 48.f, 0.f,
                      0xFF000000, 0x00000000, 0.f, false, nullptr, nullptr),
        font_asset(std::make_shared<FontAsset>(
            FontAssetModel{.family = "StaticTestFamily",
                           .name = kFontName,
                           .style = "Regular",
                           .path_origin = FontPathOrigin::kFontUrl})) {
    font_asset->SetRawData(MakeTestFontData(variant));
    font_manager.GetFontAssetMap().emplace(kFontName, font_asset);
    animations.text_keyframe =
        std::make_unique<FixedTextKeyframeAnimation>(document_data);
    data_source =
        std::make_unique<TextContentDataSource>(animations, font_manager);
    content = std::make_unique<TextContentTextra>(*data_source);
  }

  DocumentData document_data;
  std::shared_ptr<FontAsset> font_asset;
  FontAssetManagerTextra font_manager;
  TextLayerAnimations animations;
  std::unique_ptr<TextContentDataSource> data_source;
  std::unique_ptr<TextContentTextra> content;
};

class TextContentTextraStaticFontTest : public ::testing::Test {
 protected:
  void SetUp() override {
    previous_thread_type_ = ThreadAssert::GetCurrent();
    ThreadAssert::Init(ThreadAssert::Type::kGPU);
  }

  void TearDown() override { ThreadAssert::Init(previous_thread_type_); }

  std::vector<uint8_t> Draw(StaticTextRun& run) {
    skity::Bitmap bitmap(kWidth, kHeight);
    auto skity_canvas = skity::Canvas::MakeSoftwareCanvas(&bitmap);
    EXPECT_NE(nullptr, skity_canvas);
    if (!skity_canvas) {
      return {};
    }
    skity_canvas->Clear(skity::Color{0});
    Canvas canvas(skity_canvas.get(), kWidth, kHeight, nullptr);
    canvas.Save();
    canvas.Translate(20.f, 100.f);
    run.content->Draw(canvas, 255);
    canvas.Restore();
    skity_canvas->Flush();
    const auto* pixels = bitmap.GetPixelAddr();
    return {pixels, pixels + bitmap.RowBytes() * bitmap.Height()};
  }

  void ExpectSameBounds(const RectF& actual, const RectF& expected) {
    EXPECT_NEAR(expected.GetLeft(), actual.GetLeft(), 0.01f);
    EXPECT_NEAR(expected.GetTop(), actual.GetTop(), 0.01f);
    EXPECT_NEAR(expected.GetRight(), actual.GetRight(), 0.01f);
    EXPECT_NEAR(expected.GetBottom(), actual.GetBottom(), 0.01f);
  }

  ThreadAssert::Type previous_thread_type_ = ThreadAssert::Type::kUnknown;
};

TEST_F(TextContentTextraStaticFontTest,
       DrawRefreshesCachedStaticLayoutAfterFontDataChanges) {
  StaticTextRun run(TestFontVariant::kNarrow);
  StaticTextRun reference(TestFontVariant::kWide);
  ASSERT_TRUE(run.data_source->GetLayoutOnlyOnce());
  ASSERT_TRUE(reference.data_source->GetLayoutOnlyOnce());

  const auto initial_pixels = Draw(run);
  ASSERT_TRUE(std::any_of(initial_pixels.begin(), initial_pixels.end(),
                          [](uint8_t value) { return value != 0; }));
  RectF initial_bounds;
  ASSERT_TRUE(run.content->GetRect(initial_bounds));
  ASSERT_GT(initial_bounds.GetWidth(), 0.f);
  const auto reference_pixels = Draw(reference);
  ASSERT_TRUE(std::any_of(reference_pixels.begin(), reference_pixels.end(),
                          [](uint8_t value) { return value != 0; }));
  RectF reference_bounds;
  ASSERT_TRUE(reference.content->GetRect(reference_bounds));
  ASSERT_GT(reference_bounds.GetWidth(), 0.f);
  ASSERT_GT(std::abs(initial_bounds.GetWidth() - reference_bounds.GetWidth()),
            0.1f);
  ASSERT_NE(initial_pixels, reference_pixels);

  run.font_asset->SetRawData(MakeTestFontData(TestFontVariant::kWide));
  EXPECT_EQ(reference_pixels, Draw(run));
  RectF refreshed_bounds;
  ASSERT_TRUE(run.content->GetRect(refreshed_bounds));
  ExpectSameBounds(refreshed_bounds, reference_bounds);
}

TEST_F(TextContentTextraStaticFontTest,
       GetRectRefreshesCachedStaticLayoutWithoutAnotherDraw) {
  StaticTextRun run(TestFontVariant::kNarrow);
  StaticTextRun reference(TestFontVariant::kWide);
  ASSERT_TRUE(run.data_source->GetLayoutOnlyOnce());
  ASSERT_TRUE(reference.data_source->GetLayoutOnlyOnce());

  Draw(run);
  RectF initial_bounds;
  ASSERT_TRUE(run.content->GetRect(initial_bounds));
  ASSERT_GT(initial_bounds.GetWidth(), 0.f);
  Draw(reference);
  RectF reference_bounds;
  ASSERT_TRUE(reference.content->GetRect(reference_bounds));
  ASSERT_GT(reference_bounds.GetWidth(), 0.f);
  ASSERT_GT(std::abs(initial_bounds.GetWidth() - reference_bounds.GetWidth()),
            0.1f);

  run.font_asset->SetRawData(MakeTestFontData(TestFontVariant::kWide));
  RectF refreshed_bounds;
  // Querying bounds must invalidate the old static layout without a Draw call.
  ASSERT_TRUE(run.content->GetRect(refreshed_bounds));
  ExpectSameBounds(refreshed_bounds, reference_bounds);
}

}  // namespace
}  // namespace animax
}  // namespace lynx
