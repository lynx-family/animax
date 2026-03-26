// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.util;

import static org.junit.Assert.fail;

import com.lynx.animax.util.AnimaX;
import junit.framework.TestCase;
import org.junit.Before;
import org.junit.Test;

public class AnimaXLogTest extends TestCase {
  private static final String TEST_TAG = "AnimaXLogTest";
  private static final String TEST_MESSAGE = "test message";

  @Before
  public void setUp() {
    AnimaX.inst().init();
  }

  @Test
  public void testInitializedLogging() {
    AnimaXLog.v(TEST_TAG, TEST_MESSAGE);
    AnimaXLog.d(TEST_TAG, TEST_MESSAGE);
    AnimaXLog.i(TEST_TAG, TEST_MESSAGE);
    AnimaXLog.w(TEST_TAG, TEST_MESSAGE);
    AnimaXLog.e(TEST_TAG, TEST_MESSAGE);
    assertTrue(true);
  }

  @Test
  public void testNullParameters() {
    try {
      AnimaXLog.i(null, TEST_MESSAGE);
      AnimaXLog.i(TEST_TAG, null);
      assertTrue(true);
    } catch (Exception e) {
      fail("Logging with null parameters should not throw exception");
    }
  }

  @Test
  public void testLogLevel() {
    assertEquals(0, AnimaXLog.LogLevel.VERBOSE.ordinal());
    assertEquals(1, AnimaXLog.LogLevel.DEBUG.ordinal());
    assertEquals(2, AnimaXLog.LogLevel.INFO.ordinal());
    assertEquals(3, AnimaXLog.LogLevel.WARN.ordinal());
    assertEquals(4, AnimaXLog.LogLevel.ERROR.ordinal());
  }
}
