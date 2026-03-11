// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.ui;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.RestrictTo;
import com.lynx.animax.base.VisibilityState;
import com.lynx.animax.base.bridge.JavaOnlyMap;
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
import com.lynx.animax.util.AnimaXMetricsCallback;

/**
 * Public AnimaX player's protocol
 * Provide all public methods for control AnimaXView status
 * This will implemented by AnimaXPlayer (in the future)
 */
public interface IAnimaXPlayer {
  // Player lifecycle

  /**
   * Returns the AnimaX context bound to this player.
   * @return a {@link AnimaXContext} that provides runtime facilities.
   */
  AnimaXContext getAnimaXContext();

  /**
   * Enables software rasterization path instead of hardware when true.
   * This is useful on devices/contexts where GPU rendering is unavailable or undesirable.
   * @param softwareRender true to enable software rendering; false for hardware.
   */
  void enableSoftwareRender(boolean softwareRender);

  /**
   * Releases all resources associated with this player.
   * After calling this, the instance should not be used again.
   */
  void release();

  // Surface

  /**
   * create a new surface and bound to this player.
   * @param surfaceDrawable The drawable wrapping the platform surface.
   */
  void createAnimaXSurface(AnimaXSurfaceDrawable surfaceDrawable);

  /**
   * Update the underlying surface (e.g., size or format).
   * @param surfaceDrawable The drawable wrapping the updated surface.
   */
  void updateAnimaXSurface(AnimaXSurfaceDrawable surfaceDrawable);

  /**
   * Marks the platform surface state so the player can skip rendering when invalid.
   * @param isInvalid true if the surface is invalid/unavailable; false otherwise.
   */
  @RestrictTo(RestrictTo.Scope.LIBRARY) void markPlatformSurfaceAsInvalid(boolean isInvalid);

  // Resource Loading

  /**
   * Sets the folder path for external image assets.
   * To take effect, this needs to be called prior to setJson or setSrc.
   * @param imageFolder The path to the directory containing image assets.
   */
  void setImageFolder(String imageFolder);

  /**
   * Sets the animation composition directly.
   * @param composition The {@link AnimaXComposition} object to be played.
   */
  void setComposition(AnimaXComposition composition);

  /**
   * Gets the currently loaded animation composition.
   * @return The current {@link AnimaXComposition}, or null if none is loaded.
   */
  AnimaXComposition getComposition();

  /**
   * Loads an animation from a JSON string.
   * @param json The Lottie JSON string representing the animation.
   */
  void setJson(String json);

  /**
   * Loads an animation from a source URL or local file path.
   * @param src The URL or path to the animation resource (e.g., a .json or .zip file).
   */
  void setSrc(String src);

  /**
   * Provides a polyfill map for the animation source, allowing for dynamic replacements or
   * configurations.
   * @param polyfill A map containing polyfill data.
   */
  void setSrcPolyfill(JavaOnlyMap polyfill);

  /**
   * Enables or disables dynamic resource loading.
   * When enabled, resources like images can be updated during playback.
   * @param dynamicResource if true, enables dynamic resource loading.
   */
  void setDynamicResource(boolean dynamicResource);

  // Animation-playing(setProperties)

  /**
   * Sets whether the animation should loop indefinitely.
   * Do not call this with setLoopCount at the same time.
   * @param enable if true, the animation will loop.
   */
  void setLoop(boolean enable);

  /**
   * Sets the number of times the animation should loop.
   * Do not call this with setLoop at the same time.
   * A value of 0 means infinite looping.
   * @param loopCount The number of repetitions.
   */
  void setLoopCount(int loopCount);

  /**
   * Sets the frame at which the animation should start playing.
   * @param startFrame The starting frame number.
   */
  void setStartFrame(int startFrame);

  /**
   * Sets the frame at which the animation should end.
   * @param endFrame The ending frame number.
   */
  void setEndFrame(int endFrame);

  /**
   * Sets the animation's progress to a specific point.
   * @param progress A value from 0.0 to 1.0, where 0 is the start and 1 is the end of the
   *     animation.
   */
  void setProgress(float progress);

  /**
   * Sets the playback speed of the animation.
   * @param speed The playback speed multiplier. 1.0 is normal speed.
   */
  void setSpeed(float speed);

  /**
   * Sets whether the animation should play in reverse after reaching the end.
   * @param isAutoReverse if true, enables auto-reverse.
   */
  void setAutoReverse(boolean isAutoReverse);

  /**
   * Sets the maximum frame rate for the animation playback.
   * @param maxFrameRate The maximum frames per second.
   */
  void setMaxFrameRate(double maxFrameRate);

  /**
   * Sets whether the animation should start playing automatically once it's loaded.
   * @param autoPlay if true, enables auto-play.
   */
  void setAutoPlay(boolean autoPlay);

  /**
   * Specifies how the animation content should be resized to fit its container.
   * @param objectFit The {@link ObjectFit} mode to apply (e.g., contain, cover).
   */
  void setObjectFit(ObjectFit objectFit);

  /**
   * Specifies the alignment of the animation content within its container when using object-fit.
   * @param objectPosition The {@link ObjectPosition} to apply.
   */
  void setObjectPosition(ObjectPosition objectPosition);

  /**
   * Sets whether the view should keep displaying the last frame of the animation after it has
   * finished playing.
   * @param enable if true, keeps the last frame, otherwise seeks to the first frame.
   */
  void setKeepLastFrame(boolean enable);

  // Animation-playing(commands)

  /**
   * Play the animation.
   */
  void play();

  /**
   * Pauses the animation.
   */
  void pause();

  /**
   * Resumes the animation.
   */
  void resume();

  /**
   * Stops the animation.
   */
  void stop();

