// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.player;

import android.media.MediaFormat;
import com.lynx.animax.base.CalledByNative;
import com.lynx.animax.util.AnimaXLog;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Collections;

public class VideoAsset {
  private static final String TAG = "VideoAsset";

  // Android use the following codec-specific data buffer keys
  private static final String[] CODEC_SPECIFIC_DATA_KEYS = {"csd-0", "csd-1", "csd-2"};

  private String mFileUrl;
  private VideoInfo mVideoInfo;
  private boolean mIsValid;
  private VideoRawData mRawData;

  private VideoAsset() {}

  /**
   * create a video asset
   * @return video asset
   */
  @CalledByNative
  public static VideoAsset create() {
    return new VideoAsset();
  }

  public String getFileUrl() {
    return mFileUrl;
  }

  public MediaFormat getFormat() {
    if (null == mVideoInfo) {
      return null;
    }
    return mVideoInfo.getFormat();
  }

  @CalledByNative
  public int getWidth() {
    if (null == mVideoInfo) {
      return 0;
    }
    return mVideoInfo.getWidth();
  }

  @CalledByNative
  public int getHeight() {
    if (null == mVideoInfo) {
      return 0;
    }
    return mVideoInfo.getHeight();
  }

  @CalledByNative
  public int getFrameCount() {
    if (null == mVideoInfo) {
      return 0;
    }
    return mVideoInfo.getFrameCount();
  }

  @CalledByNative
  public float getFrameRate() {
    if (null == mVideoInfo) {
      return 0.f;
    }
    return mVideoInfo.getFrameRate();
  }

  /**
   * whether the video asset is valid
   * @return (width > 0 && height > 0 && frameCount > 0 && frameRate > 0.f)
   */
  public boolean isValid() {
    return mIsValid;
  }

  @CalledByNative
  public boolean loadLocal(String filePath) {
    if (null == filePath || filePath.isEmpty()) {
      AnimaXLog.e(TAG, "video file path is empty");
      mIsValid = false;
      return mIsValid;
    }
    mVideoInfo = VideoAssetExtractor.extractVideoInfo(filePath);
    mIsValid = mVideoInfo != null && mVideoInfo.getWidth() > 0 && mVideoInfo.getHeight() > 0
        && mVideoInfo.getFrameCount() > 0 && mVideoInfo.getFrameRate() > 0.f;
    if (mIsValid) {
      mFileUrl = filePath;
      prepareFrameBufferIfNecessary();
    } else {
      mVideoInfo = null;
    }
    return mIsValid;
  }

  public boolean prepareFrameBufferIfNecessary() {
    if (null == mRawData) {
      prepareFrameBuffer();
    }
    return IsKeyFramesValid();
  }

  private void prepareFrameBuffer() {
    AnimaXLog.i(TAG, "prepareFrameBuffer");
    if (null == mVideoInfo) {
      return;
    }
    mRawData = VideoAssetExtractor.extractVideoRawData(mFileUrl, mVideoInfo.getVideoTrackIndex());
    if (null == mRawData || !IsKeyFramesValid()) {
      // fail
      return;
    }

    ArrayList<FrameInfo> infos = mRawData.getFrameInfos();
    if (mVideoInfo.getFrameCount() != infos.size()) {
      AnimaXLog.e(TAG,
          "prepareFrameBuffer: frameCount change from " + mVideoInfo.getFrameCount() + " to "
              + infos.size());
      mVideoInfo.setFrameCount(infos.size());
    }
  }

  private boolean IsKeyFramesValid() {
    if (null == mRawData) {
      return false;
    }
    ArrayList<Integer> keyFrames = mRawData.getKeyFrames();
    return !keyFrames.isEmpty() && (keyFrames.get(0) == 0);
  }

  @CalledByNative
  public int getPrevKeyFrame(int frame) {
    // mRawData != null
    ArrayList<Integer> keyFrames = mRawData.getKeyFrames();
    int index = Collections.binarySearch(keyFrames, frame);
    if (index < 0) {
      // 1. if index is less than 0, the input frame isn't key frame and the input frame should be
      // inserted at position (-index-1). The key frame is the previous one, (-index-2).
      // 2. mKeyFrames.get(0) must be 0, so (-index-2) must be greater or equal to 0.
      index = -index - 2;
    }
    return keyFrames.get(index);
  }

  public FrameInfo getFrameInfo(int frame) {
    if (null == mRawData) {
      return null;
    }
    ArrayList<FrameInfo> infos = mRawData.getFrameInfos();
    if (frame < 0 || frame >= infos.size()) {
      return null;
    }
    return infos.get(frame);
  }

  @CalledByNative
  public ByteBuffer getFrameData(int frame) {
    FrameInfo frameInfo = getFrameInfo(frame);
    return getFrameData(frameInfo);
  }

  @CalledByNative
  public ByteBuffer getVideoParameterSets() {
    MediaFormat format = mVideoInfo.getFormat();
    if (format == null) {
      return null;
    }

    ArrayList<ByteBuffer> buffers = new ArrayList<>();
    int totalSize = 0;

    for (String key : CODEC_SPECIFIC_DATA_KEYS) {
      if (!format.containsKey(key)) {
        break;
      }
      ByteBuffer buffer = format.getByteBuffer(key);
      if (buffer != null && buffer.remaining() > 0) {
        buffers.add(buffer);
        totalSize += buffer.remaining();
      }
    }

    if (buffers.isEmpty()) {
      return null;
    }

    ByteBuffer mergedBuffer = ByteBuffer.allocate(totalSize);
    for (ByteBuffer buffer : buffers) {
      mergedBuffer.put(buffer.duplicate());
    }
    mergedBuffer.flip();
    return mergedBuffer;
  }

  @CalledByNative
  public boolean isHEVCFormat() {
    String mimeType = getMimeType();
    return mimeType != null && mimeType.equalsIgnoreCase(MediaFormat.MIMETYPE_VIDEO_HEVC);
  }

  @CalledByNative
  public boolean isSupportDecodeBySystem(boolean requireHardwareAccelerated) {
    String mimeType = getMimeType();
    return mimeType != null
        && CodecDecodeCapability.getInstance().isSupportDecodeBySystem(
            mimeType, requireHardwareAccelerated);
  }

  private String getMimeType() {
    if (null == mVideoInfo) {
      return null;
    }
    MediaFormat format = mVideoInfo.getFormat();
    if (null == format) {
      return null;
    }
    return format.getString(MediaFormat.KEY_MIME);
  }

  public ByteBuffer getFrameData(FrameInfo frameInfo) {
    if (null == frameInfo || null == mRawData) {
      return null;
    }
    ByteBuffer frameBuffer = mRawData.getFrameBuffer();
    if (null == frameBuffer) {
      return null;
    }
    frameBuffer.position(0);
    frameBuffer.limit(frameInfo.end());
    frameBuffer.position(frameInfo.begin());
    return frameBuffer;
  }
}
