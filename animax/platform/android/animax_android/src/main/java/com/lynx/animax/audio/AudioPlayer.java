// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.audio;

import androidx.annotation.NonNull;
import com.lynx.animax.base.CalledByNative;
import com.lynx.animax.util.AnimaXLog;
import java.nio.ByteBuffer;

public class AudioPlayer implements IAudioRendererListener {
  static private final String TAG = "AudioPlayer";
  @NonNull private final AudioRenderer mRenderer;
  @NonNull private final AudioInfo mAudioInfo;
  @NonNull private final ByteBuffer mAudioData;
  private final int mAudioDataCapacity;

  private int mAudioStartPosition = 0;

  private AudioPlayer(@NonNull AudioInfo info, @NonNull ByteBuffer data) {
    mAudioInfo = info;
    mAudioData = data;
    mAudioDataCapacity = mAudioData.capacity();
    mRenderer = new AudioRenderer(mAudioInfo);
    mRenderer.setRendererListener(this);
  }

  public static AudioPlayer create(@NonNull AudioAsset asset) {
    if (!asset.isValid() || asset.getAudioData() == null) {
      AnimaXLog.e(TAG,
          "Failed to create AudioPlayer: "
              + "asset is not ready");
      return null;
    }
    try {
      return new AudioPlayer(asset.getAudioInfo(), asset.getAudioData());
    } catch (Exception e) {
      AnimaXLog.e(TAG, "Failed to create AudioPlayer: " + e.getMessage());
      return null;
    }
  }

  @CalledByNative
  public void play() {
    mRenderer.play();
  }

  @CalledByNative
  public void pause() {
    mRenderer.pause();
  }

  @CalledByNative
  public double getDuration() {
    return mAudioInfo.getDuration() / 1000.0;
  }

  @CalledByNative
  public double getAudioTime() {
    return (double) ((mAudioStartPosition + mRenderer.getPlaybackHeadPositionInByte())
               % mAudioDataCapacity)
        / mAudioDataCapacity * mAudioInfo.getDuration() / 1000.0;
  }

  @CalledByNative
  public void seekToProgress(double progress) {
    mRenderer.pauseAndFlush();
    mAudioStartPosition = mRenderer.alignByteToFrames((int) (progress * mAudioDataCapacity));
  }

  @Override
  public void onDataRequest(long offset, int size, ByteBuffer buffer) {
    int startPosition = Math.toIntExact((mAudioStartPosition + offset) % mAudioDataCapacity);
    int endPosition = Math.min(mAudioDataCapacity, startPosition + size);
    mAudioData.position(startPosition);
    mAudioData.limit(endPosition);
    buffer.put(mAudioData);
    if (mAudioDataCapacity < startPosition + size) {
      endPosition = (startPosition + size) % mAudioDataCapacity;
      mAudioData.position(0);
      mAudioData.limit(endPosition);
      buffer.put(mAudioData);
    }
    mAudioData.position(0);
    mAudioData.limit(mAudioDataCapacity);
  }
}
