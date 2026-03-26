// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RENDER_SURFACE_H_
#define ANIMAX_SRC_RENDER_SURFACE_H_

#include <cstdint>

namespace lynx {
namespace animax {
class Canvas;
class Surface {
 public:
  virtual ~Surface() = default;

  virtual Canvas *GetCanvas() = 0;

  virtual void Clear() = 0;
  virtual void Flush() = 0;

  virtual void Destroy() = 0;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RENDER_SURFACE_H_
