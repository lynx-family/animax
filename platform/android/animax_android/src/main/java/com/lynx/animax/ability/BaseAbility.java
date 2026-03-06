// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.ability;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.RestrictTo;
import com.lynx.animax.AnimaXPlayer;
import com.lynx.animax.audio.AudioAsset;
import com.lynx.animax.audio.AudioPlayer;
import com.lynx.animax.base.CalledByNative;
import com.lynx.animax.base.IAnimaXCompositionReadyListener;
import com.lynx.animax.base.bridge.ReadableMap;
import com.lynx.animax.listener.AnimaXErrorParam;
import com.lynx.animax.listener.AnimaXFPSParam;
import com.lynx.animax.listener.AnimaXParam;
import com.lynx.animax.listener.AnimaXTapParam;
import com.lynx.animax.listener.IAnimationListener;
import com.lynx.animax.monitor.MonitorAbilityDelegate;
import com.lynx.animax.player.IVideoPlayer;
import com.lynx.animax.player.VideoPlayerConfig;
import com.lynx.animax.player.VideoPlayerFactory;
import com.lynx.animax.service.IAnimaXService;
import com.lynx.animax.service.ServiceRegistry;
import com.lynx.animax.service.ServiceScope;
import com.lynx.animax.util.DeviceUtil;
import com.lynx.animax.util.UIThreadUtils;
import java.lang.ref.WeakReference;
import java.util.List;
import java.util.Map;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * The BaseAbility class provides basic functionalities for managing an animation.
 * It is designed to handle specific differences between native and Lynx UI platforms,
 * offering a unified interface for registering and notifying animation listeners,
 * dealing with animation events, and manipulating animation parameters.
 */
public class BaseAbility {
  // Maintain all registered animation listeners
  private final List<IAnimationListener> mListeners = new CopyOnWriteArrayList<>();

  // Notify the callback when composition is loaded, then check and create a valid surface
  private WeakReference<IAnimaXCompositionReadyListener> mCompositionReadyListener;

  private final ServiceRegistry mServiceRegistry = new ServiceRegistry(getScope());

  private final VideoPlayerConfig mVideoPlayerConfig = new VideoPlayerConfig(mServiceRegistry);

  private final MonitorAbilityDelegate mMonitorDelegate =
      new MonitorAbilityDelegate(mServiceRegistry);

  @RestrictTo(RestrictTo.Scope.LIBRARY)
  public void initAnimaXPlayer(@NonNull AnimaXPlayer player) {
    mMonitorDelegate.setAnimaXPlayer(player);
    addAnimationListener(mMonitorDelegate);
  }

  /**
   * Called by Native code, used for sending events
   * @param event
   * @param map
   */
  @CalledByNative
  public void sendEvent(int event, ReadableMap map) {
    sendEventOnUI(event, map.asHashMap());
  }

  @CalledByNative
  public IVideoPlayer createVideoPlayer(long nativePtr) {
    mVideoPlayerConfig.setSurfaceDestroyTimeout(DeviceUtil.getVideoSurfaceDestroyTimeout(this));
    return VideoPlayerFactory.create(nativePtr, mVideoPlayerConfig);
  }

  @CalledByNative
  public AudioPlayer createAudioPlayer(AudioAsset asset) {
    return AudioPlayer.create(asset);
  }

  @CalledByNative
  public boolean disableByteVC1Decoder() {
    return DeviceUtil.disableByteVC1Decoder(this);
  }

  /**
   * Send events on the UI thread
   * @param event
   * @param params
   */
  public void sendEventOnUI(final int event, Map<String, Object> params) {
    UIThreadUtils.runOnUiThreadImmediately(() -> onEvent(event, params));
  }

  /**
   * Add an animation listener
   * @param listener
   */
  public void addAnimationListener(IAnimationListener listener) {
    mListeners.add(listener);
  }

  /**
   * Remove an animation listener
   * @param listener
   */
  public void removeAnimationListener(IAnimationListener listener) {
    mListeners.remove(listener);
  }

  /**
   * Gets a service of the specified type.
   *
   * @param serviceClass The class of the service to retrieve
   * @return The service instance if found, null otherwise
   */
  @Nullable
  public <T extends IAnimaXService> T getService(Class<T> serviceClass) {
    return mServiceRegistry.getService(serviceClass);
  }

