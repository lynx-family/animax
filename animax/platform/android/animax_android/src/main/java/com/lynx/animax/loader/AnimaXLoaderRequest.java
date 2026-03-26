// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.animax.loader;

import android.net.Uri;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.RestrictTo;
import com.lynx.animax.base.CalledByNative;
import com.lynx.animax.base.bridge.JavaOnlyMap;
import com.lynx.animax.util.UriUtil;
import java.util.Map;

@RestrictTo(RestrictTo.Scope.LIBRARY)
public class AnimaXLoaderRequest implements IAnimaXLoaderRequest {
  private final static String PARAMS_IMAGE_WIDTH_KEY = "image_width";
  private final static String PARAMS_IMAGE_HEIGHT_KEY = "image_height";
  private final static String PARAMS_SCHEME_KEY = "scheme";

  private final String mUri;

  private final AnimaXLoaderScheme mScheme;
  private final Map<String, Object> mParams;

  @NonNull
  public String getUri() {
    if (!isUriWithValidScheme(this.mUri)) {
      return createUriWithScheme(this.mUri, this.mScheme);
    }

    return this.mUri;
  }

  @Nullable
  public Map<String, Object> getParams() {
    return this.mParams;
  }

  public AnimaXLoaderRequest(String uri, Map<String, Object> params) {
    this.mUri = uri;
    this.mParams = params;
    if (null != mParams && null != mParams.get(PARAMS_SCHEME_KEY)) {
      this.mScheme = AnimaXLoaderScheme.values()[(int) mParams.get(PARAMS_SCHEME_KEY)];
    } else {
      this.mScheme = null;
    }
  }

  @CalledByNative
  static AnimaXLoaderRequest createRequestWithParams(String url, JavaOnlyMap params) {
    return new AnimaXLoaderRequest(url, params);
  }

  @CalledByNative
  static AnimaXLoaderRequest createRequest(String url) {
    return new AnimaXLoaderRequest(url, null);
  }

  @Override
  public @Nullable IImageInfo getImageInfo() {
    if (mParams != null) {
      Object width = mParams.get(PARAMS_IMAGE_WIDTH_KEY);
      Object height = mParams.get(PARAMS_IMAGE_HEIGHT_KEY);
      if (width instanceof Integer && height instanceof Integer) {
        return new IImageInfo() {
          @Override
          public int getWidth() {
            return (int) width;
          }

          @Override
          public int getHeight() {
            return (int) height;
          }
        };
      }
    }
    return null;
  }

  private static String createUriWithScheme(String uri, AnimaXLoaderScheme scheme) {
    if (scheme == null) {
      return uri;
    }

    if (scheme == AnimaXLoaderScheme.ASSET) {
      return UriUtil.fromLocalAsset(uri);
    } else if (scheme == AnimaXLoaderScheme.FILE) {
      return UriUtil.fromLocalFile(uri);
    }

    return uri;
  }

  private static boolean isUriWithValidScheme(String mUri) {
    Uri uri = UriUtil.safeParse(mUri);
    return UriUtil.isLocalAssetUri(uri) || UriUtil.isLocalFileUri(uri) || UriUtil.isNetworkUri(uri);
  }
}
