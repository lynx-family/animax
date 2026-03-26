// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.player;

public class FrameInfo {
  private int mBeg;
  private int mEnd;
  private long mPresentationTimeUs;
  public FrameInfo(int beg, int end, long presentationTimeUs) {
    mBeg = beg;
    mEnd = end;
    mPresentationTimeUs = presentationTimeUs;
  }

  public int begin() {
    return mBeg;
  }

  public int end() {
    return mEnd;
  }

  public long getPresentationTimeUs() {
    return mPresentationTimeUs;
  }
}
