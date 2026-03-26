// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/layer/base_layer.h"

#include <algorithm>
#include <memory>

#include "src/base/log/log.h"
#include "src/layer/alpha_video_layer.h"
#include "src/layer/composition_layer.h"
#include "src/layer/image_layer.h"
#include "src/layer/null_layer.h"
#include "src/layer/shape_layer.h"
#include "src/layer/solid_layer.h"
#include "src/layer/text_layer.h"
#include "src/model/animatable/animatable_transform_model.h"
#include "src/property/key_path_resolve_util.h"

namespace lynx {
namespace animax {

BaseLayer::BaseLayer(LayerModel& layer_model, CompositionModel& model)
    : layer_model_(layer_model),
      composition_(model),
      matrix_(std::make_unique<Matrix>()),
      parent_matrix_(std::make_unique<Matrix>()),
      path_(std::make_unique<Path>()),
      bounds_matrix_(std::make_unique<Matrix>()),
      content_paint_(std::make_unique<Paint>()),
      dst_in_paint_(std::make_unique<Paint>()),
      dst_out_paint_(std::make_unique<Paint>()),
      matte_paint_(std::make_unique<Paint>()),
      clear_paint_(std::make_unique<Paint>()) {
  content_paint_->SetAntiAlias(true);

  dst_in_paint_->SetAntiAlias(true);
  dst_in_paint_->SetXfermode(PaintXfermode::kDstIn);

  dst_out_paint_->SetAntiAlias(true);
  dst_out_paint_->SetXfermode(PaintXfermode::kDstOut);

  matte_paint_->SetAntiAlias(true);

  clear_paint_->SetXfermode(PaintXfermode::kClear);
}

void BaseLayer::Init() {
  if (layer_model_.GetMatteType() == MatteType::kAlphaInverted) {
    matte_paint_->SetXfermode(PaintXfermode::kDstOut);
  } else {
    matte_paint_->SetXfermode(PaintXfermode::kDstIn);
  }

  transform_ = layer_model_.GetTransform()->CreateAnimation();
  transform_->AddAnimationToLayer(*this);
  transform_->AddListener(this);

  auto& masks = layer_model_.GetMasks();
  if (!masks.empty()) {
    mask_ = std::unique_ptr<MaskKeyframeAnimation>(
        new MaskKeyframeAnimation(masks));
    for (auto& animation : mask_->GetMaskAnimations()) {
      animation->AddUpdateListener(this);
    }
    for (auto& animation : mask_->GetOpacityAnimations()) {
      AddAnimation(animation.get());
      animation->AddUpdateListener(this);
    }
  }

  SetUpInOutAnimations();
}

void BaseLayer::SetUpInOutAnimations() {
  if (!layer_model_.GetInOutFrames().empty()) {
    in_out_animation_ = std::unique_ptr<FloatKeyframeAnimation>(
        new FloatKeyframeAnimation(layer_model_.GetSharedInOutFrames()));
    in_out_animation_->SetIsDiscrete();
    in_out_listener_ = std::unique_ptr<InOutAnimationListener>(
        new InOutAnimationListener(*this));
    in_out_animation_->AddUpdateListener(in_out_listener_.get());
    SetVisible(in_out_animation_->GetValue().Get() == 1.0);
    AddAnimation(in_out_animation_.get());
  } else {
    SetVisible(true);
  }
}

void BaseLayer::Draw(Canvas& canvas, Matrix& parent_matrix,
                     int32_t parent_alpha) {
  if (!visible_ || layer_model_.IsHidden()) {
    return;
  }

  BuildParentLayerListIfNeeded();

  matrix_->Set(parent_matrix);

  for (auto it = parent_layers_.rbegin(); it != parent_layers_.rend(); it++) {
    matrix_->PreConcat((*it)->transform_->GetMatrix());
  }

  parent_matrix_->Set(*matrix_);

  int32_t opacity = 100;
  auto opacity_animation = transform_->GetOpacity();
  if (opacity_animation) {
    auto opacity_value = opacity_animation->GetValue().Get();
    if (opacity_value != Integer::kMin) {
      opacity = opacity_value;
    }
  }

  auto alpha = (parent_alpha / 255.0 * (opacity / 100.0) * 255.0);
  if (!HasMatteOnThisLayer() && !HasMaskOnThisLayer()) {
    matrix_->PreConcat(transform_->GetMatrix());
    DrawLayer(canvas, *matrix_, alpha);
    return;
  }

  GetBounds(rect_, *matrix_, false);

  IntersectBoundsWithMatte(rect_, parent_matrix);

  matrix_->PreConcat(transform_->GetMatrix());
  IntersectBoundsWithMask(rect_, *matrix_);

  canvas_bounds_.Set(0, 0, canvas.GetWidth(), canvas.GetHeight());
  canvas_matrix_ = canvas.GetMatrix();

  if (!canvas_matrix_->IsIdentity()) {
    canvas_matrix_->Invert(*canvas_matrix_);
    canvas_matrix_->MapRect(canvas_bounds_);
  }
  if (!rect_.Intersect(canvas_bounds_)) {
    rect_.Set(0, 0, 0, 0);
  }

  if (rect_.GetWidth() >= 1 && rect_.GetHeight() >= 1) {
    content_paint_->SetAlpha(255);
    canvas.SaveLayer(rect_, *content_paint_);

    ClearCanvas(canvas);

    DrawLayer(canvas, *matrix_, alpha);

    if (HasMaskOnThisLayer()) {
      ApplyMasks(canvas, *matrix_);
    }

    if (HasMatteOnThisLayer()) {
      canvas.SaveLayer(rect_, *matte_paint_);

      ClearCanvas(canvas);
      matte_layer_->Draw(canvas, parent_matrix, alpha);
      canvas.Restore();
    }

    canvas.Restore();
  }

  if (outline_maks_and_mattes_ && outline_masks_and_mattes_paint_) {
    // stroke
    outline_masks_and_mattes_paint_->SetStyle(PaintStyle::kStroke);
    outline_masks_and_mattes_paint_->SetColor(Color(0xFFFC2803));
    outline_masks_and_mattes_paint_->SetStrokeWidth(4);
    canvas.DrawRect(rect_, *outline_masks_and_mattes_paint_);

    // fill
    outline_masks_and_mattes_paint_->SetStyle(PaintStyle::kFill);
    outline_masks_and_mattes_paint_->SetColor(Color(0x50EBEBEB));
    canvas.DrawRect(rect_, *outline_masks_and_mattes_paint_);
  }
}

void BaseLayer::ApplyMasks(Canvas& canvas, Matrix& matrix) {
  canvas.SaveLayer(rect_, *dst_in_paint_);

  auto& masks = mask_->GetMasks();
  for (auto i = 0; i < masks.size(); i++) {
    auto& mask = masks[i];
    auto& mask_animation = *mask_->GetMaskAnimations()[i];
    auto& opacity_animation = *mask_->GetOpacityAnimations()[i];
    switch (mask->mask_mode_) {
      case MaskMode::kNone:
        if (AreAllMasksNone()) {
          content_paint_->SetAlpha(255);
          canvas.DrawRect(rect_, *content_paint_);
        }
        break;
      case MaskMode::kAdd:
        if (mask->inverted_) {
          ApplyInvertedAddMask(canvas, matrix, mask_animation,
                               opacity_animation);
        } else {
          ApplyAddMask(canvas, matrix, mask_animation, opacity_animation);
        }
        break;
      case MaskMode::kSubtract:
        if (i == 0) {
          content_paint_->SetColor(Color(255, 0, 0, 0));
          content_paint_->SetAlpha(255);
          canvas.DrawRect(rect_, *content_paint_);
        }
        if (mask->inverted_) {
          ApplyInvertedSubtractMask(canvas, matrix, mask_animation,
                                    opacity_animation);
        } else {
          ApplySubtractMask(canvas, matrix, mask_animation, opacity_animation);
        }
        break;
      case MaskMode::kIntersect:
        if (mask->inverted_) {
          ApplyInvertedIntersectMask(canvas, matrix, mask_animation,
                                     opacity_animation);
        } else {
          ApplyIntersectMask(canvas, matrix, mask_animation, opacity_animation);
        }
        break;
    }
  }
  canvas.Restore();
}

bool BaseLayer::AreAllMasksNone() {
  if (mask_->GetMaskAnimations().empty()) {
    return false;
  }
  for (auto& mask : mask_->GetMasks()) {
    if (mask->mask_mode_ != MaskMode::kNone) {
      return false;
    }
  }
  return true;
}

void BaseLayer::GetBounds(RectF& out_bounds, Matrix& parent_matrix,
                          bool apply_parent) {
  rect_.Set(0, 0, 0, 0);
  BuildParentLayerListIfNeeded();
  bounds_matrix_->Set(parent_matrix);

  if (apply_parent) {
    if (!parent_layers_.empty()) {
      for (auto it = parent_layers_.rbegin(); it != parent_layers_.rend();
           it++) {
        bounds_matrix_->PreConcat((*it)->transform_->GetMatrix());
      }
    } else if (parent_layer_) {
      bounds_matrix_->PreConcat(parent_layer_->transform_->GetMatrix());
    }
  }

  bounds_matrix_->PreConcat(transform_->GetMatrix());
}

void BaseLayer::SetVisible(bool visible) {
  if (visible_ != visible) {
    visible_ = visible;
  }
}

void BaseLayer::SetProgress(float progress) {
  transform_->SetProgress(progress);

  if (mask_) {
    for (auto& animation : mask_->GetMaskAnimations()) {
      animation->SetProgress(progress);
    }
  }

  if (in_out_animation_) {
    in_out_animation_->SetProgress(progress);
  }

  if (matte_layer_) {
    matte_layer_->SetProgress(progress);
  }

  for (auto& animation : animations_) {
    animation->SetProgress(progress);
  }
}

void BaseLayer::BuildParentLayerListIfNeeded() {
  if (parent_layer_ == nullptr) {
    return;
  }

  if (!parent_layers_.empty()) {
    return;
  }

  auto layer = parent_layer_;
  while (layer) {
    parent_layers_.push_back(layer);
    layer = layer->parent_layer_;
  }
}

BlurEffectModel* BaseLayer::GetBlurEffect() {
  auto effect = layer_model_.GetBlurEffect();
  if (effect) {
    return effect;
  }
  if (composition_layer_) {
    return composition_layer_->GetBlurEffect();
  }
  return nullptr;
}

DropShadowEffectModel* BaseLayer::GetDropEffect() {
  auto* effect = layer_model_.GetDropEffect();
  if (effect) {
    return effect;
  }
  if (composition_layer_) {
    return composition_layer_->GetDropEffect();
  }
  return nullptr;
}

void BaseLayer::IntersectBoundsWithMatte(RectF& rect, Matrix& matrix) {
  if (!HasMatteOnThisLayer()) {
    return;
  }

  if (layer_model_.GetMatteType() == MatteType::kAlphaInverted) {
    return;
  }

  matte_bounds_rect_.Set(0, 0, 0, 0);
  matte_layer_->GetBounds(matte_bounds_rect_, matrix, true);
  bool intersect = rect.Intersect(matte_bounds_rect_);
  if (!intersect) {
    rect.Set(0, 0, 0, 0);
  }
}

void BaseLayer::IntersectBoundsWithMask(RectF& rect, Matrix& matrix) {
  mask_bounds_rect_.Set(0, 0, 0, 0);
  if (!HasMaskOnThisLayer()) {
    return;
  }
  auto& masks = mask_->GetMasks();
  for (auto i = 0; i < masks.size(); i++) {
    auto& mask = masks[i];
    auto& mask_animation = mask_->GetMaskAnimations()[i];
    auto& mask_path = mask_animation->GetValue();
    if (mask_path.IsEmpty()) {
      continue;
    }

    path_->Set(mask_path);
    path_->Transform(matrix);

    switch (mask->mask_mode_) {
      case MaskMode::kNone:
        return;
      case MaskMode::kSubtract:
        return;
      case MaskMode::kIntersect:
      case MaskMode::kAdd:
        if (mask->inverted_) {
          return;
        }
      default:
        path_->ComputeBounds(temp_mask_bounds_rect_, false);
        if (i == 0) {
          mask_bounds_rect_.Set(temp_mask_bounds_rect_);
        } else {
          mask_bounds_rect_.Set(std::min(mask_bounds_rect_.GetLeft(),
                                         temp_mask_bounds_rect_.GetLeft()),
                                std::min(mask_bounds_rect_.GetTop(),
                                         temp_mask_bounds_rect_.GetTop()),
                                std::max(mask_bounds_rect_.GetRight(),
                                         temp_mask_bounds_rect_.GetRight()),
                                std::max(mask_bounds_rect_.GetBottom(),
                                         temp_mask_bounds_rect_.GetBottom()));
        }
    }
  }

  bool intersects = rect.Intersect(mask_bounds_rect_);
  if (!intersects) {
    rect.Set(0, 0, 0, 0);
  }
}

void BaseLayer::ClearCanvas(Canvas& canvas) {
  canvas.DrawRect(rect_.GetLeft() - 1, rect_.GetTop() - 1, rect_.GetRight() + 1,
                  rect_.GetBottom() + 1, *clear_paint_);
}

void BaseLayer::ApplyInvertedAddMask(
    Canvas& canvas, Matrix& matrix, ShapeKeyframeAnimation& mask_animation,
    IntegerKeyframeAnimation& opacity_animation) {
  BaseLayer::ApplyInvertedMask(canvas, matrix, mask_animation,
                               opacity_animation, *content_paint_,
                               *content_paint_);
}

void BaseLayer::ApplyAddMask(Canvas& canvas, Matrix& matrix,
                             ShapeKeyframeAnimation& mask_animation,
                             IntegerKeyframeAnimation& opacity_animation) {
  auto& mask_path = mask_animation.GetValue();
  path_->Set(mask_path);
  path_->Transform(matrix);
  content_paint_->SetAlpha(opacity_animation.GetValue().Get() * 2.55);
  canvas.DrawPath(*path_, *content_paint_);
}

void BaseLayer::ApplyInvertedSubtractMask(
    Canvas& canvas, Matrix& matrix, ShapeKeyframeAnimation& mask_animation,
    IntegerKeyframeAnimation& opacity_animation) {
  BaseLayer::ApplyInvertedMask(canvas, matrix, mask_animation,
                               opacity_animation, *dst_out_paint_,
                               *dst_out_paint_);
}

void BaseLayer::ApplySubtractMask(Canvas& canvas, Matrix& matrix,
                                  ShapeKeyframeAnimation& mask_animation,
                                  IntegerKeyframeAnimation& opacity_animation) {
  auto& mask_path = mask_animation.GetValue();
  path_->Set(mask_path);
  path_->Transform(matrix);
  canvas.DrawPath(*path_, *dst_out_paint_);
}

void BaseLayer::ApplyInvertedIntersectMask(
    Canvas& canvas, Matrix& matrix, ShapeKeyframeAnimation& mask_animation,
    IntegerKeyframeAnimation& opacity_animation) {
  BaseLayer::ApplyInvertedMask(canvas, matrix, mask_animation,
                               opacity_animation, *dst_in_paint_,
                               *dst_out_paint_);
}

void BaseLayer::ApplyIntersectMask(
    Canvas& canvas, Matrix& matrix, ShapeKeyframeAnimation& mask_animation,
    IntegerKeyframeAnimation& opacity_animation) {
  canvas.SaveLayer(rect_, *dst_in_paint_);
  auto& mask_path = mask_animation.GetValue();
  path_->Set(mask_path);
  path_->Transform(matrix);
  content_paint_->SetAlpha(opacity_animation.GetValue().Get() * 2.55);
  canvas.DrawPath(*path_, *content_paint_);
  canvas.Restore();
}

void BaseLayer::ApplyInvertedMask(Canvas& canvas, Matrix& matrix,
                                  ShapeKeyframeAnimation& mask_animation,
                                  IntegerKeyframeAnimation& opacity_animation,
                                  Paint& save_paint, Paint& alpha_paint) {
  canvas.SaveLayer(rect_, save_paint);
  canvas.DrawRect(rect_, *content_paint_);
  alpha_paint.SetAlpha(opacity_animation.GetValue().Get() * 2.55);
  auto& mask_path = mask_animation.GetValue();
  path_->Set(mask_path);
  path_->Transform(matrix);
  canvas.DrawPath(*path_, *dst_out_paint_);
  canvas.Restore();
}

MaskFilter* BaseLayer::GetBlurMaskFilter(float radius) {
  if (blur_mask_filter_radius_ == radius) {
    return blur_mask_filter_.get();
  }
  blur_mask_filter_ = MaskFilter::MakeBlur(radius / 2.f);
  blur_mask_filter_radius_ = radius;
  return blur_mask_filter_.get();
}

void BaseLayer::ResolveKeyPath(const AnimaXKeyPath& path, int32_t depth,
                               ResolvedKeyPathElements& match_elements,
                               const AnimaXKeyPath& current_partial_path) {
  // Handle matte layer if present
  if (matte_layer_) {
    auto result = KeyPathResolveUtil::ResolveElement(
        *matte_layer_, path, depth, match_elements, current_partial_path);
    if (result.should_propagate_to_child_) {
      matte_layer_->ResolveChildKeyPath(
          path, result.child_depth_, match_elements, result.updated_key_path_);
    }
  }

  // Handle current layer
  auto result = KeyPathResolveUtil::ResolveElement(
      *this, path, depth, match_elements, current_partial_path);
  if (result.should_propagate_to_child_) {
    ResolveChildKeyPath(path, result.child_depth_, match_elements,
                        result.updated_key_path_);
  }
}

void BaseLayer::ResolveChildKeyPath(const AnimaXKeyPath& path, int32_t depth,
                                    ResolvedKeyPathElements& match_elements,
                                    const AnimaXKeyPath& current_partial_path) {
}

PropertyUpdateResult BaseLayer::UpdateLayerProperty(
    PropertyUpdateContext& context) {
  if (context.GetLayerType() == LayerPropertyType::kVisibility) {
    layer_model_.SetHidden(!context.GetValue().GetBooleanValue());
    return PropertyUpdateResult::kSuccess;
  }

  return Content::UpdateLayerProperty(context);
}

KeyframeAnimation* BaseLayer::GetAnimationForProperty(LayerPropertyType type) {
  if (transform_) {
    auto* animation = transform_->GetAnimationForProperty(type);
    if (animation) {
      return animation;
    }
  }
  return Content::GetAnimationForProperty(type);
}

void BaseLayer::HitTest(float x, float y,
                        std::unordered_set<std::string>& hit_layers) {
  if (!visible_ || layer_model_.IsHidden()) {
    return;
  }

  GetBounds(rect_, *parent_matrix_, false);

  if (rect_.Contains(x, y)) {
    hit_layers.insert(layer_model_.GetName());
  }
}

void InOutAnimationListener::OnValueChanged() {
  auto in_out_value = layer_.in_out_animation_->GetValue().Get();
  layer_.SetVisible(in_out_value == 1.0);
}

}  // namespace animax
}  // namespace lynx
