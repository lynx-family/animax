// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.animax.service;

import com.lynx.animax.loader.IAnimaXLoader;
import java.util.List;

/**
 * Interface defining a factory service for creating AnimaX resource loaders.
 * This service is responsible for instantiating and managing AnimaX resource loaders
 * that handle different types of animation resources.
 */
public interface IAnimaXResourceFactoryService extends IAnimaXService {
  /**
   * Creates and returns a list of AnimaX loaders.
   *
   * @return A list of IAnimaXLoader instances that can handle different types
   *         of animation resources and formats.
   */
  List<IAnimaXLoader> createAnimaXLoaders();
}
