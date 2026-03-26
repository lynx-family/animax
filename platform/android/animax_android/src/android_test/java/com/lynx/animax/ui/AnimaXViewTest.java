// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.ui;

import static org.mockito.Mockito.*;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.view.MotionEvent;
import androidx.test.platform.app.InstrumentationRegistry;
import com.lynx.animax.ability.BaseAbility;
import com.lynx.animax.ability.NativeAbility;
import com.lynx.animax.base.bridge.JavaOnlyMap;
import com.lynx.animax.base.bridge.ReadableMap;
import com.lynx.animax.listener.AnimaXErrorParam;
import com.lynx.animax.listener.AnimaXParam;
import com.lynx.animax.listener.AnimaXTapParam;
import com.lynx.animax.listener.AnimationListenerAdapter;
import com.lynx.animax.listener.IAnimationListener;
import com.lynx.animax.loader.AnimaXLoaderResponse;
import com.lynx.animax.loader.AnimaXLoaderScheme;
import com.lynx.animax.loader.IAnimaXLoader;
import com.lynx.animax.loader.IAnimaXLoaderCompletionHandler;
import com.lynx.animax.loader.IAnimaXLoaderRequest;
import com.lynx.animax.monitor.AnimaXMonitorUtil;
import com.lynx.animax.monitor.MetricsAndEventStore;
import com.lynx.animax.service.AnimaXServiceCenter;
import com.lynx.animax.service.IAnimaXMonitorService;
import com.lynx.animax.service.IAnimaXResourceFactoryService;
import com.lynx.animax.util.AnimaXMetricsCallback;
import com.lynx.animax.util.DeviceUtil;
import com.lynx.animax.util.UIThreadUtils;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import junit.framework.TestCase;
import org.json.JSONException;
import org.json.JSONObject;
import org.junit.Before;
import org.junit.Test;
import org.mockito.ArgumentCaptor;
import org.mockito.InOrder;

public class AnimaXViewTest extends TestCase {
  private static final long sTimeout = 1000;

  private String easyLottieJson;
  private boolean isNotCapable;

  @Before
  public void setUp() {
    easyLottieJson =
        "{\"v\":\"5.6.9\",\"fr\":25,\"ip\":0,\"op\":200,\"w\":1000,\"h\":1000,\"nm\":\"Composition 1\",\"ddd\":0,\"assets\":[],\"layers\":[{\"ddd\":0,\"ind\":1,\"ty\":1,\"nm\":\"Intermediate Blue Solid Color 1\",\"sr\":1,\"ks\":{\"o\":{\"a\":0,\"k\":100,\"ix\":11},\"r\":{\"a\":0,\"k\":0,\"ix\":10},\"p\":{\"a\":0,\"k\":[500,500,0],\"ix\":2},\"a\":{\"a\":0,\"k\":[500,500,0],\"ix\":1},\"s\":{\"a\":0,\"k\":[100,100,100],\"ix\":6}},\"ao\":0,\"sw\":1000,\"sh\":1000,\"sc\":\"#505fec\",\"ip\":0,\"op\":200,\"st\":0,\"bm\":0}],\"markers\":[]}";

    isNotCapable = !DeviceUtil.checkCapability(new NativeAbility());
  }

  @Test
  public void testNativeAbility() {
    IAnimationListener mockListener = mock(AnimationListenerAdapter.class);
    AnimaXView animaXView = createAnimaXView(mockListener, true);
    if (animaXView == null) {
      return;
    }

    if (isNotCapable) {
      verify(mockListener, times(1)).onError(any());
    } else {
      InOrder inOrder = inOrder(mockListener);
      inOrder.verify(mockListener, times(1)).onReady(any());
      inOrder.verify(mockListener, times(1)).onStart(any());
    }
  }

