// Copyright 2021 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

// This file has been auto-generated from the Jinja2 template
// third_party/binding/idl-codegen/templates/napi_interface.cc.tmpl
// by the script code_generator_napi.py.
// DO NOT MODIFY!

// clang-format off
#include "jsbridge/bindings/animax/napi_font.h"

#include <vector>
#include <utility>

#include "base/include/vector.h"

#include "src/base/log/log.h"
#include "src/jsbridge/bindings/animax/preloaded_resource.h"
#include "third_party/binding/napi/exception_message.h"
#include "third_party/binding/napi/napi_base_wrap.h"

#ifdef USE_PRIMJS_NAPI
#include "third_party/napi/include/primjs_napi_defines.h"
#endif

using Napi::Array;
using Napi::CallbackInfo;
using Napi::Error;
using Napi::Function;
using Napi::FunctionReference;
using Napi::Number;
using Napi::Object;
using Napi::ObjectWrap;
using Napi::String;
using Napi::TypeError;
using Napi::Value;

using Napi::ArrayBuffer;
using Napi::Int8Array;
using Napi::Uint8Array;
using Napi::Int16Array;
using Napi::Uint16Array;
using Napi::Int32Array;
using Napi::Uint32Array;
using Napi::Float32Array;
using Napi::Float64Array;
using Napi::DataView;

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
using lynx::binding::IDLInt8Array;
using lynx::binding::IDLInt16Array;
using lynx::binding::IDLInt32Array;
using lynx::binding::IDLUint8ClampedArray;
using lynx::binding::IDLUint8Array;
using lynx::binding::IDLUint16Array;
using lynx::binding::IDLUint32Array;
using lynx::binding::IDLFloat32Array;
using lynx::binding::IDLFloat64Array;
using lynx::binding::IDLArrayBuffer;
using lynx::binding::IDLArrayBufferView;
using lynx::binding::IDLDictionary;
using lynx::binding::IDLSequence;
using lynx::binding::NativeValueTraits;

using lynx::binding::ExceptionMessage;

namespace lynx {
namespace animax {

namespace {
const uint64_t kFontClassID = reinterpret_cast<uint64_t>(&kFontClassID);
const uint64_t kFontConstructorID = reinterpret_cast<uint64_t>(&kFontConstructorID);

using Wrapped = binding::NapiBaseWrapped<NapiFont>;
typedef Value (NapiFont::*InstanceCallback)(const CallbackInfo& info);
typedef void (NapiFont::*InstanceSetterCallback)(const CallbackInfo& info, const Value& value);

__attribute__((unused))
void AddAttribute(base::Vector<Wrapped::PropertyDescriptor>& props,
                  const char* name,
                  InstanceCallback getter,
                  InstanceSetterCallback setter) {
  props.push_back(
      Wrapped::InstanceAccessor(name, getter, setter, napi_default_jsproperty));
}

__attribute__((unused))
void AddInstanceMethod(base::Vector<Wrapped::PropertyDescriptor>& props,
                       const char* name,
                       InstanceCallback method) {
  props.push_back(
      Wrapped::InstanceMethod(name, method, napi_default_jsproperty));
}
}  // namespace

NapiFont::NapiFont(const CallbackInfo& info, bool skip_init_as_base)
    : NapiBridge(info) {
  set_type_id((void*)&kFontClassID);

  // If this is a base class or created by native, skip initialization since
  // impl side needs to have control over the construction of the impl object.
  if (skip_init_as_base || (info.Length() == 1 && info[0].IsExternal())) {
    return;
  }
  ExceptionMessage::IllegalConstructor(info.Env(), InterfaceName());
  return;
}

PreloadedFont* NapiFont::ToImplUnsafe() {
  return impl_.get();
}

// static
Object NapiFont::Wrap(std::unique_ptr<PreloadedFont> impl, Napi::Env env) {
  DCHECK(impl);
  auto obj = Constructor(env).New({Napi::External::New(env, nullptr, nullptr, nullptr)});
  ObjectWrap<NapiFont>::Unwrap(obj)->Init(std::move(impl));
  return obj;
}

// static
bool NapiFont::IsInstance(Napi::ScriptWrappable* wrappable) {
  if (!wrappable) {
    return false;
  }
  if (static_cast<NapiFont*>(wrappable)->type_id() == &kFontClassID) {
    return true;
  }
  return false;
}

void NapiFont::Init(std::unique_ptr<PreloadedFont> impl) {
  DCHECK(impl);
  DCHECK(!impl_);

  impl_ = std::move(impl);

  // We only associate and call OnWrapped() once, when we init the root base.
  impl_->AssociateWithWrapper(this);
}

Value NapiFont::DisposeMethod(const CallbackInfo& info) {
  DCHECK(impl_);

  impl_->Dispose();
  return info.Env().Undefined();
}

// static
Napi::Class* NapiFont::Class(Napi::Env env) {
  auto* clazz = env.GetInstanceData<Napi::Class>(kFontClassID);
  if (clazz) {
    return clazz;
  }

  base::InlineVector<Wrapped::PropertyDescriptor, 1> props;

  // Attributes

  // Methods
  AddInstanceMethod(props, "dispose", &NapiFont::DisposeMethod);

  // Cache the class
  clazz = new Napi::Class(Wrapped::DefineClass(env, "Font", props.size(), props.data<const napi_property_descriptor>()));
  env.SetInstanceData<Napi::Class>(kFontClassID, clazz);
  return clazz;
}

// static
Function NapiFont::Constructor(Napi::Env env) {
  FunctionReference* ref = env.GetInstanceData<FunctionReference>(kFontConstructorID);
  if (ref) {
    return ref->Value();
  }

  // Cache the constructor for future use
  ref = new FunctionReference();
  ref->Reset(Class(env)->Get(env), 1);
  env.SetInstanceData<FunctionReference>(kFontConstructorID, ref);
  return ref->Value();
}

// static
void NapiFont::Install(Napi::Env env, Object& target) {
  if (target.Has("Font").FromMaybe(false)) {
    return;
  }
  target.Set("Font", Constructor(env));
}

}  // namespace animax
}  // namespace lynx
