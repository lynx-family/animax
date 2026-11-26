// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.animax.loader;

import androidx.annotation.RestrictTo;
import com.lynx.animax.base.CalledByNative;

@RestrictTo(RestrictTo.Scope.LIBRARY)
public class AnimaXLoader {
  private final IAnimaXLoader mLoader;
  private final AnimaXResourceLoadInfo mLoadInfo;

  AnimaXLoader(IAnimaXLoader loader) {
    this(loader, null);
  }

  AnimaXLoader(IAnimaXLoader loader, AnimaXResourceLoadInfo loadInfo) {
    mLoadInfo = loadInfo;
    this.mLoader = loader;
  }

  @CalledByNative
  public static AnimaXLoader create(IAnimaXLoader loader) {
    return new AnimaXLoader(loader);
  }

  @CalledByNative
  public void loadWithType(AnimaXLoaderRequest request,
      AnimaXLoaderCompletionHandler completionHandler, int contentType) {
    // Exact-size images go through load() so they are resized; they are not reported yet.
    if (mLoadInfo == null || request.requiresExactImageSize()) {
      load(request, completionHandler);
      return;
    }
    mLoadInfo.load(mLoader, request, completionHandler, contentType);
  }

  @CalledByNative
  public void load(AnimaXLoaderRequest request, AnimaXLoaderCompletionHandler completionHandler) {
    IAnimaXLoaderRequest.IImageInfo imageInfo = request.getImageInfo();
    if (imageInfo == null || !request.requiresExactImageSize()) {
      mLoader.load(request, completionHandler);
      return;
    }
    mLoader.load(request, response -> {
      if (response.getType() == AnimaXLoaderResponse.Type.BITMAP) {
        response = AnimaXLoaderResponse.createBitmapResponse(
            new ExactSizeBitmapReference((IAnimaXCloseableBitmapReference) response.getData(),
                imageInfo.getWidth(), imageInfo.getHeight()));
      }
      completionHandler.onComplete(response);
    });
  }
}
