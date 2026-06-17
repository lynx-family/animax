// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_BASE_UTIL_ANDROID_EGL_EXT_FUNCTIONS_H_
#define ANIMAX_SRC_BASE_UTIL_ANDROID_EGL_EXT_FUNCTIONS_H_

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "base/include/no_destructor.h"

namespace lynx {
namespace animax {

// EGL extension function pointer types, loaded via eglGetProcAddress.
// EGL_KHR_fence_sync / EGL_ANDROID_native_fence_sync:
using CreateSyncFn = EGLSyncKHR(EGLDisplay, EGLenum, const EGLint*);
using DestroySyncFn = EGLBoolean(EGLDisplay, EGLSyncKHR);
using DupNativeFenceFn = EGLint(EGLDisplay, EGLSyncKHR);

// Central loader for EGL extension functions as a process-wide singleton
// (mirrors HardwareBufferFunctions). New EGL extension entry points
// (e.g. EGL_KHR_image) should be added here and exposed as thin wrappers
// instead of being loaded ad hoc per call site. Currently wraps the EGL native
// fence sync functions used by the GL -> Vulkan video bridge.
class EglExtFunctions {
 public:
  // Whether the EGL native fence sync functions could be loaded. create_sync /
  // dup are required to produce a fence fd; destroy may still be null
  // (DestroySync is best-effort then).
  static bool IsFenceSyncSupported();
  static EglExtFunctions& GetInstance();

  EglExtFunctions(const EglExtFunctions&) = delete;
  EglExtFunctions& operator=(const EglExtFunctions&) = delete;

  // Thin wrappers. CreateSync / DupNativeFenceFD are guaranteed non-null when
  // IsFenceSyncSupported(); DestroySync tolerates a null load (no-op).
  EGLSyncKHR CreateSync(EGLDisplay display, EGLenum type,
                        const EGLint* attrib_list);
  EGLBoolean DestroySync(EGLDisplay display, EGLSyncKHR sync);
  EGLint DupNativeFenceFD(EGLDisplay display, EGLSyncKHR sync);

 private:
  friend class lynx::base::NoDestructor<EglExtFunctions>;
  EglExtFunctions();
  CreateSyncFn* create_sync_ = nullptr;
  DestroySyncFn* destroy_sync_ = nullptr;
  DupNativeFenceFn* dup_native_fence_ = nullptr;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_BASE_UTIL_ANDROID_EGL_EXT_FUNCTIONS_H_
