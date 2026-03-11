// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.animax.service;

import com.lynx.animax.setting.AnimaXSettingValue;

/**
 * Service interface for managing AnimaX settings and configurations.
 * Provides methods to access various device and feature settings.
 */
public interface IAnimaXSettingService extends IAnimaXService {
  /**
   * Gets a setting value for the specified key.
   * The value can be either a string or a collection of strings.
   *
   * @param key The settings key to retrieve
   * @return A AnimaXSettingValue instance containing either a string or collection value.
   *         Returns an empty AnimaXSettingValue if the key is not found or an error occurs.
   */
  AnimaXSettingValue getValueByKey(String key);
}
