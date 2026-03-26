// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.animax.drawable;

import static org.junit.Assert.*;

import android.content.Context;
import android.graphics.Bitmap;
import android.os.Build;
import android.view.View;
import androidx.test.ext.junit.runners.AndroidJUnit4;
import androidx.test.platform.app.InstrumentationRegistry;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

/**
 * Instrumented unit tests for {@link BitmapBufferGroup}.
 * These tests run on a real Android device or emulator to verify the behavior of all public
 * interfaces.
 */
@RunWith(AndroidJUnit4.class)
public class BitmapBufferGroupTest {
  private BitmapBufferGroup bitmapBufferGroup;

  @Before
  public void setUp() {
    Context testContext = InstrumentationRegistry.getInstrumentation().getTargetContext();
    View view = new View(testContext);

    bitmapBufferGroup = new BitmapBufferGroup(view, false);
  }

  @After
  public void tearDown() {
    // Ensure resources are released after each test.
    if (bitmapBufferGroup != null) {
      bitmapBufferGroup.release();
    }
  }

  // --- Tests for setBufferSize ---

  @Test
  public void testSetBufferSize_initializesBuffersWithCorrectSize() {
    bitmapBufferGroup.setBufferSize(100, 150);

    BitmapBuffer bitmapBuffer = bitmapBufferGroup.getBitmapBufferForOffscreenRendering();
    assertNotNull(bitmapBuffer);
    Bitmap bitmap = bitmapBuffer.getBitmap();
    assertNotNull("getDrawBitmapBuffer() should not be null after calling setBufferSize", bitmap);
    assertEquals("Bitmap width should be correct", 100, bitmap.getWidth());
    assertEquals("Bitmap height should be correct", 150, bitmap.getHeight());
  }

  @Test
  public void testSetBufferSize_doesNotInitializeWithInvalidSize() {
    bitmapBufferGroup.setBufferSize(0, 150);
    assertNull("Buffer should not be created for zero width",
        bitmapBufferGroup.getBitmapBufferForOffscreenRendering());

    bitmapBufferGroup.setBufferSize(100, -1);
    assertNull("Buffer should not be created for negative height",
        bitmapBufferGroup.getBitmapBufferForOffscreenRendering());
  }

  @Test
  public void testSetBufferSize_recreatesBuffersOnResize() {
    BitmapBuffer buffer;
    bitmapBufferGroup.setBufferSize(50, 50);
    buffer = bitmapBufferGroup.getBitmapBufferForOffscreenRendering();
    assertNotNull(buffer);
    Bitmap oldBitmap = buffer.getBitmap();

    bitmapBufferGroup.setBufferSize(100, 100);
    buffer = bitmapBufferGroup.getBitmapBufferForOffscreenRendering();
    assertNotNull(buffer);
    Bitmap newBitmap = buffer.getBitmap();

    assertNotNull("New bitmap should not be null", newBitmap);
    assertNotSame("A new Bitmap object should be created on resize", oldBitmap, newBitmap);
    assertEquals("New bitmap's width should be 100", 100, newBitmap.getWidth());
  }

  // --- Tests for onBufferUpdate and swapBitmap ---

  @Test
  public void testOnBufferUpdate_swapsBuffersAndInvalidatesView() {
    bitmapBufferGroup.setBufferSize(100, 100);
    BitmapBuffer initialDrawBitmapBuffer = bitmapBufferGroup.getBitmapBufferForOffscreenRendering();
    assertNotNull("Initial draw bitmap should not be null", initialDrawBitmapBuffer);
    BitmapBuffer renderBitmapBuffer = bitmapBufferGroup.swapBitmapBuffer(null);
    assertNull("Render bitmap should be null before update", renderBitmapBuffer);

    initialDrawBitmapBuffer.setState(BitmapBuffer.BufferState.Ready);
    bitmapBufferGroup.onBufferUpdate();
    BitmapBuffer newDrawBitmapBuffer = bitmapBufferGroup.getBitmapBufferForOffscreenRendering();
    assertNotNull("After update, draw bitmap should not be null", newDrawBitmapBuffer);
    assertNotSame("After update, draw bitmap should be a different buffer", initialDrawBitmapBuffer,
        newDrawBitmapBuffer);
    renderBitmapBuffer = bitmapBufferGroup.swapBitmapBuffer(null);
    assertNotNull("Render bitmap should not be null after update", renderBitmapBuffer);
    assertSame("After update, initial draw bitmap is ready and to be render", renderBitmapBuffer,
        initialDrawBitmapBuffer);

    newDrawBitmapBuffer.setState(BitmapBuffer.BufferState.Ready);
    bitmapBufferGroup.onBufferUpdate();
    bitmapBufferGroup.swapBitmapBuffer(renderBitmapBuffer);
    assertSame("after swap, initial draw bitmap is not ready now",
        BitmapBuffer.BufferState.NotReady, initialDrawBitmapBuffer.getState());
  }

