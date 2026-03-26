// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.example;

import android.app.Application;
import android.content.Context;
import androidx.annotation.NonNull;
import com.bytedance.fresco.heif.HeifDecoder;
import com.facebook.drawee.backends.pipeline.Fresco;
import com.facebook.imagepipeline.core.ImagePipelineConfig;
import com.facebook.imagepipeline.decoder.ImageDecoderConfig;
import com.facebook.imagepipeline.listener.RequestListener;
import com.facebook.imagepipeline.memory.PoolConfig;
import com.facebook.imagepipeline.memory.PoolFactory;
import com.lynx.animax.util.AnimaX;
import com.lynx.animax.util.AnimaXLog;
import com.lynx.lottieadapter.AnimationType;
import com.lynx.lottieadapter.ILottieAdapter;
import com.lynx.lottieadapter.ILottieAdapterCreator;
import com.lynx.lottieadapter.LottieAdapterAnimaX;
import com.lynx.lottieadapter.LottieAdapterFactory;
import com.lynx.lottieadapter.LottieAdapterLottie;
import com.optimize.statistics.FrescoTraceListener;
import java.util.HashSet;
import java.util.Set;

public class AnimaXExampleApplication extends Application {
  private static final String TAG = "AnimaXExampleApplication";

  @Override
  public void onCreate() {
    super.onCreate();

    Set<RequestListener> listeners = new HashSet<>();
    listeners.add(new FrescoTraceListener());
    final PoolFactory factory = new PoolFactory(PoolConfig.newBuilder().build());
    ImagePipelineConfig.Builder builder =
        ImagePipelineConfig.newBuilder(this)
            .setRequestListeners(listeners)
            .setPoolFactory(factory)
            .setDownsampleEnabled(true)
            .setImageDecoderConfig(
                ImageDecoderConfig.newBuilder()
                    .addDecodingCapability(HeifDecoder.HEIF_FORMAT,
                        new HeifDecoder.HeifFormatChecker(),
                        new HeifDecoder.HeifFormatDecoder(factory.getPooledByteBufferFactory()))
                    .build());
    Fresco.initialize(this, builder.build());

    AnimaXLog.i(TAG, "onCreate");

    AnimaX.inst().init(this);
    LottieAdapterFactory.registerCreator(AnimationType.LOTTIE, new ILottieAdapterCreator() {
      @NonNull
      @Override
      public ILottieAdapter create(@NonNull Context context) {
        return new LottieAdapterLottie(context);
      }
    });

    LottieAdapterFactory.registerCreator(AnimationType.ANIMAX, new ILottieAdapterCreator() {
      @NonNull
      @Override
      public ILottieAdapter create(@NonNull Context context) {
        return new LottieAdapterAnimaX(context);
      }
    });
  }
}