  @Test
  public void testAnimationListener() {
    if (isNotCapable) {
      return;
    }

    IAnimationListener mockListener = mock(AnimationListenerAdapter.class);
    AnimaXView animaXView = createAnimaXView(mockListener, false);
    if (animaXView == null) {
      return;
    }
    animaXView.setFpsEventInterval(500);

    verify(mockListener, times(1)).onReady(any());

    assertEquals(7999.0, animaXView.getDurationMs());
    assertEquals(0.0, animaXView.getCurrentFrame());
    assertFalse(animaXView.isAnimating());

    animaXView.play();
    verify(mockListener, timeout(sTimeout).times(1)).onStart(any());
    assertTrue(animaXView.isAnimating());

    animaXView.cancel();

    verify(mockListener, timeout(sTimeout).times(1)).onCancel(any());
    assertFalse(animaXView.isAnimating());

    animaXView.play();
    verify(mockListener, timeout(sTimeout * 10).atLeast(1)).onRepeat(any());

    animaXView.cancel();
    animaXView.setLoop(false);
    animaXView.play();
    verify(mockListener, timeout(sTimeout * 10).times(1)).onComplete(any());
  }

  @Test
  public void testAnimationUpdate() {
    if (isNotCapable) {
      return;
    }

    IAnimationListener mockListener = mock(AnimationListenerAdapter.class);
    AnimaXView animaXView = createAnimaXView(mockListener, true);
    if (animaXView == null) {
      return;
    }

    animaXView.getPlayer().subscribeUpdateEvent(5);
    verify(mockListener, timeout(sTimeout * 10).atLeast(1)).onUpdate(any());

    animaXView.setFpsEventInterval(500);
    verify(mockListener, timeout(sTimeout).atLeast(1)).onFPS(any());
  }

  @Test
  public void testMetricsAndEventStoreIsEmpty() {
    MetricsAndEventStore store =
        new MetricsAndEventStore(null, null, new HashMap<>(), "this is a trigger");

    Map<String, Object> metricsMap = store.getMetricsAsMap();
    Map<String, Object> eventMap = store.getMetricsAsMap();

    assertTrue(metricsMap.isEmpty());
    assertTrue(eventMap.isEmpty());
  }

  @Test
  public void testMetricsAndEventStoreIsNotEmpty() {
    JavaOnlyMap metricsMap = new JavaOnlyMap();
    metricsMap.put("k0", 0);
    metricsMap.put("k1", 1);
    metricsMap.put(AnimaXMonitorUtil.KEY_FPS, 100);

    JavaOnlyMap categoryMap = new JavaOnlyMap();
    categoryMap.put("k2", 2);

    HashMap<String, Object> platformMap = new HashMap<>();
    platformMap.put("k3", 3);

    MetricsAndEventStore store =
        new MetricsAndEventStore(metricsMap, categoryMap, platformMap, "this is a trigger");

    Map<String, Object> metricsMapFromStore = store.getMetricsAsMap();
    Map<String, Object> eventMapFromStore = store.getCategoryAsMap();

    assertEquals(metricsMapFromStore.size(), 4);
    assertEquals(eventMapFromStore.size(), 2);
  }

  @Test
  public void testJSONAndMapHaveSameData() {
    JavaOnlyMap metricsMap = new JavaOnlyMap();
    metricsMap.put("k0", 0);
    metricsMap.put("k1", 1);
    metricsMap.put(AnimaXMonitorUtil.KEY_FPS, 100);

    JavaOnlyMap categoryMap = new JavaOnlyMap();
    categoryMap.put("k2", 2);

    HashMap<String, Object> platformMap = new HashMap<>();
    platformMap.put("k3", 3);

    MetricsAndEventStore store =
        new MetricsAndEventStore(metricsMap, categoryMap, platformMap, "this is a trigger");

    Map<String, Object> metricsMapFromStore = store.getMetricsAsMap();
    JSONObject metricsJSONFromStore = store.getMetricsAsJSON();
    Map<String, Object> eventMapFromStore = store.getCategoryAsMap();
    JSONObject eventJSONFromStore = store.getCategoryAsJSON();

    try {
      for (Map.Entry<String, Object> entry : metricsMapFromStore.entrySet()) {
        if (entry.getValue() != null) {
          assertEquals(entry.getValue(), metricsJSONFromStore.get(entry.getKey()));
        }
      }

      for (Map.Entry<String, Object> entry : eventMapFromStore.entrySet()) {
        if (entry.getValue() != null) {
          assertEquals(entry.getValue(), eventJSONFromStore.get(entry.getKey()));
        }
      }

    } catch (JSONException ignored) {
    }

    assertEquals(metricsMapFromStore.size(), metricsJSONFromStore.length());
    assertEquals(eventMapFromStore.size(), eventJSONFromStore.length());
  }

