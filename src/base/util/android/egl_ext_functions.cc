// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/base/util/android/egl_ext_functions.h"

#include "src/base/gl/gl_include.h"
#include "src/base/log/log.h"

namespace lynx {
namespace animax {

EglExtFunctions::EglExtFunctions() {
  // eglGetProcAddress is safe to call without a current EGL context; the
  // returned pointers are valid for the lifetime of the process.
  create_sync_ =
      reinterpret_cast<CreateSyncFn*>(eglGetProcAddress("eglCreateSyncKHR"));
  destroy_sync_ =
      reinterpret_cast<DestroySyncFn*>(eglGetProcAddress("eglDestroySyncKHR"));
  dup_native_fence_ = reinterpret_cast<DupNativeFenceFn*>(
      eglGetProcAddress("eglDupNativeFenceFDANDROID"));
}

EglExtFunctions& EglExtFunctions::GetInstance() {
  static lynx::base::NoDestructor<EglExtFunctions> functions;
  return *functions;
}

bool EglExtFunctions::IsFenceSyncSupported() {
  EglExtFunctions& fns = GetInstance();
  return fns.create_sync_ != nullptr && fns.dup_native_fence_ != nullptr &&
         fns.destroy_sync_ != nullptr;
}

EGLSyncKHR EglExtFunctions::CreateSync(EGLDisplay display, EGLenum type,
                                       const EGLint* attrib_list) {
  DCHECK(create_sync_);
  return create_sync_(display, type, attrib_list);
}

EGLBoolean EglExtFunctions::DestroySync(EGLDisplay display, EGLSyncKHR sync) {
  // destroy_sync_ is not covered by IsFenceSyncSupported(); tolerate a null
  // load so the caller can always call DestroySync to best-effort release the
  // sync object.
  if (destroy_sync_ == nullptr) {
    return EGL_FALSE;
  }
  return destroy_sync_(display, sync);
}

EGLint EglExtFunctions::DupNativeFenceFD(EGLDisplay display, EGLSyncKHR sync) {
  DCHECK(dup_native_fence_);
  return dup_native_fence_(display, sync);
}

}  // namespace animax
}  // namespace lynx
