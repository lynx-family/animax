// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RENDER_PATH_MEASURE_H_
#define ANIMAX_SRC_RENDER_PATH_MEASURE_H_

#include "src/render/path.h"

namespace skity {
class PathMeasure;
}
namespace lynx {
namespace animax {
class PointF;
class PathMeasure {
 public:
  PathMeasure();
  ~PathMeasure();

  void SetPath(Path& path, bool force_close);

  bool NextContour();

  float GetLength();

  bool GetPosTan(float distance, PointF* out_pos);

  bool GetSegment(float start, float stop, Path& dst, bool start_with_move_to);

 private:
  std::unique_ptr<skity::PathMeasure> pm_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RENDER_PATH_MEASURE_H_
