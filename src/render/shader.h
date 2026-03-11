// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RENDER_SHADER_H_
#define ANIMAX_SRC_RENDER_SHADER_H_

#include "src/model/value/base_value.h"
#include "src/render/matrix.h"

namespace skity {
class Shader;
}
namespace lynx {
namespace animax {

enum class ShaderTileMode : uint8_t { kClamp = 0, kRepeat, kMirror, kDecal };

class Shader {
 public:
  static std::unique_ptr<Shader> MakeLinear(PointF const& sp, PointF const& ep,
                                            int32_t size, int32_t* colors,
                                            float* positions,
                                            ShaderTileMode mode,
                                            Matrix& matrix);

  static std::unique_ptr<Shader> MakeRadial(PointF const& sp, float r,
                                            int32_t size, int32_t* colors,
                                            float* positions,
                                            ShaderTileMode mode,
                                            Matrix& matrix);

  static std::unique_ptr<Shader> MakeTwoPointConical(
      PointF const& start, float start_radius, PointF const& end,
      float end_radius, int32_t size, int32_t* colors, float* positions,
      ShaderTileMode mode, Matrix& matrix);

  explicit Shader(std::shared_ptr<skity::Shader> shader)
      : shader_(std::move(shader)) {}

  ~Shader() = default;

  std::shared_ptr<skity::Shader> const& GetShader() const { return shader_; }

 private:
  std::shared_ptr<skity::Shader> shader_ = {};
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RENDER_SHADER_H_