  @Test
  public void testOnBufferUpdate_swapBuffersAndHold() {
    bitmapBufferGroup.setBufferSize(100, 100);
    BitmapBuffer initialDrawBitmap = bitmapBufferGroup.getBitmapBufferForOffscreenRendering();
    assertNotNull(initialDrawBitmap);
    initialDrawBitmap.setState(BitmapBuffer.BufferState.Ready);
    bitmapBufferGroup.onBufferUpdate();
    BitmapBuffer renderBitmap = bitmapBufferGroup.swapBitmapBuffer(null);
    assertSame(
        "After update, render bitmap holds initial draw bitmap", initialDrawBitmap, renderBitmap);
    for (int i = 0; i < 5; ++i) {
      BitmapBuffer buffer = bitmapBufferGroup.getBitmapBufferForOffscreenRendering();
      assertNotSame(
          "When render bitmap holds, it accessed by getDrawBitmapBuffer()", renderBitmap, buffer);
      buffer.setState(BitmapBuffer.BufferState.Ready);
      bitmapBufferGroup.onBufferUpdate();
    }
    BitmapBuffer newRenderBitmap = bitmapBufferGroup.swapBitmapBuffer(renderBitmap);
    assertNotSame("After swap, render bitmap is different", renderBitmap, newRenderBitmap);
  }

  @Test
  public void testOnBufferUpdate_setSizeWhenDrawing() {
    bitmapBufferGroup.setBufferSize(100, 100);
    BitmapBuffer buffer = bitmapBufferGroup.getBitmapBufferForOffscreenRendering();
    bitmapBufferGroup.setBufferSize(100, 50);
    assertNotNull(buffer);
    buffer.setState(BitmapBuffer.BufferState.Ready);
    bitmapBufferGroup.onBufferUpdate();
    BitmapBuffer render = bitmapBufferGroup.swapBitmapBuffer(null);
    assertNull("After setSize, swapBuffer is invalid until getDrawBitmap() is called", render);
    buffer = bitmapBufferGroup.getBitmapBufferForOffscreenRendering();
    assertNotNull(buffer);
    buffer.setState(BitmapBuffer.BufferState.Ready);
    bitmapBufferGroup.onBufferUpdate();
    buffer = bitmapBufferGroup.swapBitmapBuffer(null);
    assertNotNull("After setSize, swapBuffer is valid after getDrawBitmap() is called", buffer);
  }
  // --- Tests for HardwareBuffer related getters ---

  @Test
  public void testHardwareBufferGetters_behaviorBasedOnApiLevel() {
    bitmapBufferGroup.setBufferSize(100, 100);
    BitmapBuffer buffer = bitmapBufferGroup.getBitmapBufferForOffscreenRendering();
    assertNotNull(buffer);
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
      assertNotNull(
          "On API 26+, getDrawHardwareBuffer() should not be null", buffer.getHardwareBuffer());
    } else {
      assertNull(
          "Below API 26, getDrawHardwareBuffer() should be null", buffer.getHardwareBuffer());
    }
  }

  // --- Test for release ---

  @Test
  public void testRelease_clearsAllBuffersAndResources() {
    bitmapBufferGroup.setBufferSize(100, 100);
    assertNotNull("Before release, draw bitmap should not be null",
        bitmapBufferGroup.getBitmapBufferForOffscreenRendering());

    // Trigger release.
    bitmapBufferGroup.release();
    assertNull("After release, getDrawBitmap() should be null",
        bitmapBufferGroup.getBitmapBufferForOffscreenRendering());
  }
}
