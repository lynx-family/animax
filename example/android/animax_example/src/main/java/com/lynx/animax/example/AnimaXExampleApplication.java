// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.example;

import android.app.Application;
import com.facebook.drawee.backends.pipeline.Fresco;
import com.facebook.imagepipeline.core.ImagePipelineConfig;
import com.facebook.imagepipeline.decoder.ImageDecoderConfig;
import com.facebook.imagepipeline.listener.RequestListener;
import com.facebook.imagepipeline.memory.PoolConfig;
import com.facebook.imagepipeline.memory.PoolFactory;
import com.lynx.animax.util.AnimaX;
import java.util.HashSet;
import java.util.Set;

public class AnimaXExampleApplication extends Application {
  private static final String TAG = "AnimaXExampleApplication";

  @Override
  public void onCreate() {
    super.onCreate();

    Set<RequestListener> listeners = new HashSet<>();
    final PoolFactory factory = new PoolFactory(PoolConfig.newBuilder().build());
    ImagePipelineConfig.Builder builder =
        ImagePipelineConfig.newBuilder(this)
            .setRequestListeners(listeners)
            .setPoolFactory(factory)
            .setDownsampleEnabled(true)
            .setImageDecoderConfig(ImageDecoderConfig.newBuilder().build());
    Fresco.initialize(this, builder.build());

    AnimaX.inst().init(this);
  }
}
