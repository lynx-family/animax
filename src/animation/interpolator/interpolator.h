// Copyright 2023 The Lynx Authors. All rights reserved.

#ifndef ANIMAX_SRC_ANIMATION_INTERPOLATOR_INTERPOLATOR_H_
#define ANIMAX_SRC_ANIMATION_INTERPOLATOR_INTERPOLATOR_H_

#include "src/model/value/base_value.h"

namespace lynx {
namespace animax {

class Interpolator {
 public:
  virtual ~Interpolator() = default;
  virtual float GetInterpolation(float input) = 0;
};

class LinearInterpolator : public Interpolator {
 public:
  static std::unique_ptr<Interpolator> Make();
  ~LinearInterpolator() override = default;
  float GetInterpolation(float input) override;
};

class PathInterpolator : public Interpolator {
 public:
  static std::unique_ptr<Interpolator> Make(PointF &cp1, PointF cp2);

  PathInterpolator(const float cp1_x, const float cp1_y, const float cp2_x,
                   const float cp2_y);
  ~PathInterpolator() override { Destroy(); }
  float GetInterpolation(float input) override;

 private:
  /**
   * Interpolate the point on line based on (p1_x, p1_y) and (p2_x, p2_y).
   * vector from (p1_x, p1_y) to (p_x, p2) is in the same direction with origin
   * vector from (p1_x, p1_y) to (p2_x, p2_y), and the length is equal to origin
   * length multiplied with t.
   * @param p1_x x of begin point.
   * @param p1_y y of begin point.
   * @param p2_x x of end point.
   * @param p2_y y of end point.
   * @param t    multiplier of length.
   * @param p_x  x of computed point.
   * @param p_y  y of computed point.
   */
  void Mix(float p1_x, float p1_y, float p2_x, float p2_y, float t, float &p_x,
           float &p_y);
  /**
   * Build cubic Bezier curve.
   * Start point:              (    0,     0)
   * The first control point:  (cp1_x, cp1_y)
   * The second control point: (cp2_x, cp2_y)
   * End point:                (    1,     1)
   * @param cp1_x x of the first control point.
   * @param cp1_y y of the first control point.
   * @param cp2_x x of the second control point.
   * @param cp2_y y of the second control point.
   */
  void Build(const float cp1_x, const float cp1_y, const float cp2_x,
             const float cp2_y);
  void Destroy() { n_ = 0; }
  bool IsValid() { return !!n_; }
  std::unique_ptr<float[]> x_;
  std::unique_ptr<float[]> y_;
  size_t n_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_ANIMATION_INTERPOLATOR_INTERPOLATOR_H_
