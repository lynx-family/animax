// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.base;

public class Status {
  public final boolean mSuccess;
  public final String mErrMsg;
  public Status(boolean success) {
    mSuccess = success;
    mErrMsg = success ? null : "unknown error";
  }
  public Status(String errMsg) {
    mSuccess = (null == errMsg);
    mErrMsg = errMsg;
  }
}
