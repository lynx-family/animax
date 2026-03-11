// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_INCLUDE_PROPERTY_PROPERTY_UPDATE_RESPONSE_H_
#define ANIMAX_INCLUDE_PROPERTY_PROPERTY_UPDATE_RESPONSE_H_

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "include/base/macros.h"

namespace lynx {
namespace animax {

enum class PropertyUpdateResult : uint16_t {
  kSuccess = 0,
  kPropertyNull,            // Target property is null
  kPropertyInvalidIndex,    // Target property keyframe index is not valid
  kPropertyNotImplemented,  // Target property update is not implemented
  kPropertyAnimationNull,   // Target property animation is null
  kEnvInvalid,              // Composition layer or model is nullptr
  kResourceNotFound,        // Get resource failed by resource id
  kResourceTypeInvalid,     // Target resource property target is not valid
  kElementInvalid,          // The element is not valid
  kValueInvalid,            // The property value is not valid
  kFrameIndexInvalid,       // The frame index is not valid
  kKeyPathMismatched,       // Key path does not match any elements
  kPropertyNotFound,        // Property not found in the matching elements
  kInvalidOperation,  // Operation not supported or invalid in this context
  kEnumCount,
};

/**
 * Property update response that will return back to JS
 */
class ANIMAX_EXPORT PropertyUpdateResponse {
 public:
  PropertyUpdateResponse() = default;
  explicit PropertyUpdateResponse(PropertyUpdateResult error_type);
  virtual ~PropertyUpdateResponse() = default;

  // Determines if the operation was successful (no errors reported).
  bool IsSuccess() const;

  /**
   * Adds a new error to the response.
   * @param target Description of the target that failed.
   * @param error_type The type of error that occurred.
   */
  void AddErrorType(std::string target, PropertyUpdateResult error_type);

  /**
   * Retrieves all formatted update messages.
   * @return A vector of strings, each containing a target and its
   * corresponding update message.
   */
  std::vector<std::string> GetUpdateMessages() const;

  /**
   * Set the pre-check update result
   *
   */
  void SetErrorType(PropertyUpdateResult error_type);

  /**
   * Get the error type object
   *
   * @return PropertyUpdateResult
   */
  PropertyUpdateResult GetErrorType() const;

 private:
  // Stores the result of a pre-update validation. If this check fails, the
  // entire update operation is aborted.
  PropertyUpdateResult pre_check_result_ = PropertyUpdateResult::kSuccess;
  // Maps target identifiers to their individual update results. This is
  // populated only if the pre-update validation passes.
  std::unordered_map<std::string, PropertyUpdateResult> target_errors_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_INCLUDE_PROPERTY_PROPERTY_UPDATE_RESPONSE_H_
