// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_JSBRIDGE_BINDINGS_ANIMAX_ANIMAX_PLAYER_DELEGATE_H_
#define ANIMAX_SRC_JSBRIDGE_BINDINGS_ANIMAX_ANIMAX_PLAYER_DELEGATE_H_

#include "base/include/closure.h"
#include "src/jsbridge/bindings/animax/napi_task_runner.h"
#include "src/jsbridge/bindings/animax/napi_value_param.h"
#include "third_party/binding/napi/napi_bridge.h"

namespace lynx {
namespace animax {

class AnimaXPlayer;
class NapiOnPropertyCallback;
class NapiLayerPropertyCallback;
class NapiOnLayerBoundsCallback;

/**
 * AnimaXPlayerDelegate is a delegate that bridge the call between napi and
 * AnimaXPlayer
 *
 * A property Napi call to AnimaX runtime include following steps:
 * 1. Unwrap the Napi data
 * 2. Convert Napi types to AnimaX types
 * 3. Execute property update via AnimaXPropertyUpdater
 * 4. Convert result back to Napi and send to JS runtime
 */
class AnimaXPlayerDelegate : public binding::ImplBase {
 public:
  static std::unique_ptr<AnimaXPlayerDelegate> Create(const std::string& id);

  explicit AnimaXPlayerDelegate(std::weak_ptr<AnimaXPlayer> weak_player);
  ~AnimaXPlayerDelegate() override;

  /**
   * Update the layer property by type, layer_name and value param
   * @param type The type of layer property to update
   * @param layer_name The name of the layer to update
   * @param value The value to update
   * @param callback Callback to be invoked when the operation completes
   */
  void UpdateLayerProperty(const Napi::Number& type,
                           const Napi::String& layer_name,
                           std::unique_ptr<ValueParam> value,
                           std::unique_ptr<NapiOnPropertyCallback> callback);

  /**
   * Set the resource property by type, id and value param
   * @param type The type of resource property to update
   * @param id The resource id to update
   * @param value The value to update
   * @param callback Callback to be invoked when the operation completes
   */
  void SetResourceProperty(const Napi::Number& type, const Napi::String& id,
                           std::unique_ptr<ValueParam> value,
                           std::unique_ptr<NapiOnPropertyCallback> callback);

  /**
   * Load assets from resource properties.
   *  @param callback Callback to be invoked when the operation completes
   */
  void SubmitResourcePropertiesUpdate(
      std::unique_ptr<NapiOnPropertyCallback> callback);

  /**
   * Get the bounds of a layer.
   * @param layer_name The name of the layer to get bounds of
   * @param layerBoundsSpace The space to get bounds of
   * @param callback Callback to be invoked when the operation completes
   */
  void GetLayerBounds(const Napi::String& layer_name,
                      const Napi::Number& layer_bounds_space,
                      std::unique_ptr<NapiOnLayerBoundsCallback> callback);

  /**
   * Run on GPU thread and call AnimaXPlayer's play function
   */
  void Play();

 private:
  std::weak_ptr<AnimaXPlayer> weak_player_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_JSBRIDGE_BINDINGS_ANIMAX_ANIMAX_PLAYER_DELEGATE_H_
