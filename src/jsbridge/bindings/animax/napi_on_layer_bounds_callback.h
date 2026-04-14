// Copyright 2021 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

// This file has been auto-generated from the Jinja2 template
// third_party/binding/idl-codegen/templates/napi_callback_function.h.tmpl
// by the script code_generator_napi.py.
// DO NOT MODIFY!

// clang-format off
#ifndef ANIMAX_ANIMAX_SRC_JSBRIDGE_BINDINGS_ANIMAX_NAPI_ON_LAYER_BOUNDS_CALLBACK_H_
#define ANIMAX_ANIMAX_SRC_JSBRIDGE_BINDINGS_ANIMAX_NAPI_ON_LAYER_BOUNDS_CALLBACK_H_

#include <utility>
#include <memory>

#include "third_party/binding/napi/callback_helper.h"
#include "third_party/binding/napi/napi_bridge.h"
#include "third_party/binding/napi/native_value_traits.h"

#include "src/base/log/log.h"

namespace lynx {
namespace animax {
extern const uint64_t kNapiOnLayerBoundsCallbackClassID;

class NapiOnLayerBoundsCallback {
 public:
  NapiOnLayerBoundsCallback(Napi::Function callback);

  NapiOnLayerBoundsCallback(const NapiOnLayerBoundsCallback& cb) = delete;

  void Invoke(bool arg0, float arg1, float arg2, float arg3, float arg4) {
    bool valid;
    Napi::Env env = Env(&valid);
    if (!valid) {
      return;
    }

    Napi::ContextScope cs(env);
    Napi::HandleScope hs(env);

    binding::HolderStorage *storage = reinterpret_cast<binding::HolderStorage*>(env.GetInstanceData(kNapiOnLayerBoundsCallbackClassID));
    DCHECK(storage);

    auto cb = storage->PopHolder(reinterpret_cast<uintptr_t>(this));

    Napi::Value arg0_success;
    arg0_success = Napi::Boolean::New(env, arg0);

    Napi::Value arg1_x;
    arg1_x = Napi::Number::New(env, arg1);

    Napi::Value arg2_y;
    arg2_y = Napi::Number::New(env, arg2);

    Napi::Value arg3_width;
    arg3_width = Napi::Number::New(env, arg3);

    Napi::Value arg4_height;
    arg4_height = Napi::Number::New(env, arg4);

    // The JS callback object is stolen after the call.
    binding::CallbackHelper::Invoke(std::move(cb), result_, exception_handler_, { arg0_success, arg1_x, arg2_y, arg3_width, arg4_height });
  }

  Napi::Value GetResult() { return result_; }
  Napi::Env Env(bool *valid);

  void SetExceptionHandler(std::function<void(Napi::Env)> handler) {
    exception_handler_ = std::move(handler);
  }

 private:
  std::weak_ptr<binding::InstanceGuard> storage_guard_;
  Napi::Value result_;
  std::function<void(Napi::Env)> exception_handler_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_ANIMAX_SRC_JSBRIDGE_BINDINGS_ANIMAX_NAPI_ON_LAYER_BOUNDS_CALLBACK_H_
