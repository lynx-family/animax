// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.audio;

import com.lynx.animax.base.CalledByNative;
import com.lynx.animax.util.AnimaXLog;
import java.nio.ByteBuffer;

public class AudioAsset {
  private static final String TAG = "AudioAsset";

  private String mFileUrl;
  private AudioInfo mAudioInfo;
  private ByteBuffer mAudioData;

  private AudioAsset() {}

  @CalledByNative
  public static AudioAsset create() {
    return new AudioAsset();
  }

  @CalledByNative
  public boolean loadLocal(String filePath) {
    if (null == filePath || filePath.isEmpty()) {
      AnimaXLog.e(TAG, "audio file path is empty");
      return false;
    }
    mAudioInfo = AudioAssetExtractor.extractAudioInfo(filePath);
    if (!isValid()) {
      mAudioInfo = null;
      return false;
    }
    mFileUrl = filePath;
    return prepareAudioDataIfNecessary();
  }

  public boolean isValid() {
    return mAudioInfo != null && mAudioInfo.getChannelCount() > 0 && mAudioInfo.getSampleRate() > 0
        && mAudioInfo.getDuration() > 0;
  }

  public AudioInfo getAudioInfo() {
    return mAudioInfo;
  }

  public ByteBuffer getAudioData() {
    return mAudioData;
  }

  private boolean prepareAudioDataIfNecessary() {
    if (!isValid()) {
      return false;
    }
    if (null == mAudioData) {
      mAudioData = AudioAssetExtractor.extractDecodedAudioData(
          mAudioInfo, mFileUrl, mAudioInfo.getAudioTrackIndex());
    }
    return mAudioData != null;
  }
}
