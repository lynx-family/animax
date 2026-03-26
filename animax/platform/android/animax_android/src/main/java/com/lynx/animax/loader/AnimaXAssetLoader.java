// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.loader;

import android.content.Context;
import android.content.res.AssetManager;
import android.net.Uri;
import com.lynx.animax.service.AnimaXServiceCenter;
import com.lynx.animax.service.IAnimaXImageService;
import com.lynx.animax.util.AnimaX;
import com.lynx.animax.util.ResourceUtil;
import com.lynx.animax.util.UriUtil;

public class AnimaXAssetLoader implements IAnimaXLoader {
  @Override
  public void load(IAnimaXLoaderRequest request, IAnimaXLoaderCompletionHandler completionHandler) {
    IAnimaXImageService imageService =
        AnimaXServiceCenter.inst().getService(IAnimaXImageService.class);
    if (imageService != null && imageService.loadImage(request, completionHandler)) {
      return;
    }

    // Fallback to direct asset loading if service is not available
    Uri uri = UriUtil.safeParse(request.getUri());
    Context context = AnimaX.inst().getAppContext();
    AssetManager assetManager = context != null ? context.getAssets() : null;
    byte[] data = null;
    if (assetManager != null) {
      data = ResourceUtil.getByteArrayFromAsset(UriUtil.getAssetName(uri), assetManager);
    }
    AnimaXLoaderResponse<?> response = null;
    if (data == null) {
      response = AnimaXLoaderResponse.createErrorResponse(new Throwable("failed to load asset"));
    } else {
      response = AnimaXLoaderResponse.createByteArrayResponse(data);
    }
    completionHandler.onComplete(response);
  }

  @Override
  public AnimaXLoaderScheme getScheme() {
    return AnimaXLoaderScheme.ASSET;
  }
}
