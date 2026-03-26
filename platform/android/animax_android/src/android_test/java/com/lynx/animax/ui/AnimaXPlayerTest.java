// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.ui;

import static com.lynx.animax.util.UIPropertyUtil.convertStringToObjectFit;
import static com.lynx.animax.util.UIPropertyUtil.convertStringToObjectPosition;
import static junit.framework.TestCase.assertEquals;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.inOrder;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

import android.content.Context;
import androidx.test.platform.app.InstrumentationRegistry;
import com.lynx.animax.AnimaXPlayer;
import com.lynx.animax.ability.NativeAbility;
import com.lynx.animax.drawable.AnimaXSurfaceDrawable;
import com.lynx.animax.drawable.FirstFrameAwareSurfaceTexture;
import com.lynx.animax.listener.AnimaXErrorParam;
import com.lynx.animax.listener.AnimaXParam;
import com.lynx.animax.listener.AnimationListenerAdapter;
import com.lynx.animax.listener.IAnimationListener;
import com.lynx.animax.util.DeviceUtil;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.mockito.InOrder;

public class AnimaXPlayerTest {
  private static IAnimationListener mMockListener;
  private static AnimaXPlayer mPlayer;
  private boolean mIsNotCapable;

  @BeforeClass
  public static void suiteSetUp() {
    suiteSetUp(false);
  }

  public static void suiteSetUp(boolean enable) {
    Context context = InstrumentationRegistry.getInstrumentation().getTargetContext();
    AnimaXContext animaXContext = new AnimaXContext.Builder(new NativeAbility(), context)
                                      .multiThreadAccelerate(enable)
                                      .build();
    mPlayer = new AnimaXPlayer(animaXContext);

    FirstFrameAwareSurfaceTexture surfaceTexture = new FirstFrameAwareSurfaceTexture();
    surfaceTexture.detachFromGLContext();
    AnimaXSurfaceDrawable drawable =
        AnimaXSurfaceDrawable.createFromTexture(surfaceTexture, 100, 100);
    mPlayer.createAnimaXSurface(drawable);

    // prepare listener
    mMockListener = mock(AnimationListenerAdapter.class);
    mPlayer.addAnimationListener(mMockListener);
  }

  @Before
  public void setUp() {
    // data
    mIsNotCapable = !DeviceUtil.checkCapability(new NativeAbility());
  }

  @Test
  public void testBuildWithMultiThread() {
    suiteSetUp(true);
  }

  @Test
  public void testUIPropertyUtil() {
    assertEquals(convertStringToObjectFit("cover"), ObjectFit.COVER);
    assertEquals(convertStringToObjectFit("invalid"), ObjectFit.CONTAIN);

    assertEquals(convertStringToObjectPosition("left"), ObjectPosition.LEFT);
    assertEquals(convertStringToObjectPosition("right"), ObjectPosition.RIGHT);
    assertEquals(convertStringToObjectPosition("top"), ObjectPosition.TOP);
    assertEquals(convertStringToObjectPosition("bottom"), ObjectPosition.BOTTOM);
    assertEquals(convertStringToObjectPosition("invalid"), ObjectPosition.CENTER);
  }

  @Test
  public void testPlayingInvalidLottie() {
    // missing {
    String simpleLottieJson =
        "\"v\":\"5.6.9\",\"fr\":25,\"ip\":0,\"op\":200,\"w\":1000,\"h\":1000,\"nm\":\"Composition 1\",\"ddd\":0,\"assets\":[],\"layers\":[{\"ddd\":0,\"ind\":1,\"ty\":1,\"nm\":\"Intermediate Blue Solid Color 1\",\"sr\":1,\"ks\":{\"o\":{\"a\":0,\"k\":100,\"ix\":11},\"r\":{\"a\":0,\"k\":0,\"ix\":10},\"p\":{\"a\":0,\"k\":[500,500,0],\"ix\":2},\"a\":{\"a\":0,\"k\":[500,500,0],\"ix\":1},\"s\":{\"a\":0,\"k\":[100,100,100],\"ix\":6}},\"ao\":0,\"sw\":1000,\"sh\":1000,\"sc\":\"#505fec\",\"ip\":0,\"op\":200,\"st\":0,\"bm\":0}],\"markers\":[]}";

    loadAndPlayAnimation(mPlayer, simpleLottieJson, true);
    verify(mMockListener, times(1)).onError(any());
  }

