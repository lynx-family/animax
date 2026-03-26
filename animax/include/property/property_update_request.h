// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_INCLUDE_PROPERTY_PROPERTY_UPDATE_REQUEST_H_
#define ANIMAX_INCLUDE_PROPERTY_PROPERTY_UPDATE_REQUEST_H_

#include <memory>
#include <string>

#include "base/include/closure.h"
#include "include/base/macros.h"
#include "include/property/animax_key_path.h"
#include "include/property/animax_value_param.h"
#include "include/property/property_update_response.h"
#include "src/property/animax_value_callback.h"
#include "src/property/property_type.h"

namespace lynx {
namespace animax {

using AnimaXPropertyCallback =
    base::MoveOnlyClosure<void, const PropertyUpdateResponse&>;

using AnimaXKeyPathCallback =
    base::MoveOnlyClosure<void, std::vector<AnimaXKeyPath>>;

/**
 * Base interface for all property update requests.
 * Provides common functionality and type identification.
 */
class ANIMAX_EXPORT PropertyUpdateRequest {
 public:
  /**
   * The type of request, indicating which concrete subclass is used.
   */
  enum class Type { kLayerStatic, kLayerCallback, kResource };

  /**
   * Virtual destructor for proper cleanup
   */
  virtual ~PropertyUpdateRequest() = default;

  /**
   * Returns the type of this request
   */
  Type GetRequestType() const;

  /**
   * Invokes the callback with the given response.
   * @param response The response to pass to the callback
   */
  void InvokeCallback(const PropertyUpdateResponse& response) const;

 protected:
  /**
   * Constructor for the base class
   * @param callback The callback to be invoked when the operation completes
   */
  PropertyUpdateRequest(Type type, AnimaXPropertyCallback callback);

  /**
   * The completion callback
   */
  const AnimaXPropertyCallback callback_;

 private:
  const Type type_;
};

/**
 * Request for static layer property updates (one-time updates to keyframes).
 */
class ANIMAX_EXPORT LayerStaticRequest : public PropertyUpdateRequest {
 public:
  /**
   * Creates a layer property update request.
   * @param type The type of layer property to update
   * @param key_path The key path identifying the property to update
   * @param value The new value for the property
   * @param callback Callback to be invoked when the operation completes
   */
  static std::unique_ptr<LayerStaticRequest> Make(
      LayerPropertyType type, std::unique_ptr<AnimaXKeyPath> key_path,
      std::unique_ptr<AnimaXValueParam> value, AnimaXPropertyCallback callback);

  /**
   * Gets the layer property type.
   */
  LayerPropertyType GetLayerType() const;

  /**
   * Gets the key path for layer property updates.
   */
  const AnimaXKeyPath& GetKeyPath() const;

  /**
   * Gets the value parameter for the update.
   */
  const AnimaXValueParam& GetValue() const;

 private:
  LayerStaticRequest(LayerPropertyType type,
                     std::unique_ptr<AnimaXKeyPath> key_path,
                     std::unique_ptr<AnimaXValueParam> value,
                     AnimaXPropertyCallback callback);

  const LayerPropertyType layer_type_;
  std::unique_ptr<AnimaXKeyPath> key_path_;
  std::unique_ptr<AnimaXValueParam> value_;
};

/**
 * Request for dynamic layer property updates via callbacks.
 */
class ANIMAX_EXPORT LayerCallbackRequest : public PropertyUpdateRequest {
 public:
  /**
   * Creates a layer property update request with a value callback.
   * @param type The type of layer property to update
   * @param key_path The key path identifying the property to update
   * @param value_callback The callback to use for dynamic value updates
   */
  static std::unique_ptr<LayerCallbackRequest> Make(
      LayerPropertyType type, std::unique_ptr<AnimaXKeyPath> key_path,
      std::shared_ptr<AnimaXValueCallback> value_callback,
      AnimaXPropertyCallback callback);

  /**
   * Gets the layer property type.
   */
  LayerPropertyType GetLayerType() const;

  /**
   * Gets the key path for layer property updates.
   */
  const AnimaXKeyPath& GetKeyPath() const;

  /**
   * Gets the value callback.
   */
  std::shared_ptr<AnimaXValueCallback> GetValueCallback() const;

 private:
  LayerCallbackRequest(LayerPropertyType type,
                       std::unique_ptr<AnimaXKeyPath> key_path,
                       std::shared_ptr<AnimaXValueCallback> value_callback,
                       AnimaXPropertyCallback callback);

  const LayerPropertyType layer_type_;
  std::unique_ptr<AnimaXKeyPath> key_path_;
  std::shared_ptr<AnimaXValueCallback> value_callback_;
};

/**
 * Request for resource property updates.
 */
class ANIMAX_EXPORT ResourceUpdateRequest : public PropertyUpdateRequest {
 public:
  /**
   * Creates a resource property update request.
   * @param type The type of resource property to update
   * @param resource_id The resource id identifying the resource property to
   * update
   * @param value The new value for the property
   * @param callback Callback to be invoked when the operation completes
   */
  static std::unique_ptr<ResourceUpdateRequest> Make(
      ResourcePropertyType type, std::string resource_id,
      std::unique_ptr<AnimaXValueParam> value, AnimaXPropertyCallback callback);

  /**
   * Gets the resource property type.
   */
  ResourcePropertyType GetResourceType() const;

  /**
   * Gets the resource ID for resource property updates.
   */
  const std::string& GetResourceId() const;

  /**
   * Gets the value parameter for the update.
   */
  const AnimaXValueParam& GetValue() const;

 private:
  ResourceUpdateRequest(ResourcePropertyType type, std::string resource_id,
                        std::unique_ptr<AnimaXValueParam> value,
                        AnimaXPropertyCallback callback);

  const ResourcePropertyType resource_type_;
  std::string resource_id_;
  std::unique_ptr<AnimaXValueParam> value_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_INCLUDE_PROPERTY_PROPERTY_UPDATE_REQUEST_H_
