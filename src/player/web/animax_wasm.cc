// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/player/web/animax_wasm.h"

#include <emscripten/html5_webgl.h>

#include <vector>

#include "include/base/util/color_util.h"
#include "include/player/animax_player.h"
#include "include/player/animax_player_builder.h"
#include "include/property/animax_value_param.h"
#include "skity/io/data.hpp"
#include "skity/text/font_manager.hpp"
#include "skity/text/typeface.hpp"
#include "src/base/log/log.h"
#include "src/base/thread/task_runner.h"
#include "src/player/web/animax_surface_web.h"
#include "src/player/web/animax_web_gpu_context.h"
#include "src/player/web/vsync_monitor_web.h"
#include "src/render/web/font_registry_web.h"
#include "src/resource/resource_loader/web/resource_loader_web.h"

namespace lynx {
namespace animax {
namespace {
constexpr double kUnlimitedFrameRate = 1000000000.0;

std::string GetEventName(Event event) {
  switch (event) {
    case Event::kCompletion:
      return "completion";
    case Event::kStart:
      return "start";
    case Event::kRepeat:
      return "repeat";
    case Event::kCancel:
      return "cancel";
    case Event::kReady:
      return "ready";
    case Event::kUpdate:
      return "update";
    case Event::kError:
      return "error";
    case Event::kWarning:
      return "warning";
    case Event::kFps:
      return "fps";
    case Event::kTapLayer:
      return "taplayers";
    case Event::kFirstFrame:
      return "firstframe";
    case Event::kCompositionReady:
      return "compositionready";
    default:
      return "";
  }
}

emscripten::val ConvertParamsToJsObject(const EventParamMap& params) {
  emscripten::val param_obj = emscripten::val::object();
  for (const auto& pair : params) {
    const std::string& key = pair.first;
    const animax::EventParamValue& value = pair.second;
    switch (value.type) {
      case animax::EventParamValue::Type::kInt32:
        if (value.int_val.has_value()) {
          param_obj.set(key, *value.int_val);
        } else {
          param_obj.set(key, 0);
        }
        break;
      case animax::EventParamValue::Type::kDouble:
        if (value.double_val.has_value()) {
          param_obj.set(key, *value.double_val);
        } else {
          param_obj.set(key, 0.0);
        }
        break;
      case animax::EventParamValue::Type::kString:
        if (value.string_val) {
          param_obj.set(key, *value.string_val);
        } else {
          param_obj.set(key, "");
        }
        break;
      case animax::EventParamValue::Type::kStringVector:
        if (value.string_vector_val) {
          emscripten::val arr = emscripten::val::array();
          int32_t index = 0;
          for (const auto& s : *value.string_vector_val) {
            arr.set(index++, s);
          }
          param_obj.set(key, arr);
        } else {
          param_obj.set(key, emscripten::val::array());
        }
        break;
      default:
        param_obj.set(key, emscripten::val::null());
        break;
    }
  }
  return param_obj;
}

std::unique_ptr<AnimaXValueParam> ToAnimaXValueParam(
    const emscripten::val& value) {
  if (value.hasOwnProperty("stringValue")) {
    return std::make_unique<AnimaXValueParam>(
        value["stringValue"].as<std::string>(),
        value.hasOwnProperty("frameIndex") ? value["frameIndex"].as<int>() : 0);
  } else if (value.hasOwnProperty("boolValue")) {
    return std::make_unique<AnimaXValueParam>(
        value["boolValue"].as<bool>(),
        value.hasOwnProperty("frameIndex") ? value["frameIndex"].as<int>() : 0);
  } else if (value.hasOwnProperty("pointX") && value.hasOwnProperty("pointY")) {
    return std::make_unique<AnimaXValueParam>(
        value["pointX"].as<double>(), value["pointY"].as<double>(),
        value.hasOwnProperty("frameIndex") ? value["frameIndex"].as<int>() : 0);
  } else if (value.hasOwnProperty("doubleValue")) {
    return std::make_unique<AnimaXValueParam>(
        value["doubleValue"].as<double>(),
        value.hasOwnProperty("frameIndex") ? value["frameIndex"].as<int>() : 0);
  }
  return std::make_unique<AnimaXValueParam>();
}

std::unique_ptr<AnimaXValueParam> ToAnimaXValueParamForLayer(
    const emscripten::val& value, LayerPropertyType layer_type) {
  int32_t frame_idx = value.hasOwnProperty("frameIndex")
                          ? value["frameIndex"].as<int32_t>()
                          : 0;
  switch (layer_type) {
    case LayerPropertyType::kTextColor:
    case LayerPropertyType::kColor:
    case LayerPropertyType::kStrokeColor:
    case LayerPropertyType::kDropShadowColor: {
      if (value.hasOwnProperty("stringValue")) {
        int32_t color_value =
            ColorUtil::ParseHexColor(value["stringValue"].as<std::string>());
        return std::make_unique<AnimaXValueParam>(color_value, frame_idx);
      }
      if (value.hasOwnProperty("doubleValue")) {
        int32_t color_value =
            static_cast<int32_t>(value["doubleValue"].as<double>());
        return std::make_unique<AnimaXValueParam>(color_value, frame_idx);
      }
      break;
    }
    default:
      break;
  }
  return ToAnimaXValueParam(value);
}

template <typename RequestCreator, typename UpdateMethod>
void UpdateProperty(std::shared_ptr<AnimaXPlayer> player,
                    std::unique_ptr<AnimaXValueParam> animax_value,
                    AnimaXWasm::PropertyUpdateCallback callback,
                    RequestCreator&& request_creator,
                    UpdateMethod&& update_method) {
  if (!animax_value || !callback) {
    return;
  }
  auto request = request_creator(
      std::move(animax_value),
      [callback = std::move(callback), task_runner = GetAnimaXMainThread()](
          const PropertyUpdateResponse& response) mutable {
        task_runner->PostTask([response, callback = std::move(callback)]() {
          callback(response.IsSuccess(),
                   static_cast<uint16_t>(response.GetErrorType()));
        });
      });

  update_method(player, std::move(request));
}

}  // namespace

base::NoDestructor<
    std::map<std::weak_ptr<AnimaXPlayer>, AnimaXWasm::EventCallback,
             std::owner_less<std::weak_ptr<AnimaXPlayer>>>>
    AnimaXWasm::event_callback_map_;

AnimaXWasm::AnimaXWasm(float scale) {
  ANIMAX_LOGI("AnimaXWasm constructor, scale: " << scale);
  resource_loader_ = std::make_shared<ResourceLoaderWeb>();
  vsync_monitor_ = std::make_shared<VSyncMonitorWeb>();

  AnimaXPlayerBuilder builder;
  builder.SetScale(scale)
      .SetVSyncMonitor(vsync_monitor_)
      .SetResourceLoader(resource_loader_)
      .AddEventListener([](std::weak_ptr<AnimaXPlayer> weak_player,
                           const Event event,
                           const animax::EventParamMap& params) {
        auto it = event_callback_map_->find(weak_player);
        if (it != event_callback_map_->end() && it->second) {
          it->second(GetEventName(event), ConvertParamsToJsObject(params));
        }
      });
  player_ = builder.Build();
  // VSyncMonitorWeb owns Web frame limiting so playback can keep using the
  // browser's unmodified RAF timestamp.
  player_->SetMaxFrameRate(kUnlimitedFrameRate);
}

AnimaXWasm::~AnimaXWasm() {
  ANIMAX_LOGI("AnimaXWasm destructor");
  event_callback_map_->erase(player_);
}

void AnimaXWasm::SetSurfaceWithCanvas(
    const std::string& canvas_id, int32_t width, int32_t height,
    const std::shared_ptr<AnimaXWebGPUContext>& web_gpu_ctx,
    uintptr_t gl_context_handle) {
  player_->UpdateSurface(
      [canvas_id, width, height, ctx = web_gpu_ctx, gl_context_handle,
       frame_capture_callback = frame_capture_callback_](
          std::unique_ptr<lynx::animax::AnimaXSurface> old_surface) mutable {
        const auto desc = AnimaXSurfaceWeb::Description{
            .canvas_id = canvas_id,
            .width = width,
            .height = height,
            .gl_context_handle = gl_context_handle,
            .frame_capture_callback = frame_capture_callback,
        };
        if (old_surface && old_surface->Valid()) {
          auto web_surface = static_cast<AnimaXSurfaceWeb*>(old_surface.get());
          ANIMAX_LOGI("AnimaXWasm Reconfigure "
                      << canvas_id << " " << width << " " << height << " "
                      << (ctx == nullptr ? "gl" : "webgpu"));
          web_surface->Reconfigure(desc);
          return old_surface;
        }
        ANIMAX_LOGI("AnimaXWasm Create " << canvas_id << " " << width << " "
                                         << height << " "
                                         << (ctx == nullptr ? "gl" : "webgpu"));
        return std::unique_ptr<AnimaXSurface>(new AnimaXSurfaceWeb(desc, ctx));
      });
}

void AnimaXWasm::SetResourceLoaderImpl(ResourceLoaderImpl impl) {
  DCHECK(resource_loader_);
  resource_loader_->SetLoaderImpl(impl);
}

void AnimaXWasm::SetEventCallback(EventCallback callback) {
  ANIMAX_LOGI("SetEventCallback success");
  (*event_callback_map_)[player_] = callback;
}

void AnimaXWasm::SetFrameCaptureCallback(FrameCaptureCallback callback) {
  frame_capture_callback_ = std::move(callback);
}

void AnimaXWasm::RequestFrameCapture() {
  player_->UpdateSurface(
      [frame_capture_callback = frame_capture_callback_](
          std::unique_ptr<lynx::animax::AnimaXSurface> surface) mutable {
        if (surface && surface->Valid() &&
            surface->Type() == AnimaXBackend::kGL) {
          static_cast<AnimaXSurfaceWeb*>(surface.get())
              ->SetFrameCaptureCallback(std::move(frame_capture_callback));
        }
        return surface;
      });
}

bool AnimaXWasm::RegisterFontWithData(const std::string& family_name,
                                      bool is_default,
                                      int32_t fallback_priority,
                                      const emscripten::val& buffer) {
  auto data = emscripten::convertJSArrayToNumberVector<uint8_t>(buffer);
  if (data.empty()) {
    ANIMAX_LOGE("RegisterFontWithData failed, data is empty, family_name: "
                << family_name);
    return false;
  }

  auto result = FontRegistryWeb::Get().RegisterFontData(
      family_name, data.data(), data.size(), is_default, fallback_priority);
  if (!result) {
    ANIMAX_LOGE("RegisterFontWithData failed, family_name: " << family_name);
    return false;
  }

  if (is_default) {
    auto raw_data = skity::Data::MakeWithCopy(data.data(), data.size());
    auto typeface = skity::Typeface::MakeFromData(raw_data);
    if (typeface) {
      skity::FontManager::RefDefault()->SetDefaultTypeface(typeface);
    } else {
      ANIMAX_LOGE(
          "RegisterFontWithData failed to set default typeface, "
          "family_name: "
          << family_name);
    }
  }
  return true;
}

void AnimaXWasm::OnResourceLoadSuccess(int32_t callback_id,
                                       const emscripten::val& buffer,
                                       uint32_t width, uint32_t height,
                                       uint64_t texture) {
  std::optional<std::vector<uint8_t>> data;
  if (!buffer.isNull() && !buffer.isUndefined()) {
    data = emscripten::convertJSArrayToNumberVector<uint8_t>(buffer);
  }
  ResourceLoaderWeb::OnResourceLoaded({
      .callback_id = callback_id,
      .success = true,
      .data = std::move(data),
      .width = width,
      .height = height,
      .texture = texture,
  });
}

void AnimaXWasm::OnResourceLoadError(int32_t callback_id,
                                     const std::string& error_message) {
  ResourceLoaderWeb::OnResourceLoaded({.callback_id = callback_id,
                                       .success = false,
                                       .error_message = error_message});
}

void AnimaXWasm::UpdateLayerProperty(uint16_t layer_type,
                                     const std::string& layer_name_utf8,
                                     const emscripten::val& value,
                                     PropertyUpdateCallback callback) {
  LayerPropertyType type = static_cast<LayerPropertyType>(layer_type);
  std::vector<std::string> keys = {AnimaXKeyPath::kGlobstarKey};
  if (layer_name_utf8 != AnimaXKeyPath::kGlobstarKey) {
    keys.emplace_back(layer_name_utf8);
  }
  auto key_path = std::make_unique<AnimaXKeyPath>(keys);
  auto animax_value = ToAnimaXValueParamForLayer(value, type);
  UpdateProperty(
      player_, std::move(animax_value), std::move(callback),
      [type, key_path = std::move(key_path)](
          std::unique_ptr<AnimaXValueParam> value,
          AnimaXPropertyCallback callback) mutable {
        return LayerStaticRequest::Make(type, std::move(key_path),
                                        std::move(value), std::move(callback));
      },
      [](auto& updater, auto request) {
        updater->UpdateLayerProperty(std::move(request));
      });
}

void AnimaXWasm::SetResourceProperty(uint16_t resource_type,
                                     const std::string& resource_id,
                                     const emscripten::val& value,
                                     PropertyUpdateCallback callback) {
  ResourcePropertyType type = static_cast<ResourcePropertyType>(resource_type);
  auto animax_value = ToAnimaXValueParam(value);
  UpdateProperty(
      player_, std::move(animax_value), std::move(callback),
      [type, resource_id = std::move(resource_id)](
          std::unique_ptr<AnimaXValueParam> value,
          AnimaXPropertyCallback callback) mutable {
        return ResourceUpdateRequest::Make(type, std::move(resource_id),
                                           std::move(value),
                                           std::move(callback));
      },
      [](auto& updater, auto request) {
        updater->SetResourceProperty(std::move(request));
      });
}

void AnimaXWasm::UpdateVisibilityStates(uint16_t state) {
  if (current_visible_states_ == state) {
    return;
  }

  const uint16_t max_visibility_state = 5;  // see VisibilityState
  for (uint16_t i = 0; i < max_visibility_state; i++) {
    uint16_t val = 1 << i;
    if ((state & val) == (current_visible_states_ & val)) {
      continue;
    }
    if ((state & val) == 0) {
      player_->OnShow(static_cast<VisibilityState>(val));
    } else {
      player_->OnHide(static_cast<VisibilityState>(val));
    }
  }

  current_visible_states_ = state;
}

void AnimaXWasm::SetMaxFrameRate(double max_frame_rate) {
  vsync_monitor_->SetMaxFrameRate(max_frame_rate);
}

void AnimaXWasm::GetLayerBounds(
    const std::string& layer_name_utf8, uint16_t bounds_space,
    std::function<void(bool, float, float, float, float)> js_callback) {
  player_->GetLayerBounds(
      layer_name_utf8, static_cast<LayerBoundsSpace>(bounds_space),
      [js_callback](bool success, float x, float y, float width, float height) {
        if (js_callback) {
          js_callback(success, x, y, width, height);
        }
      });
}

}  // namespace animax
}  // namespace lynx
