// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.player;

import android.graphics.SurfaceTexture;
import android.view.Surface;
import androidx.annotation.NonNull;
import com.lynx.animax.base.AnimaXError;
import com.lynx.animax.listener.AnimaXErrorParam;
import com.lynx.animax.service.IAnimaXMonitorService;
import com.lynx.animax.util.AnimaXLog;
import java.util.HashMap;

public abstract class AbsVideoPlayer implements IVideoPlayer {
  private static final String TAG = "AbsVideoPlayer";

  protected VideoAsset mAsset;

  protected SurfaceTexture mSurfaceTexture;
  protected Surface mSurface;
  protected float[] mTransformMatrix;

  protected long mNativePlayer;

  // VideoPlayerErrorReporter
  private boolean mHasReportedError = false;
  private int mErrorCount = 0;
  private boolean mHasDrewOnce = false;
  private boolean mHasDrewOnceAfterError = false;

  @NonNull protected VideoPlayerConfig mConfig;

  public AbsVideoPlayer(long nativePlayer, @NonNull VideoPlayerConfig config) {
    AnimaXLog.i(TAG, "create: " + this);
    mNativePlayer = nativePlayer;
    mConfig = config;
  }

  @Override
  public void attachAsset(VideoAsset asset) {
    if (null != mAsset) {
      AnimaXLog.e(TAG, "Attach asset more than once");
      return;
    }
    if (null == asset || !asset.isValid()) {
      AnimaXLog.e(TAG, "attachAsset error: asset isn't valid");
      return;
    }
    mAsset = asset;
  }

  @Override
  public void setSurface(int texture) {
    // release current Surface/SurfaceTexture
    releaseSurface();
    mSurfaceTexture = new SurfaceTexture(texture);
    mSurface = new Surface(mSurfaceTexture);
  }

  @Override
  public float[] getTransformMatrix() {
    if (null == mTransformMatrix) {
      mTransformMatrix = new float[] {
          1.f, 0.f, 0.f, 0.f, 0.f, -1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 1.f};
    }
    if (null != mSurfaceTexture) {
      mSurfaceTexture.getTransformMatrix(mTransformMatrix);
    }
    return mTransformMatrix;
  }

  @Override
  public void destroy() {
    AnimaXLog.i(TAG, "destroy: " + this);
    mAsset = null;
    releaseSurface();
    mNativePlayer = 0;
    reportErrorHasOccurredIfNecessary();
  }

  private void releaseSurface() {
    if (null != mSurface) {
      mSurface.release();
      mSurface = null;
    }
    if (null != mSurfaceTexture) {
      mSurfaceTexture.setOnFrameAvailableListener(null);
      mSurfaceTexture.release();
      mSurfaceTexture = null;
    }
  }

  protected void hasDrewOnce() {
    mHasDrewOnce = true;
    mHasDrewOnceAfterError = (mErrorCount > 0);
  }

  private void reportErrorInner(@NonNull String errMsg) {
    logError(errMsg);

    IAnimaXMonitorService monitor =
        mConfig.getServiceRegistry().getService(IAnimaXMonitorService.class);
    if (monitor == null) {
      return;
    }

    HashMap<String, Object> category = new HashMap<>();
    category.put(AnimaXErrorParam.KEY_CODE, AnimaXError.VIDEO_PLAYER_ERROR_HAS_OCCURRED.ordinal());
    category.put(AnimaXErrorParam.KEY_MESSAGE, errMsg);
    category.put("error_count", mErrorCount);

    AnimaXErrorParam errorParam = new AnimaXErrorParam(category);
    monitor.reportError(errorParam);
  }

  protected void reportErrorOnce(@NonNull String errMsg) {
    ++mErrorCount;
    mHasDrewOnceAfterError = false;
    if (mHasReportedError) {
      logError(errMsg);
      return;
    }
    mHasReportedError = true;
    reportErrorInner(errMsg);
  }

  private void logError(@NonNull String errMsg) {
    AnimaXLog.e(TAG, "Error(" + mErrorCount + "): " + errMsg);
  }

  private void reportErrorHasOccurredIfNecessary() {
    if (mErrorCount <= 0) {
      return;
    }

    String errMsg = "[" + getClass().getName() + "]: Error has occurred, mHasDrewOnce: "
        + mHasDrewOnce + ", mHasDrewOnceAfterError: " + mHasDrewOnceAfterError;
    reportErrorInner(errMsg);
  }
}
