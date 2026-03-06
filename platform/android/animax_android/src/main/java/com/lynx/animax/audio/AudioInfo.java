// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.audio;

import android.media.AudioFormat;
import android.media.MediaFormat;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

public class AudioInfo {
  private MediaFormat mFormat;
  private int mChannelCount = 0;
  private int mSampleRate = 0;
  private long mDuration = 0;
  private int mPCMEncoding = AudioFormat.ENCODING_PCM_16BIT;

  private int mAudioTrackIndex = -1;

  @Nullable
  public MediaFormat getFormat() {
    return mFormat;
  }

  public void setFormat(@NonNull MediaFormat format) {
    mFormat = format;
  }

  public int getChannelCount() {
    return mChannelCount;
  }

  public void setChannelCount(int channelCount) {
    mChannelCount = channelCount;
  }

  public int getSampleRate() {
    return mSampleRate;
  }

  public void setSampleRate(int sampleRate) {
    mSampleRate = sampleRate;
  }

  public long getDuration() {
    return mDuration;
  }

  public void setDuration(long duration) {
    mDuration = duration;
  }

  public int getTotalFrames() {
    return (int) (getSampleRate() * getDuration() / (1000.0 * 1000.0));
  }

  public int getPCMEncodingFormat() {
    return mPCMEncoding;
  }

  public void setPCMEncoding(int pcmEncoding) {
    mPCMEncoding = pcmEncoding;
  }

  public int getAudioTrackIndex() {
    return mAudioTrackIndex;
  }

  public void setAudioTrackIndex(int index) {
    mAudioTrackIndex = index;
  }
}
