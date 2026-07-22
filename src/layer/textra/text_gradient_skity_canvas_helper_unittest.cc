// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/layer/textra/text_gradient_skity_canvas_helper.h"

#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

#include "gtest/gtest.h"
#include "skity/effect/shader.hpp"
#include "skity/graphic/paint.hpp"
#include "skity/render/canvas.hpp"
#include "skity/text/typeface.hpp"
#include "textra/platform/skity/skity_canvas_helper.h"
#include "textra/platform/skity/skity_typeface_helper.h"

namespace lynx {
namespace animax {
namespace {

constexpr char kTestFontPath[] =
    "example/darwin/ios/lynx_example/LynxExample/Resource/Fonts/"
    "notoserif.ttf";

class RecordingSkityCanvas final : public skity::Canvas {
 public:
  RecordingSkityCanvas() : skity::Canvas(skity::kMaxCullRect) {}

  const std::vector<skity::Paint>& GetBlobPaints() const {
    return blob_paints_;
  }

  const std::vector<const skity::TextBlob*>& GetBlobs() const { return blobs_; }

 protected:
  void OnClipPath(const skity::Path&, skity::Canvas::ClipOp) override {}
  void OnDrawPath(const skity::Path&, const skity::Paint&) override {}
  void OnSaveLayer(const skity::Rect&, const skity::Paint&) override {}
  void OnDrawBlob(const skity::TextBlob* blob, float, float,
                  const skity::Paint& paint) override {
    blobs_.push_back(blob);
    blob_paints_.push_back(paint);
  }
  void OnDrawImageRect(std::shared_ptr<skity::Image>, const skity::Rect&,
                       const skity::Rect&, const skity::SamplingOptions&,
                       const skity::Paint*) override {}
  void OnDrawGlyphs(uint32_t, const skity::GlyphID[], const float[],
                    const float[], const skity::Font&,
                    const skity::Paint&) override {}
  void OnDrawPaint(const skity::Paint&) override {}
  void OnSave() override {}
  void OnRestore() override {}
  void OnRestoreToCount(int) override {}
  void OnFlush() override {}
  uint32_t OnGetWidth() const override { return 100; }
  uint32_t OnGetHeight() const override { return 100; }

