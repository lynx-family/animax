// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax;

import android.content.Context;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.RestrictTo;
import com.lynx.animax.ability.BaseAbility;
import com.lynx.animax.ability.Event;
import com.lynx.animax.base.AnimaXError;
import com.lynx.animax.base.CleanupReference;
import com.lynx.animax.base.VisibilityState;
import com.lynx.animax.base.bridge.JavaOnlyMap;
import com.lynx.animax.base.bridge.ReadableMap;
import com.lynx.animax.composition.AnimaXComposition;
import com.lynx.animax.drawable.AnimaXSurfaceDrawable;
import com.lynx.animax.listener.IAnimationListener;
import com.lynx.animax.loader.AnimaXResourceLoaderHolder;
import com.lynx.animax.property.AnimaXKeyPath;
import com.lynx.animax.property.AnimaXKeyPathListCallback;
import com.lynx.animax.property.AnimaXPropertyCallback;
import com.lynx.animax.property.AnimaXValueCallback;
import com.lynx.animax.property.AnimaXValueParam;
import com.lynx.animax.property.LayerPropertyType;
import com.lynx.animax.property.ResourcePropertyType;
import com.lynx.animax.service.IAnimaXResourceFactoryService;
import com.lynx.animax.ui.AnimaXContext;
import com.lynx.animax.ui.IAnimaXPlayer;
import com.lynx.animax.ui.ObjectFit;
import com.lynx.animax.ui.ObjectPosition;
import com.lynx.animax.util.AnimaX;
import com.lynx.animax.util.AnimaXLog;
import com.lynx.animax.util.AnimaXMetricsCallback;
import com.lynx.animax.util.DeviceUtil;

public class AnimaXPlayer implements IAnimaXPlayer {
  private static final String TAG = "AnimaXPlayer";

  private volatile long mPtr;

  @NonNull private final BaseAbility mAbility;
  @NonNull private final AnimaXContext mAnimaXContext;

  private AnimaXResourceLoaderHolder mResourceLoaderHolder;

  private boolean mAutoPlay = false;
  private boolean mHasDestroyed = false;
  private boolean mEnableAntiAliasing = false;
  private boolean mEnableSoftwareRender = false;

  private boolean mIsPlatformSurfaceInitiallyInvalid = false;

  private CleanupReference mCleanUpReference = null;
  private @Nullable AnimaXComposition mAnimaXComposition;

  // mVisibilityFlag defaults to 0, where 0 means visible and 1 means invisible.
  private int mVisibilityFlag = 0;

  private static class CleanupOnUiThread implements Runnable {
    private long mNativePtr;

    public CleanupOnUiThread(long nativePtr) {
      mNativePtr = nativePtr;
    }

    @Override
    public void run() {
      if (mNativePtr == 0) {
        return;
      }
      AnimaXLog.i(TAG, "Call nativeDestroy.");
      nativeDestroy(mNativePtr);
      mNativePtr = 0;
    }
  }

  @RestrictTo(RestrictTo.Scope.LIBRARY)
  public AnimaXPlayer(@NonNull AnimaXContext animaXContext) {
    AnimaXLog.i(TAG, "create AnimaXPlayer: " + this);
    mAnimaXContext = animaXContext;
    mAbility = animaXContext.getAbility();
    mAbility.initAnimaXPlayer(this);
    init();
  }

  // Initialization, Destruction, and Lifecycle

  private boolean init() {
    // Make sure Ability is valid.
    if (!DeviceUtil.checkCapability(mAbility)) {
      mAbility.sendEventOnUI(Event.ERROR.ordinal(), AnimaXError.createBlockErrorParam());
      return false;
    }

    // Make sure Context is valid.
    Context context = mAnimaXContext.getContext();
    if (context == null) {
      AnimaXLog.e(TAG, "init fail, context is null");
      return false;
    }

    // Checks if the global instance of AnimaX is initialized; initializes it if it's not.
    if (!AnimaX.inst().hasInitialized()) {
      AnimaX.inst().init();
    }

    // Check if the instance again, if not, abort the player's initialization.
    if (!AnimaX.inst().hasInitialized()) {
      AnimaXLog.e(TAG, "AnimaX.inst().init() fail again.");
      mAbility.sendEventOnUI(Event.ERROR.ordinal(), AnimaXError.createLibraryInitErrorParam());
      return false;
    }

    // Set the application's context into AnimaX instance if is unset.
    AnimaX.inst().setAppContextIfUnset(context.getApplicationContext());

    IAnimaXResourceFactoryService resourceFactory =
        mAbility.getService(IAnimaXResourceFactoryService.class);
    mResourceLoaderHolder = new AnimaXResourceLoaderHolder(resourceFactory);
    mPtr = nativeCreate(mResourceLoaderHolder.getNativePtr(), mAnimaXContext);
    mCleanUpReference = new CleanupReference(this, new CleanupOnUiThread(mPtr), true);

    if (!hasInitialized()) {
      AnimaXLog.e(TAG, "init fail");
      return false;
    }

    setUpInitialProperties();

    return true;
  }

