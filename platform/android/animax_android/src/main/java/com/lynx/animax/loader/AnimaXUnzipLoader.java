// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.loader;

import androidx.annotation.Keep;
import com.lynx.animax.base.CalledByNative;
import com.lynx.animax.base.Status;
import com.lynx.animax.util.UnzipUtil;

public class AnimaXUnzipLoader {
  @Keep
  public static class UnzipResult {
    private String error;
    private String path;

    // Constructor
    public UnzipResult(String error, String path) {
      this.error = error;
      this.path = path;
    }

    // Getters
    public String getError() {
      return error;
    }

    public String getPath() {
      return path;
    }

    // Setters
    public void setError(String error) {
      this.error = error;
    }

    public void setPath(String path) {
      this.path = path;
    }
  }

  @CalledByNative
  public static String getError(UnzipResult result) {
    return result.getError();
  }

  @CalledByNative
  public static String getPath(UnzipResult result) {
    return result.getPath();
  }

  @CalledByNative
  public static UnzipResult unzip(final String zippedFilePath, final String unzippedFilePath) {
    if (null == zippedFilePath || zippedFilePath.isEmpty()) {
      return new UnzipResult("ZippedFilePath is null.", null);
    }
    if (null == unzippedFilePath || unzippedFilePath.isEmpty()) {
      return new UnzipResult("UnzippedFilePath is null.", null);
    }

    Status status = UnzipUtil.unzip(zippedFilePath, unzippedFilePath);

    if (!status.mSuccess) {
      return new UnzipResult(status.mErrMsg, null);
    }

    return new UnzipResult(null, unzippedFilePath);
  }
}
