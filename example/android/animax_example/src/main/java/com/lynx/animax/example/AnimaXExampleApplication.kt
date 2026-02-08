// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.example

import android.app.Application
import com.facebook.drawee.backends.pipeline.Fresco
import com.facebook.imagepipeline.core.ImagePipelineConfig
import com.facebook.imagepipeline.decoder.ImageDecoderConfig
import com.facebook.imagepipeline.listener.RequestListener
import com.facebook.imagepipeline.memory.PoolConfig
import com.facebook.imagepipeline.memory.PoolFactory
import com.lynx.animax.example.loaders.AnimaXFrescoImageService
import com.lynx.animax.example.loaders.HttpLoader
import com.lynx.animax.loader.IAnimaXLoader
import com.lynx.animax.service.AnimaXServiceCenter
import com.lynx.animax.service.IAnimaXImageService
import com.lynx.animax.service.IAnimaXResourceFactoryService
import com.lynx.animax.util.AnimaX

class AnimaXExampleApplication : Application() {

    companion object {
        private const val TAG = "AnimaXExampleApplication"
    }

    override fun onCreate() {
        super.onCreate()

        // Initialize Fresco for image loading
        val listeners = HashSet<RequestListener>()
        val factory = PoolFactory(PoolConfig.newBuilder().build())
        val builder = ImagePipelineConfig.newBuilder(this)
            .setRequestListeners(listeners)
            .setPoolFactory(factory)
            .setDownsampleEnabled(true)
            .setImageDecoderConfig(ImageDecoderConfig.newBuilder().build())
        Fresco.initialize(this, builder.build())

        // Register resource factory service with loaders
        AnimaXServiceCenter.inst().registerService(
            IAnimaXResourceFactoryService::class.java,
            object : IAnimaXResourceFactoryService {
                override fun createAnimaXLoaders(): List<IAnimaXLoader> {
                    // Create HTTP loader
                    return listOf(HttpLoader())
                }
            }
        )

        // Register Fresco image service
        AnimaXServiceCenter.inst().registerService(
            IAnimaXImageService::class.java,
            AnimaXFrescoImageService()
        )

        // Initialize AnimaX
        AnimaX.inst().init(this)
    }
}
