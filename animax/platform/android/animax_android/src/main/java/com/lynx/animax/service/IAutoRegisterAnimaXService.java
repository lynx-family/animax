// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.animax.service;

import androidx.annotation.Keep;
import androidx.annotation.NonNull;

/**
 * Interface for AnimaX services that should be automatically registered on AnimaXServiceCenter.
 * Services that implement this interface will be automatically discovered and registered
 * by the AnimaXServiceCenter during initialization using ServiceLoader.
 *
 * Example usage:
 * {@code
 * @AutoService(IAutoRegisterAnimaXService.class)
 * public class MyCustomService implements IAnimaXCustomService, IAutoRegisterAnimaXService {
 *     @Override
 *     public Class<? extends IAnimaXService> getServiceClass() {
 *         return IAnimaXCustomService.class;
 *     }
 *
 *     @Override
 *     public ServiceScope getServiceScope() {
 *         return ServiceScope.CUSTOM;
 *     }
 *
 *     // Service implementation
 * }
 *
 * @see AnimaXServiceCenter
 * @see IAnimaXService
 */
@Keep
public interface IAutoRegisterAnimaXService extends IAnimaXService {
  @NonNull Class<? extends IAnimaXService> getServiceClass();

  @NonNull
  default ServiceScope getAutoRegisterServiceScope() {
    return ServiceScope.DEFAULT;
  }
}