  @RestrictTo(RestrictTo.Scope.LIBRARY)
  @Override
  public AnimaXContext getAnimaXContext() {
    return mAnimaXContext;
  }

  private void setUpInitialProperties() {
    Context context = mAnimaXContext.getContext();
    if (context == null) {
      return;
    }

    // Devices running Android versions lower than 8.0
    // or 32-bit devices will have their frame rate limited to 30 fps.
    if (DeviceUtil.needLimitFrameRate()) {
      AnimaXLog.i(TAG, "Limit frame rate to 30.");
      this.setMaxFrameRate(30);
    }

    if (DeviceUtil.useSoftwareRender(context, mAbility)) {
      AnimaXLog.i(TAG, "useSoftwareRender");
      this.enableSoftwareRender(true);
    }
  }

  private void updateSurfaceProperties(AnimaXSurfaceDrawable surfaceDrawable) {
    if (surfaceDrawable != null) {
      surfaceDrawable.setEnableAntiAliasing(mEnableAntiAliasing);
      surfaceDrawable.setEnableSoftwareRender(mEnableSoftwareRender);
      surfaceDrawable.setEnableAutoDestroyEGLContext(DeviceUtil.needAutoDestroyEGLContext());
      surfaceDrawable.setPlatformSurfaceInitiallyInvalid(mIsPlatformSurfaceInitiallyInvalid);
    }
  }

  @Override
  public void enableSoftwareRender(boolean softwareRender) {
    mEnableSoftwareRender = softwareRender;
  }

  private boolean hasInitialized() {
    return mPtr != 0;
  }

  @Override
  public void release() {
    if (!checkNativeReady()) {
      return;
    }

    mAbility.release();
    mCleanUpReference.cleanupNow();
    if (mResourceLoaderHolder != null) {
      mResourceLoaderHolder.release();
      mResourceLoaderHolder = null;
    }
    mHasDestroyed = true;
    mPtr = 0;
    AnimaXLog.i(TAG, "AnimaXPlayer destroyed");
  }

  // Surface
  @Override
  public void createAnimaXSurface(AnimaXSurfaceDrawable surfaceDrawable) {
    if (!checkNativeReady()) {
      return;
    }
    updateSurfaceProperties(surfaceDrawable);
    nativeCreateAnimaXSurface(mPtr, surfaceDrawable);
  }

  @Override
  public void updateAnimaXSurface(AnimaXSurfaceDrawable surfaceDrawable) {
    if (!checkNativeReady()) {
      return;
    }
    updateSurfaceProperties(surfaceDrawable);
    nativeUpdateAnimaXSurface(mPtr, surfaceDrawable);
  }

  @Override
  public void markPlatformSurfaceAsInvalid(boolean isInvalid) {
    this.mIsPlatformSurfaceInitiallyInvalid = isInvalid;
  }

  // Animation Player
  @Override
  public void setAutoPlay(boolean enable) {
    if (!checkNativeReady()) {
      return;
    }
    nativeSetAutoPlay(mPtr, enable);
    mAutoPlay = enable;
  }

  public boolean isAutoPlay() {
    return mAutoPlay;
  }

  @Override
  public void setSpeed(float speed) {
    if (!checkNativeReady()) {
      return;
    }
    nativeSetSpeed(mPtr, speed);
  }

  @Override
  public void setProgress(float progress) {
    if (!checkNativeReady()) {
      return;
    }
    nativeSetProgress(mPtr, progress);
  }

  @Override
  public void setKeepLastFrame(boolean enable) {
    if (!checkNativeReady()) {
      return;
    }
    nativeSetKeepLastFrame(mPtr, enable);
  }

  @Override
  public void setJson(String json) {
    if (!checkNativeReady()) {
      return;
    }
    nativeSetJson(mPtr, json);
    mAbility.getMonitorDelegate().updateUrl("json");
  }

  @Override
  public void setLoop(boolean enable) {
    if (!checkNativeReady()) {
      return;
    }
    nativeSetLoop(mPtr, enable);
  }

  @Override
  public void setLoopCount(int loopCount) {
    if (!checkNativeReady()) {
      return;
    }
    nativeSetLoopCount(mPtr, loopCount);
  }

