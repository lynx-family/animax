// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.property;

import androidx.annotation.NonNull;
import com.lynx.animax.listener.AnimaXParam;
import com.lynx.animax.listener.AnimationListenerAdapter;
import com.lynx.animax.ui.AnimaXView;
import java.util.List;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicReference;
import junit.framework.TestCase;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class AnimaXGetKeysForKeyPathTest extends TestCase {
  private static final long sTimeout = 3000;
  private boolean mIsNotCapable;
  private AnimaXView mAnimaXView;

  @Before
  public void setUp() {
    mIsNotCapable = !PropertyTestUtil.checkCapability();
  }

  @After
  public void tearDown() {
    if (mAnimaXView != null) {
      mAnimaXView.release();
      mAnimaXView = null;
    }
  }

  private void doKeyPathTest(
      final AnimaXKeyPath keyPath, final int expectedSize, final String msg) {
    if (mIsNotCapable) {
      return;
    }
    final CountDownLatch latch = new CountDownLatch(1);
    final AtomicReference<Throwable> errorRef = new AtomicReference<>(null);

    mAnimaXView = PropertyTestUtil.createAnimaXView(new AnimationListenerAdapter() {
      @Override
      public void onReady(AnimaXParam param) {
        mAnimaXView.getPlayer().getKeysForKeyPath(keyPath, new AnimaXKeyPathListCallback() {
          @Override
          public void onCallback(@NonNull List<AnimaXKeyPath> keyPaths) {
            try {
              String keyPathsString = (keyPaths == null) ? "null" : keyPaths.toString();
              assertTrue(msg + " (actual: " + (keyPaths == null ? 0 : keyPaths.size())
                      + ", items: " + keyPathsString + ")",
                  keyPaths != null && keyPaths.size() == expectedSize);
            } catch (Throwable t) {
              errorRef.set(t);
            } finally {
              latch.countDown();
            }
          }
        });
      }
    }, false);

    try {
      assertTrue(msg + " timed out", latch.await(sTimeout, TimeUnit.MILLISECONDS));
      if (errorRef.get() != null) {
        throw new AssertionError(errorRef.get());
      }
    } catch (InterruptedException e) {
      fail("Test interrupted: " + e.getMessage());
      Thread.currentThread().interrupt();
    }
  }

  @Test
  public void testGetKeysForAllLayers() {
    doKeyPathTest(new AnimaXKeyPath("**"), 40, "Should have 41 key paths for **");
  }

  @Test
  public void testGetKeysForSpecificLayer() {
    doKeyPathTest(new AnimaXKeyPath("rect"), 1, "Should have 1 key path for rect");
  }

  @Test
  public void testGetKeysForNestedProperties() {
    doKeyPathTest(
        new AnimaXKeyPath("rect", "**", "fill"), 0, "Should have 1 key path for rect/**/fill");
  }

  @Test
  public void testGetKeysForStar() {
    doKeyPathTest(new AnimaXKeyPath("*"), 5, "Should have 6 key paths for *");
  }

  @Test
  public void testGetKeysForStarStar() {
    doKeyPathTest(new AnimaXKeyPath("*", "*"), 7, "Should have 7 key paths for *,*");
  }

  @Test
  public void testGetKeysForStarStarStar() {
    doKeyPathTest(new AnimaXKeyPath("*", "*", "*"), 13, "Should have 13 key paths for *,*,*");
  }

  @Test
  public void testGetKeysForStarStarStarStar() {
    doKeyPathTest(new AnimaXKeyPath("*", "*", "*", "*"), 9, "Should have 9 key paths for *,*,*,*");
  }

  @Test
  public void testGetKeysForStarStarStarStarStar() {
    doKeyPathTest(
        new AnimaXKeyPath("*", "*", "*", "*", "*"), 6, "Should have 6 key paths for *,*,*,*,*");
  }

  @Test
  public void testGetKeysForStarStarStarStarStarStar() {
    doKeyPathTest(new AnimaXKeyPath("*", "*", "*", "*", "*", "*"), 0,
        "Should have 0 key paths for *,*,*,*,*,*");
  }

  @Test
  public void testGetKeysForMatteDoubleStar() {
    doKeyPathTest(new AnimaXKeyPath("matte", "**"), 5, "Should have 5 key paths for matte,**");
  }

  @Test
  public void testGetKeysForDoubleStarDoubleStar() {
    doKeyPathTest(new AnimaXKeyPath("**", "**"), 40, "Should have 0 key paths for **,**");
  }

  @Test
  public void testGetKeysForDoubleStarFill() {
    doKeyPathTest(new AnimaXKeyPath("**", "fill"), 6, "Should have 6 key paths for **,fill");
  }

  @Test
  public void testGetKeysForDoubleStarFillDoubleStar() {
    doKeyPathTest(
        new AnimaXKeyPath("**", "fill", "**"), 6, "Should have 6 key paths for **,fill,**");
  }

  @Test
  public void testGetKeysForStarStarFillDoubleStar() {
    doKeyPathTest(
        new AnimaXKeyPath("*", "*", "fill", "**"), 2, "Should have 2 key paths for *,*,fill,**");
  }

  @Test
  public void testGetKeysForComp1Star() {
    doKeyPathTest(new AnimaXKeyPath("comp1", "*"), 3, "Should have 4 key paths for comp1,*");
  }

  @Test
  public void testGetKeysForComp1DoubleStar() {
    doKeyPathTest(new AnimaXKeyPath("comp1", "**"), 24, "Should have 24 key paths for comp1,**");
  }

  @Test
  public void testGetKeysForComp1Comp3Star() {
    doKeyPathTest(
        new AnimaXKeyPath("comp1", "comp3", "*"), 3, "Should have 3 key paths for comp1,comp3,*");
  }

  @Test
  public void testGetKeysForComp1Comp3DoubleStar() {
    doKeyPathTest(new AnimaXKeyPath("comp1", "comp3", "**"), 12,
        "Should have 12 key paths for comp1,comp3,**");
  }
}
