// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/jsbridge/bindings/animax/animax_player_delegate.h"

#include "base/include/closure.h"
#include "include/base/util/color_util.h"
#include "include/player/animax_player.h"
#include "include/property/animax_player_global.h"
#include "include/property/animax_value_param.h"
#include "include/property/property_update_request.h"
#include "include/property/property_update_response.h"
#include "src/jsbridge/bindings/animax/napi_on_layer_bounds_callback.h"
#include "src/jsbridge/bindings/animax/napi_on_property_callback.h"
#include "src/jsbridge/bindings/animax/napi_task_runner.h"
#include "src/jsbridge/bindings/animax/napi_value_param.h"

namespace lynx {
namespace animax {

namespace {

std::unique_ptr<AnimaXValueParam> ToAnimaXValueParam(const ValueParam& value) {
  if (value.hasStringValue()) {
    return std::make_unique<AnimaXValueParam>(value.stringValue(),
                                              value.frameIndex());
  } else if (value.hasBoolValue()) {
    return std::make_unique<AnimaXValueParam>(value.boolValue(),
                                              value.frameIndex());
  } else if (value.hasPointX() && value.hasPointY()) {
    return std::make_unique<AnimaXValueParam>(
        value.pointX(), value.pointY(),
        static_cast<AnimaXValueParam::ApplyMode>(value.valueApplyMode()),
        value.frameIndex());
  } else if (value.hasDoubleValue()) {
    return std::make_unique<AnimaXValueParam>(
        value.doubleValue(),
        static_cast<AnimaXValueParam::ApplyMode>(value.valueApplyMode()),
        value.frameIndex());
  }
  return std::make_unique<AnimaXValueParam>();
}

std::unique_ptr<AnimaXValueParam> ToAnimaXValueParamForLayer(
    const ValueParam& value, LayerPropertyType layer_type) {
  switch (layer_type) {
    case LayerPropertyType::kTextColor:
    case LayerPropertyType::kColor:
    case LayerPropertyType::kStrokeColor:
    case LayerPropertyType::kDropShadowColor:
      if (value.hasDoubleValue()) {
        return std::make_unique<AnimaXValueParam>(
            static_cast<int32_t>(value.doubleValue()),
            static_cast<AnimaXValueParam::ApplyMode>(value.valueApplyMode()),
            value.frameIndex());
      } else if (value.hasStringValue()) {
        int32_t color_value = ColorUtil::ParseHexColor(value.stringValue());
        return std::make_unique<AnimaXValueParam>(color_value,
                                                  value.frameIndex());
      }
      break;
    default:
      break;
  }
  return ToAnimaXValueParam(value);
}

template <typename RequestCreator, typename UpdateMethod>
void UpdateProperty(std::weak_ptr<AnimaXPlayer> player,
                    std::unique_ptr<AnimaXValueParam> animax_value,
                    std::unique_ptr<NapiOnPropertyCallback> callback,
                    RequestCreator&& request_creator,
                    UpdateMethod&& update_method) {
  auto shared_player = player.lock();
  if (!shared_player || !animax_value || !callback) {
    return;
  }

  auto task_runner = NapiTaskRunner{callback->Env(nullptr)};

  auto request = request_creator(
      std::move(animax_value),
      [callback = std::move(callback), task_runner = std::move(task_runner)](
          const PropertyUpdateResponse& response) mutable {
        task_runner.PostTask([response, callback = std::move(callback)]() {
          callback->Invoke(response.IsSuccess(),
                           static_cast<int16_t>(response.GetErrorType()));
        });
      });

  update_method(shared_player, std::move(request));
}

}  // namespace

std::unique_ptr<AnimaXPlayerDelegate> AnimaXPlayerDelegate::Create(
    const std::string& id) {
  auto weak_player = AnimaXPlayerGlobal::Find(id);
  // If the id was not found, the player will be nullptr, the later napi call
  // will fail
  return std::unique_ptr<AnimaXPlayerDelegate>(
      new AnimaXPlayerDelegate(std::move(weak_player)));
}

AnimaXPlayerDelegate::AnimaXPlayerDelegate(
    std::weak_ptr<AnimaXPlayer> weak_player)
    : weak_player_(std::move(weak_player)) {}

AnimaXPlayerDelegate::~AnimaXPlayerDelegate() = default;

void AnimaXPlayerDelegate::UpdateLayerProperty(
    const Napi::Number& type, const Napi::String& layer_name,
    std::unique_ptr<ValueParam> value,
    std::unique_ptr<NapiOnPropertyCallback> callback) {
  if (value == nullptr) {
    return;
  }

  auto layer_type = static_cast<LayerPropertyType>(type.Int32Value());
  std::vector<std::string> keys = {AnimaXKeyPath::kGlobstarKey};
  auto layer_name_utf8 = layer_name.Utf8Value();
  if (layer_name_utf8 != AnimaXKeyPath::kGlobstarKey) {
    keys.emplace_back(layer_name_utf8);
  }
  auto key_path = std::make_unique<AnimaXKeyPath>(keys);
  auto animax_value = ToAnimaXValueParamForLayer(*value, layer_type);

  UpdateProperty(
      weak_player_, std::move(animax_value), std::move(callback),
      [layer_type, key_path = std::move(key_path)](
          std::unique_ptr<AnimaXValueParam> value,
          AnimaXPropertyCallback callback) mutable {
        return LayerStaticRequest::Make(layer_type, std::move(key_path),
                                        std::move(value), std::move(callback));
      },
      [](auto& updater, auto request) {
        updater->UpdateLayerProperty(std::move(request));
      });
}

void AnimaXPlayerDelegate::SetResourceProperty(
    const Napi::Number& type, const Napi::String& id,
    std::unique_ptr<ValueParam> value,
    std::unique_ptr<NapiOnPropertyCallback> callback) {
  if (value == nullptr) {
    return;
  }

  auto resource_type = static_cast<ResourcePropertyType>(type.Int32Value());
  auto resource_id = id.Utf8Value();

  auto animax_value = ToAnimaXValueParam(*value);
  UpdateProperty(
      weak_player_, std::move(animax_value), std::move(callback),
      [resource_type, resource_id = std::move(resource_id)](
          std::unique_ptr<AnimaXValueParam> value,
          AnimaXPropertyCallback callback) mutable {
        return ResourceUpdateRequest::Make(
            resource_type, std::move(resource_id), std::move(value),
            std::move(callback));
      },
      [](auto& updater, auto request) {
        updater->SetResourceProperty(std::move(request));
      });
}

void AnimaXPlayerDelegate::SubmitResourcePropertiesUpdate(
    std::unique_ptr<NapiOnPropertyCallback> callback) {
  auto player = weak_player_.lock();
  if (player) {
    player->LoadAssetsWithCallback([callback = std::move(callback)]() mutable {
      if (!callback) {
        return;
      }
      auto task_runner = NapiTaskRunner{callback->Env(nullptr)};
      task_runner.PostTask(
          [callback = std::move(callback)]() { callback->Invoke(true, 0); });
    });
  }
}

void AnimaXPlayerDelegate::Play() {
  auto player = weak_player_.lock();
  if (player) {
    player->Play();
  }
}

void AnimaXPlayerDelegate::GetLayerBounds(
    const Napi::String& layer_name, const Napi::Number& layer_bounds_space,
    std::unique_ptr<NapiOnLayerBoundsCallback> callback) {
  auto shared_player = weak_player_.lock();
  if (!shared_player || !callback) {
    return;
  }
  auto task_runner = NapiTaskRunner{callback->Env(nullptr)};
  shared_player->GetLayerBounds(
      layer_name.Utf8Value(),
      static_cast<LayerBoundsSpace>(layer_bounds_space.Int32Value()),
      [callback = std::move(callback), task_runner = std::move(task_runner)](
          bool success, float x, float y, float width, float height) mutable {
        task_runner.PostTask(
            [success, x, y, width, height, callback = std::move(callback)]() {
              callback->Invoke(success, x, y, width, height);
            });
      });
}
}  // namespace animax
}  // namespace lynx
