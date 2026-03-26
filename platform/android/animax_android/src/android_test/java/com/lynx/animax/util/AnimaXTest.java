// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.util;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.fail;

import androidx.test.platform.app.InstrumentationRegistry;
import com.lynx.animax.base.INativeLibraryLoader;
import com.lynx.animax.util.AnimaX;
import java.lang.reflect.Method;
import junit.framework.TestCase;
import org.junit.Before;
import org.junit.Test;

public class AnimaXTest extends TestCase {
  @Before
  public void setUp() {
    AnimaX.inst().init(new INativeLibraryLoader() {
      @Override
      public void loadLibrary(String libName) throws UnsatisfiedLinkError {
        System.loadLibrary(libName);
      }
    });

    AnimaX.inst().init();
  }

  @Test
  public void testAnimaXInit() {
    AnimaX.inst().init();
    assertTrue(AnimaX.inst().hasInitialized());
  }

  @Test
  public void testAppContext() {
    AnimaX.inst().setAppContextIfUnset(
        InstrumentationRegistry.getInstrumentation().getTargetContext());
    assertNotNull(AnimaX.inst().getAppContext());
  }

  @Test
  public void testGetALogNativeAddress() {
    try {
      Method method = AnimaX.class.getDeclaredMethod("getALogNativeAddress");
      method.setAccessible(true);
      long address = (long) method.invoke(AnimaX.inst());

      assertEquals("ALog native address should be 0 on UT", 0L, address);
    } catch (Exception e) {
      fail("Failed to test getALogNativeAddress: " + e.getMessage());
    }
  }
}
