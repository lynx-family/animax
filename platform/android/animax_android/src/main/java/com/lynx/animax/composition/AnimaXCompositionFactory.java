// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.animax.composition;

import androidx.annotation.NonNull;
import com.lynx.animax.loader.AnimaXResourceLoaderHolder;
import com.lynx.animax.service.ServiceScope;
import com.lynx.animax.util.AnimaX;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

/**
 * Factory class for creating and managing AnimaX compositions in different service scopes.
 * This class follows the singleton pattern and manages resource loaders for different service
 * scopes.
 */
public class AnimaXCompositionFactory {
  private static final String TAG = "AnimaXCompositionFactory";

  private static AnimaXCompositionFactory sInstance;

  private final Map<ServiceScope, AnimaXResourceLoaderHolder> mLoaderMap;

  private AnimaXCompositionFactory() {
    mLoaderMap = new ConcurrentHashMap<>();
  }

  /**
   * Gets the singleton instance of AnimaXCompositionFactory.
   * Creates a new instance if none exists using double-checked locking pattern.
   *
   * @return The singleton instance of AnimaXCompositionFactory
   */
  public static AnimaXCompositionFactory inst() {
    if (sInstance == null) {
      synchronized (AnimaXCompositionFactory.class) {
        if (sInstance == null) {
          sInstance = new AnimaXCompositionFactory();
        }
      }
    }
    return sInstance;
  }

  /**
   * Loads an animation from JSON string in the specified service scope.
   *
   * @param scope The service scope to load the animation in
   * @param json The JSON string containing animation data
   * @param listener Callback listener for composition loading events
   */
  public void loadJson(@NonNull ServiceScope scope, @NonNull String json,
      @NonNull AnimaXCompositionListener listener) {
    ensureLoader(scope);
    AnimaXCompositionNativeCallback callback = new AnimaXCompositionNativeCallback(listener);
    try {
      nativeLoadJson(scope.toString(), json, AnimaX.inst().getDensity(), callback);
    } catch (Exception e) {
      listener.onCompositionFailed("loadJson fail, message:" + e.getMessage());
    }
  }

  /**
   * Loads an animation from JSON string in the default service scope.
   *
   * @param json The JSON string containing animation data
   * @param listener Callback listener for composition loading events
   */
  public void loadJson(@NonNull String json, @NonNull AnimaXCompositionListener listener) {
    loadJson(ServiceScope.DEFAULT, json, listener);
  }

  /**
   * Loads an animation from URI in the specified service scope.
   *
   * @param scope The service scope to load the animation in
   * @param uri The URI pointing to the animation resource
   * @param listener Callback listener for composition loading events
   */
  public void loadUri(@NonNull ServiceScope scope, @NonNull String uri,
      @NonNull AnimaXCompositionListener listener) {
    ensureLoader(scope);
    AnimaXCompositionNativeCallback callback = new AnimaXCompositionNativeCallback(listener);
    try {
      nativeLoadUri(scope.toString(), uri, AnimaX.inst().getDensity(), callback);
    } catch (Exception e) {
      listener.onCompositionFailed("loadUri fail, message:" + e.getMessage());
    }
  }

  /**
   * Loads an animation from URI in the default service scope.
   *
   * @param uri The URI pointing to the animation resource
   * @param listener Callback listener for composition loading events
   */
  public void loadUri(@NonNull String uri, @NonNull AnimaXCompositionListener listener) {
    loadUri(ServiceScope.DEFAULT, uri, listener);
  }

  /**
   * Releases all resources held by this factory.
   * Clears all loaders in all service scopes.
   */
  public void release() {
    for (Map.Entry<ServiceScope, AnimaXResourceLoaderHolder> entry : mLoaderMap.entrySet()) {
      AnimaXResourceLoaderHolder loader = entry.getValue();
      if (loader != null) {
        loader.release();
      }
    }
    mLoaderMap.clear();
    nativeRelease();
  }

  /**
   * Ensures a loader exists for the specified service scope.
   * If no loader exists for the given scope, creates a new one.
   *
   * @param scope The service scope to ensure loader for
   */
  private void ensureLoader(@NonNull ServiceScope scope) {
    AnimaXResourceLoaderHolder loader = mLoaderMap.get(scope);
    if (loader == null) {
      loader = new AnimaXResourceLoaderHolder(scope);
      // Create loader in related service scope
      nativeCreate(scope.toString(), loader.getNativePtr());
      mLoaderMap.put(scope, loader);
    }
  }

  // Native method declarations
  private native void nativeCreate(String scopeName, long holderPtr);
  private native void nativeRelease();
  private native void nativeLoadJson(
      String scopeName, String json, float density, AnimaXCompositionNativeCallback callback);
  private native void nativeLoadUri(
      String scopeName, String uri, float density, AnimaXCompositionNativeCallback callback);
}
