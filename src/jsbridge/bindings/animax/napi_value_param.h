// Copyright 2021 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

// This file has been auto-generated from the Jinja2 template
// third_party/binding/idl-codegen/templates/napi_dictionary.h.tmpl
// by the script code_generator_napi.py.
// DO NOT MODIFY!

// clang-format off
#ifndef ANIMAX_ANIMAX_SRC_JSBRIDGE_BINDINGS_ANIMAX_NAPI_VALUE_PARAM_H_
#define ANIMAX_ANIMAX_SRC_JSBRIDGE_BINDINGS_ANIMAX_NAPI_VALUE_PARAM_H_

#include "third_party/binding/napi/native_value_traits.h"

namespace lynx {
namespace animax {

class ValueParam {
 public:
  ValueParam() = default;
  ValueParam(const Napi::Value&);

  static std::unique_ptr<ValueParam> ToImpl(const Napi::Value&);

  Napi::Object ToJsObject(Napi::Env);

  bool hasBoolValue() const { return has_boolValue_; }
  bool boolValue() const {
    return boolValue_;
  }

  bool hasDoubleValue() const { return has_doubleValue_; }
  double doubleValue() const {
    return doubleValue_;
  }

  bool hasFrameIndex() const { return has_frameIndex_; }
  int32_t frameIndex() const {
    return frameIndex_;
  }

  bool hasPointX() const { return has_pointX_; }
  double pointX() const {
    return pointX_;
  }

  bool hasPointY() const { return has_pointY_; }
  double pointY() const {
    return pointY_;
  }

  bool hasStringValue() const { return has_stringValue_; }
  std::string stringValue() const {
    return stringValue_;
  }

  bool hasValueApplyMode() const { return has_valueApplyMode_; }
  int32_t valueApplyMode() const {
    return valueApplyMode_;
  }

  // Dictionary name
  static constexpr const char* DictionaryName() {
    return "ValueParam";
  }

 private:
  bool has_boolValue_ = false;
  bool has_doubleValue_ = false;
  bool has_frameIndex_ = true;
  bool has_pointX_ = false;
  bool has_pointY_ = false;
  bool has_stringValue_ = false;
  bool has_valueApplyMode_ = true;

  bool boolValue_;
  double doubleValue_;
  int32_t frameIndex_ = -1;
  double pointX_;
  double pointY_;
  std::string stringValue_;
  int32_t valueApplyMode_ = 0;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_ANIMAX_SRC_JSBRIDGE_BINDINGS_ANIMAX_NAPI_VALUE_PARAM_H_