  @Override
  public void setFpsEventInterval(long interval) {
    if (!checkNativeReady()) {
      return;
    }
    nativeSetFpsEventInterval(mPtr, interval);
  }

  @Override
  public void setDynamicResource(boolean enableDynamicResource) {
    if (!checkNativeReady()) {
      return;
    }
    nativeSetDynamicResource(mPtr, enableDynamicResource);
  }

  @Override
  public void setMuted(boolean mute) {
    if (!checkNativeReady()) {
      return;
    }
    nativeSetMuted(mPtr, mute);
  }

  @Override
  public void setEnableAudio(boolean enable) {
    if (!checkNativeReady()) {
      return;
    }
    nativeSetEnableAudio(mPtr, enable);
  }

  @Override
  public void setSrc(String src) {
    if (!checkNativeReady()) {
      return;
    }
    nativeSetSrc(mPtr, src);
    mAbility.getMonitorDelegate().updateUrl(src);
  }

  @Override
  public void setComposition(@NonNull AnimaXComposition composition) {
    mAnimaXComposition = composition;
    if (!checkNativeReady()) {
      return;
    }
    nativeSetComposition(mPtr, composition.getNativePtr());
  }

  @Override
  public AnimaXComposition getComposition() {
    return mAnimaXComposition;
  }

  @Override
  public void setImageFolder(String imageFolder) {
    if (imageFolder == null || imageFolder.isEmpty()) {
      return;
    }
    if (!checkNativeReady()) {
      return;
    }
    nativeSetImageFolder(mPtr, imageFolder);
  }

  @Override
  public void setStartFrame(int startFrame) {
    if (!checkNativeReady()) {
      return;
    }
    nativeSetStartFrame(mPtr, startFrame);
  }

  @Override
  public void setEndFrame(int endFrame) {
    if (!checkNativeReady()) {
      return;
    }
    nativeSetEndFrame(mPtr, endFrame);
  }

  @Override
  public void setAutoReverse(boolean isAutoReverse) {
    if (!checkNativeReady()) {
      return;
    }
    nativeSetAutoReverse(mPtr, isAutoReverse);
  }

  @Override
  public void setMaxFrameRate(double maxFrameRate) {
    if (!checkNativeReady()) {
      return;
    }
    nativeSetMaxFrameRate(mPtr, maxFrameRate);
  }

  @Override
  public void setAntiAliasing(boolean antiAliasing) {
    mEnableAntiAliasing = antiAliasing;
  }

  @Override
  public void play() {
    if (!checkNativeReady()) {
      return;
    }
    mAbility.getMonitorDelegate().onPlay();
    nativePlay(mPtr);
  }

  @Override
  public void pause() {
    if (!checkNativeReady()) {
      return;
    }
    nativePause(mPtr);
  }

  @Override
  public void resume() {
    if (!checkNativeReady()) {
      return;
    }
    mAbility.getMonitorDelegate().onResume();
    nativeResume(mPtr);
  }

  @Override
  public void stop() {
    if (!checkNativeReady()) {
      return;
    }
    nativeStop(mPtr);
  }

  @Override
  public double getDurationMs() {
    if (!checkNativeReady()) {
      return 0.f;
    }
    return nativeGetDurationMs(mPtr);
  }

  @Override
  public boolean isAnimating() {
    if (!checkNativeReady()) {
      return false;
    }
    return nativeIsAnimating(mPtr);
  }

  @Override
  public void seek(int frame) {
    if (!checkNativeReady()) {
      return;
    }
    nativeSeek(mPtr, frame);
  }

  @Override
  public double getCurrentFrame() {
    if (!checkNativeReady()) {
      return 0.f;
    }
    return nativeGetCurrentFrame(mPtr);
  }

  @Override
  public void playSegment(int startFrame, int endFrame) {
    if (!checkNativeReady()) {
      return;
    }
    mAbility.getMonitorDelegate().onPlaySegment();
    nativePlaySegment(mPtr, startFrame, endFrame);
  }

  private boolean checkNativeReady() {
    if (mHasDestroyed) {
      return false;
    }

    boolean initialized = hasInitialized();
    if (!initialized) {
      AnimaXLog.e(TAG, "checkNativeReady failed, ptr is 0");
    }

    return initialized;
  }

  @Override
  public void onShow(VisibilityState state) {
    if (!checkNativeReady()) {
      return;
    }
    nativeOnShow(mPtr, state.getValue());
  }

