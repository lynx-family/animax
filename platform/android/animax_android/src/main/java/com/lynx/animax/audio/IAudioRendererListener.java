// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.audio;

import java.nio.ByteBuffer;

public interface IAudioRendererListener {
  /**
   * Provides a chunk of audio data to the renderer upon request.
   * This method is called by the audio renderer when it needs more data to fill its buffer.
   * The implementation should treat the audio source as a continuous, infinitely looping stream.
   * @param offset   The starting position of the requested data chunk, in bytes.
   * @param size     The required size of the data chunk, in bytes.
   * @param buffer   The destination ByteBuffer to be filled with audio data.
   */
  public void onDataRequest(long offset, int size, ByteBuffer buffer);
}