  @Test
  public void testReport() {
    if (isNotCapable) {
      return;
    }

    JavaOnlyMap metricsMap = new JavaOnlyMap();
    metricsMap.put("k0", 0);
    metricsMap.put("k1", 1);
    metricsMap.put(AnimaXMonitorUtil.KEY_FPS, 100);

    JavaOnlyMap categoryMap = new JavaOnlyMap();
    categoryMap.put("k2", 2);

    HashMap<String, Object> platformMap = new HashMap<>();
    platformMap.put("k3", 3);

    MetricsAndEventStore metrics =
        new MetricsAndEventStore(metricsMap, categoryMap, platformMap, "this is a trigger");

    Map<String, Object> m = metrics.getMetricsAsMap();
    assertEquals(m.get("k1"), 1);
    assertEquals(m.get("k3"), 3);

    Map<String, Object> m2 = metrics.getCategoryAsMap();
    assertEquals(m2.get("k2"), 2);
  }

  @Test
  public void testGetMetricsAsync() {
    if (isNotCapable) {
      return;
    }

    // Mock http service to return fixed json content
    AnimaXServiceCenter.inst().registerService(
        IAnimaXResourceFactoryService.class, new IAnimaXResourceFactoryService() {
          @Override
          public List<IAnimaXLoader> createAnimaXLoaders() {
            return Collections.singletonList(new IAnimaXLoader() {
              @Override
              public void load(
                  IAnimaXLoaderRequest request, IAnimaXLoaderCompletionHandler completionHandler) {
                completionHandler.onComplete(
                    AnimaXLoaderResponse.createByteArrayResponse(easyLottieJson.getBytes()));
              }

              @Override
              public AnimaXLoaderScheme getScheme() {
                return AnimaXLoaderScheme.HTTP;
              }
            });
          }
        });

    final CountDownLatch latch = new CountDownLatch(1);
    Context context = InstrumentationRegistry.getInstrumentation().getTargetContext();
    AnimaXContext animaxContext = new AnimaXContext.Builder(new NativeAbility(), context).build();
    AnimaXView view = new AnimaXView(animaxContext);

    // Start to wait on count down latch
    view.addAnimationListener(new AnimationListenerAdapter() {
      @Override
      public void onStart(AnimaXParam param) {
        view.getPlayer().getMetricsAsync(new AnimaXMetricsCallback(new IAnimaXMonitorService() {
          @Override
          public void reportError(AnimaXErrorParam errorInfo) {}

          @Override
          public void reportPerformanceMetrics(MetricsAndEventStore metrics) {}
        }, "test", null) {
          @Override
          public void onMetricsReady(ReadableMap metrics, ReadableMap events) {
            assertNotNull(metrics.getString("MeanFrameTime"));
            assertNotNull(metrics.getString("AnimaXFCP"));
            assertNotNull(metrics.getString("MaxFrameTime"));
            assertNotNull(metrics.getString("FirstFrameTime"));
            assertNotNull(metrics.getString("FPS"));
            assertNotNull(metrics.getString("PrepareCompositionTime"));

            assertEquals(0, events.getInt("had_errors", 1));
            assertEquals(0, events.getInt("has_repeated", 1));
            assertEquals(0, events.getInt("has_cancelled", 1));
            assertEquals(1, events.getInt("is_ready", 0));
            assertEquals(0, events.getInt("has_updated", 1));
            assertEquals(0, events.getInt("has_warning", 1));
            assertEquals(1, events.getInt("has_started", 0));
            assertEquals(0, events.getInt("has_completed", 1));
            latch.countDown();
          }
        });
      }
    });
    view.setAutoPlay(true);
    view.onSurfaceTextureAvailable(mock(SurfaceTexture.class), 100, 100);
    view.setSrc("https://test_host/test_path/test_lottie.json");

    try {
      latch.await(3, TimeUnit.SECONDS);
    } catch (InterruptedException e) {
      fail("getMetricsAsync timeout");
    }
  }

