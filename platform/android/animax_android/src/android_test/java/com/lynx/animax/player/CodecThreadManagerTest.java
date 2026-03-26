// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.animax.player;

import static org.junit.Assert.*;

import org.junit.Before;
import org.junit.Test;

public class CodecThreadManagerTest {
  private CodecThreadManager manager;

  @Before
  public void setUp() {
    manager = CodecThreadManager.getInstance();
  }

  @Test
  public void testGetInstance() {
    CodecThreadManager manager1 = CodecThreadManager.getInstance();
    CodecThreadManager manager2 = CodecThreadManager.getInstance();

    assertNotNull(manager1);
    assertSame(manager1, manager2);
  }

  @Test
  public void testRunNowOrPostToCodecThread() {
    manager.runNowOrPostToCodecThread(() -> {});
  }

  @Test
  public void testPostAtFrontAndClearQueue() {
    manager.postAtFrontAndClearQueue(() -> {});
  }

  @Test
  public void testEnsureOnCodecThread() {
    try {
      manager.ensureOnCodecThread();
      fail("Expected IllegalStateException was not thrown");
    } catch (IllegalStateException e) {
      assertTrue(e.getMessage().contains("Must be called on codec thread"));
    }
  }
}
