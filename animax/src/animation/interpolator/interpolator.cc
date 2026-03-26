// Copyright 2023 The Lynx Authors. All rights reserved.

#include "src/animation/interpolator/interpolator.h"

namespace lynx {
namespace animax {

std::unique_ptr<Interpolator> LinearInterpolator::Make() {
  return std::unique_ptr<LinearInterpolator>(new LinearInterpolator());
}

float LinearInterpolator::GetInterpolation(float input) {
  if (input <= 0) {
    return 0;
  } else if (input >= 1) {
    return 1;
  }

  return input;
}

std::unique_ptr<Interpolator> PathInterpolator::Make(PointF &cp1, PointF cp2) {
  return std::unique_ptr<PathInterpolator>(
      new PathInterpolator(cp1.GetX(), cp1.GetY(), cp2.GetX(), cp2.GetY()));
}

PathInterpolator::PathInterpolator(const float cp1_x, const float cp1_y,
                                   const float cp2_x, const float cp2_y) {
  Build(cp1_x, cp1_y, cp2_x, cp2_y);
}

float PathInterpolator::GetInterpolation(float input) {
  if (input <= 0) {
    return 0;
  } else if (input >= 1) {
    return 1;
  } else if (!IsValid()) {
    // degrade to LinearInterpolator
    return input;
  }
  size_t beg = 0;
  size_t end = n_ - 1;
  while (end - beg > 1) {
    size_t mid = beg + ((end - beg) >> 1);
    if (input < x_[mid]) {
      end = mid;
    } else {
      beg = mid;
    }
  }
  float output =
      y_[beg] + (y_[end] - y_[beg]) * (input - x_[beg]) / (x_[end] - x_[beg]);
  return output;
}

void PathInterpolator::Mix(float p1_x, float p1_y, float p2_x, float p2_y,
                           float t, float &p_x, float &p_y) {
  p_x = (1.f - t) * p1_x + t * p2_x;
  p_y = (1.f - t) * p1_y + t * p2_y;
}

void PathInterpolator::Build(const float cp1_x, const float cp1_y,
                             const float cp2_x, const float cp2_y) {
  static constexpr float precision = 0.02f;
  n_ = 1.f / precision + 1;
  static_assert(((size_t)(1.f / precision)) * precision == 1.f,
                "PathInterpolator use wrong precision");
  x_ = std::make_unique<float[]>(n_);
  y_ = std::make_unique<float[]>(n_);
  float t1_x, t1_y, t2_x, t2_y, t3_x, t3_y, progress;
  for (size_t i = 0; i < n_; ++i) {
    progress = precision * i;
    // Step1: compute temporary point between Start point and CP1 to (t1_x,
    // t1_y)
    Mix(0.f, 0.f, cp1_x, cp1_y, progress, t1_x, t1_y);
    // Step2: compute temporary point between CP1 and CP2 to (t2_x, t2_y)
    Mix(cp1_x, cp1_y, cp2_x, cp2_y, progress, t2_x, t2_y);
    // Step3: compute temporary point between points of Step1 and Step2 to
    // (t1_x, t1_y)
    Mix(t1_x, t1_y, t2_x, t2_y, progress, t1_x, t1_y);
    // Step4: compute temporary point between CP2 and End point to (t3_x,
    // t3_y)
    Mix(cp2_x, cp2_y, 1.f, 1.f, progress, t3_x, t3_y);
    // Step5: compute temporary point between points of Step2 and Step4 to
    // (t2_x, t2_y)
    Mix(t2_x, t2_y, t3_x, t3_y, progress, t2_x, t2_y);
    // Step6: compute temporary point between points of Step3 and Step5 to
    // (t2_x, t2_y), this is the point what we want.
    Mix(t1_x, t1_y, t2_x, t2_y, progress, t1_x, t1_y);
    x_[i] = t1_x;
    y_[i] = t1_y;
    if (i > 0 && x_[i] <= x_[i - 1]) {
      // This curve isn't injective, thus the PathInterpolator is invalid
      Destroy();
      break;
    }
  }
}

}  // namespace animax
}  // namespace lynx