  @Override
  public void onHide(VisibilityState state) {
    if (!checkNativeReady()) {
      return;
    }
    nativeOnHide(mPtr, state.getValue());
  }

  private boolean updateVisibilityFlagIfChanged(boolean isVisible, VisibilityState state) {
    boolean currentIsVisible = (mVisibilityFlag & state.getValue()) == 0;
    if (isVisible == currentIsVisible) {
      return false;
    }
    mVisibilityFlag ^= state.getValue();
    return true;
  }

  @Override
  public void updateVisibilityState(boolean isVisible, VisibilityState state) {
    if (!updateVisibilityFlagIfChanged(isVisible, state)) {
      return;
    }
    if (isVisible) {
      onShow(state);
    } else {
      onHide(state);
    }
  }

  @Override
  public void enterForeground() {
    updateVisibilityState(true, VisibilityState.BACKGROUND);
  }

  @Override
  public void enterBackground() {
    updateVisibilityState(false, VisibilityState.BACKGROUND);
  }

  @Override
  public void cancel() {
    if (!checkNativeReady()) {
      return;
    }
    nativeCancel(mPtr);
  }

  @Override
  public void setObjectFit(ObjectFit objectFit) {
    if (!checkNativeReady()) {
      return;
    }
    nativeSetObjectFit(mPtr, objectFit.ordinal());
  }

  @Override
  public void setObjectPosition(ObjectPosition objectPosition) {
    if (!checkNativeReady()) {
      return;
    }
    nativeSetObjectPosition(mPtr, objectPosition.ordinal());
  }

  @RestrictTo(RestrictTo.Scope.LIBRARY)
  @Override
  public void setSrcPolyfill(JavaOnlyMap polyfillMap) {
    if (!checkNativeReady()) {
      return;
    }
    nativeSetSrcPolyfill(mPtr, polyfillMap);
  }

  // Event Subscription
  @Override
  public void subscribeUpdateEvent(int frame) {
    if (!checkNativeReady()) {
      return;
    }
    nativeSubscribeUpdateEvent(mPtr, frame);
  }

  @Override
  public void unsubscribeUpdateEvent(int frame) {
    if (!checkNativeReady()) {
      return;
    }
    nativeUnsubscribeUpdateEvent(mPtr, frame);
  }

  @Override
  public void subscribeUpdateEvents(int[] frame, boolean subscribe) {
    if (!checkNativeReady()) {
      return;
    }
    nativeSubscribeUpdateEvents(mPtr, frame, subscribe);
  }

  @Override
  public void addAnimationListener(IAnimationListener listener) {
    mAbility.addAnimationListener(listener);
  }

  @Override
  public void removeAnimationListener(IAnimationListener listener) {
    mAbility.removeAnimationListener(listener);
  }

  @Override
  @RestrictTo(RestrictTo.Scope.LIBRARY)
  public void onTap(float x, float y) {
    if (!checkNativeReady()) {
      return;
    }
    nativeOnTap(mPtr, x, y);
  }

  @Override
  public void reload() {
    if (!checkNativeReady()) {
      return;
    }
    nativeReload(mPtr);
  }

  // Dynamic resource
  @Override
  public void updateLayerProperty(LayerPropertyType type, AnimaXKeyPath keyPath,
      AnimaXValueParam param, AnimaXPropertyCallback callback) {
    if (!checkNativeReady()) {
      return;
    }
    nativeUpdateLayerProperty(mPtr, type.getValue(), keyPath, param, callback);
  }

  /**
   * Adds a value callback for dynamically modifying layer property values at runtime.
   * Unlike updateLayerProperty which updates static values, this registers a callback
   * that will be invoked during animation rendering.
   *
   * @param type The property type to modify
   * @param keyPath The key path to target specific layers/properties
   * @param valueCallback The callback to provide dynamic values
   * @param callback Completion callback for the operation
   */
  @Override
  public void addLayerPropertyCallback(LayerPropertyType type, @NonNull AnimaXKeyPath keyPath,
      @NonNull AnimaXValueCallback valueCallback, @Nullable AnimaXPropertyCallback callback) {
    if (!checkNativeReady()) {
      return;
    }
    nativeAddLayerPropertyCallback(mPtr, type.getValue(), keyPath, valueCallback, callback);
  }

  @Override
  public void removeLayerPropertyCallback(LayerPropertyType type, @NonNull AnimaXKeyPath keyPath,
      @Nullable AnimaXPropertyCallback callback) {
    if (!checkNativeReady()) {
      return;
    }
    nativeAddLayerPropertyCallback(mPtr, type.getValue(), keyPath, null, callback);
  }

