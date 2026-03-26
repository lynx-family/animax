// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.ui;

import static org.junit.Assert.assertEquals;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.view.ViewGroup;
import androidx.test.core.app.ApplicationProvider;
import org.junit.Before;
import org.junit.Test;

public class AnimaXContainerViewTest {
  private boolean mUseBitmapOnDraw = false;
  private int mBeforeCallCount = 0, mUseCallCount = 0;

  @Before
  public void setUp() {
    mUseBitmapOnDraw = false;
    mBeforeCallCount = 0;
    mUseCallCount = 0;
  }

  @Test
  public void testWithAnimaXView() {
    Context context = ApplicationProvider.getApplicationContext();
    AnimaXView animaXView = new AnimaXView(context);
    AnimaXContainerView.IDispatchDrawHook hook = new AnimaXContainerView.IDispatchDrawHook() {
      @Override
      public void beforeDispatchDraw(Canvas canvas) {
        ++mBeforeCallCount;
      }

      @Override
      public boolean useBitmapOnDraw() {
        ++mUseCallCount;
        return mUseBitmapOnDraw;
      }
    };
    AnimaXContainerView containerView = new AnimaXContainerView(animaXView.getContext());
    containerView.addChildAnimaXView(animaXView, hook);
    assertEquals(animaXView, containerView.getAnimaXView());

    ViewGroup.LayoutParams layoutParams = new ViewGroup.LayoutParams(
        ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
    layoutParams.width = layoutParams.height = 100;
    containerView.setLayoutParams(layoutParams);
    containerView.requestLayout();

    Bitmap bitmap = Bitmap.createBitmap(100, 100, Bitmap.Config.ARGB_8888);
    final Canvas canvas = new Canvas(bitmap);

    mUseBitmapOnDraw = false;
    containerView.draw(canvas);
    assertEquals(1, mBeforeCallCount);
    assertEquals(1, mUseCallCount);

    mUseBitmapOnDraw = true;
    containerView.draw(canvas);
    assertEquals(2, mBeforeCallCount);
    assertEquals(2, mUseCallCount);
  }

  @Test
  public void testWithAnimaXImageView() {
    Context context = ApplicationProvider.getApplicationContext();
    AnimaXImageView animaXView = new AnimaXImageView(context);
    AnimaXContainerView.IDispatchDrawHook hook = new AnimaXContainerView.IDispatchDrawHook() {
      @Override
      public void beforeDispatchDraw(Canvas canvas) {
        ++mBeforeCallCount;
      }

      @Override
      public boolean useBitmapOnDraw() {
        ++mUseCallCount;
        return mUseBitmapOnDraw;
      }
    };
    AnimaXContainerView containerView = new AnimaXContainerView(animaXView.getContext());
    containerView.addChildAnimaXView(animaXView, hook);
    assertEquals(animaXView, containerView.getAnimaXImageView());

    ViewGroup.LayoutParams layoutParams = new ViewGroup.LayoutParams(
        ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
    layoutParams.width = layoutParams.height = 100;
    containerView.setLayoutParams(layoutParams);
    containerView.requestLayout();

    Bitmap bitmap = Bitmap.createBitmap(100, 100, Bitmap.Config.ARGB_8888);
    final Canvas canvas = new Canvas(bitmap);

    mUseBitmapOnDraw = false;
    containerView.draw(canvas);
    assertEquals(1, mBeforeCallCount);
    assertEquals(1, mUseCallCount);

    mUseBitmapOnDraw = true;
    containerView.draw(canvas);
    assertEquals(2, mBeforeCallCount);
    assertEquals(2, mUseCallCount);
  }
}
