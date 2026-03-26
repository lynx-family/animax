// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.audio;
import android.media.MediaCodec;
import android.media.MediaExtractor;
import android.media.MediaFormat;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import com.lynx.animax.util.AnimaXLog;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

public class AudioAssetExtractor {
  private static final String TAG = "AudioAssetExtractor";
  private static final int MAX_INVALID_LOOP_COUNT = 100;
  private static final int MAX_FRAME_COUNT_PER_LOOP = 20;

  /**
   * extract decoded audio data.
   * @param fileUrl audio to be extracted.
   * @return        decoded audio data, nullable.
   */
  @Nullable
  public static ByteBuffer extractDecodedAudioData(
      @NonNull AudioInfo info, String fileUrl, int trackIndex) {
    if (null == fileUrl || fileUrl.isEmpty() || trackIndex < 0) {
      return null;
    }

    MediaExtractor extractor = new MediaExtractor();
    if (!setDataSource(extractor, fileUrl)) {
      AnimaXLog.e(TAG, "load fail, MediaExtractor setDataSource error");
      extractor.release();
      return null;
    }

    extractor.selectTrack(trackIndex);

    AudioAssetDecoder decoder = new AudioAssetDecoder(info);
    if (!decoder.isValid()) {
      AnimaXLog.e(TAG, "Failed to init decoder.");
      extractor.release();
      return null;
    }
    ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
    boolean needInput = true;

    try {
      decoder.start();
      ByteBuffer buffer = ByteBuffer.allocate(8 * 1024);
      int invalidLoopCount = 0;
      while (!decoder.isFinished()) {
        boolean hasInput = false;
        if (needInput) {
          ByteBuffer inputBuffer = decoder.getInputBuffer();
          if (inputBuffer != null) {
            int frameCount = 1;
            while (frameCount <= MAX_FRAME_COUNT_PER_LOOP) {
              extractor.readSampleData(buffer, 0);
              if (inputBuffer.position() + buffer.limit() > inputBuffer.capacity()) {
                break;
              }
              inputBuffer.put(buffer);
              if (!extractor.advance()) {
                needInput = false;
                break;
              }
              frameCount++;
            }
            inputBuffer.flip();
            int flags = needInput ? 0 : MediaCodec.BUFFER_FLAG_END_OF_STREAM;
            decoder.queueInputBuffer(0, inputBuffer.limit(), 0, flags);
            hasInput = true;
          }
        }
        ByteBuffer outputBuffer = decoder.getOutputBuffer();
        if (outputBuffer == null) {
          if (!hasInput) {
            invalidLoopCount++;
          }
          if (invalidLoopCount > MAX_INVALID_LOOP_COUNT) {
            break;
          } else {
            continue;
          }
        }
        if (outputBuffer.hasArray()) {
          outputStream.write(outputBuffer.array(),
              outputBuffer.arrayOffset() + decoder.getBufferInfo().offset,
              decoder.getBufferInfo().size);
        } else {
          byte[] pcmChunk = new byte[decoder.getBufferInfo().size];
          outputBuffer.get(pcmChunk);
          outputStream.write(pcmChunk);
        }
        decoder.releaseOutPutBuffer();
      }
      if (!decoder.isFinished()) {
        AnimaXLog.e(TAG, "Failed to decode audio asset: the decoder is in infinite loop.");
      }
    } catch (Exception e) {
      AnimaXLog.e(TAG, "Failed to decode audio asset: " + e.getMessage());
    } finally {
      decoder.stop();
      decoder.release();
      extractor.release();
    }
    return ByteBuffer.wrap(outputStream.toByteArray());
  }

  /**
   * extract audio info. Including media format, and audio track index.
   * @param fileUrl video to be extracted.
   * @return        videoInfo, nullable.
   */
  @Nullable
  public static AudioInfo extractAudioInfo(String fileUrl) {
    if (null == fileUrl || fileUrl.isEmpty()) {
      return null;
    }
    AudioInfo audioInfo = new AudioInfo();
    MediaExtractor extractor = new MediaExtractor();

    if (!setDataSource(extractor, fileUrl)) {
      AnimaXLog.e(TAG, "load fail, MediaExtractor setDataSource error");
      extractor.release();
      return null;
    }
    // get audio track index and format.
    findAudioTrack(extractor, audioInfo);
    if (audioInfo.getAudioTrackIndex() < 0) {
      AnimaXLog.e(TAG, "load fail, there is no audio track");
      extractor.release();
      return null;
    }

    extractor.release();
    return audioInfo;
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

  private static void findAudioTrack(
      @NonNull MediaExtractor extractor, @NonNull AudioInfo audioInfo) {
    int numTracks = extractor.getTrackCount();
    for (int i = 0; i < numTracks; ++i) {
      MediaFormat format = extractor.getTrackFormat(i);
      String mime = null;
      try {
        mime = format.getString(MediaFormat.KEY_MIME);
      } catch (ClassCastException e) {
        AnimaXLog.e(TAG, "format doesn't has mime property: " + e.getMessage());
      }
      if (mime != null && mime.startsWith("audio/")) {
        audioInfo.setAudioTrackIndex(i);
        audioInfo.setFormat(format);
        updateMetaInfo(audioInfo, format);
        AnimaXLog.i(TAG, "audio track format: " + format);
        break;
      }
    }
  }

  private static void updateMetaInfo(@NonNull AudioInfo info, @NonNull MediaFormat format) {
    try {
      info.setChannelCount(format.getInteger(MediaFormat.KEY_CHANNEL_COUNT));
      info.setSampleRate(format.getInteger(MediaFormat.KEY_SAMPLE_RATE));
      info.setDuration(format.getLong(MediaFormat.KEY_DURATION));
    } catch (ClassCastException e) {
      AnimaXLog.e(TAG, "Failed to get meta info from format: " + e.getMessage());
    }
  }
}
