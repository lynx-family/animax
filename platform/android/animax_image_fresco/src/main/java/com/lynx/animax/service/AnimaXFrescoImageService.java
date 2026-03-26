// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.animax.service;

import androidx.annotation.Keep;
import androidx.annotation.NonNull;
import com.google.auto.service.AutoService;
import com.lynx.animax.loader.FrescoUtil;
import com.lynx.animax.loader.IAnimaXLoaderCompletionHandler;
import com.lynx.animax.loader.IAnimaXLoaderRequest;
import com.lynx.animax.service.IAutoRegisterAnimaXService;

/**
 * Implementation of IAnimaXImageService using Fresco for image loading.
 */
@Keep
@AutoService(IAutoRegisterAnimaXService.class)
public class AnimaXFrescoImageService implements IAnimaXImageService, IAutoRegisterAnimaXService {
  /**
   * Loads an image using Fresco based on the provided request.
   *
   * @param request The image loading request containing URI and parameters
   * @param completionHandler The callback to be invoked when the loading operation completes
   * @return true if the image loading was handled, false otherwise
   */
  @Override
  public boolean loadImage(
      IAnimaXLoaderRequest request, IAnimaXLoaderCompletionHandler completionHandler) {
    return FrescoUtil.tryHandleLoaderRequestWithFresco(request, completionHandler);
  }

  @Override
  @NonNull
  public Class<? extends IAnimaXService> getServiceClass() {
    return IAnimaXImageService.class;
  }
}
