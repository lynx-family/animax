// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
// cspell:ignore maks

#include "src/layer/base_layer.h"

#include <memory>

#include "gtest/gtest.h"
#include "skity/render/canvas.hpp"
#include "src/model/animatable/animatable_transform_model.h"
#include "src/render/canvas.h"

namespace lynx {
namespace animax {
namespace {

class TestSkityCanvas : public skity::Canvas {
 public:
  TestSkityCanvas() : skity::Canvas(skity::kMaxCullRect) {}

 protected:
  void OnClipPath(const skity::Path&, skity::Canvas::ClipOp) override {}
  void OnDrawPath(const skity::Path&, const skity::Paint&) override {}
  void OnSaveLayer(const skity::Rect&, const skity::Paint&) override {}
  void OnDrawBlob(const skity::TextBlob*, float, float,
                  const skity::Paint&) override {}
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
  uint32_t OnGetWidth() const override { return 0; }
  uint32_t OnGetHeight() const override { return 0; }
};

class TestBaseLayer : public BaseLayer {
 public:
  TestBaseLayer(LayerModel& layer_model, CompositionModel& composition,
                RectF bounds)
      : BaseLayer(layer_model, composition), bounds_(bounds) {
    outline_maks_and_mattes_ = false;
  }

  void GetBounds(RectF& out_bounds, Matrix&, bool) override {
    out_bounds.Set(bounds_);
  }

  void DrawLayer(Canvas&, Matrix&, int32_t) override { draw_count_++; }

  int32_t GetDrawCount() const { return draw_count_; }

 private:
  RectF bounds_;
  int32_t draw_count_ = 0;
};

std::unique_ptr<LayerModel> MakeTestLayerModel(CompositionModel& composition) {
  auto layer_model = LayerModel::Make(composition);
  layer_model->Init("test", LayerType::kSolid, 1, -1, "", 0, 0,
                    std::make_unique<Color>(), 1.f, 0.f, 0.f, 0.f, nullptr,
                    nullptr, false,
                    std::make_unique<AnimatableTransformModel>(),
                    static_cast<int32_t>(MatteType::kNone), -1, false, nullptr,
                    nullptr, nullptr, true, nullptr, BlendModeType::kMultiply);
  return layer_model;
}

TEST(BaseLayerTest, CanvasBoundsUseInverseOfProjectedZ0Plane) {
  CompositionModel composition(1.f);
  composition.Init(std::make_unique<RectF>(0.f, 0.f, 200.f, 100.f), 0.f, 100.f,
                   30.f, true);
  auto layer_model = MakeTestLayerModel(composition);
  TestBaseLayer layer(*layer_model, composition,
                      RectF(150.f, 10.f, 180.f, 50.f));
  layer.Init();

  TestSkityCanvas skity_canvas;
  Canvas canvas(&skity_canvas, 100, 100, nullptr);
  Matrix canvas_matrix;
  canvas_matrix.PreRotateXYZ(0.f, 60.f, 0.f);
  canvas.Concat(canvas_matrix);

  Matrix parent_matrix;
  layer.Draw(canvas, parent_matrix, 255);

  // A 60-degree Y rotation projects the z=0 plane to half its width. The
  // screen-space canvas therefore covers local x=[0, 200], so this layer must
  // not be clipped. A full 4x4 inverse maps z=0 screen points to x=[0, 50]
  // instead and incorrectly drops the layer.
  EXPECT_EQ(layer.GetDrawCount(), 1);
}

TEST(BaseLayerTest, SingularProjectedPlaneDoesNotCullLayer) {
  CompositionModel composition(1.f);
  composition.Init(std::make_unique<RectF>(0.f, 0.f, 200.f, 100.f), 0.f, 100.f,
                   30.f, true);
  auto layer_model = MakeTestLayerModel(composition);
  TestBaseLayer layer(*layer_model, composition,
                      RectF(150.f, 10.f, 180.f, 50.f));
  layer.Init();

  TestSkityCanvas skity_canvas;
  Canvas canvas(&skity_canvas, 100, 100, nullptr);
  skity::Matrix edge_on_matrix(0.f, 0.f, 1.f, 0.f,   // First column.
                               0.f, 1.f, 0.f, 0.f,   // Second column.
                               -1.f, 0.f, 0.f, 0.f,  // Third column.
                               0.f, 0.f, 0.f, 1.f);  // Fourth column.
  Matrix canvas_matrix(edge_on_matrix);
  canvas.Concat(canvas_matrix);

  Matrix parent_matrix;
  layer.Draw(canvas, parent_matrix, 255);

  EXPECT_EQ(layer.GetDrawCount(), 1);
}

TEST(BaseLayerTest, CanvasBoundsCullOutsideProjectedZ0Plane) {
  CompositionModel composition(1.f);
  composition.Init(std::make_unique<RectF>(0.f, 0.f, 300.f, 100.f), 0.f, 100.f,
                   30.f, true);
  auto layer_model = MakeTestLayerModel(composition);
  TestBaseLayer layer(*layer_model, composition,
                      RectF(250.f, 10.f, 280.f, 50.f));
  layer.Init();

  TestSkityCanvas skity_canvas;
  Canvas canvas(&skity_canvas, 100, 100, nullptr);
  Matrix canvas_matrix;
  canvas_matrix.PreRotateXYZ(0.f, 60.f, 0.f);
  canvas.Concat(canvas_matrix);

  Matrix parent_matrix;
  layer.Draw(canvas, parent_matrix, 255);

  EXPECT_EQ(layer.GetDrawCount(), 0);
}

TEST(BaseLayerTest, IdentityCanvasBoundsStillCullOutsideLayer) {
  CompositionModel composition(1.f);
  composition.Init(std::make_unique<RectF>(0.f, 0.f, 200.f, 100.f), 0.f, 100.f,
                   30.f, false);
  auto layer_model = MakeTestLayerModel(composition);
  TestBaseLayer layer(*layer_model, composition,
                      RectF(150.f, 10.f, 180.f, 50.f));
  layer.Init();

  TestSkityCanvas skity_canvas;
  Canvas canvas(&skity_canvas, 100, 100, nullptr);
  Matrix parent_matrix;
  layer.Draw(canvas, parent_matrix, 255);

  EXPECT_EQ(layer.GetDrawCount(), 0);
}

}  // namespace
}  // namespace animax
}  // namespace lynx
