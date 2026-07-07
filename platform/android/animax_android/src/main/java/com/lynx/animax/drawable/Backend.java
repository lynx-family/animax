// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.drawable;

/**
 * Actual rendering backend selected by native. The native value mirrors the native
 * AnimaXBackend enum (include/player/animax_surface.h); Android only ever reports
 * GL/Vulkan/Software. Values are explicit (not ordinal-based) to stay aligned with C++.
 */
public enum Backend {
  UNKNOWN(-1, "unknown"),
  GL(0, "gles"),
  VULKAN(2, "vulkan"),
  SOFTWARE(3, "software");

  private final int mNativeValue;
  private final String mName;

  Backend(int nativeValue, String name) {
    mNativeValue = nativeValue;
    mName = name;
  }

  /** The int value passed across JNI; mirrors the native AnimaXBackend enum. */
  public int getNativeValue() {
    return mNativeValue;
  }

  @Override
  public String toString() {
    return mName;
  }

  /** Maps a native int (from JNI) to the matching Backend, defaulting to UNKNOWN. */
  public static Backend fromNative(int nativeValue) {
    for (Backend backend : values()) {
      if (backend.mNativeValue == nativeValue) {
        return backend;
      }
    }
    return UNKNOWN;
  }
}
