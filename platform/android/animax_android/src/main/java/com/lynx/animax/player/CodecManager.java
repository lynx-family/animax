// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.player;

import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaCodecList;
import android.media.MediaCrypto;
import android.media.MediaFormat;
import android.view.Surface;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import com.lynx.animax.base.Status;
import com.lynx.animax.util.AnimaXLog;
import java.io.IOException;
import java.nio.ByteBuffer;

// CodecManager is essentially a wrapper around a MediaCodec instance and also handles its
// allocation, deallocation, and decoding operations.
public class CodecManager {
  private static final String TAG = "CodecManager";
  private MediaCodec mDecoder;
  private String mDecoderName;

  // Decoder state
  private boolean mDecoderValid;
  private boolean mDecoderShouldRestart = false;

  // I/O-related variables
  private static final int INVALID_FRAME = -1;
  // Currently displayed frame number
  private int mCurrentPresentFrame;
  // Next frame number to feed into the decoder
  private int mNextInputFrame;
  // Next frame number expected back from the decoder
  private int mNextOutputFrame;
  // Number of frames fed into the decoder but not yet retrieved
  private int mCachedFrame;

  // Cache-related variables (how many frames can be in flight)
  // MAX_CACHE_FRAME_COUNT should be greater or equal to 1
  private static final int MAX_CACHE_FRAME_COUNT = 20;
  private int mCurrentMaxCacheFrameCount;

  // Retry-related variables
  private static final int MAX_TRY_COUNT = 3;
  private int mInfoTryAgainCount;

  // Decoder-selection variable
  private int mMediaCodecListNextTryIndex = 0;

  private long mTimeoutUs;

  private CodecErrorReporter mReporter;

  // shared with VideoPlayerImpl
  private VideoAsset mAsset;
  private Surface mSurface;

  public interface CodecErrorReporter {
    void reportError(@NonNull String errMsg);
    void markDrawnOnce();
  }

  private enum ProcessFrameStatus {
    SUCCESS,
    TRY_AGAIN_IF_NECESSARY,
    TRY_AGAIN, // dequeueOutputBuffer may return INFO_* constants. If it happens, just try again
    // regardless of try count
    FATAL,
  }

  public CodecManager(@Nullable CodecErrorReporter reporter) {
    setReporter(reporter);
  }

  /**
   * Try to initialize decoder one by one until success or last one.
   * If initialization is fail, errors will be reported.
   * If initialization is success, nothing will be reported.
   */
  public void initDecoder() {
    if (null == mAsset) {
      return;
    }
    MediaFormat format = mAsset.getFormat();
    AnimaXLog.i(TAG, "initDecoder, format: " + format);

    String mimeType = format.getString(MediaFormat.KEY_MIME);
    if (null == mimeType) {
      return;
    }

    Status status = tryInitDecoderByType(mimeType);
    boolean success = status.mSuccess;
    if (!success) {
      try {
        int codecCount = MediaCodecList.getCodecCount();
        while (mMediaCodecListNextTryIndex < codecCount) {
          MediaCodecInfo info = MediaCodecList.getCodecInfoAt(mMediaCodecListNextTryIndex++);
          if (info.isEncoder() || !supportMimeType(info, mimeType)) {
            continue;
          }
          String decoderName = info.getName();
          if (null == decoderName) {
            continue;
          }
          status = tryInitDecoder(decoderName);
          if (status.mSuccess) {
            success = true;
            break;
          }
        }
      } catch (Exception e) {
        AnimaXLog.e(TAG, "enumerate codec list failed: " + e.getMessage());
      }
    }
    if (!success) {
      reportError("initDecoder error");
      return;
    }
    prepareNextFrame();
  }

