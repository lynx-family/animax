// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.animax;

import static com.lynx.animax.util.StreamUtil.createFileInputStream;
import static com.lynx.animax.util.StreamUtil.getByteArrayFromInputStream;
import static com.lynx.animax.util.StreamUtil.saveFileFromInputStream;

import android.content.Context;
import com.lynx.animax.base.CalledByNative;
import com.lynx.animax.util.AnimaX;
import com.lynx.animax.util.AnimaXLog;
import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.InputStream;

public class AnimaXResourceLoader {
  private static final String TAG = "AnimaXResourceLoader";

  @CalledByNative
  public static String getCacheDirectory() {
    Context context = AnimaX.inst().getAppContext();
    if (null == context) {
      return null;
    }
    File cacheDir = context.getCacheDir();
    if (null == cacheDir) {
      return null;
    }
    String cacheDirPath = cacheDir.getAbsolutePath();
    if (null == cacheDirPath) {
      return null;
    }
    File animaxCacheDir = new File(cacheDirPath, "animax");

    if (!animaxCacheDir.exists() && !animaxCacheDir.mkdirs()) {
      AnimaXLog.e(
          TAG, "Failed to make animax cache directory: " + animaxCacheDir.getAbsolutePath());
    }

    if (animaxCacheDir.exists() && animaxCacheDir.isFile()) {
      AnimaXLog.e(TAG,
          "Failed to make animax cache directory, path is a file: "
              + animaxCacheDir.getAbsolutePath());
    }

    return animaxCacheDir.getAbsolutePath();
  }

  @CalledByNative
  public static byte[] readFile(String path) {
    FileInputStream fileInputStream = createFileInputStream(path);
    byte[] data = getByteArrayFromInputStream(fileInputStream);
    return data;
  }

  @CalledByNative
  public static boolean saveByteArrayIntoFile(String dstFilePath, byte[] data) {
    InputStream inputStream = new ByteArrayInputStream(data);
    return saveFileFromInputStream(inputStream, dstFilePath);
  }
}
