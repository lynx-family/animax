// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.animax.loader;

import androidx.annotation.RestrictTo;
import com.lynx.animax.base.CalledByNative;

@RestrictTo(RestrictTo.Scope.LIBRARY)
public class AnimaXLoader {
  private final IAnimaXLoader mLoader;
  AnimaXLoader(IAnimaXLoader loader) {
    this.mLoader = loader;
  }

  @CalledByNative
  public static AnimaXLoader create(IAnimaXLoader loader) {
    return new AnimaXLoader(loader);
  }

  @CalledByNative
  public void load(AnimaXLoaderRequest request, AnimaXLoaderCompletionHandler completionHandler) {
    mLoader.load(request, completionHandler);
  }
}