  @Test
  public void testPlayingLottie() {
    String simpleLottieJson =
        "{\"v\":\"5.6.9\",\"fr\":25,\"ip\":0,\"op\":200,\"w\":1000,\"h\":1000,\"nm\":\"Composition 1\",\"ddd\":0,\"assets\":[],\"layers\":[{\"ddd\":0,\"ind\":1,\"ty\":1,\"nm\":\"Intermediate Blue Solid Color 1\",\"sr\":1,\"ks\":{\"o\":{\"a\":0,\"k\":100,\"ix\":11},\"r\":{\"a\":0,\"k\":0,\"ix\":10},\"p\":{\"a\":0,\"k\":[500,500,0],\"ix\":2},\"a\":{\"a\":0,\"k\":[500,500,0],\"ix\":1},\"s\":{\"a\":0,\"k\":[100,100,100],\"ix\":6}},\"ao\":0,\"sw\":1000,\"sh\":1000,\"sc\":\"#505fec\",\"ip\":0,\"op\":200,\"st\":0,\"bm\":0}],\"markers\":[]}";

    loadAndPlayAnimation(mPlayer, simpleLottieJson, true);

    if (mIsNotCapable) {
      verify(mMockListener, times(1)).onError(any());
    } else {
      InOrder inOrder = inOrder(mMockListener);
      inOrder.verify(mMockListener, times(1)).onReady(any());
      inOrder.verify(mMockListener, times(1)).onStart(any());
    }
  }

  @Test
  public void testPlayingLottieButWithoutSurface() {
    Context context = InstrumentationRegistry.getInstrumentation().getTargetContext();
    AnimaXContext animaXContext = new AnimaXContext.Builder(new NativeAbility(), context).build();
    AnimaXPlayer player2 = new AnimaXPlayer(animaXContext);

    // prepare drawables
    player2.createAnimaXSurface(null);

    // prepare listener
    IAnimationListener mockListener2 = mock(AnimationListenerAdapter.class);
    player2.addAnimationListener(mockListener2);

    String simpleLottieJson =
        "{\"v\":\"5.6.9\",\"fr\":25,\"ip\":0,\"op\":200,\"w\":1000,\"h\":1000,\"nm\":\"Composition 1\",\"ddd\":0,\"assets\":[],\"layers\":[{\"ddd\":0,\"ind\":1,\"ty\":1,\"nm\":\"Intermediate Blue Solid Color 1\",\"sr\":1,\"ks\":{\"o\":{\"a\":0,\"k\":100,\"ix\":11},\"r\":{\"a\":0,\"k\":0,\"ix\":10},\"p\":{\"a\":0,\"k\":[500,500,0],\"ix\":2},\"a\":{\"a\":0,\"k\":[500,500,0],\"ix\":1},\"s\":{\"a\":0,\"k\":[100,100,100],\"ix\":6}},\"ao\":0,\"sw\":1000,\"sh\":1000,\"sc\":\"#505fec\",\"ip\":0,\"op\":200,\"st\":0,\"bm\":0}],\"markers\":[]}";
    loadAndPlayAnimation(player2, simpleLottieJson, true);

    verify(mockListener2, times(0)).onReady(any());
  }

  private void loadAndPlayAnimation(AnimaXPlayer player, String lottieJSON, boolean autoplay) {
    final CountDownLatch latch = new CountDownLatch(1);
    mPlayer.addAnimationListener(new AnimationListenerAdapter() {
      @Override
      public void onReady(AnimaXParam param) {
        if (!autoplay) {
          latch.countDown();
        }
      }

      @Override
      public void onStart(AnimaXParam param) {
        if (autoplay) {
          latch.countDown();
        }
      }

      @Override
      public void onError(AnimaXErrorParam param) {
        if (mIsNotCapable) {
          latch.countDown();
        }
      }
    });

    playAnimation(lottieJSON, autoplay);

    try {
      // wait for 3 seconds, if not ready or error, means the emulator may occur device error.
      boolean completed = latch.await(3, TimeUnit.SECONDS);
      if (!completed) {
        return;
      }
    } catch (InterruptedException e) {
      e.printStackTrace();
    }
  }

  private void playAnimation(String lottieJson, boolean autoplay) {
    mPlayer.setLoop(true);
    mPlayer.setAutoPlay(autoplay);
    mPlayer.setObjectFit(ObjectFit.CENTER);

    mPlayer.setJson(lottieJson);
  }
}
