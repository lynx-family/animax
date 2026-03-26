// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.animax.drawable;

import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.*;

import android.content.Context;
import android.graphics.SurfaceTexture;
import androidx.test.ext.junit.runners.AndroidJUnit4;
import androidx.test.platform.app.InstrumentationRegistry;
import com.lynx.animax.util.AnimaX;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

@RunWith(AndroidJUnit4.class)
public class FirstFrameAwareSurfaceTextureTest {
  private FirstFrameAwareSurfaceTexture testSurfaceTexture;

  @Before
  public void setUp() throws Exception {
    Context context =
        InstrumentationRegistry.getInstrumentation().getTargetContext().getApplicationContext();
    AnimaX.inst().init();
    testSurfaceTexture = new FirstFrameAwareSurfaceTexture();
  }

  @Test
  public void testOnFrameAvailable_firstTime() {
    FirstFrameAwareSurfaceTexture.FirstFrameAvailableListener mockCustomListener =
        mock(FirstFrameAwareSurfaceTexture.FirstFrameAvailableListener.class);
    testSurfaceTexture.setFirstFrameListener(mockCustomListener);

    testSurfaceTexture.onFrameAvailable(mock(SurfaceTexture.class));

    verify(mockCustomListener, times(1)).onFirstFrameAvailable();
  }

  @Test
  public void testOnFrameAvailable_subsequentTimes() {
    FirstFrameAwareSurfaceTexture.FirstFrameAvailableListener mockCustomListener =
        mock(FirstFrameAwareSurfaceTexture.FirstFrameAvailableListener.class);
    testSurfaceTexture.setFirstFrameListener(mockCustomListener);

    testSurfaceTexture.onFrameAvailable(mock(SurfaceTexture.class));
    testSurfaceTexture.onFrameAvailable(mock(SurfaceTexture.class));

    verify(mockCustomListener, times(1)).onFirstFrameAvailable();
  }

  @Test
  public void testOnFrameAvailable_listenerSetAfterOnFrameAvailableWillBeNotifiedImmediately() {
    // Create the object to test
    FirstFrameAwareSurfaceTexture texture = new FirstFrameAwareSurfaceTexture();

    // 1. Set a FirstFrameAvailableListener before the onFrameAvailable method is triggered.
    FirstFrameAwareSurfaceTexture.FirstFrameAvailableListener firstListener =
        mock(FirstFrameAwareSurfaceTexture.FirstFrameAvailableListener.class);
    texture.setFirstFrameListener(firstListener);

    // 2. Trigger the onFrameAvailable method.
    texture.onFrameAvailable(null);

    // 3. Ensure that the initial listener was notified.
    verify(firstListener, times(1)).onFirstFrameAvailable();

    // 4. Set a new FirstFrameAvailableListener.
    FirstFrameAwareSurfaceTexture.FirstFrameAvailableListener secondListener =
        mock(FirstFrameAwareSurfaceTexture.FirstFrameAvailableListener.class);
    texture.setFirstFrameListener(secondListener);

    // 5. Ensure that the new listener is immediately notified.
    verify(secondListener, times(1)).onFirstFrameAvailable();
  }

  @Test
  public void testSetFirstFrameListener_notifiesImmediatelyIfFirstFrameHasPassed() {
    testSurfaceTexture.onFrameAvailable(mock(SurfaceTexture.class));

    FirstFrameAwareSurfaceTexture.FirstFrameAvailableListener mockListener =
        mock(FirstFrameAwareSurfaceTexture.FirstFrameAvailableListener.class);
    testSurfaceTexture.setFirstFrameListener(mockListener);

    verify(mockListener, times(1)).onFirstFrameAvailable();
  }

  @Test
  public void testOnFrameAvailableListener_notifiesImmediatelyIfFirstFrameHasPassed()
      throws InterruptedException {
    final CountDownLatch latch = new CountDownLatch(1);

    SurfaceTexture.OnFrameAvailableListener mockListener =
        mock(SurfaceTexture.OnFrameAvailableListener.class);
    doAnswer(invocation -> {
      latch.countDown();
      return null;
    })
        .when(mockListener)
        .onFrameAvailable(any(SurfaceTexture.class));

    testSurfaceTexture.setOnFrameAvailableListener(mockListener);

    testSurfaceTexture.onFrameAvailable(mock(SurfaceTexture.class));
    boolean callbackOccurred = latch.await(1, TimeUnit.SECONDS);
    assertTrue("onFrameAvailable callback was not triggered within the timeout.", callbackOccurred);

    verify(mockListener, times(1)).onFrameAvailable(any(SurfaceTexture.class));
  }
}
