// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.animax.base;

import com.lynx.animax.listener.AnimaXErrorParam;
import java.util.HashMap;
import java.util.Map;

// The definitions in this file correspond to the file animax/bridge/animax_event.h
public enum AnimaXError {
  // Correspond to lynx::animax::EventError::kNoError
  NO_ERROR(0),
  // Correspond to lynx::animax::EventError::kResourceNotFound
  RESOURCE_NOT_FOUND(1),
  // Correspond to lynx::animax::EventError::kLibraryInitError
  LIBRARY_INIT_ERROR(12),
  // Correspond to lynx::animax::EventError::kVideoPlayerError
  VIDEO_PLAYER_ERROR(100),
  // Correspond to lynx::animax::EventError::kVideoPlayerErrorHasOccurred
  VIDEO_PLAYER_ERROR_HAS_OCCURRED(101),
  // The device was blocked by online setting list
  BLOCK_DEVICE(200); // Added semicolon here to terminate the enum constants list

  private final int errorCode; // Moved this line below the constants

  AnimaXError(int errorCode) { // Corrected the constructor name to match the enum name
    this.errorCode = errorCode;
  }

  public int getErrorCode() {
    return errorCode;
  }

  // Create a BLOCK_DEVICE error param, this will be read on error callback
  public static Map<String, Object> createBlockErrorParam() {
    Map<String, Object> params = new HashMap<>();
    params.put(AnimaXErrorParam.KEY_CODE, AnimaXError.BLOCK_DEVICE.ordinal());
    params.put(AnimaXErrorParam.KEY_MESSAGE, "The device is not support.");
    return params;
  }

  public static Map<String, Object> createLibraryInitErrorParam() {
    Map<String, Object> params = new HashMap<>();
    params.put(AnimaXErrorParam.KEY_CODE, AnimaXError.LIBRARY_INIT_ERROR.ordinal());
    params.put(AnimaXErrorParam.KEY_MESSAGE, "Library init error.");
    return params;
  }
}
