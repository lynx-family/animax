// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RENDER_PAINT_H_
#define ANIMAX_SRC_RENDER_PAINT_H_

#include <memory>

#include "src/model/value/blend_mode_type.h"
#include "src/render/color_filter.h"
#include "src/render/path_effect.h"

namespace skity {
class Paint;
}
namespace lynx {
namespace animax {

enum class PaintStyle : uint8_t {
  kFill = 0,
  kStroke,
  kFillAddStroke,
  kStrokeThenFill
};

enum class PaintCap : uint8_t { kButt = 0, kRound, kSquare };

enum class PaintJoin : uint8_t { kMiter = 0, kRound, kBevel };

struct SkityShadowLayer {
  float radius;
  float off_x;
  float off_y;
  int32_t color;

  SkityShadowLayer(float radius, float offX, float offY, int32_t color)
      : radius(radius), off_x(offX), off_y(offY), color(color) {}
};

class Shader;
class MaskFilter;
class DashPathEffect;

class Paint {
 public:
  Paint();
  ~Paint();

  void SetAntiAlias(bool anti_alias);

  void SetAlpha(float alpha);

  void SetColor(const Color &color);

  void SetColorFilter(ColorFilter &filter);

  void SetStyle(PaintStyle style);

  void SetFillColor(const Color &color);

  void SetStrokeColor(const Color &color);

  void SetStrokeCap(PaintCap cap);

  void SetStrokeJoin(PaintJoin join);

  void SetStrokeMiter(float miter);

  void SetStrokeWidth(float width);

  void SetBlendMode(BlendModeType mode);

  void SetShader(Shader *shader);

  void SetShadowLayer(float radius, float x, float y, int32_t color);

  void SetMaskFilter(MaskFilter *filter);

  void SetDashPathEffect(DashPathEffect &effect);

  void SetFontThreshold(float font_size);

  float GetStrokeWidth() const;

  const skity::Paint &GetPaint() const;

  SkityShadowLayer *GetShadowLayer() const { return shadow_layer_.get(); }

 private:
  std::unique_ptr<SkityShadowLayer> shadow_layer_ = {};
  std::unique_ptr<skity::Paint> paint_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RENDER_PAINT_H_
