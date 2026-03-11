// Copyright 2021 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

// This file has been auto-generated from the Jinja2 template
// third_party/binding/idl-codegen/templates/napi_dictionary.cc.tmpl
// by the script code_generator_napi.py.
// DO NOT MODIFY!

// clang-format off
#include "jsbridge/bindings/animax/napi_value_param.h"

#include "src/base/log/log.h"
#include "third_party/binding/napi/exception_message.h"

using Napi::Array;
using Napi::Number;
using Napi::Object;
using Napi::ObjectWrap;
using Napi::String;
using Napi::TypeError;
using Napi::Value;

using lynx::binding::IDLBoolean;
using lynx::binding::IDLDouble;
using lynx::binding::IDLFloat;
using lynx::binding::IDLFunction;
using lynx::binding::IDLNumber;
using lynx::binding::IDLString;
using lynx::binding::IDLUnrestrictedFloat;
using lynx::binding::IDLUnrestrictedDouble;
using lynx::binding::IDLNullable;
using lynx::binding::IDLObject;
using lynx::binding::IDLRecord;
using lynx::binding::IDLTypedArray;
using lynx::binding::IDLArrayBuffer;
using lynx::binding::IDLArrayBufferView;
using lynx::binding::IDLDictionary;
using lynx::binding::IDLSequence;
using lynx::binding::NativeValueTraits;

using lynx::binding::ExceptionMessage;

namespace lynx {
namespace animax {

// static
std::unique_ptr<ValueParam> ValueParam::ToImpl(const Value& info) {
  if (!info.IsObject()) {
    ExceptionMessage::NonObjectReceived(info.Env(), DictionaryName());
    return nullptr;
  }

  return std::make_unique<ValueParam>(info);
}

Object ValueParam::ToJsObject(Napi::Env env) {
  auto obj = Object::New(env);

  if (hasBoolValue()) {
    obj["boolValue"] = Napi::Boolean::New(env, boolValue_);
  }
  if (hasDoubleValue()) {
    obj["doubleValue"] = Number::New(env, doubleValue_);
  }
  if (hasFrameIndex()) {
    obj["frameIndex"] = Number::New(env, frameIndex_);
  }
  if (hasPointX()) {
    obj["pointX"] = Number::New(env, pointX_);
  }
  if (hasPointY()) {
    obj["pointY"] = Number::New(env, pointY_);
  }
  if (hasStringValue()) {
    obj["stringValue"] = String::New(env, stringValue_);
  }

  return obj;
}

ValueParam::ValueParam(const Value& info)
    {
  Object obj = info.As<Object>();
  if (obj.Has("boolValue").FromMaybe(false)) {
    Value boolValue_val = obj.Get("boolValue");
    if (!boolValue_val.IsUndefined()) {
      boolValue_ = NativeValueTraits<IDLBoolean>::NativeValue(boolValue_val);
      has_boolValue_ = true;
    }
  }

  if (obj.Has("doubleValue").FromMaybe(false)) {
    Value doubleValue_val = obj.Get("doubleValue");
    if (!doubleValue_val.IsUndefined()) {
      doubleValue_ = NativeValueTraits<IDLDouble>::NativeValue(doubleValue_val);
      has_doubleValue_ = true;
    }
  }

  if (obj.Has("frameIndex").FromMaybe(false)) {
    Value frameIndex_val = obj.Get("frameIndex");
    if (!frameIndex_val.IsUndefined()) {
      frameIndex_ = NativeValueTraits<IDLNumber>::NativeValue(frameIndex_val);
      has_frameIndex_ = true;
    }
  }

  if (obj.Has("pointX").FromMaybe(false)) {
    Value pointX_val = obj.Get("pointX");
    if (!pointX_val.IsUndefined()) {
      pointX_ = NativeValueTraits<IDLDouble>::NativeValue(pointX_val);
      has_pointX_ = true;
    }
  }

  if (obj.Has("pointY").FromMaybe(false)) {
    Value pointY_val = obj.Get("pointY");
    if (!pointY_val.IsUndefined()) {
      pointY_ = NativeValueTraits<IDLDouble>::NativeValue(pointY_val);
      has_pointY_ = true;
    }
  }

  if (obj.Has("stringValue").FromMaybe(false)) {
    Value stringValue_val = obj.Get("stringValue");
    if (!stringValue_val.IsUndefined()) {
      stringValue_ = NativeValueTraits<IDLString>::NativeValue(stringValue_val);
      has_stringValue_ = true;
    }
  }
}

}  // namespace animax
}  // namespace lynx
