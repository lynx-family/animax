// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.ui;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import com.lynx.animax.base.VisibilityState;
import com.lynx.animax.composition.AnimaXComposition;
import com.lynx.animax.drawable.AnimaXSurfaceDrawable;
import com.lynx.animax.listener.IAnimationListener;
import com.lynx.animax.property.AnimaXKeyPath;
import com.lynx.animax.property.AnimaXKeyPathListCallback;
import com.lynx.animax.property.AnimaXPropertyCallback;
import com.lynx.animax.property.AnimaXValueCallback;
import com.lynx.animax.property.AnimaXValueParam;
import com.lynx.animax.property.LayerPropertyType;
import com.lynx.animax.property.ResourcePropertyType;

public interface IAnimaXPlayerDelegate {
  /**
   * Gets the underlying player instance.
   * @return The {@link IAnimaXPlayer} instance.
   */
  @NonNull IAnimaXPlayer getPlayer();

  /**
   * @see IAnimaXPlayer#enableSoftwareRender(boolean)
   */
  default void enableSoftwareRender(boolean softwareRender) {
    getPlayer().enableSoftwareRender(softwareRender);
  }

  /**
   * @see IAnimaXPlayer#createAnimaXSurface(AnimaXSurfaceDrawable)
   */
  default void createAnimaXSurface(AnimaXSurfaceDrawable surfaceDrawable) {
    getPlayer().createAnimaXSurface(surfaceDrawable);
  }

  /**
   * @see IAnimaXPlayer#updateAnimaXSurface(AnimaXSurfaceDrawable)
   */
  default void updateAnimaXSurface(AnimaXSurfaceDrawable surfaceDrawable) {
    getPlayer().updateAnimaXSurface(surfaceDrawable);
  }

  /**
   * @see IAnimaXPlayer#setImageFolder(String)
   */
  default void setImageFolder(String imageFolder) {
    if (imageFolder == null || imageFolder.isEmpty()) {
      return;
    }
    getPlayer().setImageFolder(imageFolder);
  }

  /**
   * @see IAnimaXPlayer#setComposition(AnimaXComposition)
   */
  default void setComposition(AnimaXComposition composition) {
    getPlayer().setComposition(composition);
  }

  /**
   * @see IAnimaXPlayer#getComposition()
   */
  default AnimaXComposition getComposition() {
    return getPlayer().getComposition();
  }

  /**
   * @see IAnimaXPlayer#setJson(String)
   */
  default void setJson(String json) {
    getPlayer().setJson(json);
  }

  /**
   * @see IAnimaXPlayer#setSrc(String)
   */
  default void setSrc(String src) {
    getPlayer().setSrc(src);
  }

  /**
   * @see IAnimaXPlayer#setDynamicResource(boolean)
   */
  default void setDynamicResource(boolean dynamicResource) {
    getPlayer().setDynamicResource(dynamicResource);
  }

  /**
   * @see IAnimaXPlayer#setMuted(boolean)
   */
  default void setMuted(boolean mute) {
    getPlayer().setMuted(mute);
  }

  /**
   * @see IAnimaXPlayer#setEnableAudio(boolean)
   */
  default void setEnableAudio(boolean enable) {
    getPlayer().setEnableAudio(enable);
  }

  // Animation-playing(setProperties)

  /**
   * @see IAnimaXPlayer#setLoop(boolean)
   */
  default void setLoop(boolean enable) {
    getPlayer().setLoop(enable);
  }

  /**
   * @see IAnimaXPlayer#setLoopCount(int)
   */
  default void setLoopCount(int loopCount) {
    getPlayer().setLoopCount(loopCount);
  }

  /**
   * @see IAnimaXPlayer#setStartFrame(int)
   */
  default void setStartFrame(int startFrame) {
    getPlayer().setStartFrame(startFrame);
  }

  /**
   * @see IAnimaXPlayer#setEndFrame(int)
   */
  default void setEndFrame(int endFrame) {
    getPlayer().setEndFrame(endFrame);
  }

