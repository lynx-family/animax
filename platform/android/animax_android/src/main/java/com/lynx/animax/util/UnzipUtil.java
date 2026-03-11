// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.util;

import androidx.annotation.NonNull;
import com.lynx.animax.base.Status;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

public class UnzipUtil {
  private static final String TAG = "UnzipUtil";

  @NonNull
  public static Status unzip(final String zipFilePath, final String dstDirectoryPath) {
    ZipInputStream zipInputStream = getZipInputStream(zipFilePath);
    if (null == zipInputStream) {
      return new Status("open zip file error");
    }
    if (!makeDirectory(dstDirectoryPath)) {
      return new Status("make dst directory error");
    }

    while (true) {
      ZipEntry entry = getNextEntry(zipInputStream);
      if (null == entry) {
        break;
      }
      processEntry(zipInputStream, entry, dstDirectoryPath);
      closeEntry(zipInputStream);
    }
    closeZipInputStream(zipInputStream);
    return new Status(true);
  }

  private static ZipInputStream getZipInputStream(final String zipFilePath) {
    if (null == zipFilePath) {
      return null;
    }
    File zipFile = new File(zipFilePath);
    FileInputStream fileInputStream = null;
    try {
      fileInputStream = new FileInputStream(zipFile);
    } catch (FileNotFoundException e) {
      AnimaXLog.e(TAG, "new FileInputStream error: " + e.getMessage());
    }
    if (null == fileInputStream) {
      return null;
    }
    return new ZipInputStream(fileInputStream);
  }

  private static void closeZipInputStream(ZipInputStream zipInputStream) {
    try {
      zipInputStream.close();
    } catch (IOException e) {
      AnimaXLog.e(TAG, "zipInputStream close error: " + e.getMessage());
    }
  }

  private static boolean makeDirectory(final String directoryPath) {
    if (null == directoryPath) {
      return false;
    }
    boolean success = true;
    File directory = new File(directoryPath);
    if (!directory.exists()) {
      success = directory.mkdirs();
    }
    return success;
  }

  private static ZipEntry getNextEntry(ZipInputStream zipInputStream) {
    ZipEntry entry = null;
    try {
      entry = zipInputStream.getNextEntry();
    } catch (IOException e) {
      AnimaXLog.e(TAG, "getNextEntry error: " + e.getMessage());
    }
    return entry;
  }

  private static void closeEntry(ZipInputStream zipInputStream) {
    try {
      zipInputStream.closeEntry();
    } catch (IOException e) {
      AnimaXLog.e(TAG, "closeEntry error: " + e.getMessage());
    }
  }

  private static boolean processEntry(
      ZipInputStream zipInputStream, ZipEntry entry, final String dstDirectoryPath) {
    String name = entry.getName();
    if (null == name) {
      return false;
    }
    if (name.contains("__MACOSX") || name.contains(".DS_Store") || name.contains("../")) {
      // skip
      return true;
    }
    if (entry.isDirectory()) {
      File directory = new File(dstDirectoryPath + File.separator + name);
      return directory.mkdirs();
    }
    // is file
    return extractFile(zipInputStream, dstDirectoryPath + File.separator + name);
  }

  private static boolean extractFile(ZipInputStream zipInputStream, final String dstFilePath) {
    FileOutputStream outputStream = null;
    try {
      outputStream = new FileOutputStream(dstFilePath);
    } catch (FileNotFoundException e) {
      AnimaXLog.e(TAG, "extractFile, create FileOutputStream error: " + e.getMessage());
    }
    if (null == outputStream) {
      return false;
    }
    BufferedOutputStream bufferedOutputStream = new BufferedOutputStream(outputStream);
    byte[] buffer = new byte[4096];
    int size;
    while (-1 != (size = readZipFile(zipInputStream, buffer))) {
      if (!writeToStream(buffer, size, bufferedOutputStream)) {
        break;
      }
    }
    try {
      bufferedOutputStream.close();
    } catch (IOException e) {
      AnimaXLog.e(TAG, "close bufferedOutputStream error: " + e.getMessage());
    }
    return true;
  }

  private static int readZipFile(ZipInputStream zipInputStream, byte[] buffer) {
    int size = -1;
    try {
      size = zipInputStream.read(buffer);
    } catch (IOException e) {
      AnimaXLog.e(TAG, "readZipFile error: " + e.getMessage());
      size = -1;
    }
    return size;
  }

  private static boolean writeToStream(byte[] buffer, int size, BufferedOutputStream stream) {
    try {
      stream.write(buffer, 0, size);
    } catch (IOException e) {
      AnimaXLog.e(TAG, "writeToStream error: " + e.getMessage());
      return false;
    }
    return true;
  }
}
