// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.animax.service;

import androidx.annotation.NonNull;

/**
 * Defines the scope of services in the AnimaX service system.
 * This class allows both predefined and custom service scopes.
 *
 * Custom scopes can be created using {@link #of(String)}.
 *
 * @see AnimaXServiceCenter
 * @see ServiceRegistry
 */
public final class ServiceScope {
  private final String value;

  /**
   * Default scope for public services.
   */
  public static final ServiceScope DEFAULT = new ServiceScope("default");

  /**
   * Creates a new ServiceScope with the specified value.
   * The value will be converted to lowercase to ensure consistent comparison.
   *
   * @param value The scope value
   * @return A new ServiceScope instance
   */
  public static ServiceScope of(@NonNull String value) {
    return new ServiceScope(value);
  }

  private ServiceScope(String value) {
    this.value = value.toLowerCase();
  }

  @Override
  public boolean equals(Object o) {
    if (this == o)
      return true;
    if (!(o instanceof ServiceScope))
      return false;
    ServiceScope that = (ServiceScope) o;
    return value.equals(that.value);
  }

  @Override
  public int hashCode() {
    return value.hashCode();
  }

  @Override
  public String toString() {
    return value;
  }
}
