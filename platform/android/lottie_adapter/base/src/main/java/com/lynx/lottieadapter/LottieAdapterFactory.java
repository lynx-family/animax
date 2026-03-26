// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.lottieadapter;

import android.content.Context;
import androidx.annotation.NonNull;
import androidx.annotation.RestrictTo;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

public class LottieAdapterFactory {
  private static final LottieAdapterOption sGlobalOption = new LottieAdapterOption();
  private static final Map<AnimationType, ILottieAdapterCreator> sCreatorMap =
      new ConcurrentHashMap<>();
  private static volatile boolean sInitialized = false;

  public static void setDefaultType(@NonNull AnimationType type) {
    sGlobalOption.setType(type);
  }

  public static AnimationType getDefaultType() {
    return sGlobalOption.getType();
  }

  public static void setGlobalOption(@NonNull LottieAdapterOption globalOption) {
    sGlobalOption.copyFromOption(globalOption);
  }

  public static LottieAdapterOption getGlobalOption() {
    return new LottieAdapterOption(sGlobalOption);
  }

  public static void registerCreator(
      @NonNull AnimationType type, @NonNull ILottieAdapterCreator creator) {
    sCreatorMap.put(type, creator);
  }

  public static void unregisterCreator(@NonNull AnimationType type) {
    sCreatorMap.remove(type);
  }

  @NonNull
  @RestrictTo(RestrictTo.Scope.LIBRARY)
  public static ILottieAdapter create(@NonNull Context context) {
    return create(context, sGlobalOption);
  }

  @NonNull
  @RestrictTo(RestrictTo.Scope.LIBRARY)
  public static ILottieAdapter create(
      @NonNull Context context, @NonNull LottieAdapterOption option) {
    if (!sInitialized) {
      synchronized (LottieAdapterFactory.class) {
        if (!sInitialized) {
          reflectRegisterCreator(AnimationType.LOTTIE);
          reflectRegisterCreator(AnimationType.ANIMAX);
          sInitialized = true;
        }
      }
    }
    ILottieAdapterCreator creator = sCreatorMap.get(option.getType());
    if (creator != null) {
      ILottieAdapter adapter = creator.create(context);
      adapter.setOption(option);
      return adapter;
    }
    return new EmptyLottieAdapter(context);
  }

  private static void reflectRegisterCreator(@NonNull AnimationType type) {
    String className;
    switch (type) {
      case LOTTIE:
        className = "com.lynx.lottieadapter.LottieAdapterLottie";
        break;
      case ANIMAX:
        className = "com.lynx.lottieadapter.LottieAdapterAnimaX";
        break;
      default:
        return;
    }
    try {
      final Class<?> clazz = Class.forName(className);
      registerCreator(type, new ILottieAdapterCreator() {
        private final Class<?> mAdapterClass = clazz;
        @NonNull
        @Override
        public ILottieAdapter create(@NonNull Context context) {
          try {
            return (ILottieAdapter) mAdapterClass.getConstructor(Context.class)
                .newInstance(context);
          } catch (Exception e) {
            throw new RuntimeException(
                "Failed to create adapter instance for " + mAdapterClass.getName(), e);
          }
        }
      });
    } catch (ClassNotFoundException e) {
      // do nothing, just ignore it
    }
  }
}
