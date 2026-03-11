// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PROPERTY_PROPERTY_UPDATE_CONTEXT_H_
#define ANIMAX_SRC_PROPERTY_PROPERTY_UPDATE_CONTEXT_H_

#include <memory>
#include <string>

#include "include/property/property_update_request.h"
#include "include/property/property_update_response.h"
#include "src/property/animax_value_callback.h"

namespace lynx {
namespace animax {

class AnimaXKeyPath;
class AnimaXValueParam;

/**
 * Context for property updates.
 * Contains the update request and error information.
 * This acts as an adapter between the new request types and the KeyPathElement
 * interface.
 */
class PropertyUpdateContext {
 public:
  /**
   * Creates a new PropertyUpdateContext with a layer static request.
   * @param request The layer static property update request
   */
  explicit PropertyUpdateContext(
      std::unique_ptr<PropertyUpdateRequest> request);

  /**
   * Invokes the callback with an error response.
   * Sets the error type in the response and then invokes the request's
   * callback.
   * @param error_type The type of error that occurred
   */
  void InvokeErrorCallback(PropertyUpdateResult error_type);

  /**
   * Invokes the callback with the response.
   */
  void InvokeCallback();

  /**
   * Checks if the value is null.
   * @return true if the value is null, false otherwise
   */
  bool IsValueNull() const;

  /**
   * Sets the error type for the overall response.
   * @param error_type The type of error that occurred
   */
  void SetErrorType(PropertyUpdateResult error_type);

  /**
   * Add error type to the response.
   * @param target The target of the error
   * @param error_type The type of error that occurred
   */
  void AddErrorType(std::string target, PropertyUpdateResult error_type);

  /**
   * Gets the response.
   * @return The response
   */
  PropertyUpdateResponse& GetResponse();

  /**
   * Checks if this request contains a value callback.
   * @return True if this is a callback request or has a callback
   */
  bool HasValueCallback() const;

  /**
   * Gets the shared value callback.
   * @return The value callback, or nullptr if no callback exists
   */
  std::shared_ptr<AnimaXValueCallback> GetValueCallback();

  /**
   * Gets the layer property type.
   * @return The layer property type
   */
  LayerPropertyType GetLayerType() const;

  /**
   * Gets the resource property type.
   * @return The resource property type
   */
  ResourcePropertyType GetResourceType() const;

  /**
   * Gets the key path for layer property updates.
   * @return The key path
   */
  const AnimaXKeyPath& GetKeyPath() const;

  /**
   * Gets the resource ID for resource property updates.
   * @return The resource ID
   */
  const std::string& GetResourceId() const;

  /**
   * Gets the value parameter for the update.
   * @return The value parameter
   */
  const AnimaXValueParam& GetValue() const;

  /**
   * Check if this is a static layer or callback layer request
   * @return True if this is a layer request
   */
  bool IsLayerRequest() const;

  /**
   * Check if this is a static layer request
   * @return True if this is a static layer request
   */
  bool IsStaticLayerRequest() const;

  /**
   * Check if this is a resource request
   * @return True if this is a resource request
   */
  bool IsResourceRequest() const;

  /**
   * Check if this is a layer callback request
   * @return True if this is a layer callback request
   */
  bool IsLayerCallbackRequest() const;

 private:
  template <typename T>
  const T* GetRequest() const {
    return static_cast<const T*>(request_.get());
  }

  std::unique_ptr<PropertyUpdateRequest> request_;

  PropertyUpdateResponse response_;
  std::string current_target_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PROPERTY_PROPERTY_UPDATE_CONTEXT_H_
