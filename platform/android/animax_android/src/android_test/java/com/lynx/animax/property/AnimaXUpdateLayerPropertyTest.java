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

public class AnimaXUpdateLayerPropertyTest extends TestCase {
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

  private void doUpdateLayerPropertyTest(final LayerPropertyType propertyType,
      final AnimaXKeyPath keyPath, final AnimaXValueParam valueParam, final String msg) {
    if (mIsNotCapable) {
      return;
    }
    setupReusableAnimaXView(msg);
    updatePropertyAndAwait(propertyType, keyPath, valueParam, msg);
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

  private void updatePropertyAndAwait(final LayerPropertyType propertyType,
      final AnimaXKeyPath keyPath, final AnimaXValueParam valueParam, final String msg) {
    final CountDownLatch latch = new CountDownLatch(1);
    final AtomicReference<Throwable> errorRef = new AtomicReference<>(null);
    mAnimaXView.getPlayer().updateLayerProperty(
        propertyType, keyPath, valueParam, new AnimaXPropertyCallback() {
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

  private void doUpdatePropertiesTest(
      String testName, AnimaXKeyPath keyPath, LayerPropertyType[] propertyTypes) {
    if (mIsNotCapable) {
      return;
    }
    setupReusableAnimaXView(testName + " setup");
    for (LayerPropertyType propertyType : propertyTypes) {
      AnimaXValueParam valueParam = PropertyTestUtil.createGenericValueParam(propertyType);
      updatePropertyAndAwait(
          propertyType, keyPath, valueParam, "Update property for " + propertyType.name());
    }
  }

  private void doUpdatePropertiesTestByPrefix(
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
    doUpdatePropertiesTest(testName, keyPath, propertyList.toArray(new LayerPropertyType[0]));
  }

  @Test
  public void testUpdateImageLayerProperty() {
    AnimaXKeyPath keyPath = new AnimaXKeyPath("image");
    AnimaXValueParam valueParam = AnimaXValueParam.fromNumber(50.0);
    doUpdateLayerPropertyTest(
        LayerPropertyType.TRANSFORM_OPACITY, keyPath, valueParam, "Image layer opacity update");
  }

  @Test
  public void testUpdateShapeLayerProperty() {
    AnimaXKeyPath keyPath = new AnimaXKeyPath("rect", "**", "fill");
    AnimaXValueParam valueParam = AnimaXValueParam.fromColor(0xFF00FF00);
    doUpdateLayerPropertyTest(
        LayerPropertyType.COLOR, keyPath, valueParam, "Shape layer color update");
  }

  @Test
  public void testUpdateBlurEffectProperty() {
    AnimaXKeyPath keyPath = new AnimaXKeyPath("rect", "**", "gaussianblur");
    AnimaXValueParam valueParam = AnimaXValueParam.fromNumber(30.0);
    doUpdateLayerPropertyTest(
        LayerPropertyType.BLUR_RADIUS, keyPath, valueParam, "Blur effect radius update");
  }

  @Test
  public void testUpdateMaskedLayerProperty() {
    AnimaXKeyPath keyPath = new AnimaXKeyPath("ellipse", "**", "fill");
    AnimaXValueParam valueParam = AnimaXValueParam.fromColor(0xFF0000FF);
    doUpdateLayerPropertyTest(
        LayerPropertyType.COLOR, keyPath, valueParam, "Masked layer color update");
  }

  @Test
  public void testUpdateComp1VisibilityProperty() {
    AnimaXKeyPath keyPath = new AnimaXKeyPath("comp1");
    AnimaXValueParam valueParam = AnimaXValueParam.fromNumber(0.0);
    doUpdateLayerPropertyTest(
        LayerPropertyType.VISIBILITY, keyPath, valueParam, "Comp1 visibility update to hidden");
  }

  @Test
  public void testUpdateTransformProperties() {
    doUpdatePropertiesTestByPrefix("Transform properties test", new AnimaXKeyPath("comp1"),
        "TRANSFORM_", LayerPropertyType.TRANSFORM_OPACITY);
  }

  @Test
  public void testUpdateEllipseProperties() {
    doUpdatePropertiesTestByPrefix(
        "Ellipse properties test", new AnimaXKeyPath("**", "ellipsecontent", "path"), "ELLIPSE_");
  }

  @Test
  public void testUpdatePolystarProperties() {
    doUpdatePropertiesTestByPrefix("Polystar properties test",
        new AnimaXKeyPath("**", "polystarcontent", "path"), "POLYSTAR_");
  }

  public void testUpdateRepeaterProperties() {
    doUpdatePropertiesTestByPrefix("Repeater properties test",
        new AnimaXKeyPath("**", "polystarcontent", "repeater"), "REPEATER_");
  }

  @Test
  public void testUpdateRectContentProperties() {
    LayerPropertyType[] rectPropTypes = {LayerPropertyType.POSITION,
        LayerPropertyType.CORNER_RADIUS, LayerPropertyType.RECTANGLE_SIZE};
    doUpdatePropertiesTest("Rect content properties test",
        new AnimaXKeyPath("**", "rectcontent", "path"), rectPropTypes);
  }

  @Test
  public void testUpdateStrokeProperties() {
    doUpdatePropertiesTestByPrefix(
        "Stroke properties test", new AnimaXKeyPath("**", "stroke"), "STROKE_");
  }

  @Test
  public void testUpdateOpacityProperty() {
    AnimaXKeyPath keyPath = new AnimaXKeyPath("**", "fill");
    AnimaXValueParam valueParam =
        PropertyTestUtil.createGenericValueParam(LayerPropertyType.OPACITY);
    doUpdateLayerPropertyTest(
        LayerPropertyType.OPACITY, keyPath, valueParam, "Update property for OPACITY");
  }
}
