// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RENDER_CANVAS_H_
#define ANIMAX_SRC_RENDER_CANVAS_H_

#include <memory>

#include "src/model/rect_model.h"
#include "src/render/font.h"
#include "src/render/image.h"
#include "src/render/matrix.h"
#include "src/render/paint.h"
#include "src/render/path.h"
#include "src/render/real_context.h"

namespace skity {
class Canvas;
class GPUSemaphore;
class GPUSurface;
}  // namespace skity
namespace lynx {
namespace animax {

class Canvas {
 public:
  Canvas(skity::Canvas* canvas, int32_t width, int32_t height,
         skity::GPUContext* context, skity::GPUSurface* surface = nullptr);

  ~Canvas() = default;

  void DrawRect(float left, float top, float right, float bottom,
                Paint& paint) {
    RectF rect{left, top, right, bottom};
    this->DrawRect(rect, paint);
  }

  int32_t GetWidth() const { return width_; }
  int32_t GetHeight() const { return height_; }

  void SaveLayer(const RectF& rect, Paint& paint);

  void DrawPath(Path& path, Paint& paint);

  void DrawImageRect(Image& image, const RectF& src, const RectF& dst,
                     Paint& paint);

  void DrawRect(const RectF& rect, Paint& paint);

  void DrawText(const std::string& text, float x, float y, Font& font,
                Paint& paint);

  void Save();

  void ResetMatrix();

  void Concat(Matrix& matrix);

  void Restore();

  bool ClipRect(const RectF& rect);

  void Scale(float x, float y);

  void Translate(float x, float y);

  std::unique_ptr<Matrix> GetMatrix() const;

  RealContext* GetRealContext() const;

  skity::Canvas* GetSkityCanvas();

  // Forward an external wait semaphore to the underlying surface. Used by the
  // Vulkan video path so the surface waits for GL completion before rendering.
  // Must be called between the surface's LockCanvas() and Flush().
  void AddExternalWaitSemaphore(std::shared_ptr<skity::GPUSemaphore> semaphore);

 private:
  skity::Canvas* canvas_;
  int32_t width_ = 0, height_ = 0;
  std::unique_ptr<RealContext> real_context_;
  skity::GPUSurface* surface_ = nullptr;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RENDER_CANVAS_H_
