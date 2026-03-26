// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.audio;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

import android.content.Context;
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

public class AudioAssetTest {
  private String validAudioPath;
  private String notExistAudioPath;

  @Before
  public void setUp() throws Exception {
    Context targetContext = InstrumentationRegistry.getInstrumentation().getTargetContext();

    notExistAudioPath =
        new File(targetContext.getCacheDir(), "completely_not_exist.m4a").getAbsolutePath();

    File validFile = new File(targetContext.getCacheDir(), "generated_test_audio.m4a");
    validAudioPath = validFile.getAbsolutePath();
    generateRealSilenceAudioFile(validAudioPath);
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
        MediaFormat newFormat = encoder.getOutputFormat();
        trackIndex = muxer.addTrack(newFormat);
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
    if (muxerStarted) {
      muxer.stop();
    }
    muxer.release();
  }

  @Test
  public void testCreate_InitialState() {
    AudioAsset asset = AudioAsset.create();
    assertNotNull(asset);
    assertFalse(asset.isValid());
    assertNull(asset.getAudioInfo());
    assertNull(asset.getAudioData());
  }

  @Test
  public void testLoadLocal_EmptyOrNullPath() {
    AudioAsset asset = AudioAsset.create();
    assertFalse(asset.loadLocal(null));
    assertFalse(asset.loadLocal(""));
    assertFalse(asset.isValid());
  }

  @Test
  public void testLoadLocal_FileNotExist() {
    AudioAsset asset = AudioAsset.create();
    boolean success = asset.loadLocal(notExistAudioPath);

    assertFalse(success);
    assertFalse(asset.isValid());
    assertNull(asset.getAudioInfo());
  }

  @Test
  public void testLoadLocal_SuccessExtractInfo_And_DecodeComplete() {
    AudioAsset asset = AudioAsset.create();

    boolean success = asset.loadLocal(validAudioPath);

    assertTrue(success);
    assertTrue(asset.isValid());

    AudioInfo info = asset.getAudioInfo();
    assertNotNull(info);
    assertEquals(2, info.getChannelCount());
    assertEquals(44100, info.getSampleRate());

    ByteBuffer audioData = asset.getAudioData();
    assertNotNull(audioData);
    assertTrue(audioData.limit() > 0);

    boolean loadAgain = asset.loadLocal(validAudioPath);
    assertTrue(loadAgain);
    assertEquals(audioData, asset.getAudioData());
  }
}
