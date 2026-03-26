// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "testing/conformance_test/animax_renderer_osmesa.h"

#include <algorithm>
#include <iostream>

#include "src/model/composition_model.h"
#include "src/model/layer_model.h"
#include "src/parser/layer_parser.h"

namespace lynx {
namespace animax {

namespace {

void CopyAndFlipYAxis(uint8_t* src, uint8_t* dst, int32_t width,
                      int32_t height) {
  for (int32_t y = 0; y < height; ++y) {
    const uint8_t* src_row = src + (height - 1 - y) * width * 4;
    uint8_t* dst_row = dst + y * width * 4;
    std::copy(src_row, src_row + width * 4, dst_row);
  }
}

void UnpremultiplyAlpha(uint8_t* pixels, int32_t width, int32_t height) {
  for (size_t i = 0; i < width * height * 4; i += 4) {
    auto* pixel = pixels + i;
    const auto alpha = pixel[3];

    if (alpha == 0) {
      pixel[0] = 0;
      pixel[1] = 0;
      pixel[2] = 0;
    } else {
      const auto factor = 255.0f / alpha;
      pixel[0] = static_cast<uint8_t>(std::min(255.f, pixel[0] * factor));
      pixel[1] = static_cast<uint8_t>(std::min(255.f, pixel[1] * factor));
      pixel[2] = static_cast<uint8_t>(std::min(255.f, pixel[2] * factor));
    }
  }
}

}  // namespace

AnimaXRendererOSMesa::AnimaXRendererOSMesa(
    std::shared_ptr<CompositionModel> model_, int32_t width, int32_t height)
    : model_{model_},
      layer_model_{LayerParser::Parse(*model_)},
      layer_{std::make_unique<CompositionLayer>(*layer_model_, *model_)},
      surface_(std::make_unique<AnimaXSurfaceGLOSMesa>(width, height)) {
  layer_->SetLayerModels(model_->GetLayers());
  layer_->Init();
}

int32_t AnimaXRendererOSMesa::Width() const { return surface_->Width(); }

int32_t AnimaXRendererOSMesa::Height() const { return surface_->Height(); }

[[nodiscard]] std::unique_ptr<Bitmap> AnimaXRendererOSMesa::DrawFrame(
    int32_t frame) {
  if (!frame_progress_converter_.IsFrameValid(frame)) {
    std::cerr << "Invalid frame: " << frame << ", frame must be within [0, "
              << model_->GetEndFrame() << ")" << std::endl;
    return nullptr;
  }
  const auto progress = frame_progress_converter_.ProgressForFrame(frame);
  layer_->SetProgress(progress);
  auto* canvas = surface_->Canvas();
  ResizeCanvas(*canvas);
  auto matrix = std::make_unique<Matrix>();
  layer_->Draw(*canvas, *matrix, 255);
  surface_->Flush();
  return SurfaceToBitmap();
}

[[nodiscard]] std::unique_ptr<Bitmap> AnimaXRendererOSMesa::SurfaceToBitmap() {
  const auto width = surface_->Width();
  const auto height = surface_->Height();
  auto* buffer = static_cast<uint8_t*>(surface_->Buffer());
  const auto buffer_size =
      static_cast<size_t>(width) * static_cast<size_t>(height);
  auto* copied_buffer =
      static_cast<uint8_t*>(std::malloc(buffer_size * sizeof(uint32_t)));
  CopyAndFlipYAxis(buffer, copied_buffer, width, height);
  UnpremultiplyAlpha(copied_buffer, width, height);
  return Bitmap::Make(
      width, height, copied_buffer,
      [](const void* pixels) { std::free(const_cast<void*>(pixels)); },
      copied_buffer);
}

void AnimaXRendererOSMesa::ResizeCanvas(Canvas& canvas) {
  const auto scale_factor =
      std::min(surface_->Width() / model_->GetBounds().GetWidth(),
               surface_->Height() / model_->GetBounds().GetHeight());
  canvas.ResetMatrix();
  canvas.Translate(
      (surface_->Width() - scale_factor * model_->GetBounds().GetWidth()) / 2.f,
      (surface_->Height() - scale_factor * model_->GetBounds().GetHeight()) /
          2.f);
  canvas.Scale(scale_factor, scale_factor);
}

}  // namespace animax
}  // namespace lynx
