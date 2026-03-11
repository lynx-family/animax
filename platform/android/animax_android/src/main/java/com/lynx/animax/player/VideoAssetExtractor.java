// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.player;

import android.media.MediaExtractor;
import android.media.MediaFormat;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import com.lynx.animax.util.AnimaXLog;
import java.io.File;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.ArrayList;

public class VideoAssetExtractor {
  private static final String TAG = "VideoAssetExtractor";

  /**
   * extract video info. Including media format, width, height, frame rate, frame count, and video
   * track index.
   * @param fileUrl video to be extracted.
   * @return        videoInfo, nullable.
   */
  @Nullable
  public static VideoInfo extractVideoInfo(String fileUrl) {
    if (null == fileUrl || fileUrl.isEmpty()) {
      return null;
    }
    VideoInfo videoInfo = new VideoInfo();
    MediaExtractor extractor = new MediaExtractor();
    boolean success = false;
    do {
      if (!setDataSource(extractor, fileUrl)) {
        AnimaXLog.e(TAG, "load fail, MediaExtractor setDataSource error");
        break;
      }
      // get video track index and format.
      findVideoTrack(extractor, videoInfo);
      if (videoInfo.getVideoTrackIndex() < 0) {
        AnimaXLog.e(TAG, "load fail, there is no video track");
        break;
      }

      updateWidth(videoInfo);
      updateHeight(videoInfo);
      updateFrameRate(videoInfo);
      updateFrameCount(videoInfo);
      success = true;
    } while (false);

    extractor.release();
    return success ? videoInfo : null;
  }

  /**
   * extract video raw data. Including frame data, frame infos, key frame infos.
   * @param fileUrl video to be extracted.
   * @return        rawData, nullable.
   */
  @Nullable
  public static VideoRawData extractVideoRawData(String fileUrl, int trackIndex) {
    if (null == fileUrl || fileUrl.isEmpty() || trackIndex < 0) {
      return null;
    }
    VideoRawData rawData = new VideoRawData();
    MediaExtractor extractor = new MediaExtractor();
    boolean success = false;
    do {
      if (!setDataSource(extractor, fileUrl)) {
        AnimaXLog.e(TAG, "load fail, MediaExtractor setDataSource error");
        break;
      }

      extractor.selectTrack(trackIndex);
      File videoFile = new File(fileUrl);
      doExtractVideoRawData(extractor, rawData, (int) videoFile.length());
      success = true;
    } while (false);
    extractor.release();

    return success ? rawData : null;
  }

  private static void doExtractVideoRawData(
      @NonNull MediaExtractor extractor, @NonNull VideoRawData rawData, int videoSize) {
    if (videoSize <= 0) {
      AnimaXLog.e(TAG, "doExtractVideoRawData fail, videoSize = " + videoSize);
      return;
    }
    ByteBuffer frameBuffer = ByteBuffer.allocate(videoSize);
    ArrayList<Integer> keyFrames = rawData.getKeyFrames();
    ArrayList<FrameInfo> infos = rawData.getFrameInfos();
    boolean success = false;
    int offset = 0;
    while (true) {
      int readSize = readFrameData(extractor, offset, frameBuffer);
      if (readSize > 0) {
        boolean isKeyFrame = (extractor.getSampleFlags() & MediaExtractor.SAMPLE_FLAG_SYNC) > 0;
        if (isKeyFrame) {
          keyFrames.add(infos.size());
        }
        infos.add(new FrameInfo(offset, offset + readSize, extractor.getSampleTime()));
        offset += readSize;
        extractor.advance();
      } else if (readSize == -1) {
        // End Of Stream
        success = true;
        break;
      } else {
        // Error occurs
        success = false;
        break;
      }
    }

    if (success) {
      AnimaXLog.i(TAG,
          "video size: " + offset + " bytes, frame count: " + infos.size()
              + ", key frame count: " + keyFrames.size());
      rawData.setFrameBuffer(frameBuffer);
    } else {
      AnimaXLog.e(TAG, "doExtractVideoRawData fail");
      keyFrames.clear();
      infos.clear();
    }
  }

