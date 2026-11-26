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
  @NonNull private final ByteBuffer mAudioData;
  private final int mAudioDataCapacity;
  private final double mDurationMs;

  private int mAudioStartPosition = 0;

  private AudioPlayer(@NonNull AudioInfo info, @NonNull ByteBuffer data) {
    mAudioData = data;
    mAudioDataCapacity = mAudioData.capacity();
    mRenderer = new AudioRenderer(info);
    // Playback and seeking use decoded PCM, whose length can differ from metadata.
    mDurationMs = mRenderer.getDurationMs(mAudioDataCapacity);
    mRenderer.setRendererListener(this);
  }

  public static AudioPlayer create(@NonNull AudioAsset asset) {
    ByteBuffer data = asset.getAudioData();
    if (!asset.isValid() || data == null || data.capacity() == 0) {
      AnimaXLog.e(TAG,
          "Failed to create AudioPlayer: "
              + "asset is not ready");
      return null;
    }
    try {
      return new AudioPlayer(asset.getAudioInfo(), data);
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
    return mDurationMs;
  }

  @CalledByNative
  public double getAudioTime() {
    return (double) ((mAudioStartPosition + mRenderer.getPlaybackHeadPositionInByte())
               % mAudioDataCapacity)
        / mAudioDataCapacity * mDurationMs;
  }

  @CalledByNative
  public void seekToProgress(double progress) {
    mRenderer.pauseAndFlush();
    mAudioStartPosition = mRenderer.alignByteToFrames((int) (progress * mAudioDataCapacity));
  }

  @Override
  public void onDataRequest(long offset, int size, ByteBuffer buffer) {
    int position = Math.toIntExact((mAudioStartPosition + offset) % mAudioDataCapacity);
    // A single AudioTrack request can span several loops of a short PCM clip.
    // The decoder wraps a byte array, so copying by index preserves the asset's cursor.
    int remaining = size;
    while (remaining > 0) {
      int count = Math.min(remaining, mAudioDataCapacity - position);
      buffer.put(mAudioData.array(), mAudioData.arrayOffset() + position, count);
      remaining -= count;
      position = 0;
    }
  }
}
