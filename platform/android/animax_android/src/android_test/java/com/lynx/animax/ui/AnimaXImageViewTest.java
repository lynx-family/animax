// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.ui;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.view.MotionEvent;
import androidx.annotation.NonNull;
import androidx.test.platform.app.InstrumentationRegistry;
import com.lynx.animax.ability.NativeAbility;
import com.lynx.animax.util.DeviceUtil;
import com.lynx.animax.util.UIThreadUtils;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import org.junit.Before;
import org.junit.Test;

public class AnimaXImageViewTest {
  private static class TestCanvas extends Canvas {
    @Override
    public int save() {
      return 0;
    }

    @Override
    public void drawBitmap(@NonNull Bitmap bitmap, float left, float top, Paint paint) {}

    @Override
    public void restoreToCount(int saveCount) {}
  }
  private static class TestAnimaXImageView extends AnimaXImageView {
    public TestAnimaXImageView(Context context) {
      super(context);
    }

    public int touchCount = 0;
    public int sizeChangeCount = 0;

    @Override
    public void onDraw(Canvas canvas) {
      super.onDraw(canvas);
    }

    @Override
    public void onAttachedToWindow() {
      super.onAttachedToWindow();
    }

    @Override
    public void onDetachedFromWindow() {
      super.onDetachedFromWindow();
    }

    @Override
    public void handleTouchEvent(MotionEvent e) {
      super.handleTouchEvent(e);
      touchCount++;
    }

    @Override
    protected void onSizeChanged(int w, int h, int oldW, int oldH) {
      super.onSizeChanged(w, h, oldW, oldH);
      sizeChangeCount++;
    }
  }

  private String easyLottieJson;
  private boolean isNotCapable;

  @Before
  public void setUp() {
    easyLottieJson =
        "{\"v\":\"5.6.9\",\"fr\":25,\"ip\":0,\"op\":200,\"w\":1000,\"h\":1000,\"nm\":\"Composition 1\",\"ddd\":0,\"assets\":[],\"layers\":[{\"ddd\":0,\"ind\":1,\"ty\":1,\"nm\":\"Intermediate Blue Solid Color 1\",\"sr\":1,\"ks\":{\"o\":{\"a\":0,\"k\":100,\"ix\":11},\"r\":{\"a\":0,\"k\":0,\"ix\":10},\"p\":{\"a\":0,\"k\":[500,500,0],\"ix\":2},\"a\":{\"a\":0,\"k\":[500,500,0],\"ix\":1},\"s\":{\"a\":0,\"k\":[100,100,100],\"ix\":6}},\"ao\":0,\"sw\":1000,\"sh\":1000,\"sc\":\"#505fec\",\"ip\":0,\"op\":200,\"st\":0,\"bm\":0}],\"markers\":[]}";
    isNotCapable = !DeviceUtil.checkCapability(new NativeAbility());
  }

  @Test
  public void testConstructionAndRelease() {
    if (isNotCapable) {
      return;
    }
    CountDownLatch latch = new CountDownLatch(1);

    UIThreadUtils.runOnUiThread(() -> {
      Context context = InstrumentationRegistry.getInstrumentation().getTargetContext();
      TestAnimaXImageView view = new TestAnimaXImageView(context);
      view.layout(0, 0, 100, 100);
      view.setJson(easyLottieJson);
      UIThreadUtils.getUiThreadHandler().postDelayed(() -> {
        view.onDraw(new TestCanvas());
        Bitmap bitmap = view.getCurrentBitmap();
        assertNotNull(bitmap);
        view.onDraw(new TestCanvas());
        assertEquals(bitmap, view.getCurrentBitmap());
        view.release();
        assertNull(view.getCurrentBitmap());
        latch.countDown();
      }, 1000);
    });
    try {
      assertTrue("Timeout waiting for UI assertions", latch.await(2, TimeUnit.SECONDS));
    } catch (InterruptedException e) {
      e.printStackTrace();
    }
  }

  @Test
  public void testEnableTouchEvent() {
    if (isNotCapable) {
      return;
    }
    UIThreadUtils.runOnUiThread(() -> {
      Context context = InstrumentationRegistry.getInstrumentation().getTargetContext();
      TestAnimaXImageView view = new TestAnimaXImageView(context);
      view.setEnableTapLayerEvent(true);
      view.onTouchEvent(null);
      assertEquals(1, view.touchCount);
    });
  }

  @Test
  public void testVisibilityChange() {
    if (isNotCapable) {
      return;
    }
    UIThreadUtils.runOnUiThread(() -> {
      Context context = InstrumentationRegistry.getInstrumentation().getTargetContext();
      TestAnimaXImageView view = new TestAnimaXImageView(context);
      view.setIgnoreAttachStatus(true);
      view.onCompositionReady();
      assertEquals(1, view.sizeChangeCount);
      view.setIgnoreAttachStatus(false);
      view.onDetachedFromWindow();
      view.onAttachedToWindow();
      view.setAlpha(0.7F);
      view.onVisibilityAggregated(false);
      assertEquals(0.7F, view.getAlpha(), 1e-5);
    });
  }
}
