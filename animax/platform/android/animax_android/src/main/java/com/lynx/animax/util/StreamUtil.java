// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.util;

import androidx.annotation.NonNull;
import java.io.BufferedOutputStream;
import java.io.ByteArrayOutputStream;
import java.io.Closeable;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class StreamUtil {
  private static final String TAG = "AnimaXStreamUtil";

  private static final int BUFFER_MIN_SIZE = 1024;
  private static final int BUFFER_MAX_SIZE = 1024 * 8;

  public static int clamp(int value, int min, int max) {
    if (value < min) {
      return min;
    } else if (value > max) {
      return max;
    }
    return value;
  }

  private static void copyStreamImpl(@NonNull InputStream inputStream,
      @NonNull OutputStream outputStream, int bufferLength) throws IOException {
    byte[] buffer = new byte[bufferLength];
    int nRead;
    while ((nRead = inputStream.read(buffer, 0, buffer.length)) != -1) {
      outputStream.write(buffer, 0, nRead);
    }
  }

  private static void closeQuietly(Closeable closeable) {
    if (closeable == null) {
      return;
    }
    try {
      closeable.close();
    } catch (IOException e) {
      AnimaXLog.e(TAG, "Failed to close resource: " + e);
    }
  }

  private static boolean copyStream(
      InputStream inputStream, OutputStream outputStream, int totalLength) {
    try {
      if (inputStream == null || outputStream == null) {
        AnimaXLog.e(TAG, "Invalid input stream or output stream");
        return false;
      }
      int bufferLength = clamp(totalLength, BUFFER_MIN_SIZE, BUFFER_MAX_SIZE);
      copyStreamImpl(inputStream, outputStream, bufferLength);
    } catch (IOException e) {
      AnimaXLog.e(TAG, "Failed to copy input stream to output stream, reason: " + e);
      return false;
    } finally {
      closeQuietly(inputStream);
      closeQuietly(outputStream);
    }
    return true;
  }

  private static int tryGetAvailableBytesFromInputStream(InputStream inputStream) {
    int inputStreamLengthHint = 0;
    try {
      inputStreamLengthHint = inputStream.available();
    } catch (IOException ignore) {
    }
    return inputStreamLengthHint;
  }

  public static byte[] getByteArrayFromInputStream(InputStream inputStream) {
    return getByteArrayFromInputStream(
        inputStream, tryGetAvailableBytesFromInputStream(inputStream));
  }

  public static byte[] getByteArrayFromInputStream(
      InputStream inputStream, int inputStreamLengthHint) {
    ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
    if (copyStream(inputStream, outputStream, inputStreamLengthHint)) {
      return outputStream.toByteArray();
    } else {
      return null;
    }
  }

  public static boolean saveFileFromInputStream(InputStream inputStream, String dstFilePath) {
    return saveFileFromInputStream(
        inputStream, tryGetAvailableBytesFromInputStream(inputStream), dstFilePath);
  }

  public static boolean saveFileFromInputStream(
      InputStream inputStream, int inputStreamLengthHint, String dstFilePath) {
    if (inputStream == null) {
      return false;
    }
    BufferedOutputStream outputStream = null;
    FileOutputStream fileOutputStream;
    try {
      fileOutputStream = new FileOutputStream(dstFilePath);
      outputStream = new BufferedOutputStream(fileOutputStream);
    } catch (IOException e) {
      AnimaXLog.e(TAG, "Failed to create FileOutputStream for file: " + dstFilePath);
    }

    return copyStream(inputStream, outputStream, inputStreamLengthHint);
  }

  public static FileInputStream createFileInputStream(String path) {
    if (null == path) {
      return null;
    }
    File file = new File(path);
    if (!file.exists()) {
      return null;
    }

    FileInputStream fileInputStream = null;
    try {
      fileInputStream = new FileInputStream(path);
    } catch (Exception e) {
      AnimaXLog.e(TAG, "createFileInputStream error: " + e);
    }
    return fileInputStream;
  }
}
