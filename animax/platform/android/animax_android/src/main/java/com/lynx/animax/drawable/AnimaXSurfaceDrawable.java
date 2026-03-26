// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.drawable;

import android.graphics.SurfaceTexture;
import android.view.Surface;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import com.lynx.animax.base.CalledByNative;
import com.lynx.animax.util.AnimaXLog;

/**
 * A wrapper class that encapsulates different surface types used in AnimaX rendering.
 * It supports three rendering scenarios:
 * 1. TextureView with Surface and SurfaceTexture
 * 2. SurfaceView or direct Surface
 * 3. BitmapBufferGroup with multiple BitmapBuffer
 */
public class AnimaXSurfaceDrawable {
  private static final String TAG = "AnimaXSurfaceDrawable";
  public enum DrawableType { Invalid, TextureView, SurfaceView, BitmapBufferGroup }

  private final @Nullable Surface mSurface;
  private final @Nullable SurfaceTexture mSurfaceTexture;
  private final @Nullable BitmapBufferGroup mBitmapBufferGroup;
  private final DrawableType mType;

  private int mWidth;
  private int mHeight;

  private boolean mEnableAntiAliasing = false;
  private boolean mEnableSoftwareRender = false;
  private boolean mIsPlatformSurfaceInitiallyInvalid = false;
  private boolean mEnableAutoDestroyEGLContext = false;

  private AnimaXSurfaceDrawable(@Nullable Surface surface, @Nullable SurfaceTexture texture,
      @Nullable BitmapBufferGroup buffer, int width, int height) {
    mSurface = surface;
    mSurfaceTexture = texture;
    mBitmapBufferGroup = buffer;

    if (mSurfaceTexture != null) {
      mType = DrawableType.TextureView;
    } else if (mBitmapBufferGroup != null) {
      mType = DrawableType.BitmapBufferGroup;
      mBitmapBufferGroup.setBufferSize(width, height);
    } else if (mSurface != null) {
      mType = DrawableType.SurfaceView;
    } else {
      mType = DrawableType.Invalid;
    }
    mWidth = width;
    mHeight = height;
  }

  /**
   * Creates an AnimaXSurfaceDrawable for TextureView scenario
   */
  public static AnimaXSurfaceDrawable createFromTexture(
      @NonNull SurfaceTexture texture, int width, int height) {
    return new AnimaXSurfaceDrawable(new Surface(texture), texture, null, width, height);
  }

  /**
   * Creates an AnimaXSurfaceDrawable for TextureView (FirstFrameAwareSurfaceTexture) scenario
   */
  public static AnimaXSurfaceDrawable createFromFirstFrameAwareSurfaceTexture(
      @NonNull FirstFrameAwareSurfaceTexture texture, int width, int height) {
    return new AnimaXSurfaceDrawable(texture.getSurface(), texture, null, width, height);
  }

  /**
   * Creates an AnimaXSurfaceDrawable for SurfaceView or direct Surface scenario
   */
  public static AnimaXSurfaceDrawable createFromSurface(
      @NonNull Surface surface, int width, int height) {
    return new AnimaXSurfaceDrawable(surface, null, null, width, height);
  }

  /**
   * Creates an AnimaXSurfaceDrawable for BitmapBufferGroup
   */
  public static AnimaXSurfaceDrawable createFromBitmapBufferGroup(
      @NonNull BitmapBufferGroup buffer, int width, int height) {
    return new AnimaXSurfaceDrawable(null, null, buffer, width, height);
  }

  public boolean updateSize(int width, int height) {
    // If the history size is valid, but the size is same, skip it
    if (isSizeChangeWithinRange(width, height)) {
      AnimaXLog.i(TAG, "Skip same size: " + width + " / " + height);
      return false;
    }
    setSize(width, height);

    AnimaXLog.i(TAG, "Update size success: " + width + " / " + height);

    return true;
  }

  /**
   * The layout engine might generate nearly identical heights or widths consecutively.
   * For example, it might first generate a width of 129, then 130, causing the surface to
   * constantly recreate after each layout, which could lead to performance issues. This method
   * helps to avoid such a situation by returning true only if the absolute difference in either
   * dimension is within a limit
   *
   * @param newWidth The new width value to compare.
   * @param newHeight The new height value to compare.
   * @return True if the absolute difference in either width or height is within limit, false
   *     otherwise.
   */
  private boolean isSizeChangeWithinRange(int newWidth, int newHeight) {
    return (Math.abs(mWidth - newWidth) <= 1) && (Math.abs(mHeight - newHeight) <= 1);
  }

  @Nullable
  @CalledByNative
  public Surface getSurface() {
    return mSurface;
  }

  @Nullable
  @CalledByNative
  public SurfaceTexture getSurfaceTexture() {
    return mSurfaceTexture;
  }

  @Nullable
  @CalledByNative
  public BitmapBufferGroup getBitmapBufferGroup() {
    return mBitmapBufferGroup;
  }

  @CalledByNative
  public boolean isTextureFirstFrameAware() {
    if (mSurfaceTexture == null) {
      return false;
    }
    return (mSurfaceTexture instanceof FirstFrameAwareSurfaceTexture);
  }

  public void setSize(int newWidth, int newHeight) {
    mWidth = newWidth;
    mHeight = newHeight;
    if (mBitmapBufferGroup != null) {
      mBitmapBufferGroup.setBufferSize(newWidth, newHeight);
    }
  }

  public void rebuildBufferGroup() {
    if (mBitmapBufferGroup != null) {
      mBitmapBufferGroup.setBufferSize(mWidth, mHeight);
    }
  }

  @CalledByNative
  public int getWidth() {
    return mWidth;
  }

  @CalledByNative
  public int getHeight() {
    return mHeight;
  }

  @CalledByNative
  public boolean isAntiAliasingEnabled() {
    return mEnableAntiAliasing;
  }

  public void setEnableAntiAliasing(boolean enable) {
    this.mEnableAntiAliasing = enable;
  }

  @CalledByNative
  public boolean isSoftwareRenderEnabled() {
    return mEnableSoftwareRender;
  }

  public void setEnableSoftwareRender(boolean enable) {
    this.mEnableSoftwareRender = enable;
  }

  @CalledByNative
  public boolean isPlatformSurfaceInitiallyInvalid() {
    return mIsPlatformSurfaceInitiallyInvalid;
  }

  public void setPlatformSurfaceInitiallyInvalid(boolean invalid) {
    this.mIsPlatformSurfaceInitiallyInvalid = invalid;
  }

  @CalledByNative
  public boolean isAutoDestroyEGLContextEnabled() {
    return mEnableAutoDestroyEGLContext;
  }

  public void setEnableAutoDestroyEGLContext(boolean enable) {
    this.mEnableAutoDestroyEGLContext = enable;
  }

  @CalledByNative
  private long getTypeIndex() {
    return mType.ordinal();
  }

  public DrawableType getType() {
    return mType;
  }
}
