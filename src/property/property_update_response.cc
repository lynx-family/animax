// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "include/property/property_update_response.h"

#include <utility>
#include <vector>

namespace lynx {
namespace animax {

namespace {
std::string GetErrorCodeString(PropertyUpdateResult type) {
  return std::to_string(static_cast<int16_t>(type));
}
}  // namespace

PropertyUpdateResponse::PropertyUpdateResponse(PropertyUpdateResult error_type)
    : pre_check_result_(error_type) {}

bool PropertyUpdateResponse::IsSuccess() const {
  return pre_check_result_ == PropertyUpdateResult::kSuccess &&
         target_errors_.empty();
}

void PropertyUpdateResponse::AddErrorType(std::string target,
                                          PropertyUpdateResult error_type) {
  target_errors_[std::move(target)] = error_type;
}

std::vector<std::string> PropertyUpdateResponse::GetUpdateMessages() const {
  if (pre_check_result_ != PropertyUpdateResult::kSuccess) {
    return {"pre-check failure, error_code:" +
            GetErrorCodeString(pre_check_result_)};
  }

  std::vector<std::string> messages;
  messages.reserve(target_errors_.size());
  for (const auto& pair : target_errors_) {
    messages.push_back("<" + pair.first +
                       "> , error_code:" + GetErrorCodeString(pair.second));
  }

  return messages;
}

void PropertyUpdateResponse::SetErrorType(PropertyUpdateResult error_type) {
  pre_check_result_ = error_type;
}

PropertyUpdateResult PropertyUpdateResponse::GetErrorType() const {
  if (pre_check_result_ != PropertyUpdateResult::kSuccess) {
    return pre_check_result_;
  }

  if (target_errors_.empty()) {
    return PropertyUpdateResult::kSuccess;
  } else {
    return target_errors_.begin()->second;
  }
}

}  // namespace animax
}  // namespace lynx
