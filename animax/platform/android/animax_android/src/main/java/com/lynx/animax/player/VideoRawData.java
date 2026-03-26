// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.player;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import java.nio.ByteBuffer;
import java.util.ArrayList;

public class VideoRawData {
  private ByteBuffer mFrameBuffer;
  private ArrayList<Integer> mKeyFrames = new ArrayList<>();
  private ArrayList<FrameInfo> mFrameInfos = new ArrayList<>();

  public VideoRawData() {}

  @Nullable
  public ByteBuffer getFrameBuffer() {
    return mFrameBuffer;
  }

  public void setFrameBuffer(@Nullable ByteBuffer frameBuffer) {
    mFrameBuffer = frameBuffer;
  }

  @NonNull
  public ArrayList<Integer> getKeyFrames() {
    return mKeyFrames;
  }

  @NonNull
  public ArrayList<FrameInfo> getFrameInfos() {
    return mFrameInfos;
  }
}
