// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/base/util/object_fit_util.h"

#include <algorithm>

namespace lynx {
namespace animax {

ObjectFitLayoutResult CalculateObjectFitLayout(float container_width,
                                               float container_height,
                                               float content_width,
                                               float content_height,
                                               ObjectFit object_fit,
                                               ObjectPosition object_position) {
  ObjectFitLayoutResult result;
  if (container_width <= 0 || container_height <= 0 || content_width <= 0 ||
      content_height <= 0) {
    return result;
  }

  switch (object_fit) {
    case ObjectFit::kCover: {
      float scale = std::max(container_width / content_width,
                             container_height / content_height);
      result.scale_x = scale;
      result.scale_y = scale;
      break;
    }
    case ObjectFit::kContain: {
      float scale = std::min(container_width / content_width,
                             container_height / content_height);
      result.scale_x = scale;
      result.scale_y = scale;
      break;
    }
    case ObjectFit::kFill:
      result.scale_x = container_width / content_width;
      result.scale_y = container_height / content_height;
      break;
    case ObjectFit::kScaleDown: {
      float scale = std::min(1.f, std::min(container_width / content_width,
                                           container_height / content_height));
      result.scale_x = scale;
      result.scale_y = scale;
      break;
    }
    default:
      break;
  }

  float dst_width = content_width * result.scale_x;
  float dst_height = content_height * result.scale_y;
  float dx = (container_width - dst_width) / 2.f;
  float dy = (container_height - dst_height) / 2.f;

  switch (object_position) {
    case ObjectPosition::kLeft:
      dx = 0.f;
      break;
    case ObjectPosition::kRight:
      dx = container_width - dst_width;
      break;
    case ObjectPosition::kTop:
      dy = 0.f;
      break;
    case ObjectPosition::kBottom:
      dy = container_height - dst_height;
      break;
    case ObjectPosition::kTopLeft:
      dx = 0.f;
      dy = 0.f;
      break;
    case ObjectPosition::kTopRight:
      dx = container_width - dst_width;
      dy = 0.f;
      break;
    case ObjectPosition::kBottomLeft:
      dx = 0.f;
      dy = container_height - dst_height;
      break;
    case ObjectPosition::kBottomRight:
      dx = container_width - dst_width;
      dy = container_height - dst_height;
      break;
    case ObjectPosition::kCenter:
    default:
      break;
  }

  result.rect = RectF(dx, dy, dx + dst_width, dy + dst_height);
  return result;
}

}  // namespace animax
}  // namespace lynx