  /**
   * Registers a service instance.
   *
   * @param clazz The class type to register the service as
   * @param instance The service instance to register
   * @return void. If the service type is already registered, method returns silently
   */
  public <T extends IAnimaXService> void registerService(
      @NonNull Class<T> clazz, @NonNull T instance) {
    mServiceRegistry.registerService(clazz, instance);
  }

  /**
   * Unregisters a service of the specified type.
   *
   * @param clazz The class type of the service to unregister
   * @return void. If the service is not registered, method returns silently
   */
  public <T extends IAnimaXService> void unregisterService(@NonNull Class<T> clazz) {
    mServiceRegistry.unregisterService(clazz);
  }

  /**
   * Release resources, like clearing the listener list
   */
  public void release() {
    mMonitorDelegate.onRelease();
    mServiceRegistry.release();
    mListeners.clear();
  }

  @RestrictTo(RestrictTo.Scope.LIBRARY)
  public void setCompositionReadyListener(IAnimaXCompositionReadyListener verifier) {
    mCompositionReadyListener = new WeakReference<>(verifier);
  }

  /**
   * Processes the event received from the AnimaX C++ native code or other sources.
   * Wraps the parameters and distributes them to all registered listeners.
   * @param event The event to be processed.
   * @param params The parameters associated with the event.
   */
  private void onEvent(int event, Map<String, Object> params) {
    if (mListeners.isEmpty()) {
      return;
    }

    switch (Event.values()[event]) {
      case COMPLETION: {
        AnimaXParam param = new AnimaXParam(params);
        notifyListeners(listener -> listener.onComplete(param));
        break;
      }
      case START: {
        AnimaXParam param = new AnimaXParam(params);
        notifyListeners(listener -> listener.onStart(param));
        break;
      }
      case REPEAT: {
        AnimaXParam param = new AnimaXParam(params);
        notifyListeners(listener -> listener.onRepeat(param));
        break;
      }
      case CANCEL: {
        AnimaXParam param = new AnimaXParam(params);
        notifyListeners(listener -> listener.onCancel(param));
        break;
      }
      case READY: {
        AnimaXParam param = new AnimaXParam(params);
        notifyListeners(listener -> listener.onReady(param));
        break;
      }
      case ERROR: {
        AnimaXErrorParam param = new AnimaXErrorParam(params);
        notifyListeners(listener -> listener.onError(param));
        break;
      }
      case UPDATE: {
        AnimaXParam param = new AnimaXParam(params);
        notifyListeners(listener -> listener.onUpdate(param));
        break;
      }
      case FPS: {
        AnimaXFPSParam param = new AnimaXFPSParam(params);
        notifyListeners(listener -> listener.onFPS(param));
        break;
      }
      case COMPOSITION_READY: {
        AnimaXParam param = new AnimaXParam(params);
        notifyListeners(listener -> listener.onCompositionReady(param));
        IAnimaXCompositionReadyListener verifier =
            mCompositionReadyListener != null ? mCompositionReadyListener.get() : null;
        if (verifier != null) {
          verifier.onCompositionReady();
        }
        break;
      }
      case TAP_LAYERS: {
        AnimaXTapParam param = new AnimaXTapParam(params);
        notifyListeners(listener -> listener.onTapLayers(param));
        break;
      }
      case FIRST_FRAME: {
        AnimaXParam param = new AnimaXParam(params);
        notifyListeners(listener -> listener.onFirstFrame(param));
        break;
      }
      case WARNING: {
        AnimaXErrorParam param = new AnimaXErrorParam(params);
        notifyListeners(listener -> listener.onWarning(param));
        break;
      }
      default:
        break;
    }
  }

  public String redirectUrl(String originUrl) {
    // ignore
    return originUrl;
  }

  /**
   * Iterate over all listeners and perform the specified operation
   * @param action
   */
  private void notifyListeners(Action action) {
    for (IAnimationListener listener : mListeners) {
      action.execute(listener);
    }
  }

  private interface Action {
    void execute(IAnimationListener listener);
  }

  protected ServiceScope getScope() {
    return ServiceScope.DEFAULT;
  }

  public VideoPlayerConfig getVideoPlayerConfig() {
    return mVideoPlayerConfig;
  }

  public MonitorAbilityDelegate getMonitorDelegate() {
    return mMonitorDelegate;
  }
}
