// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include "include/player/animax_surface.h"

#include <memory>

#include "gtest/gtest.h"
#include "skity/render/canvas.hpp"
#include "src/render/canvas.h"

using namespace lynx::animax;

class AnimaxSurfaceTest : public ::testing::Test {
 protected:
  AnimaxSurfaceTest() = default;
  ~AnimaxSurfaceTest() override = default;
};

class AnimaxSimpleSkityCanvas : public skity::Canvas {
 public:
  explicit AnimaxSimpleSkityCanvas(skity::Rect cull_rect = skity::kMaxCullRect)
      : skity::Canvas(cull_rect) {}

 protected:
  void OnClipPath(skity::Path const& path, skity::Canvas::ClipOp op) override {}
  void OnDrawPath(skity::Path const& path, skity::Paint const& paint) override {
  }
  void OnSaveLayer(const skity::Rect& bounds,
                   const skity::Paint& paint) override {}
  void OnDrawBlob(const skity::TextBlob* blob, float x, float y,
                  skity::Paint const& paint) override {}
  void OnDrawImageRect(std::shared_ptr<skity::Image> image,
                       const skity::Rect& src, const skity::Rect& dst,
                       const skity::SamplingOptions& sampling,
                       skity::Paint const* paint) override {}
  void OnDrawGlyphs(uint32_t count, const skity::GlyphID glyphs[],
                    const float position_x[], const float position_y[],
                    const skity::Font& font,
                    const skity::Paint& paint) override {}
  void OnDrawPaint(skity::Paint const& paint) override {}
  void OnSave() override {}
  void OnRestore() override {}
  void OnRestoreToCount(int saveCount) override {}
  void OnFlush() override {}
  uint32_t OnGetWidth() const override { return 0; }
  uint32_t OnGetHeight() const override { return 0; }
};

class AnimaxTestSurface : public AnimaXSurface {
 public:
  AnimaxTestSurface(std::shared_ptr<skity::Canvas> skity_canvas, int32_t w,
                    int32_t h)
      : AnimaXSurface(w, h), skity_canvas_(std::move(skity_canvas)) {
    if (skity_canvas_) {
      animax_canvas_ = CreateAnimaXCanvas(skity_canvas_.get(), w, h, nullptr);
    }
  }
  void Flush() override {}
  lynx::animax::Canvas* Canvas() override { return animax_canvas_.get(); }
  AnimaXBackend Type() const override { return AnimaXBackend::kSoftware; }
  bool Valid() const override { return animax_canvas_ != nullptr; }
  using AnimaXSurface::Resize;

 private:
  std::shared_ptr<skity::Canvas> skity_canvas_;
  std::unique_ptr<lynx::animax::Canvas> animax_canvas_;
};

TEST(AnimaxSurfaceTest, CreateAnimaXCanvasReturnsValidPointer) {
  auto skity_canvas = std::make_shared<AnimaxSimpleSkityCanvas>();
  AnimaxTestSurface surface(skity_canvas, 320, 240);
  EXPECT_EQ(surface.Width(), 320);
  EXPECT_EQ(surface.Height(), 240);
  EXPECT_TRUE(surface.Valid());
  EXPECT_NE(surface.Canvas(), nullptr);
  EXPECT_EQ(surface.Type(), AnimaXBackend::kSoftware);
}

TEST(AnimaxSurfaceTest, InvalidSurfaceWhenNullCanvas) {
  std::shared_ptr<AnimaxSimpleSkityCanvas> null_canvas = nullptr;
  AnimaxTestSurface surface(null_canvas, 100, 100);
  EXPECT_FALSE(surface.Valid());
  EXPECT_EQ(surface.Canvas(), nullptr);
}

TEST(AnimaxSurfaceTest, WidthHeightAfterConstruction) {
  auto skity_canvas = std::make_shared<AnimaxSimpleSkityCanvas>();
  AnimaxTestSurface surface(skity_canvas, 640, 480);
  EXPECT_EQ(surface.Width(), 640);
  EXPECT_EQ(surface.Height(), 480);
}

TEST(AnimaxSurfaceTest, ResizeChangesWidthAndHeight) {
  auto skity_canvas = std::make_shared<AnimaxSimpleSkityCanvas>();
  AnimaxTestSurface surface(skity_canvas, 320, 240);
  EXPECT_EQ(surface.Width(), 320);
  EXPECT_EQ(surface.Height(), 240);
  surface.Resize(800, 600);
  EXPECT_EQ(surface.Width(), 800);
  EXPECT_EQ(surface.Height(), 600);
}
