// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_MODEL_RECT_MODEL_H_
#define ANIMAX_SRC_MODEL_RECT_MODEL_H_

#include <cinttypes>

#include "include/base/macros.h"

namespace lynx {
namespace animax {

class ANIMAX_EXPORT RectF {
 public:
  RectF(float left, float top, float right, float bottom)
      : left_(left), top_(top), right_(right), bottom_(bottom) {}
  RectF() : left_(0), top_(0), right_(0), bottom_(0) {}
  ~RectF() = default;

  float GetWidth() const;
  float GetHeight() const;
  float GetLeft() const;
  float GetRight() const;
  float GetTop() const;
  float GetBottom() const;
  bool IsEmpty() const;

  void Set(float left, float top, float right, float bottom);
  void Set(const RectF &rect);
  void Union(float left, float top, float right, float bottom);
  void Union(const RectF &rect);
  bool Intersect(const RectF &rect);
  bool Intersect(float left, float top, float right, float bottom);
  bool Contains(float x, float y);

 private:
  float left_ = 0, top_ = 0, right_ = 0, bottom_ = 0;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_MODEL_RECT_MODEL_H_
