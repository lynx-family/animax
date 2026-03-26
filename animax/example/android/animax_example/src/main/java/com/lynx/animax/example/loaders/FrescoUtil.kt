// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.example.loaders

import android.graphics.Bitmap
import android.net.Uri
import com.facebook.common.executors.CallerThreadExecutor
import com.facebook.common.references.CloseableReference
import com.facebook.datasource.DataSource
import com.facebook.drawee.backends.pipeline.Fresco
import com.facebook.imagepipeline.common.ImageDecodeOptions
import com.facebook.imagepipeline.common.Priority
import com.facebook.imagepipeline.common.ResizeOptions
import com.facebook.imagepipeline.datasource.BaseBitmapReferenceDataSubscriber
import com.facebook.imagepipeline.image.CloseableImage
import com.facebook.imagepipeline.request.ImageRequest
import com.facebook.imagepipeline.request.ImageRequestBuilder
import com.lynx.animax.loader.AnimaXLoaderResponse
import com.lynx.animax.loader.IAnimaXCloseableBitmapReference
import com.lynx.animax.loader.IAnimaXLoaderCompletionHandler
import com.lynx.animax.loader.IAnimaXLoaderRequest

import com.lynx.animax.util.UriUtil

object FrescoUtil {
    private const val TAG = "FrescoUtil"


    interface FrescoBitmapCompletionHandler {
        fun onComplete(bitmapReference: IAnimaXCloseableBitmapReference?, errorMessage: String)
    }

    fun tryHandleLoaderRequestWithFresco(
        request: IAnimaXLoaderRequest,
        completionHandler: IAnimaXLoaderCompletionHandler
    ): Boolean {
        val imageInfo = request.imageInfo ?: return false
        val uri = UriUtil.safeParse(request.uri)
        
        fetchBitmapUriWithFresco(uri, imageInfo.width, imageInfo.height, 
            object : FrescoBitmapCompletionHandler {
                override fun onComplete(bitmapReference: IAnimaXCloseableBitmapReference?, errorMessage: String) {
                    val response = if (bitmapReference == null || errorMessage.isNotEmpty()) {
                        AnimaXLoaderResponse.createErrorResponse(Throwable(errorMessage))
                    } else {
                        AnimaXLoaderResponse.createBitmapResponse(bitmapReference)
                    }
                    completionHandler.onComplete(response)
                }
            }
        )
        return true
    }

    fun fetchBitmapUriWithFresco(
        imageUri: Uri,
        width: Int,
        height: Int,
        bitmapCompletionHandler: FrescoBitmapCompletionHandler
    ) {
        val requestBuilder = ImageRequestBuilder
            .newBuilderWithSource(imageUri)
            .setImageDecodeOptions(
                ImageDecodeOptions.newBuilder()
                    .setBitmapConfig(Bitmap.Config.ARGB_8888)
                    .setForceStaticImage(true)
                    .build()
            )
            .setRequestPriority(Priority.HIGH)

        val request: ImageRequest = requestBuilder.build()
        val dataSource: DataSource<CloseableReference<CloseableImage>> =
            Fresco.getImagePipeline().fetchDecodedImage(request, TAG)

        dataSource.subscribe(object : BaseBitmapReferenceDataSubscriber() {
            override fun onNewResultImpl(bitmapReference: CloseableReference<Bitmap>?) {
                val bitmap = bitmapReference?.get()
                if (bitmap != null) {
                    bitmapCompletionHandler.onComplete(
                        FrescoCloseableBitmapReference(bitmapReference),
                        ""
                    )
                } else {
                    bitmapCompletionHandler.onComplete(null, "Fresco returned a null bitmap on success.")
                }
                dataSource.close()
            }

            override fun onFailureImpl(dataSource: DataSource<CloseableReference<CloseableImage?>?>) {
                var errorMessage = "Fresco failed to load bitmap."
                dataSource.failureCause?.let {
                    errorMessage += " reason: ${it.message}"
                }
                bitmapCompletionHandler.onComplete(null, errorMessage)
                dataSource.close()
            }
        }, CallerThreadExecutor.getInstance())
    }
}
