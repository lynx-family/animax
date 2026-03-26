// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_TESTING_CONFORMANCE_TEST_ANIMAX_RENDERER_H_
#define ANIMAX_TESTING_CONFORMANCE_TEST_ANIMAX_RENDERER_H_

#include <cstdint>
#include <memory>

#include "src/layer/composition_layer.h"
#include "src/render/surface.h"
#include "testing/conformance_test/animax_surface_osmesa.h"
#include "testing/conformance_test/frame_progress_converter.h"

namespace lynx {
namespace animax {

class Canvas;
class CompositionModel;
class LayerModel;
class CompositionLayer;
class AnimaXSurfaceGLOSMesa;

class AnimaXRendererOSMesa {
 public:
  AnimaXRendererOSMesa(std::shared_ptr<CompositionModel> model_, int32_t width,
                       int32_t height);

  int32_t Width() const;
  int32_t Height() const;

  [[nodiscard]] std::unique_ptr<Bitmap> DrawFrame(int32_t frame);

  void ResizeCanvas(Canvas& canvas);

 private:
  [[nodiscard]] std::unique_ptr<Bitmap> SurfaceToBitmap();
  std::shared_ptr<CompositionModel> model_;
  std::unique_ptr<LayerModel> layer_model_;
  std::unique_ptr<CompositionLayer> layer_;
  std::unique_ptr<AnimaXSurfaceGLOSMesa> surface_;
  FrameProgressConverter frame_progress_converter_{*model_};
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_TESTING_CONFORMANCE_TEST_ANIMAX_RENDERER_H_
