// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_LAYER_TEXTRA_TEXT_GRADIENT_SKITY_CANVAS_HELPER_H_
#define ANIMAX_SRC_LAYER_TEXTRA_TEXT_GRADIENT_SKITY_CANVAS_HELPER_H_

#include <textra/platform/skity/skity_canvas_helper.h>

#include <cstdint>
#include <memory>
#include <vector>

namespace skity {
class Shader;
}  // namespace skity

namespace lynx {
namespace animax {

class TextGradientSkityCanvasHelper final
    : public ttoffice::tttext::SkityCanvasHelper {
 public:
  TextGradientSkityCanvasHelper(
      skity::Canvas* canvas,
      const std::vector<std::shared_ptr<skity::Shader>>& shaders,
      uint8_t gradient_alpha = 255);

  void DrawGlyphs(const ttoffice::tttext::ITypefaceHelper* font,
                  uint32_t glyph_count, const uint16_t* glyphs,
                  const char* text, uint32_t text_bytes, float origin_x,
                  float origin_y, float* x, float* y,
                  ttoffice::tttext::Painter* painter) override;

 private:
  // The base helper keeps its canvas private; cached blob passes call Skity
  // directly to avoid rebuilding the blob through the base implementation.
  skity::Canvas* canvas_ = nullptr;
  std::vector<std::shared_ptr<skity::Shader>> shaders_;
  uint8_t gradient_alpha_ = 255;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_LAYER_TEXTRA_TEXT_GRADIENT_SKITY_CANVAS_HELPER_H_
