// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_LAYER_TEXT_CONTENT_H_
#define ANIMAX_SRC_LAYER_TEXT_CONTENT_H_

#include <stdint.h>

#include "include/base/macros.h"
#include "src/layer/text_layer.h"

namespace lynx {
namespace animax {
class Canvas;

class ANIMAX_EXPORT TextContent {
 public:
  virtual ~TextContent() = default;

  void Init(TextLayer* layer) { layer_ = layer; };
  virtual void Draw(Canvas& canvas, int32_t alpha) = 0;
  virtual void GetRect(RectF& out_rect){};

 protected:
  TextLayer* layer_ = nullptr;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_LAYER_TEXT_CONTENT_H_
