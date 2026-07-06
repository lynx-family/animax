// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.animax.ui;

import android.content.Context;
import androidx.annotation.NonNull;
import com.lynx.animax.ability.BaseAbility;
import com.lynx.animax.base.CalledByNative;
import com.lynx.animax.util.DeviceUtil;

/**
 * A context class that holds configuration and runtime information for AnimaX animations.
 */
public class AnimaXContext {
  @NonNull private final BaseAbility mAbility;
  @NonNull private final Context mContext;
  private final boolean mMultiThreadAccelerate;
  private float mDeviceDensity = 0.f;

  private AnimaXContext(Builder builder) {
    this.mAbility = builder.mAbility;
    this.mContext = builder.mContext;
    this.mMultiThreadAccelerate = builder.mMultiThreadAccelerate;
  }

  /**
   * Get the ability instance associated with this context.
   * @return The BaseAbility instance.
   */
  @CalledByNative
  public BaseAbility getAbility() {
    return mAbility;
  }

  public Context getContext() {
    return mContext;
  }

  /**
   * Check if multi-thread acceleration is enabled for this context.
   * @return True if multi-thread acceleration is enabled, false otherwise.
   */
  @CalledByNative
  public boolean multiThreadAccelerate() {
    return mMultiThreadAccelerate;
  }

  /**
   * Check if playback is disabled on asset load failure.
   * @return True if disabled, false otherwise.
   */
  @CalledByNative
  public boolean disablePlaybackOnAssetLoadFailure() {
    return DeviceUtil.disablePlaybackOnAssetLoadFailure(mAbility);
  }

  /**
   * Get the device density for this context.
   * @return The device density.
   */
  @CalledByNative
  public float getDeviceDensity() {
    if (mDeviceDensity <= 0.f) {
      mDeviceDensity = mContext.getResources().getDisplayMetrics().density;
    }
    return mDeviceDensity;
  }

  /**
   * Builder class for creating AnimaXContext instances using the builder pattern
   * for more controlled construction of the context.
   */
  public static class Builder {
    private final BaseAbility mAbility;
    private final Context mContext;
    private boolean mMultiThreadAccelerate = false;

    public Builder(@NonNull BaseAbility ability, @NonNull Context context) {
      this.mAbility = ability;
      this.mContext = context;
    }

    public Builder multiThreadAccelerate(boolean enable) {
      // Shader cache with multithread acceleration can crash on Huawei Android devices.
      mMultiThreadAccelerate = enable && !DeviceUtil.isHuaWei();
      return this;
    }

    public AnimaXContext build() {
      return new AnimaXContext(this);
    }
  }
}
