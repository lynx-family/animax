// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.loader;

import android.net.Uri;
import com.lynx.animax.service.AnimaXServiceCenter;
import com.lynx.animax.service.IAnimaXImageService;
import com.lynx.animax.util.ResourceUtil;
import com.lynx.animax.util.UriUtil;

public class AnimaXFileLoader implements IAnimaXLoader {
  @Override
  public void load(IAnimaXLoaderRequest request, IAnimaXLoaderCompletionHandler completionHandler) {
    IAnimaXImageService imageService =
        AnimaXServiceCenter.inst().getService(IAnimaXImageService.class);
    if (imageService != null && imageService.loadImage(request, completionHandler)) {
      return;
    }

    // Fallback to direct file loading if service is not available
    Uri uri = UriUtil.safeParse(request.getUri());
    byte[] data = ResourceUtil.getByteArrayFromFile(UriUtil.getLocalFileName(uri));
    AnimaXLoaderResponse<?> response = null;
    if (data != null) {
      response = AnimaXLoaderResponse.createByteArrayResponse(data);
    } else {
      response = AnimaXLoaderResponse.createErrorResponse(
          new Throwable("load file failed, getByteArrayFromFile returned null."));
    }
    completionHandler.onComplete(response);
  }

  @Override
  public AnimaXLoaderScheme getScheme() {
    return AnimaXLoaderScheme.FILE;
  }
}
