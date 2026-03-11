// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PROPERTY_PROPERTY_UPDATER_H_
#define ANIMAX_SRC_PROPERTY_PROPERTY_UPDATER_H_

#include <memory>
#include <vector>

#include "include/property/property_update_request.h"

namespace lynx {
namespace animax {

class AnimaXKeyPath;

/**
 * Interface for property update operations.
 * Defines the standard methods for updating layer and resource properties.
 */
class PropertyUpdater {
 public:
  virtual ~PropertyUpdater() = default;

  virtual void GetKeysForKeyPath(std::unique_ptr<AnimaXKeyPath> key_path,
                                 AnimaXKeyPathCallback callback) = 0;

  /**
   * Updates a static layer property (keyframe data).
   * @param request The layer static property update request
   */
  virtual void UpdateLayerProperty(
      std::unique_ptr<LayerStaticRequest> request) = 0;

  /**
   * Updates a resource property.
   * @param request The resource property update request
   */
  virtual void SetResourceProperty(
      std::unique_ptr<ResourceUpdateRequest> request) = 0;

  /**
   * Adds a value callback for dynamic layer property updates.
   * @param request The layer callback request
   */
  virtual void AddLayerPropertyCallback(
      std::unique_ptr<LayerCallbackRequest> request) = 0;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PROPERTY_PROPERTY_UPDATER_H_
