// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PLAYER_WEB_ANIMAX_WASM_H_
#define ANIMAX_SRC_PLAYER_WEB_ANIMAX_WASM_H_

#include <emscripten/val.h>

#include <cstdint>
#include <functional>
#include <map>
#include <string>

#include "base/include/no_destructor.h"

namespace lynx {
namespace animax {

class AnimaXPlayer;
class AnimaXPlayerBuilder;
class AnimaXWebGPUContext;
class ResourceLoaderWeb;

class AnimaXWasm : std::enable_shared_from_this<AnimaXWasm> {
 public:
  AnimaXWasm();
  ~AnimaXWasm();

  std::shared_ptr<AnimaXPlayer> GetPlayer() { return player_; }

  void SetSurfaceWithCanvas(
      const std::string& canvas_id, int32_t width, int32_t height,
      const std::shared_ptr<AnimaXWebGPUContext>& web_gpu_ctx,
      uintptr_t gl_context_handle = 0);

  using ResourceLoaderImpl =
      std::function<void(const std::string& url, int32_t result_type,
                         int32_t content_type, int32_t callback_id)>;
  void SetResourceLoaderImpl(ResourceLoaderImpl impl);

  static void OnResourceLoadSuccess(int32_t callback_id,
                                    const emscripten::val& buffer,
                                    uint32_t width, uint32_t height,
                                    uint64_t texture);
  static void OnResourceLoadError(int32_t callback_id,
                                  const std::string& error_message);

  static bool SetDefaultTypefaceWithData(const emscripten::val& buffer);

  using EventCallback = std::function<void(const std::string& event_name,
                                           const emscripten::val& params)>;
  void SetEventCallback(EventCallback callback);

  void UpdateVisibilityStates(uint16_t states);

  using PropertyUpdateCallback =
      std::function<void(bool success, uint16_t error)>;

  void UpdateLayerProperty(uint16_t layer_type,
                           const std::string& layer_name_utf8,
                           const emscripten::val& value,
                           PropertyUpdateCallback callback);

  void SetResourceProperty(uint16_t resource_type,
                           const std::string& resource_id,
                           const emscripten::val& value,
                           PropertyUpdateCallback callback);

 private:
  std::shared_ptr<AnimaXPlayer> player_;

  std::shared_ptr<ResourceLoaderWeb> resource_loader_;

  uint16_t current_visible_states_ = 0;

  static base::NoDestructor<std::map<AnimaXPlayer*, EventCallback>>
      event_callback_map_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PLAYER_WEB_ANIMAX_WASM_H_
