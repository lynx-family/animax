// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.animax.service;

import androidx.annotation.Keep;

/**
 * Base interface for all AnimaX services.
 * Any class that needs to be managed by the AnimaX service system must implement this interface.
 *
 * Example usage:
 * {@code
 * public class MyService implements IAnimaXService {
 *     // Service implementation
 * }
 *
 * @see AnimaXServiceCenter
 * @see ServiceRegistry
 */
@Keep
public interface IAnimaXService {}
