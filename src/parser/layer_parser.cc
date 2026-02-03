// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
// Copyright 2018 Airbnb, Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "src/parser/layer_parser.h"

#include <string>
#include <vector>

#include "src/model/animatable/animatable_transform_model.h"
#include "src/model/content_model.h"
#include "src/model/layer_model.h"
#include "src/model/mask_model.h"
#include "src/parser/animatable/animatable_text_properties_parser.h"
#include "src/parser/animatable/animatable_transform_parser.h"
#include "src/parser/animatable/animatable_value_parser.h"
#include "src/parser/content_model_parser.h"
#include "src/parser/effect/blur_effect_parser.h"
#include "src/parser/effect/drop_shadow_effect_parser.h"
#include "src/parser/mask_parser.h"

namespace lynx {
namespace animax {
namespace {
BlendModeType ParseLottieBlendMode(int32_t value) {
  switch (value) {
    case 0:
      return BlendModeType::kNormal;
    case 1:
      return BlendModeType::kMultiply;
    case 2:
      return BlendModeType::kScreen;
    case 3:
      return BlendModeType::kOverlay;
    case 4:
      return BlendModeType::kDarken;
    case 5:
      return BlendModeType::kLighten;
    case 6:
      return BlendModeType::kColorDodge;
    case 7:
      return BlendModeType::kColorBurn;
    case 8:
      return BlendModeType::kHardLight;
    case 9:
      return BlendModeType::kSoftLight;
    case 10:
      return BlendModeType::kDifference;
    case 11:
      return BlendModeType::kExclusion;
    case 12:
      return BlendModeType::kHue;
    case 13:
      return BlendModeType::kSaturation;
    case 14:
      return BlendModeType::kColor;
    case 15:
      return BlendModeType::kLuminosity;
    default:
      return BlendModeType::kNormal;
  }
}
}  // namespace

std::unique_ptr<LayerModel> LayerParser::Parse(CompositionModel& composition) {
  auto layer_model = LayerModel::Make(composition);
  auto& bounds = composition.GetBounds();

  layer_model->Init(
      std::string("__container"), LayerType::kPreComp, -1, -1, std::string(""),
      0, 0, ValueFactory::Make<Color>(), 0, 0, bounds.GetWidth(),
      bounds.GetHeight(), nullptr, nullptr, false,
      std::unique_ptr<AnimatableTransformModel>(new AnimatableTransformModel()),
      static_cast<int>(MatteType::kUnknown), -1, false, nullptr, nullptr,
      nullptr, false, nullptr, BlendModeType::kNormal);
  return layer_model;
}

std::unique_ptr<LayerModel> LayerParser::Parse(rapidjson::Value& value,
                                               CompositionModel& composition) {
  auto layer_model = LayerModel::Make(composition);
  auto scale = composition.GetScale();

  std::string layer_name;
  int32_t layer_id = 0, parent_id = -1;
  std::string ref_id;
  LayerType layer_type = LayerType::kPreComp;
  int32_t solid_width = 0, solid_height = 0;
  auto solid_color = ValueFactory::Make<Color>();
  float time_stretch = 1, start_frame = 0, in_frame = 0, out_frame = 0;
  float pre_comp_width = 0, pre_comp_height = 0;
  std::string cl;
  bool hidden = false;
  bool auto_orient = false;
  BlendModeType blend_mode = BlendModeType::kNormal;

  std::unique_ptr<BlurEffectModel> blur_effect;
  std::unique_ptr<DropShadowEffectModel> drop_shadow_effect;

  int32_t matte_type_int = static_cast<int>(MatteType::kUnknown);
  int32_t matte_parent = -1;
  bool is_matte_target = false;
  std::unique_ptr<AnimatableTransformModel> transform;
  std::unique_ptr<AnimatableTextFrame> text;
  std::unique_ptr<AnimatableTextPropertyList> text_properties_list;
  std::unique_ptr<AnimatableFloatValue> time_remapping;
  bool enable_3d = false;
  std::unique_ptr<AnimatableFloatValue> perspective;

  const auto& object = value.GetObject();
  for (auto it = object.MemberBegin(); it != object.MemberEnd(); it++) {
    const auto& key = it->name.GetString();
    if (strcmp(key, "nm") == 0) {
      layer_name = it->value.GetString();
    } else if (strcmp(key, "ind") == 0) {
      layer_id = it->value.GetInt();
    } else if (strcmp(key, "refId") == 0) {
      ref_id = it->value.GetString();
    } else if (strcmp(key, "ty") == 0) {
      layer_type = static_cast<LayerType>(it->value.GetInt());
    } else if (strcmp(key, "parent") == 0) {
      parent_id = it->value.GetInt();
    } else if (strcmp(key, "sw") == 0) {
      solid_width = it->value.GetInt() * scale;
    } else if (strcmp(key, "sh") == 0) {
      solid_height = it->value.GetInt() * scale;
    } else if (strcmp(key, "sc") == 0) {
      solid_color = ValueFactory::Make<Color>(it->value.GetString());
    } else if (strcmp(key, "ks") == 0) {
      int32_t options = 0;
      if (layer_type == LayerType::kCamera) {
        options |= AnimatableTransformParser::Options::kForCamera;
      }
      transform =
          AnimatableTransformParser::Parse(it->value, composition, options);
    } else if (strcmp(key, "tt") == 0) {
      matte_type_int = it->value.GetInt();
      composition.IncrementMatteOrMaskCount(1);
    } else if (strcmp(key, "tp") == 0) {
      matte_parent = it->value.GetInt();
    } else if (strcmp(key, "td") == 0) {
      is_matte_target = it->value.GetInt();
    } else if (strcmp(key, "masksProperties") == 0) {
      const auto& mask_array = it->value.GetArray();
      auto& masks = layer_model->GetMasks();
      for (auto mask_it = mask_array.Begin(); mask_it != mask_array.End();
           mask_it++) {
        masks.push_back(MaskParser::Parse(mask_it->Move(), composition));
      }
    } else if (strcmp(key, "shapes") == 0) {
      const auto& array = it->value.GetArray();
      auto& shapes = layer_model->GetShapes();
      for (auto shape_it = array.Begin(); shape_it != array.End(); shape_it++) {
        auto shape = ContentModelParser::Parse(shape_it->Move(), composition);
        if (shape) {
          shapes.push_back(std::move(shape));
        }
      }
    } else if (strcmp(key, "t") == 0) {
      const auto& text_object = it->value.GetObject();
      for (auto text_it = text_object.MemberBegin();
           text_it != text_object.MemberEnd(); text_it++) {
        const auto& text_key = text_it->name.GetString();
        if (strcmp(text_key, "d") == 0) {
          text = AnimatableValueParser::ParseDocumentData(text_it->value,
                                                          composition);
        } else if (strcmp(text_key, "a") == 0) {
          const auto& prop_array = text_it->value.GetArray();
          for (auto array_it = prop_array.Begin(); array_it != prop_array.End();
               array_it++) {
            auto text_properties =
                AnimatableTextPropertiesParser::Parse(*array_it, composition);
            if (text_properties) {
              if (!text_properties_list) {
                text_properties_list =
                    std::make_unique<AnimatableTextPropertyList>();
              }
              text_properties_list->emplace_back(std::move(text_properties));
            }
          }
        }
      }
    } else if (strcmp(key, "ef") == 0) {
      const auto& effect_array = it->value.GetArray();

      for (auto array_it = effect_array.Begin(); array_it != effect_array.End();
           array_it++) {
        const auto& effect_object = array_it->GetObject();
        for (auto object_it = effect_object.MemberBegin();
             object_it != effect_object.MemberEnd(); object_it++) {
          auto name = object_it->name.GetString();
          if (strcmp("ty", name) == 0) {
            auto type = object_it->value.GetInt();
            auto ef_object = effect_object.FindMember("ef");
            if (ef_object == effect_object.MemberEnd()) {
              ANIMAX_LOGE("no ef member");
              continue;
            }

            if (type == 29) {
              blur_effect =
                  BlurEffectParser::Parse(ef_object->value, composition);
            } else if (type == 25) {
              drop_shadow_effect =
                  DropShadowEffectParser::Parse(ef_object->value, composition);
            } else {
              ANIMAX_LOGI("effect not support, type:" << std::to_string(type));
            }
          }
        }
      }
    } else if (strcmp(key, "sr") == 0) {
      time_stretch = it->value.GetFloat();
    } else if (strcmp(key, "st") == 0) {
      start_frame = it->value.GetFloat();
    } else if (strcmp(key, "w") == 0) {
      pre_comp_width = it->value.GetFloat() * scale;
    } else if (strcmp(key, "h") == 0) {
      pre_comp_height = it->value.GetFloat() * scale;
    } else if (strcmp(key, "ip") == 0) {
      in_frame = it->value.GetFloat();
    } else if (strcmp(key, "op") == 0) {
      out_frame = it->value.GetFloat();
    } else if (strcmp(key, "tm") == 0) {
      time_remapping =
          AnimatableValueParser::ParseFloat(it->value, composition, false);
    } else if (strcmp(key, "cl") == 0) {
      cl = it->value.GetString();
    } else if (strcmp(key, "hd") == 0) {
      hidden = it->value.GetBool();
    } else if (strcmp(key, "ao") == 0) {
      auto_orient = it->value.GetInt() == 1;
    } else if (strcmp(key, "ddd") == 0) {
      enable_3d = it->value.GetInt() == 1;
    } else if (strcmp(key, "pe") == 0) {
      perspective = AnimatableValueParser::ParseFloat(it->value, composition);
    } else if (strcmp(key, "bm") == 0) {
      blend_mode = ParseLottieBlendMode(it->value.GetInt());
    }
  }

  auto& in_out_frames = layer_model->GetInOutFrames();
  if (in_frame > 0) {
    in_out_frames.emplace_back(std::make_unique<KeyframeModel>(
        composition, ValueFactory::Make<Float>(0), ValueFactory::Make<Float>(0),
        nullptr, 0, in_frame));
  }

  out_frame = out_frame > 0 ? out_frame : composition.GetEndFrame();

  in_out_frames.emplace_back(std::make_unique<KeyframeModel>(
      composition, ValueFactory::Make<Float>(1), ValueFactory::Make<Float>(1),
      nullptr, in_frame, out_frame));

  in_out_frames.emplace_back(std::make_unique<KeyframeModel>(
      composition, ValueFactory::Make<Float>(0), ValueFactory::Make<Float>(0),
      nullptr, out_frame, Float::kMax));

  if (auto_orient) {
    if (!transform) {
      transform = std::make_unique<AnimatableTransformModel>();
    }
    transform->SetAutoOrient(true);
  }

  if ((enable_3d || layer_type == LayerType::kCamera) && transform) {
    transform->SetEnable3D(true);
  }

  if (layer_type == LayerType::kAudio && !transform) {
    transform = std::make_unique<AnimatableTransformModel>();
  }

  layer_model->Init(layer_name, layer_type, layer_id, parent_id, ref_id,
                    solid_width, solid_height, std::move(solid_color),
                    time_stretch, start_frame, pre_comp_width, pre_comp_height,
                    std::move(text), std::move(text_properties_list), hidden,
                    std::move(transform), matte_type_int, matte_parent,
                    is_matte_target, std::move(time_remapping),
                    std::move(blur_effect), std::move(drop_shadow_effect),
                    enable_3d, std::move(perspective), blend_mode);
  return layer_model;
}

}  // namespace animax
}  // namespace lynx
