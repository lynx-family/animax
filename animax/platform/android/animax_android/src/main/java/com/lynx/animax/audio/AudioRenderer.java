// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.audio;

import android.media.AudioAttributes;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import androidx.annotation.NonNull;
import com.lynx.animax.util.AnimaXLog;
import java.nio.ByteBuffer;

public class AudioRenderer implements AudioTrack.OnPlaybackPositionUpdateListener {
  private static final double DEFAULT_FPS = 30.0;
  private static final String TAG = "AudioRenderer";
  private final int mMaxBufferSize;
  private final int mBytePerFrame;
  private final ByteBuffer mBuffer;
  private IAudioRendererListener mListener;
  private int mEndPositionInByte = 0;
  @NonNull final private AudioTrack mAudioTrack;

  public AudioRenderer(@NonNull AudioInfo audioInfo) {
    int sampleRate = audioInfo.getSampleRate();
    int channelCount = audioInfo.getChannelCount();

    int channelConfig = AudioFormat.CHANNEL_OUT_DEFAULT;
    if (channelCount == 1) {
      channelConfig = AudioFormat.CHANNEL_OUT_MONO;
    } else if (channelCount == 2) {
      channelConfig = AudioFormat.CHANNEL_OUT_STEREO;
    }

    int encodingFormat = audioInfo.getPCMEncodingFormat();
    int bytePerSample = 2;
    if (encodingFormat == AudioFormat.ENCODING_PCM_8BIT) {
      bytePerSample = 1;
    } else if (encodingFormat == AudioFormat.ENCODING_PCM_FLOAT) {
      bytePerSample = 4;
    }

    mBytePerFrame = channelCount * bytePerSample;

    int bufferSizeInFrame =
        Math.min(2 * (int) Math.ceil(sampleRate / DEFAULT_FPS), audioInfo.getTotalFrames());
    mMaxBufferSize = bufferSizeInFrame * mBytePerFrame;
    mBuffer = ByteBuffer.allocate(mMaxBufferSize);

    AudioAttributes attributes = new AudioAttributes.Builder()
                                     .setUsage(AudioAttributes.USAGE_MEDIA)
                                     .setContentType(AudioAttributes.CONTENT_TYPE_MUSIC)
                                     .build();

    AudioFormat format = new AudioFormat.Builder()
                             .setEncoding(encodingFormat)
                             .setSampleRate(sampleRate)
                             .setChannelMask(channelConfig)
                             .build();

    mAudioTrack = new AudioTrack(attributes, format, mMaxBufferSize, AudioTrack.MODE_STREAM,
        AudioManager.AUDIO_SESSION_ID_GENERATE);

    if (mAudioTrack.getState() == AudioTrack.STATE_UNINITIALIZED) {
      throw new IllegalStateException("AudioTrack initialization failed. sampleRate=" + sampleRate
          + ", channelConfig=" + channelConfig + ", encoding=" + encodingFormat
          + ", bufferSize=" + mMaxBufferSize);
    } else {
      AnimaXLog.i(TAG,
          "AudioTrack initialization success. sampleRate=" + sampleRate + ", channelConfig="
              + channelConfig + ", encoding=" + encodingFormat + ", bufferSize=" + mMaxBufferSize);
    }
    mAudioTrack.setPositionNotificationPeriod(bufferSizeInFrame / 2);
    mAudioTrack.setPlaybackPositionUpdateListener(this);
  }

  public void setRendererListener(@NonNull IAudioRendererListener listener) {
    mListener = listener;
  }

  @Override
  public void onMarkerReached(AudioTrack track) {}

  @Override
  public void onPeriodicNotification(AudioTrack track) {
    if (mListener == null) {
      return;
    }
    long currentPositionByte = getPlaybackHeadPositionInByte();
    int emptyBufferSize =
        Math.toIntExact(mMaxBufferSize - (mEndPositionInByte - currentPositionByte));
    mBuffer.clear();
    mBuffer.limit(emptyBufferSize);
    mListener.onDataRequest(mEndPositionInByte, emptyBufferSize, mBuffer);
    mBuffer.flip();
    int writtenSize = mAudioTrack.write(mBuffer, emptyBufferSize, AudioTrack.WRITE_NON_BLOCKING);
    if (writtenSize > 0) {
      mEndPositionInByte += writtenSize;
    }
  }

  public void play() {
    onPeriodicNotification(mAudioTrack);
    mAudioTrack.play();
  }

  public void pause() {
    mAudioTrack.pause();
  }

  public void pauseAndFlush() {
    pause();
    mAudioTrack.flush();
    mEndPositionInByte = 0;
  }

  public long getPlaybackHeadPositionInByte() {
    return Integer.toUnsignedLong(mAudioTrack.getPlaybackHeadPosition()) * mBytePerFrame;
  }

  public int alignByteToFrames(int bytePosition) {
    return bytePosition / mBytePerFrame * mBytePerFrame;
  }
}
