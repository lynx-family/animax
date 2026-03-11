// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.animax.service;

import androidx.annotation.Keep;
import com.lynx.animax.loader.IAnimaXLoaderCompletionHandler;
import com.lynx.animax.loader.IAnimaXLoaderRequest;

/**
 * Service interface for managing image loading operations.
 * Provides methods to load images from various sources.
 */
@Keep
public interface IAnimaXImageService extends IAnimaXService {
  /**
   * Loads an image based on the provided request.
   *
   * @param request The image loading request containing URI and parameters
   * @param completionHandler The callback to be invoked when the loading operation completes
   * @return true if the image loading was handled, false otherwise
   */
  boolean loadImage(IAnimaXLoaderRequest request, IAnimaXLoaderCompletionHandler completionHandler);
}
