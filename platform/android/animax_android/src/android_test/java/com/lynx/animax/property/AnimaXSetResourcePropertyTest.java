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
import junit.framework.TestCase;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class AnimaXSetResourcePropertyTest extends TestCase {
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

  private void doSetResourcePropertyTest(final ResourcePropertyType propertyType,
      final String keyPath, final AnimaXValueParam valueParam, final boolean expectSuccess) {
    if (mIsNotCapable) {
      return;
    }

    final CountDownLatch latch = new CountDownLatch(1);
    mAnimaXView = PropertyTestUtil.createAnimaXView(new AnimationListenerAdapter() {
      @Override
      public void onReady(AnimaXParam param) {
        mAnimaXView.getPlayer().setResourceProperty(
            propertyType, keyPath, valueParam, new AnimaXPropertyCallback() {
              @Override
              public void onSuccess() {
                if (expectSuccess) {
                  latch.countDown();
                } else {
                  fail("Should not succeed for " + propertyType + " with value " + valueParam);
                }
              }

              @Override
              public void onError(@NonNull List<String> errorMessageList) {
                if (expectSuccess) {
                  fail("Failed to set " + propertyType + " for " + keyPath
                      + " with error: " + String.join(",", errorMessageList));
                } else {
                  assertTrue(
                      "Should receive error for " + propertyType + " with value " + valueParam,
                      errorMessageList != null && !errorMessageList.isEmpty());
                  latch.countDown();
                }
              }
            });
      }
    }, false);

    try {
      assertTrue(
          "Test for " + propertyType + " timed out", latch.await(sTimeout, TimeUnit.MILLISECONDS));
    } catch (InterruptedException e) {
      fail("Test interrupted: " + e.getMessage());
      Thread.currentThread().interrupt();
    }
  }

  @Test
  public void testSetInvalidImageKeyPathResource() {
    doSetResourcePropertyTest(ResourcePropertyType.IMAGE_DIR_NAME, "nonexistent_image",
        AnimaXValueParam.fromString("some_dir/"), false);
  }

  @Test
  public void testSetInvalidImagePropertyValue() {
    // Attempting to set a string value for IMAGE_WIDTH which expects an int
    doSetResourcePropertyTest(ResourcePropertyType.IMAGE_WIDTH, "image_0",
        AnimaXValueParam.fromString("not_an_int"), false);
  }
}
