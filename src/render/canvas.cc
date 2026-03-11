// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/render/canvas.h"

#include "skity/effect/mask_filter.hpp"
#include "skity/render/canvas.hpp"
#include "skity/text/text_blob.hpp"
#include "src/model/value/base_value.h"
#include "src/render/mask_filter.h"
#include "src/render/skity_util.h"

namespace lynx {
namespace animax {

Canvas::Canvas(skity::Canvas *canvas, int32_t width, int32_t height,
               skity::GPUContext *context)
    : canvas_(canvas), width_(width), height_(height) {
  if (context) {
    real_context_ = std::unique_ptr<RealContext>(new RealContext(context));
  }
}

void Canvas::SaveLayer(const RectF &rect, Paint &paint) {
  paint.SetAntiAlias(true);
  canvas_->SaveLayer(SkityUtil::MakeSkityRect(rect), paint.GetPaint());
}

void Canvas::DrawPath(Path &path, Paint &paint) {
  auto *shadow_layer = paint.GetShadowLayer();

  if (shadow_layer) {
    skity::Paint wp{paint.GetPaint()};
    // clear gradient
    wp.SetShader(nullptr);

    wp.SetMaskFilter(skity::MaskFilter::MakeBlur(skity::BlurStyle::kNormal,
                                                 shadow_layer->radius));
    Color color(shadow_layer->color);
    wp.SetColor(skity::ColorSetARGB(color.GetA(), color.GetR(), color.GetG(),
                                    color.GetB()));
    canvas_->Save();
    canvas_->Translate(shadow_layer->off_x, shadow_layer->off_y);
    canvas_->DrawPath(path.GetPath(), wp);
    canvas_->Restore();
  }

  canvas_->DrawPath(path.GetPath(), paint.GetPaint());
}

void Canvas::DrawImageRect(Image &image, const RectF &src, const RectF &dst,
                           Paint &paint) {
  // In software rendering mode, 'FilterMode::Linear' samples four times,
  // whereas 'FilterMode::Nearest' samples only once and includes NEON
  // acceleration, making it the preferred choice.
  auto filter_mode =
      real_context_ ? skity::FilterMode::kLinear : skity::FilterMode::kNearest;

  canvas_->DrawImage(image.GetImage(), SkityUtil::MakeSkityRect(dst),
                     {filter_mode, skity::MipmapMode::kNone},
                     &paint.GetPaint());
}

void Canvas::DrawRect(const RectF &rect, Paint &paint) {
  auto *shadow_layer = paint.GetShadowLayer();

  auto skity_rect = SkityUtil::MakeSkityRect(rect);

  if (shadow_layer) {
    skity::Paint wp{paint.GetPaint()};

    wp.SetMaskFilter(skity::MaskFilter::MakeBlur(skity::BlurStyle::kNormal,
                                                 shadow_layer->radius));

    Color color(shadow_layer->color);
    wp.SetColor(skity::ColorSetARGB(color.GetA(), color.GetR(), color.GetG(),
                                    color.GetB()));
    canvas_->Save();
    canvas_->Translate(shadow_layer->off_x, shadow_layer->off_y);
    canvas_->DrawRect(skity_rect, wp);
    canvas_->Restore();
  }

  canvas_->DrawRect(skity_rect, paint.GetPaint());
}

void Canvas::DrawText(const std::string &text, float x, float y, Font &font,
                      Paint &paint) {
  auto skity_paint = paint.GetPaint();
  auto skity_font = font.GetFont();

  auto delegate = Font::MakeFallbackDelegate();

  skity::TextBlobBuilder builder;

  skity_paint.SetTextSize(skity_font.GetSize());
  skity_paint.SetTypeface(skity_font.GetTypeface());

  auto blob = builder.BuildTextBlob(text.c_str(), skity_paint, delegate.get());

  canvas_->DrawTextBlob(blob, x, y, skity_paint);
}

void Canvas::Save() { canvas_->Save(); }

void Canvas::ResetMatrix() { canvas_->ResetMatrix(); }

void Canvas::Concat(Matrix &matrix) { canvas_->Concat(matrix.GetMatrix()); }

void Canvas::Restore() { canvas_->Restore(); }

bool Canvas::ClipRect(const RectF &rect) {
  // some clip may cause content not rendered
  // and most animax content does not need to be clip
  canvas_->ClipRect(SkityUtil::MakeSkityRect(rect));
  return true;
}

void Canvas::Scale(float x, float y) { canvas_->Scale(x, y); }

void Canvas::Translate(float x, float y) { canvas_->Translate(x, y); }

std::unique_ptr<Matrix> Canvas::GetMatrix() const {
  return std::unique_ptr<Matrix>(new Matrix(canvas_->GetTotalMatrix()));
}

RealContext *Canvas::GetRealContext() const {
  return real_context_ ? real_context_.get() : nullptr;
}

skity::Canvas *Canvas::GetSkityCanvas() { return canvas_; }

}  // namespace animax
}  // namespace lynx