  @Override
  public void setResourceProperty(ResourcePropertyType type, @NonNull String resourceId,
      @NonNull AnimaXValueParam param, @Nullable AnimaXPropertyCallback callback) {
    if (!checkNativeReady()) {
      return;
    }
    nativeSetResourceProperty(mPtr, type.getValue(), resourceId, param, callback);
  }

  @Override
  public void getKeysForKeyPath(
      @NonNull AnimaXKeyPath keyPath, @NonNull AnimaXKeyPathListCallback callback) {
    if (!checkNativeReady()) {
      return;
    }
    nativeGetKeysForKeyPath(mPtr, keyPath, callback);
  }

  // Monitor
  @RestrictTo(RestrictTo.Scope.LIBRARY)
  @Override
  public void getMetricsAsync(AnimaXMetricsCallback callback) {
    if (!checkNativeReady()) {
      return;
    }
    nativeGetMetricsAsync(mPtr, callback);
  }

  @RestrictTo(RestrictTo.Scope.LIBRARY)
  public long getMemoryUsageBytes() {
    if (!checkNativeReady()) {
      return 0;
    }
    return nativeGetMemoryUsageBytes(mPtr);
  }

  private native void nativeGetMetricsAsync(long player, AnimaXMetricsCallback callback);

  private native long nativeCreate(long loader, AnimaXContext context);

  private native void nativeCreateAnimaXSurface(long player, AnimaXSurfaceDrawable surfaceDrawable);

  private native void nativeUpdateAnimaXSurface(long player, AnimaXSurfaceDrawable surfaceDrawable);

  private native void nativeSetAutoPlay(long player, boolean enable);

  private native void nativeSetSpeed(long player, float speed);

  private native void nativeSetProgress(long player, float progress);

  private native void nativeSetObjectFit(long player, int objectFit);

  private native void nativeSetObjectPosition(long player, int objectPosition);

  private native void nativeSetKeepLastFrame(long player, boolean enable);

  private native void nativeSetJson(long player, String json);

  private native void nativeSetLoop(long player, boolean enable);

  private native void nativeSetLoopCount(long player, int loopCount);

  private native void nativeSetSrc(long player, String src);

  private native void nativeSetComposition(long player, long composition);

  private native void nativeSetImageFolder(long player, String imageFolder);

  private native void nativeSetSrcPolyfill(long player, JavaOnlyMap polyfillMap);

  private native void nativeSetStartFrame(long player, int startFrame);

  private native void nativeSetEndFrame(long player, int endFrame);

  private native void nativeSetAutoReverse(long player, boolean isAutoReverse);

  private native void nativePlay(long player);

  private native void nativePause(long player);

  private native void nativeResume(long player);

  private native void nativeStop(long player);

  private native double nativeGetDurationMs(long player);

  private native boolean nativeIsAnimating(long player);

  private native void nativeSeek(long player, int frame);

  private native void nativeSubscribeUpdateEvent(long player, int frame);

  private native void nativeUnsubscribeUpdateEvent(long player, int frame);

  private native void nativeSubscribeUpdateEvents(long player, int[] frame, boolean subscribe);

  private native double nativeGetCurrentFrame(long player);

  private native void nativePlaySegment(long player, int startFrame, int endFrame);

  private static native void nativeDestroy(long player);

  private native void nativeReload(long player);

  private native ReadableMap nativeGetEventTrackingMap(long player);

  private native void nativeSetFpsEventInterval(long player, long interval);

  private native void nativeSetMaxFrameRate(long player, double maxFrameRate);

  private native void nativeOnShow(long player, long state);

  private native void nativeOnHide(long player, long state);

  private native void nativeSetDynamicResource(long player, boolean dynamic);

  private native void nativeSetMuted(long player, boolean mute);

  private native void nativeSetEnableAudio(long player, boolean mute);

  private native void nativeCancel(long player);

  private native void nativeOnTap(long player, float x, float y);

  private native void nativeUpdateLayerProperty(long player, int type, AnimaXKeyPath keyPath,
      AnimaXValueParam param, AnimaXPropertyCallback callback);

  private native void nativeSetResourceProperty(long player, int type, String resourceId,
      AnimaXValueParam param, AnimaXPropertyCallback callback);

  private native void nativeAddLayerPropertyCallback(long player, int type, AnimaXKeyPath keyPath,
      AnimaXValueCallback valueCallback, AnimaXPropertyCallback callback);

  private native void nativeGetKeysForKeyPath(
      long player, AnimaXKeyPath keyPath, AnimaXKeyPathListCallback callback);

  private native long nativeGetMemoryUsageBytes(long player);
}
