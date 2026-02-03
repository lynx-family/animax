// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_LAYER_BASE_LAYER_H_
#define ANIMAX_SRC_LAYER_BASE_LAYER_H_

#include <memory>
#include <unordered_set>
#include <utility>

#include "include/player/animax_fit_position.h"
#include "src/animation/base_keyframe_animation.h"
#include "src/animation/keyframe_animation.h"
#include "src/animation/mask_keyframe_animation.h"
#include "src/animation/transform_keyframe_animation.h"
#include "src/model/composition_model.h"
#include "src/model/layer_model.h"
#include "src/player/animax_player_context.h"
#include "src/player/layer_event_listener.h"
#include "src/render/mask_filter.h"
#include "src/render/paint.h"

namespace lynx {
namespace animax {

class CompositionLayer;
class InOutAnimationListener;

class BaseLayer : public Content,
                  public AnimationHost,
                  public AnimationListener {
 public:
  BaseLayer(LayerModel& layer_model, CompositionModel& model);
  virtual ~BaseLayer() = default;

  void Init() override;

  void OnValueChanged() override {}

  void Draw(Canvas& canvas, Matrix& parent_matrix,
            int32_t parent_alpha) override;
  void GetBounds(RectF& out_bounds, Matrix& parent_matrix,
                 bool apply_parent) override;

  const std::string& GetName() override { return layer_model_.GetName(); }

  virtual void SetContents(std::vector<Content*>& contents_before,
                           std::vector<Content*>& contents_after) override {}
  virtual void SetProgress(float progress);
  virtual void DrawLayer(Canvas& canvas, Matrix& matrix, int32_t alpha) = 0;
  BlurEffectModel* GetBlurEffect();
  DropShadowEffectModel* GetDropEffect();

  LayerModel& GetLayerModel() { return layer_model_; }
  bool HasMatteOnThisLayer() { return matte_layer_ != nullptr; }
  bool HasMaskOnThisLayer() {
    return mask_ && !mask_->GetMaskAnimations().empty();
  }

  void SetUpInOutAnimations();
  void SetVisible(bool visible);
  void SetMattedLayer(BaseLayer* matted_layer) { matte_layer_ = matted_layer; }
  void SetParentLayer(BaseLayer* layer) { parent_layer_ = layer; }
  void SetCompositionLayer(CompositionLayer* layer) {
    composition_layer_ = layer;
  }

  void BuildParentLayerListIfNeeded();
  void IntersectBoundsWithMatte(RectF& rect, Matrix& matrix);
  void IntersectBoundsWithMask(RectF& rect, Matrix& matrix);

  void ClearCanvas(Canvas& canvas);
  void ApplyMasks(Canvas& canvas, Matrix& matrix);
  bool AreAllMasksNone();

  void ApplyInvertedAddMask(Canvas& canvas, Matrix& matrix,
                            ShapeKeyframeAnimation& mask_animation,
                            IntegerKeyframeAnimation& opacity_animation);

  void ApplyAddMask(Canvas& canvas, Matrix& matrix,
                    ShapeKeyframeAnimation& mask_animation,
                    IntegerKeyframeAnimation& opacity_animation);

  void ApplyInvertedSubtractMask(Canvas& canvas, Matrix& matrix,
                                 ShapeKeyframeAnimation& mask_animation,
                                 IntegerKeyframeAnimation& opacity_animation);

  void ApplySubtractMask(Canvas& canvas, Matrix& matrix,
                         ShapeKeyframeAnimation& mask_animation,
                         IntegerKeyframeAnimation& opacity_animation);

  void ApplyInvertedIntersectMask(Canvas& canvas, Matrix& matrix,
                                  ShapeKeyframeAnimation& mask_animation,
                                  IntegerKeyframeAnimation& opacity_animation);

  void ApplyIntersectMask(Canvas& canvas, Matrix& matrix,
                          ShapeKeyframeAnimation& mask_animation,
                          IntegerKeyframeAnimation& opacity_animation);

  void ApplyInvertedMask(Canvas& canvas, Matrix& matrix,
                         ShapeKeyframeAnimation& mask_animation,
                         IntegerKeyframeAnimation& opacity_animation,
                         Paint& save_paint, Paint& alpha_paint);

  CompositionModel& GetComposition() { return composition_; }
  MaskFilter* GetBlurMaskFilter(float radius);

  bool SubDrawingType() override { return true; }

  /**
   * Set layer event listener
   * make sure listener is valid in the lifecycle of layer.
   * @param listener a layer event listener
   */
  void SetEventListener(std::weak_ptr<LayerEventListener> listener) {
    weak_listener_ = std::move(listener);
  }

  /**
   * Set AnimaXPlayerContext
   * the context is used to get some properties and execute behaviors on
   * specific thread.
   * @param context a AnimaXPlayerContext instance.
   */
  virtual void SetPlayerContext(std::weak_ptr<AnimaXPlayerContext> context) {
    weak_context_ = std::move(context);
  }

  void ResolveKeyPath(const AnimaXKeyPath& path, int32_t depth,
                      ResolvedKeyPathElements& match_elements,
                      const AnimaXKeyPath& current_partial_path) override;

  virtual void ResolveChildKeyPath(const AnimaXKeyPath& path, int32_t depth,
                                   ResolvedKeyPathElements& match_elements,
                                   const AnimaXKeyPath& current_partial_path);

  PropertyUpdateResult UpdateLayerProperty(
      PropertyUpdateContext& context) override;

  KeyframeAnimation* GetAnimationForProperty(LayerPropertyType type) override;

  virtual void HitTest(float x, float y,
                       std::unordered_set<std::string>& hit_layers);

  virtual bool GetBoundsByName(const std::string& layer_name,
                               LayerBoundsSpace bounds_space,
                               RectF& out_bounds);

 protected:
  friend class InOutAnimationListener;

  LayerModel& layer_model_;
  CompositionModel& composition_;

  std::unique_ptr<Matrix> matrix_;
  std::unique_ptr<Matrix> parent_matrix_;
  std::unique_ptr<Path> path_;

  // parent
  BaseLayer* parent_layer_ = nullptr;
  CompositionLayer* composition_layer_ = nullptr;
  std::vector<BaseLayer*> parent_layers_;

  std::unique_ptr<Matrix> bounds_matrix_;
  std::unique_ptr<Matrix> canvas_matrix_;

  std::unique_ptr<TransformKeyframeAnimation> transform_;

  std::unique_ptr<Paint> content_paint_;
  std::unique_ptr<Paint> dst_in_paint_;
  std::unique_ptr<Paint> dst_out_paint_;
  std::unique_ptr<Paint> matte_paint_;
  std::unique_ptr<Paint> clear_paint_;
  std::unique_ptr<Paint> blend_paint_;

  bool outline_maks_and_mattes_;
  std::unique_ptr<Paint> outline_masks_and_mattes_paint_;

  RectF rect_;
  RectF canvas_bounds_;
  RectF mask_bounds_rect_;
  RectF matte_bounds_rect_;
  RectF temp_mask_bounds_rect_;

  bool visible_ = true;
  std::unique_ptr<FloatKeyframeAnimation> in_out_animation_;
  std::unique_ptr<InOutAnimationListener> in_out_listener_;
  std::unique_ptr<MaskKeyframeAnimation> mask_;
  BaseLayer* matte_layer_ = nullptr;

  float blur_mask_filter_radius_ = 0.0;
  std::unique_ptr<MaskFilter> blur_mask_filter_;
  std::weak_ptr<LayerEventListener> weak_listener_;
  std::weak_ptr<AnimaXPlayerContext> weak_context_;
};

class InOutAnimationListener : public AnimationListener {
 public:
  InOutAnimationListener(BaseLayer& layer) : layer_(layer) {}

  void OnValueChanged() override;

 private:
  BaseLayer& layer_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_LAYER_BASE_LAYER_H_
