// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_INCLUDE_PLAYER_ANIMAX_SURFACE_H_
#define ANIMAX_INCLUDE_PLAYER_ANIMAX_SURFACE_H_

#include <cstdint>
#include <memory>

#include "base/include/closure.h"
#include "include/base/macros.h"

namespace skity {
class Canvas;
class GPUContext;
}  // namespace skity
namespace lynx {
namespace animax {
class Canvas;
class Content;

/// enum type to specify AnimaX backend type
enum class AnimaXBackend {
  /// GPU renderer using OpenGL/GLES API
  kGL,
  /// GPU renderer using Metal API
  /// Only available in iOS or MacOS
  kMetal,
  /// GPU renderer using Vulkan API
  /// Only available in Android
  kVulkan,
  /// CPU renderer
  kSoftware,
  /// GPU renderer using WebGPU API
  /// Only available in browser environment
  kWebGPU,
};

class ANIMAX_EXPORT AnimaXSurface {
 public:
  AnimaXSurface(int32_t width, int32_t height)
      : width_{width}, height_{height} {}

  virtual ~AnimaXSurface() = default;
  virtual void Flush() = 0;
  virtual Canvas* Canvas() = 0;
  virtual AnimaXBackend Type() const = 0;

  int32_t Width() const { return width_; }

  int32_t Height() const { return height_; }

  virtual bool Valid() const = 0;

 protected:
  void Resize(float width, float height) {
    width_ = width;
    height_ = height;
  };
  static std::unique_ptr<animax::Canvas> CreateAnimaXCanvas(
      skity::Canvas* canvas, int32_t width, int32_t height,
      skity::GPUContext* context);

 private:
  int32_t width_;
  int32_t height_;
};

using SurfaceCreationFactory =
    base::MoveOnlyClosure<std::unique_ptr<AnimaXSurface>>;

using SurfaceUpdateFactory =
    base::MoveOnlyClosure<std::unique_ptr<AnimaXSurface>,
                          std::unique_ptr<AnimaXSurface>>;

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_INCLUDE_PLAYER_ANIMAX_SURFACE_H_