  /**
   * Jumps to a specific frame in the animation.
   * @param frame The frame number to seek to.
   */
  void seek(int frame);

  /**
   * Cancels the animation playback immediately.
   */
  void cancel();

  /**
   * Plays a specific segment of the animation.
   * @param startFrame The frame to start playing from.
   * @param endFrame The frame to end playing at.
   */
  void playSegment(int startFrame, int endFrame); // Play range of animation

  /**
   * Reloads the animation from its source.
   */
  void reload();

  // Animation-playing(getInfo)

  /**
   * Checks if the animation is currently playing.
   * @return true if the animation is in playing status, false otherwise.
   */
  boolean isAnimating();

  /**
   * Gets the current frame number of the animation.
   * @return The current frame number, which can be a fractional value.
   */
  double getCurrentFrame();

  /**
   * Gets the total duration of the animation in milliseconds.
   * @return The duration in milliseconds.
   */
  double getDurationMs();

  // Event(visibility)

  /**
   * Updates the visibility state, allowing the player to throttle or pause rendering.
   * @param isVisible Whether the host view is currently visible to the user.
   * @param state A finer-grained visibility state description.
   */
  void updateVisibilityState(boolean isVisible, VisibilityState state);

  /**
   * Called when the view becomes visible.
   * @param state Current visibility state.
   */
  void onShow(VisibilityState state);

  /**
   * Called when the view becomes hidden.
   * @param state Current visibility state.
   */
  void onHide(VisibilityState state);

  /**
   * Notifies the player that the view has entered the foreground.
   */
  void enterForeground();

  /**
   * Notifies the player that the view has entered the background.
   * This is typically used to pause animations.
   */
  void enterBackground();

  // Event(Listening)

  /**
   * Sets the interval for emitting FPS (Frames Per Second) update events.
   * @param interval The interval in milliseconds. A value of 0 disables the event.
   */
  void setFpsEventInterval(long interval);

  /**
   * Subscribes to an update event that fires when the animation reaches a specific frame.
   * @param frame The frame number to subscribe to.
   */
  void subscribeUpdateEvent(int frame);

  /**
   * Unsubscribes from an update event for a specific frame.
   * @param frame The frame number to unsubscribe from.
   */
  void unsubscribeUpdateEvent(int frame);

  /**
   * Subscribes or unsubscribes to update events for multiple frames at once.
   * @param frame An array of frame numbers.
   * @param subscribe if true, subscribes to events for the given frames; if false, unsubscribes.
   */
  void subscribeUpdateEvents(int[] frame, boolean subscribe);

  /**
   * Adds a listener to receive animation lifecycle events (e.g., onStart, onEnd).
   * @param listener The {@link IAnimationListener} to add.
   */
  void addAnimationListener(IAnimationListener listener);

  /**
   * Removes a previously added animation listener.
   * @param listener The {@link IAnimationListener} to remove.
   */
  void removeAnimationListener(IAnimationListener listener);

  // Event Handling

  /**
   * Processes a tap event at the specified coordinates to detect layer hits.
   * @param x The x-coordinate of the tap within the animation bounds.
   * @param y The y-coordinate of the tap within the animation bounds.
   */
  void onTap(float x, float y);

  // misc
  /**
   * Enables or disables anti-aliasing for shape layers.
   * This can improve rendering quality at the cost of performance.
   * @param antiAliasing if true, enables anti-aliasing.
   */
  void setAntiAliasing(boolean antiAliasing);

  // Dynamic resource

  /**
   * Updates a layer's static property value immediately.
   * @param type The property type to update
   * @param keyPath The target layer/property path
   * @param param Value container for the new property
   * @param callback Optional completion callback
   */
  void updateLayerProperty(LayerPropertyType type, AnimaXKeyPath keyPath, AnimaXValueParam param,
      AnimaXPropertyCallback callback);

  /**
   * Adds a value callback for dynamically modifying layer property values at runtime.
   * Unlike updateLayerProperty which updates static values, this registers a callback
   * that will be invoked during animation rendering.
   * @param type The property type to modify
   * @param keyPath The key path to target specific layers/properties
   * @param valueCallback The callback to provide dynamic values
   * @param callback Completion callback for the operation
   */
  void addLayerPropertyCallback(LayerPropertyType type, @NonNull AnimaXKeyPath keyPath,
      @NonNull AnimaXValueCallback valueCallback, @Nullable AnimaXPropertyCallback callback);

  /**
   * Removes a previously added value callback for a layer property.
   * @param type The property type to remove callback from
   * @param keyPath The target layer/property path
   * @param callback Optional completion callback
   */
  void removeLayerPropertyCallback(LayerPropertyType type, @NonNull AnimaXKeyPath keyPath,
      @Nullable AnimaXPropertyCallback callback);

  /**
   * Sets/updates a resource-scoped property (e.g., image/text/font) referenced by resourceId.
   * @param type The resource property type
   * @param resourceId The resource identifier to apply the change to
   * @param param The value parameter for the property
   * @param callback Optional completion callback
   */
  void setResourceProperty(ResourcePropertyType type, @NonNull String resourceId,
      @NonNull AnimaXValueParam param, @Nullable AnimaXPropertyCallback callback);

  /**
   * Resolves and returns all concrete keys that match a key path pattern.
   * @param keyPath The query pattern
   * @param callback Callback receiving the resolved keys
   */
  void getKeysForKeyPath(
      @NonNull AnimaXKeyPath keyPath, @NonNull AnimaXKeyPathListCallback callback);

  // Monitor

  /**
   * Asynchronously collects runtime metrics (e.g., FPS, memory) and returns them via callback.
   * @param callback The receiver for metrics results
   */
  void getMetricsAsync(AnimaXMetricsCallback callback);
}