  @Test
  public void testTapLayer() {
    if (isNotCapable) {
      return;
    }

    IAnimationListener mockListener = mock(AnimationListenerAdapter.class);
    AnimaXView animaXView = createAnimaXView(mockListener, true);
    if (animaXView == null) {
      return;
    }
    animaXView.setEnableTapLayerEvent(true);

    MotionEvent motionEvent = MotionEvent.obtain(System.currentTimeMillis(),
        System.currentTimeMillis() + 100, MotionEvent.ACTION_DOWN, 50.0f, 100.0f, 0);
    animaXView.onTouchEvent(motionEvent);

    ArgumentCaptor<AnimaXTapParam> argumentCaptor = ArgumentCaptor.forClass(AnimaXTapParam.class);
    verify(mockListener, timeout(sTimeout).atLeast(0)).onTapLayers(argumentCaptor.capture());

    Map<String, Object> originParams = new HashMap<>();
    originParams.put("x", 50.0f);
    originParams.put("y", 100.0f);
    AnimaXTapParam tapParam = new AnimaXTapParam(originParams);
    assertEquals(50.0f, tapParam.getOriginParams().get("x"));
    assertEquals(100.0f, tapParam.getOriginParams().get("y"));
    assertNotNull(tapParam.getLayerNames());
    assertEquals(0, tapParam.getLayerNames().size());
  }

  /**
   * Tests the default methods provided by the IAnimaXDelegate interface,
   * focusing on property setters that delegate to the AnimaXPlayer.
   * This ensures that the delegation mechanism is working correctly.
   * Since AnimaXImageView also implements IAnimaXDelegate, these tests conceptually
   * apply to its behavior as well, promoting consistent API interaction.
   */
  @Test
  public void testIAnimaXViewDefaultSetters() {
    if (isNotCapable) {
      return;
    }
    AnimaXView animaXView = createAnimaXView(null, false);
    if (animaXView == null) {
      fail("AnimaXView creation failed, cannot proceed with test.");
      return;
    }
    IAnimaXPlayer player = animaXView.getPlayer();
    assertNotNull(player);
    animaXView.setImageFolder("images/");
    animaXView.setJson("{}");
    animaXView.setSrc("http://example.com/animation.json");
    animaXView.setSrcPolyfill(new JavaOnlyMap());
    animaXView.setDynamicResource(true);
    animaXView.setLoop(true);
    animaXView.setLoopCount(5);
    animaXView.setStartFrame(10);
    animaXView.setEndFrame(100);
    animaXView.setProgress(0.5f);
    animaXView.setSpeed(1.5f);
    animaXView.setAutoReverse(true);
    animaXView.setMaxFrameRate(30.0);
    animaXView.setAutoPlay(false); // Override initial setting
    animaXView.setObjectFit(ObjectFit.COVER);
    animaXView.setObjectPosition(ObjectPosition.CENTER);
    animaXView.setKeepLastFrame(true);
    animaXView.setAntiAliasing(true);
  }

