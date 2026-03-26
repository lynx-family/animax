// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.example.loaders

import com.lynx.animax.loader.IAnimaXLoaderCompletionHandler
import com.lynx.animax.loader.IAnimaXLoaderRequest
import com.lynx.animax.service.IAnimaXImageService

/**
 * Implementation of IAnimaXImageService using Fresco for image loading.
 */
class AnimaXFrescoImageService : IAnimaXImageService {

    /**
     * Loads an image using Fresco based on the provided request.
     *
     * @param request The image loading request containing URI and parameters
     * @param completionHandler The callback to be invoked when the loading operation completes
     * @return true if the image loading was handled, false otherwise
     */
    override fun loadImage(
        request: IAnimaXLoaderRequest,
        completionHandler: IAnimaXLoaderCompletionHandler
    ): Boolean {
        return FrescoUtil.tryHandleLoaderRequestWithFresco(request, completionHandler)
    }
}