 private:
  std::vector<skity::Paint> blob_paints_;
  std::vector<const skity::TextBlob*> blobs_;
};

std::shared_ptr<skity::Shader> MakeLinearShader(float end_x) {
  const skity::Point points[] = {
      skity::Point{0.f, 0.f, 0.f, 1.f},
      skity::Point{end_x, 0.f, 0.f, 1.f},
  };
  const skity::Color4f colors[] = {
      skity::Color4f{1.f, 0.f, 0.f, 1.f},
      skity::Color4f{0.f, 0.f, 1.f, 1.f},
  };
  const float positions[] = {0.f, 1.f};
  return skity::Shader::MakeLinear(points, colors, positions, 2);
}

void DrawOneGlyph(TextGradientSkityCanvasHelper& helper,
                  skity::Paint::Style style, uint8_t alpha) {
  auto typeface = skity::Typeface::MakeFromFile(kTestFontPath);
  ASSERT_NE(nullptr, typeface);
  skity::textlayout::SkityTypefaceHelper font(std::move(typeface));

  ttoffice::tttext::SkityPainter painter;
  painter.SetFillColor(0xFF112233u);
  painter.SetStrokeColor(0xFF445566u);
  painter.SetStrokeWidth(2.f);
  painter.SetTextSize(20.f);
  painter.platform_painter_ = std::make_unique<skity::Paint>();
  painter.platform_painter_->SetStyle(style);
  painter.platform_painter_->SetFillColor(0xFF112233u);
  painter.platform_painter_->SetStrokeColor(0xFF445566u);
  painter.platform_painter_->SetStrokeWidth(2.f);
  painter.platform_painter_->SetAlpha(alpha);

  uint16_t glyphs[] = {font.UnicharToGlyph('A')};
  float x[] = {0.f};
  float y[] = {0.f};
  helper.DrawGlyphs(&font, 1, glyphs, nullptr, 0, 0.f, 0.f, x, y, &painter);
}

TEST(TextGradientSkityCanvasHelperTest,
     StrokeOverDrawsBaseThenBottomToTopGradientsThenStroke) {
  RecordingSkityCanvas canvas;
  auto top_shader = MakeLinearShader(100.f);
  auto bottom_shader = MakeLinearShader(200.f);
  ASSERT_NE(nullptr, top_shader);
  ASSERT_NE(nullptr, bottom_shader);
  TextGradientSkityCanvasHelper helper(&canvas, {top_shader, bottom_shader},
                                       73);

  DrawOneGlyph(helper, skity::Paint::kStrokeAndFill_Style, 73);

  const auto& paints = canvas.GetBlobPaints();
  ASSERT_EQ(4u, paints.size());
  const auto& blobs = canvas.GetBlobs();
  ASSERT_EQ(4u, blobs.size());
  for (const auto* blob : blobs) {
    EXPECT_EQ(blobs.front(), blob);
  }
  EXPECT_EQ(skity::Paint::kFill_Style, paints[0].GetStyle());
  EXPECT_EQ(nullptr, paints[0].GetShader());
  EXPECT_EQ(73, paints[0].GetAlpha());
  EXPECT_EQ(skity::Paint::kFill_Style, paints[1].GetStyle());
  EXPECT_EQ(bottom_shader.get(), paints[1].GetShader().get());
  EXPECT_EQ(73, paints[1].GetAlpha());
  EXPECT_EQ(skity::Paint::kFill_Style, paints[2].GetStyle());
  EXPECT_EQ(top_shader.get(), paints[2].GetShader().get());
  EXPECT_EQ(73, paints[2].GetAlpha());
  EXPECT_EQ(skity::Paint::kStroke_Style, paints[3].GetStyle());
  EXPECT_EQ(nullptr, paints[3].GetShader());
  EXPECT_EQ(73, paints[3].GetAlpha());
}

TEST(TextGradientSkityCanvasHelperTest,
     StrokeBehindKeepsCombinedBaseBeforeGradients) {
  RecordingSkityCanvas canvas;
  auto top_shader = MakeLinearShader(100.f);
  auto bottom_shader = MakeLinearShader(200.f);
  TextGradientSkityCanvasHelper helper_with_alpha(
      &canvas, {top_shader, bottom_shader}, 91);

  DrawOneGlyph(helper_with_alpha, skity::Paint::kStrokeThenFill_Style, 91);

  const auto& paints = canvas.GetBlobPaints();
  ASSERT_EQ(3u, paints.size());
  EXPECT_EQ(skity::Paint::kStrokeThenFill_Style, paints[0].GetStyle());
  EXPECT_EQ(nullptr, paints[0].GetShader());
  EXPECT_EQ(91, paints[0].GetAlpha());
  EXPECT_EQ(skity::Paint::kFill_Style, paints[1].GetStyle());
  EXPECT_EQ(bottom_shader.get(), paints[1].GetShader().get());
  EXPECT_EQ(91, paints[1].GetAlpha());
  EXPECT_EQ(skity::Paint::kFill_Style, paints[2].GetStyle());
  EXPECT_EQ(top_shader.get(), paints[2].GetShader().get());
  EXPECT_EQ(91, paints[2].GetAlpha());
}

TEST(TextGradientSkityCanvasHelperTest,
     MissingGradientKeepsOriginalSingleDrawPath) {
  RecordingSkityCanvas canvas;
  TextGradientSkityCanvasHelper helper(&canvas, {nullptr});

  DrawOneGlyph(helper, skity::Paint::kStrokeAndFill_Style, 123);

  const auto& paints = canvas.GetBlobPaints();
  ASSERT_EQ(1u, paints.size());
  EXPECT_EQ(skity::Paint::kStrokeAndFill_Style, paints[0].GetStyle());
  EXPECT_EQ(nullptr, paints[0].GetShader());
  EXPECT_EQ(123, paints[0].GetAlpha());
}

TEST(TextGradientSkityCanvasHelperTest,
     GradientUsesLayerAlphaInsteadOfBaseFillAlpha) {
  RecordingSkityCanvas canvas;
  auto shader = MakeLinearShader(100.f);
  ASSERT_NE(nullptr, shader);
  TextGradientSkityCanvasHelper helper(&canvas, {shader}, 41);

  DrawOneGlyph(helper, skity::Paint::kFill_Style, 73);

  const auto& paints = canvas.GetBlobPaints();
  ASSERT_EQ(2u, paints.size());
  EXPECT_EQ(73, paints[0].GetAlpha());
  EXPECT_EQ(41, paints[1].GetAlpha());
}

}  // namespace
}  // namespace animax
}  // namespace lynx