  private static boolean setDataSource(@NonNull MediaExtractor extractor, String filePath) {
    try {
      extractor.setDataSource(filePath);
      return true;
    } catch (IOException e) {
      AnimaXLog.e(TAG, "setDataSource IOException: " + e.getMessage());
    }
    return false;
  }

  private static void findVideoTrack(
      @NonNull MediaExtractor extractor, @NonNull VideoInfo videoInfo) {
    int numTracks = extractor.getTrackCount();
    for (int i = 0; i < numTracks; ++i) {
      MediaFormat format = extractor.getTrackFormat(i);
      String mine = null;
      try {
        mine = format.getString(MediaFormat.KEY_MIME);
      } catch (ClassCastException e) {
        AnimaXLog.e(TAG, "format doesn't has mine property: " + e.getMessage());
      }
      if (mine != null && mine.startsWith("video/")) {
        videoInfo.setVideoTrackIndex(i);
        videoInfo.setFormat(format);
        break;
      }
    }
  }

  private static void updateWidth(@NonNull VideoInfo videoInfo) {
    MediaFormat format = videoInfo.getFormat();
    try {
      videoInfo.setWidth(format.getInteger(MediaFormat.KEY_WIDTH));
    } catch (NullPointerException | ClassCastException e) {
      AnimaXLog.e(TAG, "format doesn't has width property: " + e.getMessage());
      videoInfo.setWidth(0);
    }
  }

  private static void updateHeight(@NonNull VideoInfo videoInfo) {
    MediaFormat format = videoInfo.getFormat();
    try {
      videoInfo.setHeight(format.getInteger(MediaFormat.KEY_HEIGHT));
    } catch (NullPointerException | ClassCastException e) {
      AnimaXLog.e(TAG, "format doesn't has height property: " + e.getMessage());
      videoInfo.setHeight(0);
    }
  }

  private static void updateFrameRate(@NonNull VideoInfo videoInfo) {
    MediaFormat format = videoInfo.getFormat();
    try {
      videoInfo.setFrameRate(format.getInteger(MediaFormat.KEY_FRAME_RATE));
    } catch (NullPointerException | ClassCastException e) {
      AnimaXLog.e(TAG, "format doesn't has int frame-rate property: " + e.getMessage());
      videoInfo.setFrameRate(0.f);
    }
    if (0.f != videoInfo.getFrameRate()) {
      return;
    }
    try {
      videoInfo.setFrameRate(format.getFloat(MediaFormat.KEY_FRAME_RATE));
    } catch (NullPointerException | ClassCastException e) {
      AnimaXLog.e(TAG, "format doesn't has float frame-rate property: " + e.getMessage());
    }
  }

  private static void updateFrameCount(@NonNull VideoInfo videoInfo) {
    long durationUs = 0;
    try {
      durationUs = videoInfo.getFormat().getLong(MediaFormat.KEY_DURATION);
    } catch (NullPointerException | ClassCastException e) {
      AnimaXLog.e(TAG, "format doesn't has long durationUs property: " + e.getMessage());
    }
    videoInfo.setFrameCount(Math.round(videoInfo.getFrameRate() * durationUs / 1000000));
    AnimaXLog.i(TAG, "frame count: " + videoInfo.getFrameCount());
  }

  private static int readFrameData(
      @NonNull MediaExtractor extractor, int currentOffset, @NonNull ByteBuffer frameBuffer) {
    int readSize = 0;
    try {
      readSize = extractor.readSampleData(frameBuffer, currentOffset);
    } catch (IllegalArgumentException e) {
      String errMsg = e.getMessage();
      if (null == errMsg) {
        errMsg = "current buffer may not have enough space to read";
      }
      AnimaXLog.e(TAG, "readSampleData IllegalArgumentException: " + errMsg);
    }
    return readSize;
  }
}