  /**
   * @see IAnimaXPlayer#setProgress(float)
   */
  default void setProgress(float progress) {
    getPlayer().setProgress(progress);
  }

  /**
   * @see IAnimaXPlayer#setSpeed(float)
   */
  default void setSpeed(float speed) {
    getPlayer().setSpeed(speed);
  }

  /**
   * @see IAnimaXPlayer#setAutoReverse(boolean)
   */
  default void setAutoReverse(boolean isAutoReverse) {
    getPlayer().setAutoReverse(isAutoReverse);
  }

  /**
   * @see IAnimaXPlayer#setMaxFrameRate(double)
   */
  default void setMaxFrameRate(double maxFrameRate) {
    getPlayer().setMaxFrameRate(maxFrameRate);
  }

  /**
   * @see IAnimaXPlayer#setAutoPlay(boolean)
   */
  default void setAutoPlay(boolean autoPlay) {
    getPlayer().setAutoPlay(autoPlay);
  }

  /**
   * @see IAnimaXPlayer#setObjectFit(ObjectFit)
   */
  default void setObjectFit(ObjectFit objectFit) {
    getPlayer().setObjectFit(objectFit);
  }

  /**
   * @see IAnimaXPlayer#setObjectPosition(ObjectPosition)
   */
  default void setObjectPosition(ObjectPosition objectPosition) {
    getPlayer().setObjectPosition(objectPosition);
  }

  /**
   * @see IAnimaXPlayer#setKeepLastFrame(boolean)
   */
  default void setKeepLastFrame(boolean enable) {
    getPlayer().setKeepLastFrame(enable);
  }

  /**
   * @see IAnimaXPlayer#play()
   */
  default void play() {
    getPlayer().play();
  }

  /**
   * @see IAnimaXPlayer#pause()
   */
  default void pause() {
    getPlayer().pause();
  }

  /**
   * @see IAnimaXPlayer#resume()
   */
  default void resume() {
    getPlayer().resume();
  }

  /**
   * @see IAnimaXPlayer#stop()
   */
  default void stop() {
    getPlayer().stop();
  }

  /**
   * @see IAnimaXPlayer#seek(int)
   */
  default void seek(int frame) {
    getPlayer().seek(frame);
  }

  /**
   * @see IAnimaXPlayer#cancel()
   */
  default void cancel() {
    getPlayer().cancel();
  }

  /**
   * @see IAnimaXPlayer#playSegment(int, int)
   */
  default void playSegment(int startFrame, int endFrame) {
    getPlayer().playSegment(startFrame, endFrame);
  }

  /**
   * @see IAnimaXPlayer#reload()
   */
  default void reload() {
    getPlayer().reload();
  }

  // Animation-playing(getInfo)
  /**
   * @see IAnimaXPlayer#isAnimating()
   */
  default boolean isAnimating() {
    return getPlayer().isAnimating();
  }

  /**
   * @see IAnimaXPlayer#getProgress()
   */
  default double getProgress() {
    return getPlayer().getProgress();
  }

  /**
   * @see IAnimaXPlayer#getCurrentFrame()
   */
  default double getCurrentFrame() {
    return getPlayer().getCurrentFrame();
  }

  /**
   * @see IAnimaXPlayer#getDurationMs()
   */
  default double getDurationMs() {
    return getPlayer().getDurationMs();
  }
  // Event(visibility)

  /**
   * @see IAnimaXPlayer#updateVisibilityState(boolean, VisibilityState)
   */
  default void updateVisibilityState(boolean isVisible, VisibilityState state) {
    getPlayer().updateVisibilityState(isVisible, state);
  }

  /**
   * @see IAnimaXPlayer#onShow(VisibilityState)
   */
  default void onShow(VisibilityState state) {
    getPlayer().onShow(state);
  }

  /**
   * @see IAnimaXPlayer#onHide(VisibilityState)
   */
  default void onHide(VisibilityState state) {
    getPlayer().onHide(state);
  }

  /**
   * @see IAnimaXPlayer#enterForeground()
   */
  default void enterForeground() {
    getPlayer().enterForeground();
  }

