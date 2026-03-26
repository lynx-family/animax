// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.loader;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.util.Base64;

public class AnimaXBase64Loader implements IAnimaXLoader {
  private static final String BASE64_PREFIX = "base64,";

  private static String getDataUrlBase64Content(String dataUrl) {
    return dataUrl.substring(dataUrl.indexOf(BASE64_PREFIX) + BASE64_PREFIX.length());
  }
  @Override
  public void load(IAnimaXLoaderRequest request, IAnimaXLoaderCompletionHandler completionHandler) {
    AnimaXLoaderResponse<?> response = null;
    try {
      String base64DataUrl = request.getUri();
      byte[] data = Base64.decode(getDataUrlBase64Content(base64DataUrl), Base64.DEFAULT);

      if (request.getImageInfo() != null) {
        Bitmap bitmap = BitmapFactory.decodeByteArray(data, 0, data.length);
        response =
            AnimaXLoaderResponse.createBitmapResponse(new SimpleCloseableBitmapReference(bitmap));
      } else {
        response = AnimaXLoaderResponse.createByteArrayResponse(data);
      }
    } catch (Exception e) {
      response = AnimaXLoaderResponse.createErrorResponse(e);
    } finally {
      completionHandler.onComplete(response);
    }
  }

  @Override
  public AnimaXLoaderScheme getScheme() {
    return AnimaXLoaderScheme.DATA_URL;
  }
}
