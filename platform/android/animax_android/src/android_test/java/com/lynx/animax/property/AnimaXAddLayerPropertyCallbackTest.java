// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.property;

import androidx.annotation.NonNull;
import com.lynx.animax.listener.AnimaXErrorParam;
import com.lynx.animax.listener.AnimaXParam;
import com.lynx.animax.listener.AnimationListenerAdapter;
import com.lynx.animax.ui.AnimaXView;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicReference;
import junit.framework.TestCase;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class AnimaXAddLayerPropertyCallbackTest extends TestCase {
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

  private void doAddLayerPropertyCallbackTest(final LayerPropertyType propertyType,
      final AnimaXKeyPath keyPath, final AnimaXValueCallback valueCallback, final String msg) {
    if (mIsNotCapable) {
      return;
    }
    setupReusableAnimaXView(msg);
    addCallbackAndAwait(propertyType, keyPath, valueCallback, msg);
  }

  private void setupReusableAnimaXView(final String msg) {
    if (mAnimaXView != null) {
      return;
    }
    final CountDownLatch latch = new CountDownLatch(1);
    final AtomicReference<Throwable> errorRef = new AtomicReference<>(null);
    mAnimaXView = PropertyTestUtil.createAnimaXView(new AnimationListenerAdapter() {
      @Override
      public void onReady(AnimaXParam param) {
        latch.countDown();
      }

      @Override
      public void onError(AnimaXErrorParam param) {
        errorRef.set(new AssertionError(msg + " failed with onError: " + param.getErrorMessage()));
        latch.countDown();
      }
    }, true);

    try {
      assertTrue(msg + " timed out", latch.await(sTimeout, TimeUnit.MILLISECONDS));
      if (errorRef.get() != null) {
        throw errorRef.get();
      }
    } catch (Throwable e) {
      fail(msg + " failed with exception: " + e.getMessage());
      if (e instanceof InterruptedException) {
        Thread.currentThread().interrupt();
      }
    }
  }

  private void addCallbackAndAwait(final LayerPropertyType propertyType,
      final AnimaXKeyPath keyPath, final AnimaXValueCallback valueCallback, final String msg) {
    final CountDownLatch latch = new CountDownLatch(1);
    final AtomicReference<Throwable> errorRef = new AtomicReference<>(null);
    mAnimaXView.getPlayer().addLayerPropertyCallback(
        propertyType, keyPath, valueCallback, new AnimaXPropertyCallback() {
          @Override
          public void onSuccess() {
            latch.countDown();
          }

          @Override
          public void onError(@NonNull List<String> errorMessageList) {
            errorRef.set(
                new AssertionError(msg + " failed: " + String.join(",", errorMessageList)));
            latch.countDown();
          }
        });
    try {
      assertTrue(msg + " timed out", latch.await(sTimeout, TimeUnit.MILLISECONDS));
      if (errorRef.get() != null) {
        throw errorRef.get();
      }
    } catch (Throwable e) {
      fail(msg + " failed with exception: " + e.getMessage());
      if (e instanceof InterruptedException) {
        Thread.currentThread().interrupt();
      }
    }
  }

  private void doAddPropertyCallbacksTest(
      String testName, AnimaXKeyPath keyPath, LayerPropertyType[] propertyTypes) {
    if (mIsNotCapable) {
      return;
    }
    setupReusableAnimaXView(testName + " setup");
    for (LayerPropertyType propertyType : propertyTypes) {
      AnimaXValueCallback valueCallback = PropertyTestUtil.createGenericValueCallback(propertyType);
      addCallbackAndAwait(
          propertyType, keyPath, valueCallback, "Add property callback for " + propertyType.name());
    }
  }

  private void doAddPropertyCallbacksTestByPrefix(
      String testName, AnimaXKeyPath keyPath, String prefix, LayerPropertyType... excludes) {
    if (mIsNotCapable) {
      return;
    }
    List<LayerPropertyType> propertyList = new ArrayList<>();
    List<LayerPropertyType> excludeList = Arrays.asList(excludes);
    for (LayerPropertyType p : LayerPropertyType.values()) {
      if (p.name().startsWith(prefix) && !excludeList.contains(p)) {
        propertyList.add(p);
      }
    }
    doAddPropertyCallbacksTest(testName, keyPath, propertyList.toArray(new LayerPropertyType[0]));
  }

  @Test
  public void testAddTransformPropertyCallbacks() {
    doAddPropertyCallbacksTestByPrefix(
        "Transform properties callback test", new AnimaXKeyPath("comp1"), "TRANSFORM_");
  }

  @Test
  public void testAddEllipsePropertyCallbacks() {
    doAddPropertyCallbacksTestByPrefix("Ellipse properties callback test",
        new AnimaXKeyPath("**", "ellipsecontent", "path"), "ELLIPSE_");
  }

  @Test
  public void testAddColorPropertyCallback() {
    AnimaXKeyPath keyPath = new AnimaXKeyPath("**", "fill");
    AnimaXValueCallback valueCallback =
        PropertyTestUtil.createGenericValueCallback(LayerPropertyType.COLOR);
    doAddLayerPropertyCallbackTest(
        LayerPropertyType.COLOR, keyPath, valueCallback, "Add property callback for COLOR");
  }

  @Test
  public void testAddPolystarPropertyCallbacks() {
    doAddPropertyCallbacksTestByPrefix("Polystar properties callback test",
        new AnimaXKeyPath("**", "polystarcontent", "path"), "POLYSTAR_");
  }

  @Test
  public void testAddRepeaterPropertyCallbacks() {
    doAddPropertyCallbacksTestByPrefix("Repeater properties callback test",
        new AnimaXKeyPath("**", "polystarcontent", "repeater"), "REPEATER_");
  }

  @Test
  public void testAddRectContentPropertyCallbacks() {
    LayerPropertyType[] rectPropTypes = {LayerPropertyType.POSITION,
        LayerPropertyType.CORNER_RADIUS, LayerPropertyType.RECTANGLE_SIZE};
    doAddPropertyCallbacksTest("Rect content properties callback test",
        new AnimaXKeyPath("**", "rectcontent", "path"), rectPropTypes);
  }

  @Test
  public void testAddStrokePropertyCallbacks() {
    doAddPropertyCallbacksTestByPrefix(
        "Stroke properties callback test", new AnimaXKeyPath("**", "stroke"), "STROKE_");
  }

  @Test
  public void testAddColorFilterPropertyCallback() {
    AnimaXKeyPath keyPath = new AnimaXKeyPath("**", "fill");
    AnimaXValueCallback valueCallback =
        PropertyTestUtil.createGenericValueCallback(LayerPropertyType.COLOR_FILTER);
    doAddLayerPropertyCallbackTest(LayerPropertyType.COLOR_FILTER, keyPath, valueCallback,
        "Add property callback for COLOR_FILTER");
  }

  @Test
  public void testAddOpacityPropertyCallback() {
    AnimaXKeyPath keyPath = new AnimaXKeyPath("**", "fill");
    AnimaXValueCallback valueCallback =
        PropertyTestUtil.createGenericValueCallback(LayerPropertyType.OPACITY);
    doAddLayerPropertyCallbackTest(
        LayerPropertyType.OPACITY, keyPath, valueCallback, "Add property callback for OPACITY");
  }
}
