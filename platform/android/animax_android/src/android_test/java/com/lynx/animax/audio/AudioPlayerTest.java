// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.audio;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

import android.content.Context;
import android.media.AudioFormat;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.media.MediaMuxer;
import androidx.test.platform.app.InstrumentationRegistry;
import java.io.File;
import java.nio.ByteBuffer;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class AudioPlayerTest {
  private String validAudioPath;
  private AudioAsset validAsset;

  @Before
  public void setUp() throws Exception {
    Context targetContext = InstrumentationRegistry.getInstrumentation().getTargetContext();

    File validFile = new File(targetContext.getCacheDir(), "generated_test_audio_for_player.m4a");
    validAudioPath = validFile.getAbsolutePath();
    generateRealSilenceAudioFile(validAudioPath);

    validAsset = AudioAsset.create();
    boolean loaded = validAsset.loadLocal(validAudioPath);
    assertTrue(loaded);
    assertNotNull(validAsset.getAudioData());
  }

  @After
  public void tearDown() {
    if (validAudioPath != null)
      new File(validAudioPath).delete();
  }

  private void generateRealSilenceAudioFile(String outputPath) throws Exception {
    MediaMuxer muxer = new MediaMuxer(outputPath, MediaMuxer.OutputFormat.MUXER_OUTPUT_MPEG_4);
    MediaCodec encoder = MediaCodec.createEncoderByType(MediaFormat.MIMETYPE_AUDIO_AAC);

    MediaFormat format = MediaFormat.createAudioFormat(MediaFormat.MIMETYPE_AUDIO_AAC, 44100, 2);
    format.setInteger(MediaFormat.KEY_AAC_PROFILE, MediaCodecInfo.CodecProfileLevel.AACObjectLC);
    format.setInteger(MediaFormat.KEY_BIT_RATE, 64000);
    format.setInteger(MediaFormat.KEY_MAX_INPUT_SIZE, 1048576);

    encoder.configure(format, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
    encoder.start();

    int trackIndex = -1;
    boolean muxerStarted = false;
    boolean inputDone = false;
    boolean outputDone = false;

    int numFrames = 20;
    int framesInput = 0;
    long presentationTimeUs = 0;
    MediaCodec.BufferInfo bufferInfo = new MediaCodec.BufferInfo();

    while (!outputDone) {
      if (!inputDone) {
        int inputBufferId = encoder.dequeueInputBuffer(10000);
        if (inputBufferId >= 0) {
          ByteBuffer inputBuffer = encoder.getInputBuffer(inputBufferId);
          inputBuffer.clear();

          if (framesInput < numFrames) {
            byte[] silence = new byte[2048];
            inputBuffer.put(silence);
            encoder.queueInputBuffer(inputBufferId, 0, silence.length, presentationTimeUs, 0);
            presentationTimeUs += 11609L;
            framesInput++;
          } else {
            encoder.queueInputBuffer(
                inputBufferId, 0, 0, presentationTimeUs, MediaCodec.BUFFER_FLAG_END_OF_STREAM);
            inputDone = true;
          }
        }
      }

      int outputBufferId = encoder.dequeueOutputBuffer(bufferInfo, 10000);
      if (outputBufferId == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {
        trackIndex = muxer.addTrack(encoder.getOutputFormat());
        muxer.start();
        muxerStarted = true;
      } else if (outputBufferId >= 0) {
        ByteBuffer outputBuffer = encoder.getOutputBuffer(outputBufferId);
        if ((bufferInfo.flags & MediaCodec.BUFFER_FLAG_CODEC_CONFIG) != 0) {
          bufferInfo.size = 0;
        }
        if (bufferInfo.size != 0 && muxerStarted) {
          outputBuffer.position(bufferInfo.offset);
          outputBuffer.limit(bufferInfo.offset + bufferInfo.size);
          muxer.writeSampleData(trackIndex, outputBuffer, bufferInfo);
        }
        encoder.releaseOutputBuffer(outputBufferId, false);
        if ((bufferInfo.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0) {
          outputDone = true;
        }
      }
    }

    encoder.stop();
    encoder.release();
    if (muxerStarted)
      muxer.stop();
    muxer.release();
  }

  @Test
  public void testAudioPlayerCreate_InvalidAsset() {
    AudioAsset invalidAsset = AudioAsset.create();
    AudioPlayer player = AudioPlayer.create(invalidAsset);
    assertNull(player);
  }

  @Test
  public void testAudioPlayer_PlayPauseAndDuration() {
    AudioPlayer player = AudioPlayer.create(validAsset);
    assertNotNull(player);

    double duration = player.getDuration();
    assertTrue(duration > 0);
    assertEquals(0.0, player.getAudioTime(), 0.001);
    player.play();
    player.pause();
  }

  @Test
  public void testAudioPlayer_SeekToProgress() {
    AudioPlayer player = AudioPlayer.create(validAsset);
    assertNotNull(player);
    player.seekToProgress(0.5);
    double audioTimeAfterSeek = player.getAudioTime();
    assertTrue(audioTimeAfterSeek > 0);
    assertTrue(audioTimeAfterSeek <= player.getDuration());
  }

  @Test
  public void testAudioPlayer_OnDataRequest_NoWrapAround() {
    AudioPlayer player = AudioPlayer.create(validAsset);
    assertNotNull(player);

    ByteBuffer dstBuffer = ByteBuffer.allocate(100);
    player.onDataRequest(0, 100, dstBuffer);

    assertEquals(100, dstBuffer.position());
  }

  @Test
  public void testAudioPlayer_OnDataRequest_WrapAround() {
    AudioPlayer player = AudioPlayer.create(validAsset);
    assertNotNull(player);

    int capacity = validAsset.getAudioData().capacity();
    ByteBuffer dstBuffer = ByteBuffer.allocate(100);

    long offset = capacity - 50;

    player.onDataRequest(offset, 100, dstBuffer);

    assertEquals(100, dstBuffer.position());
  }

  @Test
  public void testAudioRenderer_Mono_8Bit() {
    AudioInfo mono8BitInfo = new AudioInfo();
    mono8BitInfo.setSampleRate(44100);
    mono8BitInfo.setChannelCount(1);
    mono8BitInfo.setPCMEncoding(AudioFormat.ENCODING_PCM_8BIT);
    mono8BitInfo.setDuration(1000L);

    AudioRenderer renderer = new AudioRenderer(mono8BitInfo);
    assertNotNull(renderer);
  }

  @Test
  public void testAudioRenderer_Float() {
    AudioInfo floatInfo = new AudioInfo();
    floatInfo.setSampleRate(48000);
    floatInfo.setChannelCount(2);
    floatInfo.setPCMEncoding(AudioFormat.ENCODING_PCM_FLOAT);
    floatInfo.setDuration(1000L);

    AudioRenderer renderer = new AudioRenderer(floatInfo);
    assertNotNull(renderer);
  }

  @Test
  public void testAudioRenderer_InitFailed() {
    AudioInfo badInfo = new AudioInfo();
    badInfo.setSampleRate(-1);
    badInfo.setChannelCount(2);
    badInfo.setDuration(100L);

    boolean caughtException = false;
    try {
      new AudioRenderer(badInfo);
    } catch (Exception e) {
      caughtException = true;
    }

    assertTrue(caughtException);
  }
}
