// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.util;

import static com.lynx.animax.util.StreamUtil.getByteArrayFromInputStream;

import android.content.ContentResolver;
import android.content.Context;
import android.content.res.AssetManager;
import android.net.Uri;
import androidx.annotation.NonNull;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;

public class ResourceUtil {
  private static final String TAG = "ResourceUtil";

  /**
   * Read file from assets folder and return content as byte array.
   *
   * @param assetManager The assetManager.
   * @param assetName The name of the file inside the assets folder.
   * @return The content of the file as byte array.
   */
  public static byte[] getByteArrayFromAsset(
      @NonNull String assetName, @NonNull AssetManager assetManager) {
    InputStream inputStream = null;
    try {
      inputStream = assetManager.open(assetName);
      int inputStreamLengthHint = inputStream.available();
      return getByteArrayFromInputStream(inputStream, inputStreamLengthHint);
    } catch (IOException e) {
      AnimaXLog.e(TAG, "Failed to open asset: " + assetName + " , reason: " + e);
    }
    return null;
  }

  /**
   * Read file from the provided file path and return content as byte array.
   *
   * @param filePath The path of the file.
   * @return The content of the file as byte array, or null if an error occurred.
   */
  public static byte[] getByteArrayFromFile(String filePath) {
    InputStream inputStream = null;
    try {
      inputStream = new FileInputStream(filePath);
      int inputStreamLengthHint = inputStream.available();
      return getByteArrayFromInputStream(inputStream, inputStreamLengthHint);
    } catch (IOException e) {
      AnimaXLog.e(TAG, "Failed to open local file:" + filePath + " , reason: " + e);
    }
    return null;
  }

  /**
   * Read uri from the ContentResolver and return content as byte array.
   *
   * @param uri The uri of the ContentResolver.
   * @return The content of the contentUri as byte array, or null if an error occurred.
   */
  public static byte[] getByteArrayFromContentUri(@NonNull Uri uri) {
    Context context = AnimaX.inst().getAppContext();
    if (context == null) {
      AnimaXLog.e(TAG, "Context is null");
      return null;
    }
    ContentResolver resolver = context.getContentResolver();
    InputStream inputStream = null;
    try {
      inputStream = resolver.openInputStream(uri);
      if (inputStream == null) {
        AnimaXLog.e(TAG, "inputStream is null");
        return null;
      }
      return StreamUtil.getByteArrayFromInputStream(inputStream);
    } catch (Exception e) {
      AnimaXLog.e(TAG, "readUriFile has Exception:" + e.getMessage());
      return null;
    }
  }
}
