// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/render/skity_util.h"

namespace lynx {
namespace animax {

skity::Rect SkityUtil::MakeSkityRect(const RectF &rec) {
  return skity::Rect::MakeLTRB(rec.GetLeft(), rec.GetTop(), rec.GetRight(),
                               rec.GetBottom());
}

}  // namespace animax
}  // namespace lynx