  public void decodeAndUploadFrame(int toFrame) {
    if (mDecoderShouldRestart) {
      mDecoderShouldRestart = false;
      releaseDecoder();
      initDecoder();
    }
    if (!mDecoderValid) {
      return;
    }

    if (toFrame == mCurrentPresentFrame) {
      // means surface doesn't update
      return;
    }

    refreshPlayerState(toFrame);
    // refreshPlayerState may set mDecoderValid false
    if (!mDecoderValid) {
      return;
    }

    MediaCodec.BufferInfo outputBufferInfo = new MediaCodec.BufferInfo();
    int leftTryCount = MAX_TRY_COUNT;
    while (mCurrentPresentFrame != toFrame) {
      prepareFrameToMaxCacheCapacity();
      if (0 == mCachedFrame) {
        return;
      }

      ProcessFrameStatus status = processFrame(outputBufferInfo, mNextOutputFrame == toFrame);
      if (ProcessFrameStatus.SUCCESS == status) {
        ;
      } else if (ProcessFrameStatus.TRY_AGAIN == status) {
        ;
      } else if (ProcessFrameStatus.TRY_AGAIN_IF_NECESSARY == status && leftTryCount > 0) {
        --leftTryCount;
      } else {
        return;
      }
    }
    markDrawnOnce();
    prepareNextFrame();
  }

  public void attachSurface(Surface surface) {
    mSurface = surface;
  }

  public void attachAsset(VideoAsset asset) {
    mAsset = asset;
    if (null == mAsset) {
      return;
    }
    boolean success = mAsset.prepareFrameBufferIfNecessary();
    if (!success) {
      AnimaXLog.e(TAG, "attachAsset error: prepareFrameBuffer fail, reset mAsset");
      mAsset = null;
      return;
    }
    long frameRate = (long) mAsset.getFrameRate();
    frameRate = frameRate > 0l ? frameRate : 30l;
    mTimeoutUs = 1000000l / frameRate;
  }

  public void setTimeout(long timeoutUs) {
    mTimeoutUs = timeoutUs;
  }

  public void setReporter(@Nullable CodecErrorReporter reporter) {
    mReporter = reporter;
  }

  public void releaseDecoder() {
    // Codec Thread
    if (null == mDecoder) {
      return;
    }
    AnimaXLog.i(TAG, "releaseDecoder");
    stopDecoder();
    mDecoderValid = false;
    mDecoder.release();
    mDecoder = null;
    resetDecoderState();
  }

  public void resetDecoderState() {
    // Codec Thread
    mInfoTryAgainCount = 0;
    updateCurrentMaxCacheFrameCount();
    mCurrentPresentFrame = INVALID_FRAME;
    mCachedFrame = 0;
    mNextInputFrame = 0;
    mNextOutputFrame = INVALID_FRAME;
  }

  private boolean supportMimeType(
      @NonNull MediaCodecInfo mediaCodecInfo, @NonNull String mimeType) {
    String[] types = mediaCodecInfo.getSupportedTypes();
    int typeLength = (null == types) ? 0 : types.length;
    for (int i = 0; i < typeLength; ++i) {
      if (mimeType.equals(types[i])) {
        return true;
      }
    }
    return false;
  }

  private Status tryInitDecoder(@NonNull String decoderName) {
    AnimaXLog.i(TAG, "try decoder: " + decoderName);
    if (null == mAsset) {
      return new Status("tryInitDecoder error: mAsset is null");
    }
    Status status = createCodec(decoderName);
    if (!status.mSuccess) {
      return status;
    }
    return configureAndStartDecoder(decoderName);
  }

  private Status tryInitDecoderByType(@NonNull String mimeType) {
    AnimaXLog.i(TAG, "try decoder by type: " + mimeType);
    if (null == mAsset) {
      return new Status("tryInitDecoderByType error: mAsset is null");
    }
    try {
      mDecoder = MediaCodec.createDecoderByType(mimeType);
    } catch (Exception e) {
      return new Status("createDecoderByType Exception: " + e.getMessage());
    }
    String decoderName = mDecoder.getName();
    return configureAndStartDecoder(decoderName);
  }

