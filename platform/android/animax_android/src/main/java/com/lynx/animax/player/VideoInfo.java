// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.player;

import android.media.MediaFormat;
import androidx.annotation.Nullable;

public class VideoInfo {
  private MediaFormat mFormat;
  private int mWidth;
  private int mHeight;
  private float mFrameRate;
  private int mFrameCount;

  private int mVideoTrackIndex = -1;

  public VideoInfo() {}

  @Nullable
  public MediaFormat getFormat() {
    return mFormat;
  }

  public void setFormat(@Nullable MediaFormat format) {
    mFormat = format;
  }

  public int getWidth() {
    return mWidth;
  }

  public void setWidth(int width) {
    mWidth = width;
  }

  public int getHeight() {
    return mHeight;
  }

  public void setHeight(int height) {
    mHeight = height;
  }

  public float getFrameRate() {
    return mFrameRate;
  }

  public void setFrameRate(float frameRate) {
    mFrameRate = frameRate;
  }

  public int getFrameCount() {
    return mFrameCount;
  }

  public void setFrameCount(int frameCount) {
    mFrameCount = frameCount;
  }

  public int getVideoTrackIndex() {
    return mVideoTrackIndex;
  }

  public void setVideoTrackIndex(int index) {
    mVideoTrackIndex = index;
  }
}
