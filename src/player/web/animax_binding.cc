// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <emscripten/bind.h>

#include <unordered_map>

#include "include/player/animax_player.h"
#include "src/player/web/animax_wasm.h"
#include "src/player/web/animax_web_gpu_context.h"

EMSCRIPTEN_BINDINGS(animax) {
  void (lynx::animax::AnimaXPlayer::*AnimaXPlayerSetJsonString)(std::string) =
      &lynx::animax::AnimaXPlayer::SetJson;

  emscripten::class_<lynx::animax::AnimaXWebGPUContext>("AnimaXWebGPUContext")
      .smart_ptr<std::shared_ptr<lynx::animax::AnimaXWebGPUContext>>(
          "AnimaXWebGPUContext")
      .class_function("create", &lynx::animax::AnimaXWebGPUContext::Create);

  emscripten::class_<lynx::animax::AnimaXPlayer>("AnimaXPlayer")
      .smart_ptr<std::shared_ptr<lynx::animax::AnimaXPlayer>>("AnimaXPlayer")
      .function("setLoop", &lynx::animax::AnimaXPlayer::SetLoop)
      .function("setLoopCount", &lynx::animax::AnimaXPlayer::SetLoopCount)
      .function("setProgress", &lynx::animax::AnimaXPlayer::SetProgress)
      .function("setAutoReverse", &lynx::animax::AnimaXPlayer::SetAutoReverse)
      .function("setAutoPlay", &lynx::animax::AnimaXPlayer::SetAutoplay)
      .function("setSpeed", &lynx::animax::AnimaXPlayer::SetSpeed)
      .function(
          "setObjectFit",
          emscripten::optional_override(
              [](lynx::animax::AnimaXPlayer& self, uint8_t fit) {
                self.SetObjectFit(static_cast<lynx::animax::ObjectFit>(fit));
              }))
      .function("setObjectPosition",
                emscripten::optional_override(
                    [](lynx::animax::AnimaXPlayer& self, uint8_t pos) {
                      self.SetObjectPosition(
                          static_cast<lynx::animax::ObjectPosition>(pos));
                    }))
      .function("setKeepLastFrame",
                &lynx::animax::AnimaXPlayer::SetKeepLastFrame)
      .function("setJson", AnimaXPlayerSetJsonString)
      .function("setSrc", &lynx::animax::AnimaXPlayer::SetSrc)
      .function("setImageFolder", &lynx::animax::AnimaXPlayer::SetImageFolder)
      .function(
          "setSrcPolyfill",
          emscripten::optional_override(
              [](lynx::animax::AnimaXPlayer& self, emscripten::val polyfill) {
                std::unordered_map<std::string, std::string> map;
                if (!polyfill.isUndefined() && !polyfill.isNull()) {
                  emscripten::val keys =
                      emscripten::val::global("Object").call<emscripten::val>(
                          "keys", polyfill);
                  int length = keys["length"].as<int>();
                  for (int i = 0; i < length; ++i) {
                    std::string key = keys[i].as<std::string>();
                    std::string value = polyfill[key].as<std::string>();
                    map.emplace(key, value);
                  }
                }
                self.SetSrcPolyfill(map);
              }))
      .function("setStartFrame", &lynx::animax::AnimaXPlayer::SetStartFrame)
      .function("setEndFrame", &lynx::animax::AnimaXPlayer::SetEndFrame)
      .function("setFpsEventInterval",
                &lynx::animax::AnimaXPlayer::SetFpsEventInterval)
      .function("setMaxFrameRate", &lynx::animax::AnimaXPlayer::SetMaxFrameRate)
      .function("setDynamicResource",
                &lynx::animax::AnimaXPlayer::SetDynamicResource)
      .function("play", &lynx::animax::AnimaXPlayer::Play)
      .function("pause", &lynx::animax::AnimaXPlayer::Pause)
      .function("resume", &lynx::animax::AnimaXPlayer::Resume)
      .function("stop", &lynx::animax::AnimaXPlayer::Stop)
      .function("reload", &lynx::animax::AnimaXPlayer::Reload)
      .function("getDurationMs", &lynx::animax::AnimaXPlayer::GetDurationMs)
      .function("isAnimating", &lynx::animax::AnimaXPlayer::IsAnimating)
      .function("seek", &lynx::animax::AnimaXPlayer::Seek)
      .function("getCurrentFrame", &lynx::animax::AnimaXPlayer::GetCurrentFrame)
      .function("playSegment", &lynx::animax::AnimaXPlayer::PlaySegment)
      .function(
          "onShow",
          emscripten::optional_override(
              [](lynx::animax::AnimaXPlayer& self, int state) {
                self.OnShow(static_cast<lynx::animax::VisibilityState>(state));
              }))
      .function(
          "onHide",
          emscripten::optional_override(
              [](lynx::animax::AnimaXPlayer& self, int state) {
                self.OnHide(static_cast<lynx::animax::VisibilityState>(state));
              }))
      .function("onTap", &lynx::animax::AnimaXPlayer::OnTap)
      .function("subscribeUpdateEvent",
                &lynx::animax::AnimaXPlayer::SubscribeUpdateEvent)
      .function("unsubscribeUpdateEvent",
                &lynx::animax::AnimaXPlayer::UnsubscribeUpdateEvent)
      .function("subscribeUpdateEvents",
                emscripten::optional_override(
                    [](lynx::animax::AnimaXPlayer& self,
                       emscripten::val js_frames, bool subscribe) {
                      std::unordered_set<int32_t> frames;
                      if (js_frames.isArray()) {
                        int length = js_frames["length"].as<int>();
                        for (int i = 0; i < length; i++) {
                          int32_t frame = js_frames[i].as<int32_t>();
                          frames.insert(frame);
                        }
                        self.SubscribeUpdateEvents(frames, subscribe);
                      }
                    }));

  emscripten::class_<lynx::animax::AnimaXWasm>("AnimaXWasm")
      .smart_ptr<std::shared_ptr<lynx::animax::AnimaXWasm>>("AnimaXWasm")
      .constructor<float>()
      .class_function("onResourceLoadSuccess",
                      &lynx::animax::AnimaXWasm::OnResourceLoadSuccess,
                      emscripten::allow_raw_pointer<emscripten::arg<1>>())
      .class_function("onResourceLoadError",
                      &lynx::animax::AnimaXWasm::OnResourceLoadError)
      .class_function("registerFontWithData",
                      &lynx::animax::AnimaXWasm::RegisterFontWithData)
      .function("getPlayer", &lynx::animax::AnimaXWasm::GetPlayer)
      .function("setMaxFrameRate", &lynx::animax::AnimaXWasm::SetMaxFrameRate)
      .function(
          "setSurfaceWithCanvas",
          emscripten::optional_override(
              [](lynx::animax::AnimaXWasm& self, const std::string& canvas_id,
                 int32_t width, int32_t height,
                 const std::shared_ptr<lynx::animax::AnimaXWebGPUContext>& ctx,
                 uintptr_t gl_context_handle) {
                self.SetSurfaceWithCanvas(canvas_id, width, height, ctx,
                                          gl_context_handle);
              }))
      .function("updateVisibilityStates",
                &lynx::animax::AnimaXWasm::UpdateVisibilityStates)
      .function("setEventCallback",
                emscripten::optional_override([](lynx::animax::AnimaXWasm& self,
                                                 emscripten::val js_func) {
                  auto func = [js_func](const std::string& event_name,
                                        const emscripten::val& params) {
                    js_func(event_name, params);
                  };
                  self.SetEventCallback(func);
                }))
      .function("setFrameCaptureCallback",
                emscripten::optional_override([](lynx::animax::AnimaXWasm& self,
                                                 emscripten::val js_func) {
                  self.SetFrameCaptureCallback(
                      [js_func](std::vector<uint8_t> pixels, int32_t width,
                                int32_t height) {
                        auto view =
                            emscripten::val(emscripten::typed_memory_view(
                                pixels.size(), pixels.data()));
                        js_func(view, width, height);
                      });
                }))
      .function("requestFrameCapture",
                &lynx::animax::AnimaXWasm::RequestFrameCapture)
      .function("setResourceLoaderImpl",
                emscripten::optional_override([](lynx::animax::AnimaXWasm& self,
                                                 emscripten::val js_func) {
                  auto func = [js_func](
                                  const std::string& url, int32_t result_type,
                                  int32_t content_type, int32_t callback_id) {
                    js_func(url, result_type, content_type, callback_id);
                  };
                  self.SetResourceLoaderImpl(func);
                }))
      .function(
          "updateLayerProperty",
          emscripten::optional_override(
              [](lynx::animax::AnimaXWasm& self, uint16_t layer_type,
                 const std::string& layer_name_utf8, emscripten::val value,
                 emscripten::val js_callback) {
                lynx::animax::AnimaXWasm::PropertyUpdateCallback cpp_callback =
                    [js_callback](bool success, uint16_t error) {
                      js_callback(success, error);
                    };
                self.UpdateLayerProperty(layer_type, layer_name_utf8, value,
                                         cpp_callback);
              }))
      .function(
          "getLayerBounds",
          emscripten::optional_override([](lynx::animax::AnimaXWasm& self,
                                           const std::string& layer_name_utf8,
                                           uint16_t bounds_space,
                                           emscripten::val js_callback) {
            lynx::animax::AnimaXWasm::LayerBoundsCallback cpp_callback =
                [js_callback](bool success, float x, float y, float width,
                              float height) {
                  js_callback(success, x, y, width, height);
                };
            self.GetLayerBounds(layer_name_utf8, bounds_space, cpp_callback);
          }))
      .function(
          "setResourceProperty",
          emscripten::optional_override(
              [](lynx::animax::AnimaXWasm& self, uint16_t resource_type,
                 const std::string& resource_id, emscripten::val value,
                 emscripten::val js_callback) {
                lynx::animax::AnimaXWasm::PropertyUpdateCallback cpp_callback =
                    [js_callback](bool success, uint16_t error) {
                      js_callback(success, error);
                    };
                self.SetResourceProperty(resource_type, resource_id, value,
                                         cpp_callback);
              }));
}