  private Status configureAndStartDecoder(@NonNull String decoderName) {
    Status status =
        configureCodec(mAsset.getFormat(), mSurface, null, 0 /*0: decoder, 1: encoder*/);
    if (!status.mSuccess) {
      mDecoder.release();
      mDecoder = null;
      return status;
    }
    status = startDecoder();
    if (!status.mSuccess) {
      mDecoder.release();
      mDecoder = null;
      return status;
    }
    mDecoderName = decoderName;
    mDecoderValid = true;
    AnimaXLog.i(TAG, "chosen decoder: " + decoderName);
    return new Status(true);
  }

  private Status createCodec(String name) {
    String errMsg = null;
    try {
      mDecoder = MediaCodec.createByCodecName(name);
    } catch (MediaCodec.CodecException e) {
      errMsg = "createByCodecName CodecException: " + e.getMessage();
    } catch (IOException | IllegalArgumentException e) {
      errMsg = "createByCodecName Exception: " + e.getMessage();
    }
    return new Status(errMsg);
  }

  private Status configureCodec(
      MediaFormat format, Surface surface, MediaCrypto crypto, int flags) {
    String errMsg = null;
    try {
      mDecoder.configure(format, surface, crypto, flags);
    } catch (MediaCodec.CodecException e) {
      errMsg = "configureCodec CodecException: " + e.getMessage();
    } catch (MediaCodec.CryptoException e) {
      errMsg = "configureCodec CryptoException: " + e.getMessage();
    } catch (IllegalArgumentException | IllegalStateException e) {
      errMsg = "configureCodec Exception: " + e.getMessage();
    }
    return new Status(errMsg);
  }

  private Status startDecoder() {
    String errMsg = null;
    try {
      mDecoder.start();
    } catch (MediaCodec.CodecException e) {
      errMsg = "startDecoder CodecException: " + e.getMessage();
    } catch (IllegalArgumentException | IllegalStateException e) {
      errMsg = "startDecoder Exception: " + e.getMessage();
    }
    return new Status(errMsg);
  }

  private Status stopDecoder() {
    String errMsg = null;
    try {
      mDecoder.stop();
    } catch (OutOfMemoryError | IllegalArgumentException | IllegalStateException e) {
      errMsg = "stopDecoder Exception: " + e.getMessage();
    }
    return new Status(errMsg);
  }

  private void refreshPlayerState(int toFrame) {
    if (null == mAsset) {
      return;
    }
    int keyFrame = mAsset.getPrevKeyFrame(toFrame);
    if (INVALID_FRAME == mNextOutputFrame) {
      mCachedFrame = 0;
      mNextInputFrame = keyFrame;
      return;
    }

    int frameFromKeyFrame = toFrame - keyFrame + 1;
    int frameFromCurrent = (mCurrentPresentFrame <= toFrame)
        ? (toFrame - mCurrentPresentFrame)
        : (toFrame + mAsset.getFrameCount() - mCurrentPresentFrame);
    if (frameFromCurrent <= mCachedFrame) {
      frameFromCurrent = 0;
    } else {
      frameFromCurrent = frameFromCurrent - mCachedFrame;
    }
    if (frameFromCurrent > frameFromKeyFrame) {
      discardCachedFrame();
      mNextInputFrame = keyFrame;
      mNextOutputFrame = INVALID_FRAME;
    }
  }

