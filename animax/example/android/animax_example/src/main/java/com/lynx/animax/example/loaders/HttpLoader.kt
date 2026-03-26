// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.example.loaders

import com.lynx.animax.loader.AnimaXLoaderResponse
import com.lynx.animax.loader.AnimaXLoaderScheme
import com.lynx.animax.loader.IAnimaXLoader
import com.lynx.animax.loader.IAnimaXLoaderCompletionHandler
import com.lynx.animax.loader.IAnimaXLoaderRequest
import com.lynx.animax.service.AnimaXServiceCenter
import com.lynx.animax.service.IAnimaXImageService
import com.lynx.animax.util.AnimaXLog
import okhttp3.Call
import okhttp3.Callback
import okhttp3.OkHttpClient
import okhttp3.Request
import okhttp3.Response
import java.io.IOException
import java.util.concurrent.TimeUnit


/**
 * HTTP loader for AnimaX that uses OkHttp for network requests.
 * Falls back to Fresco for image loading when image info is present.
 */
class HttpLoader : IAnimaXLoader {

    companion object {
        private const val TAG = "HttpLoader"
        private const val CONNECT_TIMEOUT_SECONDS = 30L
        private const val READ_TIMEOUT_SECONDS = 30L
    }

    private val httpClient: OkHttpClient by lazy {
        OkHttpClient.Builder()
            .connectTimeout(CONNECT_TIMEOUT_SECONDS, TimeUnit.SECONDS)
            .readTimeout(READ_TIMEOUT_SECONDS, TimeUnit.SECONDS)
            .build()
    }

    override fun load(
        request: IAnimaXLoaderRequest,
        completionHandler: IAnimaXLoaderCompletionHandler
    ) {
        val imageService =
            AnimaXServiceCenter.inst()
                .getService<IAnimaXImageService?>(IAnimaXImageService::class.java)
        if (imageService != null && imageService.loadImage(request, completionHandler)) {
            return
        }

        val uri = request.uri
        // Otherwise, use OkHttp for raw data loading
        val httpRequest = Request.Builder()
            .url(uri)
            .build()

        httpClient.newCall(httpRequest).enqueue(object : Callback {
            override fun onFailure(call: Call, e: IOException) {
                AnimaXLog.e(TAG, "HTTP request failed: ${e.message}")
                completionHandler.onComplete(
                    AnimaXLoaderResponse.createErrorResponse(e)
                )
            }

            override fun onResponse(call: Call, response: Response) {
                if (!response.isSuccessful) {
                    AnimaXLog.e(TAG, "HTTP response not successful: ${response.code}")
                    completionHandler.onComplete(
                        AnimaXLoaderResponse.createErrorResponse(
                            Throwable("HTTP error: ${response.code}")
                        )
                    )
                    return
                }

                try {
                    val bytes = response.body?.bytes()
                    if (bytes != null) {
                        completionHandler.onComplete(
                            AnimaXLoaderResponse.createByteArrayResponse(bytes)
                        )
                    } else {
                        completionHandler.onComplete(
                            AnimaXLoaderResponse.createErrorResponse(
                                Throwable("Empty response body")
                            )
                        )
                    }
                } catch (e: Exception) {
                    AnimaXLog.e(TAG, "Error reading response: ${e.message}")
                    completionHandler.onComplete(
                        AnimaXLoaderResponse.createErrorResponse(e)
                    )
                }
            }
        })
    }

    override fun getScheme(): AnimaXLoaderScheme {
        return AnimaXLoaderScheme.HTTP
    }
}
