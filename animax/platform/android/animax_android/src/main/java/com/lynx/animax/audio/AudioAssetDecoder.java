// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.audio;

import android.media.MediaCodec;
import android.media.MediaFormat;
import android.os.Build;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import com.lynx.animax.util.AnimaXLog;
import java.nio.ByteBuffer;

public class AudioAssetDecoder {
  private static final String TAG = "AudioAssetDecoder";

  private static final long TIME_OUT_US = 1000; // 1ms
  @NonNull final private AudioInfo mInfo;
  @Nullable private MediaCodec mCodec;
  private boolean mIsFinished = false;

  private int mCurrentInputIndex = -1;
  private int mCurrentOutputIndex = -1;
  private final MediaCodec.BufferInfo mBufferInfo = new MediaCodec.BufferInfo();

  public AudioAssetDecoder(@NonNull AudioInfo info) {
    mInfo = info;
    MediaFormat format = mInfo.getFormat();
    if (format == null) {
      return;
    }
    String mime = format.getString(MediaFormat.KEY_MIME);
    if (mime == null) {
      return;
    }
    try {
      mCodec = MediaCodec.createDecoderByType(mime);
      mCodec.configure(format, null, null, 0);
    } catch (Exception e) {
      AnimaXLog.e(TAG, "Failed to init audio asset decoder: " + e.getMessage());
    }
  }

  public MediaCodec.BufferInfo getBufferInfo() {
    return mBufferInfo;
  }

  public void start() {
    if (mCodec == null) {
      return;
    }
    mIsFinished = false;
    mCodec.start();
  }

  public void stop() {
    if (mCodec == null) {
      return;
    }
    mCodec.stop();
  }

  public void release() {
    if (mCodec == null) {
      return;
    }
    mCodec.release();
  }

  public void queueInputBuffer(int offset, int size, long time, int flags) {
    if (mCodec == null) {
      return;
    }
    mCodec.queueInputBuffer(mCurrentInputIndex, offset, size, time, flags);
  }

  public void releaseOutPutBuffer() {
    if (mCodec == null) {
      return;
    }
    mCodec.releaseOutputBuffer(mCurrentOutputIndex, false);
  }

  public boolean isFinished() {
    return mIsFinished;
  }

  @Nullable
  public ByteBuffer getInputBuffer() {
    if (mCodec == null) {
      return null;
    }
    int inputBufferIndex = mCodec.dequeueInputBuffer(TIME_OUT_US);
    mCurrentInputIndex = inputBufferIndex;
    if (inputBufferIndex < 0) {
      return null;
    }
    return mCodec.getInputBuffer(inputBufferIndex);
  }

  @Nullable
  public ByteBuffer getOutputBuffer() {
    if (mCodec == null) {
      return null;
    }

    int outputBufferIndex = mCodec.dequeueOutputBuffer(mBufferInfo, TIME_OUT_US);
    ByteBuffer buffer = null;
    if (outputBufferIndex >= 0) {
      buffer = mCodec.getOutputBuffer(outputBufferIndex);
      mCurrentOutputIndex = outputBufferIndex;
    } else if (outputBufferIndex == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED
        && Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
      MediaFormat format = mCodec.getOutputFormat();
      AnimaXLog.i(TAG, "decoded audio format: " + format);
      if (format.containsKey(MediaFormat.KEY_PCM_ENCODING)) {
        mInfo.setPCMEncoding(format.getInteger(MediaFormat.KEY_PCM_ENCODING));
      }
    }
    if ((mBufferInfo.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0) {
      mIsFinished = true;
    }
    return buffer;
  }

  public boolean isValid() {
    return mCodec != null;
  }
}