  private ProcessFrameStatus processFrame(MediaCodec.BufferInfo outputBufferInfo, boolean render) {
    // Codec Thread
    if (null == mAsset) {
      return ProcessFrameStatus.FATAL;
    }
    int outputBufferIndex = MediaCodec.INFO_TRY_AGAIN_LATER;
    try {
      outputBufferIndex = mDecoder.dequeueOutputBuffer(outputBufferInfo, mTimeoutUs);
    } catch (MediaCodec.CodecException e) {
      reportError("[" + mDecoderName + "]: "
          + "dequeueOutputBuffer CodecException: " + e.getMessage());
      return ProcessFrameStatus.FATAL;
    } catch (IllegalStateException e) {
      reportError("[" + mDecoderName + "]: "
          + "dequeueOutputBuffer IllegalStateException: " + e.getMessage());
      mDecoderValid = false;
      mDecoderShouldRestart = true;
      return ProcessFrameStatus.FATAL;
    }

    if (MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED == outputBufferIndex) {
      AnimaXLog.i(TAG, "output buffers changed");
      return ProcessFrameStatus.TRY_AGAIN;
    } else if (MediaCodec.INFO_OUTPUT_FORMAT_CHANGED == outputBufferIndex) {
      AnimaXLog.i(TAG, "output format changed: " + getOutputFormatSafely());
      return ProcessFrameStatus.TRY_AGAIN;
    } else if (MediaCodec.INFO_TRY_AGAIN_LATER == outputBufferIndex) {
      ++mInfoTryAgainCount;
      updateCurrentMaxCacheFrameCount();
      return ProcessFrameStatus.TRY_AGAIN_IF_NECESSARY;
    } else if (outputBufferIndex < 0) {
      // should never reach
      reportError("[" + mDecoderName + "]: "
          + "outputBufferIndex: " + outputBufferIndex);
      return ProcessFrameStatus.FATAL;
    }

    releaseOutputBuffer(outputBufferIndex, render);
    --mCachedFrame;
    mCurrentPresentFrame = mNextOutputFrame;
    mNextOutputFrame = (mNextOutputFrame + 1) % mAsset.getFrameCount();
    return ProcessFrameStatus.SUCCESS;
  }

  private MediaFormat getOutputFormatSafely() {
    // Check if decoder is null
    if (mDecoder == null) {
      return null;
    }

    // Try to get output format safely
    try {
      return mDecoder.getOutputFormat();
    } catch (IllegalStateException e) {
      AnimaXLog.e(TAG, "getOutputFormat failed: " + e.getMessage());
      return null;
    }
  }

  private void updateCurrentMaxCacheFrameCount() {
    if (mInfoTryAgainCount < 0) {
      // should never reach
      mInfoTryAgainCount = 0;
    }
    if (mInfoTryAgainCount > 1) {
      mCurrentMaxCacheFrameCount = Math.min(MAX_CACHE_FRAME_COUNT, 5 + mInfoTryAgainCount);
    } else {
      // mInfoTryAgainCount <= 1
      mCurrentMaxCacheFrameCount = Math.min(MAX_CACHE_FRAME_COUNT, 3 + 3 * mInfoTryAgainCount);
    }
  }

  private void discardCachedFrame() {
    if (mCachedFrame > 0) {
      flush();
      mCachedFrame = 0;
    }
  }

  private void flush() {
    // Codec Thread
    String errMsg = null;
    try {
      mDecoder.flush();
    } catch (MediaCodec.CodecException e) {
      errMsg = "[" + mDecoderName + "]: "
          + "flush CodecException: " + e.getMessage();
    } catch (IllegalStateException e) {
      errMsg = "[" + mDecoderName + "]: "
          + "flush IllegalStateException: " + e.getMessage();
    }
    if (null != errMsg) {
      reportError(errMsg);
      mDecoderValid = false;
    }
  }

  private boolean queueInputBuffer(final int index, final int offset, final int size,
      final long presentationTimeUs, final int flags) {
    // Codec Thread
    String errMsg = null;
    try {
      mDecoder.queueInputBuffer(index, offset, size, presentationTimeUs, flags);
    } catch (MediaCodec.CryptoException e) {
      errMsg = "[" + mDecoderName + "]: "
          + "queueInputBuffer CryptoException: " + e.getMessage();
    } catch (MediaCodec.CodecException e) {
      errMsg = "[" + mDecoderName + "]: "
          + "queueInputBuffer CodecException: " + e.getMessage();
    } catch (IllegalStateException e) {
      errMsg = "[" + mDecoderName + "]: "
          + "queueInputBuffer IllegalStateException: " + e.getMessage();
    }
    if (null != errMsg) {
      reportError(errMsg);
      return false;
    }
    return true;
  }

