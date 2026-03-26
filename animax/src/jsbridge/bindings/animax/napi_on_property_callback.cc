// Copyright 2021 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

// This file has been auto-generated from the Jinja2 template
// third_party/binding/idl-codegen/templates/napi_callback_function.cc.tmpl
// by the script code_generator_napi.py.
// DO NOT MODIFY!

// clang-format off
#include "jsbridge/bindings/animax/napi_on_property_callback.h"

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

namespace lynx {
namespace animax {
const uint64_t kNapiOnPropertyCallbackClassID = reinterpret_cast<uint64_t>(&kNapiOnPropertyCallbackClassID);

NapiOnPropertyCallback::NapiOnPropertyCallback(Napi::Function callback) {
  Napi::Env env = callback.Env();
  binding::HolderStorage *storage = reinterpret_cast<binding::HolderStorage*>(env.GetInstanceData(kNapiOnPropertyCallbackClassID));
  if (storage == nullptr) {
    storage = new binding::HolderStorage();
    env.SetInstanceData(kNapiOnPropertyCallbackClassID, storage, [](napi_env env, void* finalize_data,
                                                                   void* finalize_hint) { delete reinterpret_cast<binding::HolderStorage*>(finalize_data); }, nullptr);
  }

  storage->PushHolder(reinterpret_cast<uintptr_t>(this), Napi::Persistent(callback));

  storage_guard_ = storage->instance_guard();
}

Napi::Env NapiOnPropertyCallback::Env(bool *valid) {
  if (valid != nullptr) {
    *valid = false;
  }

  auto strong_guard = storage_guard_.lock();
  if (!strong_guard) {
    // if valid is nullptr, it must be valid.
    DCHECK(valid);
    return Napi::Env(nullptr);
  }

  auto storage = strong_guard->Get();
  auto &cb = storage->PeekHolder(reinterpret_cast<uintptr_t>(this));
  if (cb.IsEmpty()) {
    // if valid is nullptr, it must be valid.
    DCHECK(valid);
    return Napi::Env(nullptr);
  }

  if (valid != nullptr) {
    *valid = true;
  }
  return cb.Env();
}

}  // namespace animax
}  // namespace lynx