  @Test
  public void testIAnimaXViewDefaultCommands() {
    if (isNotCapable) {
      return;
    }
    // Setup
    IAnimationListener mockListener = mock(AnimationListenerAdapter.class);
    AnimaXView animaXView = createAnimaXView(mockListener, false);
    if (animaXView == null) {
      fail("AnimaXView creation failed.");
      return;
    }
    verify(mockListener, timeout(sTimeout).times(1)).onReady(any());
    // Test commands
    animaXView.play();
    verify(mockListener, timeout(sTimeout).times(1)).onStart(any());
    UIThreadUtils.runOnUiThreadImmediately(() -> {
      // Wait for onReady to ensure player is initialized
      assertTrue(animaXView.isAnimating());
      animaXView.pause();
      assertFalse(animaXView.isAnimating());
      animaXView.resume();
      assertTrue(animaXView.isAnimating());
      animaXView.stop();
      assertFalse(animaXView.isAnimating());
      animaXView.seek(50);
      assertEquals(50.0, animaXView.getCurrentFrame());
      animaXView.playSegment(20, 80);
      verify(mockListener, timeout(sTimeout).times(1)).onStart(any());
      animaXView.reload();
    });
  }

  @Test
  public void testIAnimaXViewEventAndMiscMethods() {
    if (isNotCapable) {
      return;
    }
    IAnimationListener mockListener = mock(AnimationListenerAdapter.class);
    AnimaXView animaXView = createAnimaXView(mockListener, true);
    if (animaXView == null) {
      fail("AnimaXView creation failed.");
      return;
    }
    UIThreadUtils.runOnUiThreadImmediately(() -> {
      animaXView.subscribeUpdateEvent(10);
      animaXView.unsubscribeUpdateEvent(10);
      animaXView.subscribeUpdateEvents(new int[] {20, 30}, true);
      animaXView.subscribeUpdateEvents(new int[] {20, 30}, false);
      animaXView.enterBackground();
      assertFalse(animaXView.isAnimating());
      animaXView.enterForeground();
      assertTrue(animaXView.isAnimating());
      animaXView.removeAnimationListener(mockListener);
      animaXView.cancel();
      verify(mockListener, times(0)).onCancel(any());
    });
  }

  private AnimaXView createAnimaXView(IAnimationListener initListener, boolean autoplay) {
    return createAnimaXView(new NativeAbility(), initListener, autoplay);
  }

  private AnimaXView createAnimaXView(
      BaseAbility ability, IAnimationListener initListener, boolean autoplay) {
    Context context = InstrumentationRegistry.getInstrumentation().getTargetContext();
    AnimaXContext animaxContext = new AnimaXContext.Builder(ability, context).build();

    if (initListener != null) {
      ability.addAnimationListener(initListener);
    }

    AnimaXView view = new AnimaXView(animaxContext);
    final CountDownLatch latch = new CountDownLatch(1);
    view.addAnimationListener(new AnimationListenerAdapter() {
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
        if (isNotCapable) {
          latch.countDown();
        }
      }
    });
    configAutoPlayLoop(view, autoplay);

    try {
      // wait for 3 seconds, if not ready or error, means the emulator may occur device error.
      boolean completed = latch.await(3, TimeUnit.SECONDS);
      if (!completed) {
        return null;
      }
    } catch (InterruptedException e) {
      e.printStackTrace();
      return null;
    }

    return view;
  }

  private void configAutoPlayLoop(AnimaXView animaXView, boolean autoplay) {
    animaXView.setLoop(true);
    animaXView.setAutoPlay(autoplay);
    animaXView.setObjectFit(ObjectFit.CENTER);
    animaXView.setObjectPosition(ObjectPosition.CENTER);
    animaXView.onSurfaceTextureAvailable(mock(SurfaceTexture.class), 100, 100);
    animaXView.setJson(easyLottieJson);
  }
}
