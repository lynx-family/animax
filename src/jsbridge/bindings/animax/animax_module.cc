// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/jsbridge/bindings/animax/napi_composition_element.h"
#include "src/jsbridge/bindings/animax/preload_loader.h"
#include "src/jsbridge/bindings/animax/preloaded_resource.h"

#ifdef USE_PRIMJS_NAPI
#include "third_party/napi/include/primjs_napi_defines.h"
#endif

namespace lynx {
namespace animax {

Napi::Object Load(Napi::Env env, Napi::Object animax) {
  NapiCompositionElement::Install(env, animax);
  InstallPreloadBindings(env, animax);
  InstallPreloadLoaders(env, animax);
  return animax;
}

NODE_API_MODULE(animax, Load)

}  // namespace animax
}  // namespace lynx
