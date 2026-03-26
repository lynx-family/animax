// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PROPERTY_ANIMAX_PROPERTY_UPDATER_H_
#define ANIMAX_SRC_PROPERTY_ANIMAX_PROPERTY_UPDATER_H_

#include <memory>

#include "src/property/property_updater.h"

namespace lynx {
namespace animax {

class AnimaXRenderer;

/**
 * Implementation of PropertyUpdater for AnimaX.
 * Handles property updates for both layer and resource properties.
 */
class AnimaXPropertyUpdater : public PropertyUpdater {
 public:
  explicit AnimaXPropertyUpdater(AnimaXRenderer& renderer);

  ~AnimaXPropertyUpdater() override;

  void GetKeysForKeyPath(std::unique_ptr<AnimaXKeyPath> key_path,
                         AnimaXKeyPathCallback callback) override;

  /**
   * Updates a static layer property on the GPU thread.
   * @param request The layer static property update request
   */
  void UpdateLayerProperty(
      std::unique_ptr<LayerStaticRequest> request) override;

  /**
   * Updates a resource property on the GPU thread.
   * @param request The resource property update request
   */
  void SetResourceProperty(
      std::unique_ptr<ResourceUpdateRequest> request) override;

  /**
   * Adds a value callback for dynamic layer property updates on the GPU thread.
   * @param request The layer callback request
   */
  void AddLayerPropertyCallback(
      std::unique_ptr<LayerCallbackRequest> request) override;

 private:
  AnimaXRenderer& renderer_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PROPERTY_ANIMAX_PROPERTY_UPDATER_H_
