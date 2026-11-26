// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.animax.loader;

import androidx.annotation.RestrictTo;
import java.util.Map;

public interface IAnimaXLoader {
  // Only for XElement built against older SDKs. Remove once it uses ResourceLoadReporter.
  @RestrictTo(RestrictTo.Scope.LIBRARY_GROUP)
  default void loadWithCacheSource(IAnimaXLoaderRequest request,
      IAnimaXLoaderCompletionHandler completion, Map<AnimaXLoaderResponse<?>, String> sources) {
    load(request, completion);
  }

  void load(IAnimaXLoaderRequest request, IAnimaXLoaderCompletionHandler completionHandler);

  AnimaXLoaderScheme getScheme();
}
