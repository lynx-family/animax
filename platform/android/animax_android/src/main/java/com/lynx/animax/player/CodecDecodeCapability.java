// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.player;

import android.media.MediaCodecInfo;
import android.media.MediaCodecList;
import android.os.Build;
import com.lynx.animax.util.AnimaXLog;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

public class CodecDecodeCapability {
  private static final String TAG = "CodecDecodeCapability";
  private static final String SUFFIX_HARDWARE_ACCELERATED = "_HW";
  private static final String SUFFIX_SOFTWARE = "_SW";
  private static CodecDecodeCapability sInstance;
  private Map<String, Boolean> mCache = new ConcurrentHashMap<String, Boolean>();
  private volatile MediaCodecInfo[] mCachedCodecInfos;

  private CodecDecodeCapability() {}

  public static synchronized CodecDecodeCapability getInstance() {
    if (sInstance == null) {
      sInstance = new CodecDecodeCapability();
    }
    return sInstance;
  }

  /**
   * Checks if decoding is supported for the specified MIME type by system.
   *
   * @param mimeType MIME type, e.g., "video/avc", "video/hevc", etc.
   * @param requireHardwareAccelerated Whether hardware-accelerated decoding is required:
   *   - When requireHardwareAccelerated == true:
   *       • API >= 29: Strictly checks for hardware-accelerated decoding support
   *       • API < 29: Checks for decoding support (include both hardware-accelerated
   *         decoding and software decoding, since hardware-accelerated detection API is
   *         only available from API 29 onwards)
   *   - When requireHardwareAccelerated == false: Checks for decoding support (include
   *         both hardware-accelerated decoding and software decoding)
   */
  public boolean isSupportDecodeBySystem(String mimeType, boolean requireHardwareAccelerated) {
    if (mimeType == null || mimeType.isEmpty()) {
      return false;
    }

    String cacheKey = generateCacheKey(mimeType, requireHardwareAccelerated);
    if (mCache.containsKey(cacheKey)) {
      return mCache.get(cacheKey);
    }

    boolean result = isDecodeSupportInternal(mimeType, requireHardwareAccelerated);
    mCache.put(cacheKey, result);
    return result;
  }

  private boolean isDecodeSupportInternal(String mimeType, boolean requireHardwareAccelerated) {
    if (mimeType == null || mimeType.isEmpty()) {
      return false;
    }

    try {
      MediaCodecInfo[] codecInfos = getCachedCodecInfos();
      for (MediaCodecInfo codecInfo : codecInfos) {
        if (codecInfo.isEncoder()) {
          continue;
        }

        if (requireHardwareAccelerated) {
          if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
            if (!codecInfo.isHardwareAccelerated()) {
              continue;
            }
          }
        }

        for (String type : codecInfo.getSupportedTypes()) {
          if (type.equalsIgnoreCase(mimeType)) {
            return true;
          }
        }
      }
    } catch (Exception e) {
      AnimaXLog.e(
          TAG, "checkCodecSupport failed, mimeType: " + mimeType + ", message: " + e.getMessage());
    }

    return false;
  }

  private MediaCodecInfo[] getCachedCodecInfos() {
    if (mCachedCodecInfos != null) {
      return mCachedCodecInfos;
    }

    synchronized (this) {
      if (mCachedCodecInfos == null) {
        int codecCount = MediaCodecList.getCodecCount();
        MediaCodecInfo[] codecInfos = new MediaCodecInfo[codecCount];
        for (int i = 0; i < codecCount; i++) {
          codecInfos[i] = MediaCodecList.getCodecInfoAt(i);
        }
        mCachedCodecInfos = codecInfos;
      }
      return mCachedCodecInfos;
    }
  }

  private String generateCacheKey(String mimeType, boolean requireHardwareAccelerated) {
    return mimeType.toLowerCase()
        + (requireHardwareAccelerated ? SUFFIX_HARDWARE_ACCELERATED : SUFFIX_SOFTWARE);
  }
}
