// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_JSBRIDGE_BINDINGS_ANIMAX_PRELOAD_LOADER_H_
#define ANIMAX_SRC_JSBRIDGE_BINDINGS_ANIMAX_PRELOAD_LOADER_H_

#include "third_party/binding/napi/shim/shim_napi.h"

namespace lynx {
namespace animax {

// Register the independent load functions on this Env's JS thread. Registration
// does not acquire host services or start loading; each invocation looks up its
// current PreloadHost. Native completions use (ok, value, code, message), which
// the SDK adapts to Promises.
void InstallPreloadLoaders(Napi::Env env, Napi::Object& module);

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_JSBRIDGE_BINDINGS_ANIMAX_PRELOAD_LOADER_H_
