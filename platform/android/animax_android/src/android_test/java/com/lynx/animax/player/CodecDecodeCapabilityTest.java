// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.player;

import static org.junit.Assert.*;
import static org.junit.Assert.assertTrue;

import android.media.MediaFormat;
import android.os.Build;
import org.junit.Test;

public class CodecDecodeCapabilityTest {
  @Test
  public void testGetInstance() {
    CodecDecodeCapability instance1 = CodecDecodeCapability.getInstance();
    CodecDecodeCapability instance2 = CodecDecodeCapability.getInstance();
    assertNotNull(instance1);
    assertSame(instance1, instance2);
  }

  @Test
  public void testIsDecodeSupported() {
    boolean ret = CodecDecodeCapability.getInstance().isSupportDecodeBySystem(
        MediaFormat.MIMETYPE_VIDEO_HEVC, false);
    if (Build.VERSION.SDK_INT > Build.VERSION_CODES.ECLAIR) {
      assertTrue(ret);
    }
  }
}
