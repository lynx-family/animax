// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PLAYER_ANDROID_ANDROID_NATIVE_WINDOW_H_
#define ANIMAX_SRC_PLAYER_ANDROID_ANDROID_NATIVE_WINDOW_H_

#include <android/native_window.h>

#include "base/include/platform/android/jni_utils.h"

namespace lynx {
namespace animax {

// In the event of a failure to set the buffer's pixel dimensions(in either
// constructors or Resize()), the AndroidNativeWindow will invoke `Release()` to
// clean up resources, effectively resetting the instance to an uninitialized
// state, similar to one created by the default constructor.
class AndroidNativeWindow {
 public:
  /**
   * Default constructor. Initializes an instance without an associated native
   * window.
   */
  AndroidNativeWindow();

  /**
   * Constructs an instance from an existing ANativeWindow pointer. This
   * constructor sets the buffer geometry (width and height) of the native
   * window but does not change its screen dimensions.
   *
   * @param window A pointer to an ANativeWindow object.
   * @param width The width in pixels for the window's buffers.
   * @param height The height in pixels for the window's buffers.
   **/
  AndroidNativeWindow(ANativeWindow* window, int32_t width, int32_t height);

  /**
   * Constructs an instance from a Java surface object. Similar to the
   * ANativeWindow* constructor, it sets the buffer geometry without altering
   * the window's screen dimensions.
   *
   * @param surface A jobject representing a Java Surface.
   * @param width The width in pixels for the window's buffers.
   * @param height The height in pixels for the window's buffers.
   */
  AndroidNativeWindow(jobject surface, int32_t width, int32_t height);

  AndroidNativeWindow(const AndroidNativeWindow&) = delete;
  AndroidNativeWindow& operator=(const AndroidNativeWindow&) = delete;

  AndroidNativeWindow(AndroidNativeWindow&&);
  AndroidNativeWindow& operator=(AndroidNativeWindow&&);

  ~AndroidNativeWindow();

  operator ANativeWindow*() { return window_; }
  operator bool() const { return window_ != nullptr; }

  /**
   * Resizes the buffer of the native window. This changes the buffer's pixel
   * dimensions but does not affect the window's screen size.
   *
   * If the width or height <= 0, this method will be noop.
   * If failed to changes the buffer's pixel dimensions, this
   * AndroidNativeWindow will be released.
   *
   * @param width New width in pixels for the window's buffers.
   * @param height New height in pixels for the window's buffers.
   */
  void Resize(int32_t width, int32_t height);

  /**
   * Releases the associated native window, if any, and resets the internal
   * state of the instance. After calling `Release()`, the `AndroidNativeWindow`
   * instance behaves as if it were constructed using the default constructor.
   */
  void Release();

 private:
  void Acquire();
  ANativeWindow* window_ = nullptr;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PLAYER_ANDROID_ANDROID_NATIVE_WINDOW_H_
