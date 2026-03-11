// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_MODEL_LAYER_MODEL_H_
#define ANIMAX_SRC_MODEL_LAYER_MODEL_H_

#include <stdint.h>

#include <memory>
#include <string>
#include <vector>

#include "src/model/animatable/animatable_text_properties.h"
#include "src/model/animatable/animatable_transform_model.h"
#include "src/model/animatable/animatable_value.h"
#include "src/model/content_model.h"
#include "src/model/effect/blur_effect_model.h"
#include "src/model/effect/drop_shadow_effect_model.h"
#include "src/model/mask_model.h"

namespace lynx {
namespace animax {

class ContentModel;
class CompositionModel;
class AnimatableTransformModel;
class AnimatableFloatValue;

enum class LayerType : int32_t {
  kPreComp = 0,
  kSolid = 1,
  kImage = 2,
  kNull = 3,
  kShape = 4,
  kText = 5,
  kAudio = 6,
  kCamera = 13,
  kAlphaVideo = 1009,
};

enum class MatteType : uint8_t {
  kNone = 0,
  kAlpha,
  kAlphaInverted,
  kLuma,
  kLumaInverted,
  kUnknown
};

class LayerModel {
 public:
  static std::unique_ptr<LayerModel> Make(CompositionModel& composition);

  LayerModel(CompositionModel& composition);
  ~LayerModel() = default;

  void Init(std::string layer_name, LayerType layer_type, int32_t layer_id,
            int32_t parent_id, std::string ref_id, int32_t solid_width,
            int32_t solid_height, std::unique_ptr<Color> solid_color,
            float time_stretch, float start_frame, float pre_comp_width,
            float pre_comp_height, std::unique_ptr<AnimatableTextFrame> text,
            std::unique_ptr<AnimatableTextPropertyList> text_properties_list,
            bool hidden, std::unique_ptr<AnimatableTransformModel> transform,
            int32_t matte_type_int, int32_t matte_parent, bool is_matte_target,
            std::unique_ptr<AnimatableFloatValue> time_remapping,
            std::unique_ptr<BlurEffectModel> blur_effect,
            std::unique_ptr<DropShadowEffectModel> drop_shadow_effect,
            bool enable_3d, std::unique_ptr<AnimatableFloatValue> perspective);

  LayerType GetLayerType() { return layer_type_; }
  int32_t GetId() { return layer_id_; }
  std::string& GetRefId() { return ref_id_; }
  std::string& GetName() { return layer_name_; }

  std::vector<std::unique_ptr<ContentModel>>& GetShapes() { return shapes_; }
  std::vector<std::unique_ptr<MaskModel>>& GetMasks() { return masks_; }
  std::shared_ptr<KeyframeModelList> GetSharedInOutFrames() {
    return in_out_frames_;
  }
  KeyframeModelList& GetInOutFrames() {
    DCHECK(in_out_frames_);
    return *in_out_frames_;
  }
  AnimatableTransformModel* GetTransform();

  Color& GetSolidColor() { return *solid_color_; }
  int32_t GetSolidWidth() { return solid_width_; }
  int32_t GetSolidHeight() { return solid_height_; }

  float GetPreCompWidth() { return pre_comp_width_; }
  float GetPreCompHeight() { return pre_comp_height_; }
  int32_t GetParentId() { return parent_id_; }
  float GetStartProgress();
  float GetTimeStretch() { return time_stretch_; }

  bool IsHidden() { return hidden_; }
  void SetHidden(bool hidden) { hidden_ = hidden; }

  MatteType& GetMatteType() { return matte_type_; }
  int32_t GetMatteParent() const { return matte_parent_; }
  bool IsMatteTarget() const { return is_matte_target_; }
  void SetIsMatteTarget(const bool is_matte_target) {
    is_matte_target_ = is_matte_target;
  }

  AnimatableTextFrame* GetText() { return text_.get(); }
  const AnimatableTextPropertyList* GetTextPropertiesList() {
    return text_properties_list_.get();
  }
  AnimatableFloatValue* GetTimeRemapping() { return time_remapping_.get(); }

  BlurEffectModel* GetBlurEffect() { return blur_effect_.get(); }
  DropShadowEffectModel* GetDropEffect() { return drop_shadow_effect_.get(); }

  bool GetEnable3D() const { return enable_3d_; }
  AnimatableFloatValue* GetPerspective() { return perspective_.get(); }

 private:
  CompositionModel& composition_;

  std::vector<std::unique_ptr<ContentModel>> shapes_;
  std::vector<std::unique_ptr<MaskModel>> masks_;

  std::string layer_name_;
  LayerType layer_type_;
  int32_t layer_id_ = -1;
  int32_t parent_id_ = -1;
  std::string ref_id_;

  // TODO(aiyongbiao): masks p1
  std::unique_ptr<AnimatableTransformModel> transform_;
  int32_t solid_width_ = 0;
  int32_t solid_height_ = 0;
  std::unique_ptr<Color> solid_color_;
  float time_stretch_ = 0;
  float start_frame_ = 0;
  float pre_comp_width_ = 0;
  float pre_comp_height_ = 0;
  bool hidden_ = false;
  bool enable_3d_ = false;

  // text
  std::unique_ptr<AnimatableTextFrame> text_;
  std::unique_ptr<AnimatableTextPropertyList> text_properties_list_;

  std::shared_ptr<KeyframeModelList> in_out_frames_ =
      std::make_shared<KeyframeModelList>();

  MatteType matte_type_ = MatteType::kNone;
  int32_t matte_parent_ = -1;
  bool is_matte_target_ = false;
  std::unique_ptr<AnimatableFloatValue> time_remapping_;

  // effect
  std::unique_ptr<BlurEffectModel> blur_effect_;
  std::unique_ptr<DropShadowEffectModel> drop_shadow_effect_;

  std::unique_ptr<AnimatableFloatValue> perspective_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_MODEL_LAYER_MODEL_H_