  private boolean releaseOutputBuffer(final int index, final boolean render) {
    // Codec Thread
    String errMsg = null;
    try {
      mDecoder.releaseOutputBuffer(index, render);
    } catch (MediaCodec.CodecException e) {
      errMsg = "[" + mDecoderName + "]: "
          + "releaseOutputBuffer CodecException: " + e.getMessage();
    } catch (IllegalStateException e) {
      errMsg = "[" + mDecoderName + "]: "
          + "releaseOutputBuffer IllegalStateException: " + e.getMessage();
    }
    if (null != errMsg) {
      reportError(errMsg);
      return false;
    }
    return true;
  }

  private int prepareInputBuffer(ByteBuffer buffer) {
    // Codec Thread
    int inputBufferIndex = -1;
    try {
      inputBufferIndex = mDecoder.dequeueInputBuffer(mTimeoutUs);
    } catch (MediaCodec.CodecException e) {
      reportError("[" + mDecoderName + "]: "
          + "dequeueInputBuffer CodecException: " + e.getMessage());
    } catch (IllegalStateException e) {
      reportError("[" + mDecoderName + "]: "
          + "dequeueInputBuffer IllegalStateException: " + e.getMessage());
      mDecoderValid = false;
      mDecoderShouldRestart = true;
    }
    if (inputBufferIndex < 0) {
      return -1;
    }
    ByteBuffer inputBuffer = getInputBuffer(inputBufferIndex);
    if (null == inputBuffer) {
      return -1;
    }
    if (null != buffer && inputBuffer.remaining() >= buffer.remaining()) {
      try {
        inputBuffer.put(buffer);
      } catch (RuntimeException e) {
        return -1;
      }
    }
    return inputBufferIndex;
  }

  private ByteBuffer getInputBuffer(int index) {
    // Codec Thread
    ByteBuffer inputBuffer = null;
    try {
      if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.LOLLIPOP) {
        inputBuffer = mDecoder.getInputBuffer(index);
      } else {
        inputBuffer = mDecoder.getInputBuffers()[index];
      }
    } catch (MediaCodec.CodecException e) {
      reportError("[" + mDecoderName + "]: "
          + "getInputBuffer CodecException: " + e.getMessage());
    } catch (IllegalStateException e) {
      reportError("[" + mDecoderName + "]: "
          + "getInputBuffer IllegalStateException: " + e.getMessage());
    }
    return inputBuffer;
  }

  private void prepareFrameToMaxCacheCapacity() {
    while (true) {
      if (!prepareNextFrame()) {
        return;
      }
    }
  }

  private boolean prepareNextFrame() {
    if (!mDecoderValid || null == mAsset) {
      return false;
    }
    if (mCachedFrame >= mCurrentMaxCacheFrameCount) {
      return false;
    }
    FrameInfo frameInfo = mAsset.getFrameInfo(mNextInputFrame);
    ByteBuffer frameData = mAsset.getFrameData(frameInfo);
    if (null == frameData) {
      return false;
    }
    int inputBufferIndex = prepareInputBuffer(frameData);
    if (inputBufferIndex < 0) {
      return false;
    }
    int offset = 0;
    int size = frameInfo.end() - frameInfo.begin();
    long presentationTimeUs = frameInfo.getPresentationTimeUs();
    int flags = 0;
    boolean success = queueInputBuffer(inputBufferIndex, offset, size, presentationTimeUs, flags);
    if (success) {
      ++mCachedFrame;
      if (INVALID_FRAME == mNextOutputFrame) {
        mNextOutputFrame = mNextInputFrame;
      }
      mNextInputFrame = (mNextInputFrame + 1) % mAsset.getFrameCount();
    }
    return success;
  }

  private void reportError(@NonNull String errorMsg) {
    if (mReporter != null) {
      mReporter.reportError(errorMsg);
    }
  }

  private void markDrawnOnce() {
    if (mReporter != null) {
      mReporter.markDrawnOnce();
    }
  }
}
