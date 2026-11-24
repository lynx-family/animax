// Copyright 2021 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

// This file has been auto-generated from the Jinja2 template
// third_party/binding/idl-codegen/templates/napi_interface.h.tmpl
// by the script code_generator_napi.py.
// DO NOT MODIFY!

// clang-format off
#ifndef ANIMAX_ANIMAX_SRC_JSBRIDGE_BINDINGS_ANIMAX_NAPI_ALPHA_VIDEO_H_
#define ANIMAX_ANIMAX_SRC_JSBRIDGE_BINDINGS_ANIMAX_NAPI_ALPHA_VIDEO_H_

#include <memory>

#include "third_party/binding/napi/napi_bridge.h"
#include "third_party/binding/napi/native_value_traits.h"

namespace lynx {
namespace animax {

using binding::NapiBridge;
using binding::ImplBase;

class PreloadedAlphaVideo;

class NapiAlphaVideo : public NapiBridge {
 public:
  NapiAlphaVideo(const Napi::CallbackInfo&, bool skip_init_as_base = false);

  PreloadedAlphaVideo* ToImplUnsafe();

  static Napi::Object Wrap(std::unique_ptr<PreloadedAlphaVideo>, Napi::Env);
  static bool IsInstance(Napi::ScriptWrappable*);

  void Init(std::unique_ptr<PreloadedAlphaVideo>);

  // Attributes

  // Methods
  Napi::Value DisposeMethod(const Napi::CallbackInfo&);

  // Overload Hubs

  // Overloads

  // Injection hook
  static void Install(Napi::Env, Napi::Object&);

  static Napi::Function Constructor(Napi::Env);
  static Napi::Class* Class(Napi::Env);

  // Interface name
  static constexpr const char* InterfaceName() {
    return "AlphaVideo";
  }

 private:
  std::unique_ptr<PreloadedAlphaVideo> impl_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_ANIMAX_SRC_JSBRIDGE_BINDINGS_ANIMAX_NAPI_ALPHA_VIDEO_H_
