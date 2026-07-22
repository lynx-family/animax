// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_LAYER_TEXTRA_TEXT_CONTENT_TEXTRA_H_
#define ANIMAX_SRC_LAYER_TEXTRA_TEXT_CONTENT_TEXTRA_H_

#include <memory>
#include <vector>

#include "src/layer/text_content.h"
#include "src/layer/text_content_data_source.h"
#include "src/render/canvas.h"

namespace skity {
class Shader;
}  // namespace skity

namespace lynx {
namespace animax {
class TextContentLayoutContext;
class TextContentTextra : public TextContent {
 public:
  TextContentTextra(const TextContentDataSource& data_source);
  ~TextContentTextra();

  void Draw(Canvas& canvas, int32_t alpha) override;
  bool GetRect(RectF& out_rect) override;

 private:
  void ConfigurePlatformPainter(const TextContentDataSource& data_source,
                                int32_t alpha);

  const TextContentDataSource& data_source_;
  std::unique_ptr<TextContentLayoutContext> layout_context_;
  std::unique_ptr<Paint> paint_;
  std::vector<std::shared_ptr<skity::Shader>> gradient_shaders_;
  bool gradient_shaders_initialized_ = false;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_LAYER_TEXTRA_TEXT_CONTENT_TEXTRA_H_
