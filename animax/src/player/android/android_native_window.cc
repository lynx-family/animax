// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/player/android/android_native_window.h"

#include <android/native_window_jni.h>

#include "src/base/log/log.h"

namespace lynx {
namespace animax {

namespace {
ANativeWindow* NativeWindowFromSurface(jobject surface) {
  JNIEnv* env = base::android::AttachCurrentThread();
  return ANativeWindow_fromSurface(env, surface);
}
}  // namespace

AndroidNativeWindow::AndroidNativeWindow() : window_{nullptr} {}

AndroidNativeWindow::AndroidNativeWindow(ANativeWindow* window, int32_t width,
                                         int32_t height)
    : window_{window} {
  Resize(width, height);
}

AndroidNativeWindow::AndroidNativeWindow(jobject surface, int32_t width,
                                         int32_t height)
    : AndroidNativeWindow{NativeWindowFromSurface(surface), width, height} {}

AndroidNativeWindow::~AndroidNativeWindow() { Release(); }

AndroidNativeWindow::AndroidNativeWindow(AndroidNativeWindow&& other) {
  window_ = other.window_;
  other.window_ = nullptr;
}

AndroidNativeWindow& AndroidNativeWindow::operator=(
    AndroidNativeWindow&& other) {
  Release();
  window_ = other.window_;
  other.window_ = nullptr;
  return *this;
}

void AndroidNativeWindow::Release() {
  if (window_) {
    ANIMAX_LOGI("Release window: " << window_);
    ANativeWindow_release(window_);
    window_ = nullptr;
  }
}

void AndroidNativeWindow::Acquire() {
  if (window_) {
    ANIMAX_LOGI("Acquire window: " << window_);
    ANativeWindow_acquire(window_);
  }
}

void AndroidNativeWindow::Resize(int32_t width, int32_t height) {
  if (!window_ || width <= 0 || height <= 0) {
    return;
  }

  if (ANativeWindow_setBuffersGeometry(window_, width, height,
                                       AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM)) {
    ANIMAX_LOGE("Failed to set ANativeWindow buffers geometry.");
    Release();
  };
}
}  // namespace animax
}  // namespace lynx