  /**
   * @see IAnimaXPlayer#enterBackground()
   */
  default void enterBackground() {
    getPlayer().enterBackground();
  }

  // Event(Listening)

  /**
   * @see IAnimaXPlayer#setFpsEventInterval(long)
   */
  default void setFpsEventInterval(long interval) {
    getPlayer().setFpsEventInterval(interval);
  }

  /**
   * @see IAnimaXPlayer#subscribeUpdateEvent(int)
   */
  default void subscribeUpdateEvent(int frame) {
    getPlayer().subscribeUpdateEvent(frame);
  }

  /**
   * @see IAnimaXPlayer#unsubscribeUpdateEvent(int)
   */
  default void unsubscribeUpdateEvent(int frame) {
    getPlayer().unsubscribeUpdateEvent(frame);
  }

  /**
   * @see IAnimaXPlayer#subscribeUpdateEvents(int[], boolean)
   */
  default void subscribeUpdateEvents(int[] frame, boolean subscribe) {
    getPlayer().subscribeUpdateEvents(frame, subscribe);
  }

  /**
   * @see IAnimaXPlayer#addAnimationListener(IAnimationListener)
   */
  default void addAnimationListener(IAnimationListener listener) {
    getPlayer().addAnimationListener(listener);
  }

  /**
   * @see IAnimaXPlayer#removeAnimationListener(IAnimationListener)
   */
  default void removeAnimationListener(IAnimationListener listener) {
    getPlayer().removeAnimationListener(listener);
  }

  /**
   * @see IAnimaXPlayer#setAntiAliasing(boolean)
   */
  default void setAntiAliasing(boolean antiAliasing) {
    getPlayer().setAntiAliasing(antiAliasing);
  }

  // Dynamic Resource

  /**
   * @see IAnimaXPlayer#updateLayerProperty(LayerPropertyType, AnimaXKeyPath, AnimaXValueParam,
   *     AnimaXPropertyCallback)
   */
  default void updateLayerProperty(LayerPropertyType type, AnimaXKeyPath keyPath,
      AnimaXValueParam param, AnimaXPropertyCallback callback) {
    getPlayer().updateLayerProperty(type, keyPath, param, callback);
  }

  /**
   * @see IAnimaXPlayer#addLayerPropertyCallback(LayerPropertyType, AnimaXKeyPath,
   *     AnimaXValueCallback, AnimaXPropertyCallback)
   */
  default void addLayerPropertyCallback(LayerPropertyType type, @NonNull AnimaXKeyPath keyPath,
      @NonNull AnimaXValueCallback valueCallback, @Nullable AnimaXPropertyCallback callback) {
    getPlayer().addLayerPropertyCallback(type, keyPath, valueCallback, callback);
  }

  /**
   * @see IAnimaXPlayer#removeLayerPropertyCallback(LayerPropertyType, AnimaXKeyPath,
   *     AnimaXPropertyCallback)
   */
  default void removeLayerPropertyCallback(LayerPropertyType type, @NonNull AnimaXKeyPath keyPath,
      @Nullable AnimaXPropertyCallback callback) {
    getPlayer().removeLayerPropertyCallback(type, keyPath, callback);
  }

  /**
   * @see IAnimaXPlayer#setResourceProperty(ResourcePropertyType, String, AnimaXValueParam,
   *     AnimaXPropertyCallback)
   */
  default void setResourceProperty(ResourcePropertyType type, @NonNull String resourceId,
      @NonNull AnimaXValueParam param, @Nullable AnimaXPropertyCallback callback) {
    getPlayer().setResourceProperty(type, resourceId, param, callback);
  }

  /**
   * @see IAnimaXPlayer#getKeysForKeyPath(AnimaXKeyPath, AnimaXKeyPathListCallback)
   */
  default void getKeysForKeyPath(
      @NonNull AnimaXKeyPath keyPath, @NonNull AnimaXKeyPathListCallback callback) {
    getPlayer().getKeysForKeyPath(keyPath, callback);
  }
}
