// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.drawable;

import android.graphics.Bitmap;
import android.graphics.ColorSpace;
import android.hardware.HardwareBuffer;
import android.os.Build;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.RequiresApi;
import androidx.annotation.RestrictTo;
import com.lynx.animax.base.CalledByNative;
import com.lynx.animax.util.AnimaXLog;
import java.util.concurrent.atomic.AtomicReference;

/**
 * A wrapper class that encapsulates a {@link Bitmap} and its optional, underlying
 * {@link HardwareBuffer}. This class simplifies buffer management by automatically choosing
 * the best creation strategy: it attempts to create an efficient, hardware-accelerated buffer
 * on supported Android versions, and gracefully falls back to a standard software bitmap if
 * hardware acceleration is unavailable or disabled. Instances must be created via the static
 *  factory method {@link #create(int, int, boolean)}, which returns {@code null} on
 * creation failure. The user of this class is responsible for calling {@link #release()} when the
 * object is no longer needed to prevent memory leaks.
 *
 * <p><b>Hybrid Buffer Strategy:</b>
 * To optimize for performance, this class uses a hybrid approach depending on the Android API
 * level: <ul>
 *   <li><b>API 29+ (Q+):</b> Utilizes {@link HardwareBuffer} wrapped by a {@link Bitmap} via
 *       {@code Bitmap.wrapHardwareBuffer()}. This is the most efficient mode, enabling true
 *       hardware-accelerated rendering with zero-copy memory sharing.</li>
 *   <li><b>API 26-28 (Oreo, Pie):</b> For software rendering, it performs exactly like API 26-,
 *       since an extra copy to {@link HardwareBuffer} is unnecessary.
 *       For hardware rendering(GL), still uses {@link HardwareBuffer} for native rendering, but
 *       since it cannot be directly wrapped by a {@link Bitmap}, a separate standard {@link Bitmap}
 *       is created. This mode is less efficient than API 29+ as it may involve data copies between
 *       the hardware buffer and the bitmap.</li>
 *   <li><b>API 26-:</b> Falls back to using only standard {@link Bitmap} objects for both
 *       rendering and display, ensuring broad compatibility at the cost of performance.</li>
 * </ul>
 */
@RestrictTo(RestrictTo.Scope.LIBRARY)
public class BitmapBuffer {
  private static final String TAG = "BitmapBuffer";
  private final @NonNull Bitmap mBitmap;
  private final @Nullable HardwareBuffer mHardwareBuffer;

  public enum BufferState { NotReady, Prepare, Ready, Destroyed }
  private final AtomicReference<BufferState> mBufferState =
      new AtomicReference<>(BufferState.NotReady);

  private BitmapBuffer(@NonNull Bitmap bitmap, @Nullable HardwareBuffer hardwareBuffer) {
    this.mBitmap = bitmap;
    this.mHardwareBuffer = hardwareBuffer;
  }

  @NonNull
  public static BitmapBuffer create(int width, int height, boolean enableSoftwareRendering) {
    Bitmap bitmap = null;
    HardwareBuffer hardwareBuffer = null;
    if (isHardwareBufferSupported(enableSoftwareRendering)) {
      hardwareBuffer = createHardwareBuffer(width, height, enableSoftwareRendering);
    }
    if (isBitmapWrappedSupported() && hardwareBuffer != null) {
      try {
        bitmap = Bitmap.wrapHardwareBuffer(hardwareBuffer, ColorSpace.get(ColorSpace.Named.SRGB));
      } catch (Exception e) {
        AnimaXLog.e(TAG, "Failed to wrap HardwareBuffer: " + e.getMessage());
        hardwareBuffer.close();
        hardwareBuffer = null;
      }
    }
    if (bitmap == null) {
      bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
    }
    return new BitmapBuffer(bitmap, hardwareBuffer);
  }

  @Nullable
  @CalledByNative
  public Bitmap getBitmap() {
    if (mBufferState.get() == BufferState.Destroyed) {
      return null;
    }
    // The UI thread may release this BitmapBuffer at any time; native code must check the lock
    // result for safety.
    return mBitmap;
  }

  @Nullable
  @CalledByNative
  public HardwareBuffer getHardwareBuffer() {
    if (mBufferState.get() == BufferState.Destroyed) {
      return null;
    }
    // The UI thread may release this BitmapBuffer at any time; native code must check the lock
    // result for safety.
    return mHardwareBuffer;
  }

  @CalledByNative
  public void setState(int state) {
    if (state < 0 || state >= BufferState.values().length) {
      return;
    }
    BufferState oldState = mBufferState.getAndSet(BufferState.values()[state]);
    if (oldState == BufferState.Destroyed) {
      release();
    }
  }

  public void setState(BufferState state) {
    mBufferState.set(state);
  }

  public BufferState getState() {
    return mBufferState.get();
  }

  public void release() {
    BufferState oldState = mBufferState.getAndSet(BufferState.Destroyed);
    if (oldState == BufferState.Destroyed || oldState == BufferState.Prepare) {
      return;
    }
    if (!mBitmap.isRecycled()) {
      mBitmap.recycle();
    }
    if (mHardwareBuffer != null && Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
      mHardwareBuffer.close();
    }
  }

  public static boolean isHardwareBufferSupported(boolean enableSoftwareRendering) {
    return Build.VERSION.SDK_INT >= Build.VERSION_CODES.O
        && (!enableSoftwareRendering || isBitmapWrappedSupported());
  }

  public static boolean isBitmapWrappedSupported() {
    return Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q;
  }

  @RequiresApi(api = Build.VERSION_CODES.O)
  private static HardwareBuffer createHardwareBuffer(
      int width, int height, boolean enableSoftwareRendering) {
    if (width <= 0 || height <= 0) {
      return null;
    }
    long usage;
    if (enableSoftwareRendering) {
      usage = HardwareBuffer.USAGE_CPU_WRITE_OFTEN | HardwareBuffer.USAGE_GPU_SAMPLED_IMAGE;
    } else if (isBitmapWrappedSupported()) {
      usage = HardwareBuffer.USAGE_GPU_COLOR_OUTPUT | HardwareBuffer.USAGE_GPU_SAMPLED_IMAGE;
    } else {
      usage = HardwareBuffer.USAGE_GPU_COLOR_OUTPUT | HardwareBuffer.USAGE_CPU_READ_OFTEN;
    }
    try {
      return HardwareBuffer.create(width, height, HardwareBuffer.RGBA_8888, 1, usage);
    } catch (Exception e) {
      AnimaXLog.e(TAG, "Failed to create HardwareBuffer: " + e.getMessage());
      return null;
    }
  }
}
