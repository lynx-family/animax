// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/render/shader.h"

#include <vector>

#include "skity/effect/shader.hpp"
#include "skity/graphic/color.hpp"

namespace lynx {
namespace animax {
namespace {
static skity::TileMode ConvertToSkityMode(ShaderTileMode mode) {
  switch (mode) {
    case ShaderTileMode::kClamp:
      return skity::TileMode::kClamp;
    case ShaderTileMode::kDecal:
      return skity::TileMode::kDecal;
    case ShaderTileMode::kMirror:
      return skity::TileMode::kMirror;
    case ShaderTileMode::kRepeat:
      return skity::TileMode::kRepeat;
    default:
      return skity::TileMode::kClamp;
  }
}
}  // namespace

std::unique_ptr<Shader> Shader::MakeLinear(const PointF &sp, const PointF &ep,
                                           int32_t size, int32_t *colors,
                                           float *pos, ShaderTileMode mode,
                                           Matrix &matrix) {
  std::vector<skity::Point> pts = {
      skity::Point{sp.GetX(), sp.GetY(), 0.f, 1.f},
      skity::Point{ep.GetX(), ep.GetY(), 0.f, 1.f},
  };

  std::vector<skity::Color4f> skity_colors;

  for (int32_t i = 0; i < size; i++) {
    skity_colors.emplace_back(skity::Color4fFromColor(colors[i]));
  }

  auto skity_mode = ConvertToSkityMode(mode);

  auto ret = std::make_unique<Shader>(skity::Shader::MakeLinear(
      pts.data(), skity_colors.data(), pos, size, skity_mode));

  auto &shader = ret->GetShader();
  if (shader) {
    shader->SetLocalMatrix(matrix.GetMatrix());
  }
  return ret;
}

std::unique_ptr<Shader> Shader::MakeRadial(const PointF &sp, float r,
                                           int32_t size, int32_t *colors,
                                           float *pos, ShaderTileMode mode,
                                           Matrix &matrix) {
  auto skity_mode = ConvertToSkityMode(mode);

  skity::Point center{sp.GetX(), sp.GetY(), 0.f, 1.f};

  std::vector<skity::Color4f> skity_colors;

  for (int32_t i = 0; i < size; i++) {
    skity_colors.emplace_back(skity::Color4fFromColor(colors[i]));
  }

  auto ret = std::make_unique<Shader>(skity::Shader::MakeRadial(
      center, r, skity_colors.data(), pos, size, skity_mode));

  auto &shader = ret->GetShader();
  if (shader) {
    shader->SetLocalMatrix(matrix.GetMatrix());
  }

  return ret;
}

std::unique_ptr<Shader> Shader::MakeTwoPointConical(
    PointF const &start, float start_radius, PointF const &end,
    float end_radius, int32_t size, int32_t *colors, float *positions,
    ShaderTileMode mode, Matrix &matrix) {
  auto skity_mode = ConvertToSkityMode(mode);

  skity::Point start_point{start.GetX(), start.GetY(), 0.f, 1.f};
  skity::Point end_point{end.GetX(), end.GetY(), 0.f, 1.f};

  std::vector<skity::Color4f> skity_colors;

  for (int32_t i = 0; i < size; i++) {
    skity_colors.emplace_back(skity::Color4fFromColor(colors[i]));
  }

  auto ret = std::make_unique<Shader>(skity::Shader::MakeTwoPointConical(
      start_point, start_radius, end_point, end_radius, skity_colors.data(),
      positions, size, skity_mode));

  auto &shader = ret->GetShader();
  if (shader) {
    shader->SetLocalMatrix(matrix.GetMatrix());
  }

  return ret;
}

}  // namespace animax
}  // namespace lynx
