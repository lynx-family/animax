// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.animax.loader;

import androidx.annotation.Nullable;
import java.util.Map;

public interface IAnimaXLoaderRequest {
  interface IImageInfo {
    int getWidth();
    int getHeight();
  }

  String getUri();
  Map<String, Object> getParams();

  @Nullable IImageInfo getImageInfo();
}
