// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/model/rect_model.h"

namespace lynx {
namespace animax {

float RectF::GetWidth() const { return right_ - left_; }
float RectF::GetHeight() const { return bottom_ - top_; }
float RectF::GetLeft() const { return left_; }
float RectF::GetRight() const { return right_; }
float RectF::GetTop() const { return top_; }
float RectF::GetBottom() const { return bottom_; }
bool RectF::IsEmpty() const { return left_ >= right_ || top_ >= bottom_; }

void RectF::Set(float left, float top, float right, float bottom) {
  left_ = left;
  top_ = top;
  right_ = right;
  bottom_ = bottom;
}

void RectF::Set(const RectF &rect) {
  left_ = rect.GetLeft();
  top_ = rect.GetTop();
  right_ = rect.GetRight();
  bottom_ = rect.GetBottom();
}

void RectF::Union(float left, float top, float right, float bottom) {
  if ((left < right) && (top < bottom)) {
    if ((left_ < right_) && (top_ < bottom_)) {
      if (left_ > left) left_ = left;
      if (top_ > top) top_ = top;
      if (right_ < right) right_ = right;
      if (bottom_ < bottom) bottom_ = bottom;
    } else {
      left_ = left;
      top_ = top;
      right_ = right;
      bottom_ = bottom;
    }
  }
}

void RectF::Union(const RectF &rect) {
  Union(rect.left_, rect.top_, rect.right_, rect.bottom_);
}

bool RectF::Intersect(const RectF &rect) {
  return Intersect(rect.left_, rect.top_, rect.right_, rect.bottom_);
}

bool RectF::Intersect(float left, float top, float right, float bottom) {
  if (left_ < right && left < right_ && top_ < bottom && top < bottom_) {
    if (left_ < left) {
      left_ = left;
    }
    if (top_ < top) {
      top_ = top;
    }
    if (right_ > right) {
      right_ = right;
    }
    if (bottom_ > bottom) {
      bottom_ = bottom;
    }
    return true;
  }
  return false;
}

bool RectF::Contains(float x, float y) {
  return x >= left_ && x <= right_ && y >= top_ && y <= bottom_;
}

}  // namespace animax
}  // namespace lynx
