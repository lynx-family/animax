// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.example

import com.lynx.animax.util.UriUtil

object LottieFiles {
    val SimpleShape = "simple_shape.json"
    val SimpleShapeSchema = UriUtil.fromLocalAsset(SimpleShape)
    val DynamicProperty = "dp/data.json"
    val DynamicPropertySchema = UriUtil.fromLocalAsset(DynamicProperty)
}