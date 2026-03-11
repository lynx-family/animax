// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.animax.drawable;

import android.graphics.SurfaceTexture;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.view.Surface;
import android.view.TextureView;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import com.lynx.animax.base.CalledByNative;
import com.lynx.animax.util.AnimaXLog;
import com.lynx.animax.util.DeviceUtil;
import com.lynx.animax.util.UIThreadUtils;
import java.lang.ref.WeakReference;
import java.lang.reflect.Field;

public class FirstFrameAwareSurfaceTexture
    extends SurfaceTexture implements SurfaceTexture.OnFrameAvailableListener {
  private static final String TAG = "AnimaXFirstFrameAwareSurfaceTexture";
  private @Nullable WeakReference<FirstFrameAwareSurfaceTexture.FirstFrameAvailableListener>
      mCustomFirstFrameAvailableListener;
  private @Nullable Handler mOnFrameAvailableHandler;
  private boolean mFirstFrameHasPassed;
  private WeakReference<TextureView> mTextureView;
  private final Handler mMainThreadHandler = new Handler(Looper.getMainLooper());
  private boolean mHasFinalized = false;
  private boolean mIsReleased = false;

  /**
   * Surface instance owned and managed by FirstFrameAwareSurfaceTexture.
   *
   * In AnimaX client integration scenarios, clients may not actively call the release method.
   * Therefore, Surface is managed by FirstFrameAwareSurfaceTexture which is responsible for its
   * release.
   *
   * If release is not explicitly called, cleanup is handled through CleanUpReference (using
   * PhantomReference). The cleanup chain is as follows:
   *   1. CleanUpReference#run
   *   2. AnimaXPlayer(C++)#Destroy
   *   3. FirstFrameAwareSurfaceTexture(C++)#release
   *   4. Surface#release
   *   5. FirstFrameAwareSurfaceTexture#release
   */
  private WeakReference<Surface> mSurface;

  private long getSurfaceTextureField() {
    long surfaceTextureField = -1;
    try {
      Field field = SurfaceTexture.class.getDeclaredField("mSurfaceTexture");
      field.setAccessible(true);
      surfaceTextureField = field.getLong(this);
    } catch (Throwable e) {
      AnimaXLog.e(TAG, "fail to getSurfaceTextureField: " + e.getMessage());
    }
    return surfaceTextureField;
  }

  /**
   * Constructs a FirstFrameAwareSurfaceTexture with the texture name set to 0.
   * Note: This behavior isn't explicitly documented, but is evident from the source code.
   * This ensures no texture object is attached to the current OpenGL ES Context upon creation.
   */
  public FirstFrameAwareSurfaceTexture() {
    super(0);
    // On Android >= 5.x, there are one-parameter and two-parameter setOnFrameAvailableListener
    // method, But some vendor may keep call one-parameter.
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
      // to support createViewAsync, but textureview may use the create looper when pass null
      Handler handler = UIThreadUtils.getUiThreadHandler();
      super.setOnFrameAvailableListener(this, handler);
    } else {
      super.setOnFrameAvailableListener(this);
    }
  }

  public synchronized @Nullable Surface getSurface() {
    return mSurface != null ? mSurface.get() : null;
  }

  public synchronized void setSurface(Surface surface) {
    mSurface = new WeakReference<>(surface);
  }

  /**
   * Adapt Android 4.x devices:
   *
   * On Android 4.x, there is only one-parameter setOnFrameAvailableListener method,
   * In order to support these devices, we have to override this method.
   *
   * On Android >= 5.x, there are one-parameter and two-parameter setOnFrameAvailableListener
   * method, But some vendor may keep call one-parameter.
   *
   * @param listener The listener given by the framework
   */
  @Override
  public void setOnFrameAvailableListener(@Nullable OnFrameAvailableListener listener) {
    setOnFrameAvailableListenerInternal(listener, null);
  }

  @Override
  public void setOnFrameAvailableListener(
      @Nullable OnFrameAvailableListener listener, @Nullable Handler handler) {
    setOnFrameAvailableListenerInternal(listener, handler);
    clearInternalFrameRateListenerAsync();
  }

  private void setOnFrameAvailableListenerInternal(
      @Nullable final OnFrameAvailableListener listener, @Nullable Handler handler) {
    updateTextureLayerIfNecessary(listener);

    // copy from SurfaceTexture.java but we do not cache creation looper
    if (listener != null) {
      Looper looper = handler != null ? handler.getLooper() : Looper.getMainLooper();
      mOnFrameAvailableHandler = new OnFrameAvailableHandler(looper, this, listener);
    } else {
      mOnFrameAvailableHandler = null;
    }
  }

  /**
   * Update TextureView's texture layer to recover content lost after onPause().
   *
   * Problem:
   * TextureView content gets lost when placed in a Dialog that undergoes pause/resume cycles.
   * Since there's no API to detect TextureView's Dialog parent or directly call
   * updateLayerAndInvalidate(), we need a workaround.
   *
   * Solution:
   * Temporarily toggle view opacity to trigger internal updateLayerAndInvalidate() method.
   * Only executes when frame handlers are active (both listener and handler non-null)
   * to minimize unnecessary updates.
   *
   * @param listener Frame availability callback, used to determine execution necessity
   */
  private void updateTextureLayerIfNecessary(@Nullable final OnFrameAvailableListener listener) {
    // Skip for Android versions below 7.1.1
    if (Build.VERSION.SDK_INT < Build.VERSION_CODES.N_MR1) {
      return;
    }

    // Early return if the listener is not valid or handler has not created yet.
    if (listener == null || mOnFrameAvailableHandler == null) {
      return;
    }

    // Safe null check and weak reference access
    final TextureView view = mTextureView != null ? mTextureView.get() : null;
    if (view == null) {
      return;
    }

    final Runnable updateLayerAction = () -> {
      AnimaXLog.i(TAG, "Triggering layer update via opacity toggle");
      final boolean originalOpaque = view.isOpaque();
      view.setOpaque(!originalOpaque);
      view.setOpaque(originalOpaque);
    };

    UIThreadUtils.runOnUiThreadImmediately(updateLayerAction);
  }

  public void setTextureView(TextureView textureView) {
    mTextureView = new WeakReference<>(textureView);
  }

  @Override
  public void onFrameAvailable(SurfaceTexture surfaceTexture) {
    // dispatch to downstream like TextureView
    if (mOnFrameAvailableHandler != null) {
      Message message = mOnFrameAvailableHandler.obtainMessage(0);
      if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP_MR1) {
        message.setAsynchronous(true);
      }
      mOnFrameAvailableHandler.sendMessage(message);
    }

    if (!mFirstFrameHasPassed) {
      mFirstFrameHasPassed = true;

      FirstFrameAvailableListener listener = mCustomFirstFrameAvailableListener != null
          ? mCustomFirstFrameAvailableListener.get()
          : null;
      AnimaXLog.i(TAG, "trigger first frame callback with " + listener);
      if (listener != null) {
        listener.onFirstFrameAvailable();
      }
    }
  }

  /**
   *  You can set the listener as many times as you want, but only the last set listener
   *  will be the "active" one that receives notifications. If the first frame has already
   *  been processed, any new listener you set will be immediately notified of the first frame.
   *
   *  @param listener the listener to set, will be hold by WeakReference
   */
  public void setFirstFrameListener(
      @NonNull FirstFrameAwareSurfaceTexture.FirstFrameAvailableListener listener) {
    mCustomFirstFrameAvailableListener = new WeakReference<>(listener);
    if (mFirstFrameHasPassed) {
      listener.onFirstFrameAvailable();
    }
  }

  /**
   * Prevents memory leaks on Android 15 (API 35) and above by asynchronously removing
   * the internal frame rate listener and its associated Handler from SurfaceTexture.
   *
   * This method posts the cleanup action to the main thread to ensure the framework
   * has finished any internal registration before removal.
   */
  private void clearInternalFrameRateListenerAsync() {
    if (android.os.Build.VERSION.SDK_INT < 35) {
      return;
    }
    final WeakReference<FirstFrameAwareSurfaceTexture> weakSelf = new WeakReference<>(this);
    AnimaXLog.i(TAG, "clearInternalFrameRateListenerAsync: Posting cleanup action");
    UIThreadUtils.runOnUiThread(() -> {
      FirstFrameAwareSurfaceTexture self = weakSelf.get();
      if (self != null) {
        AnimaXLog.i(TAG, "clearInternalFrameRateListenerAsync: Executing cleanup");
        self.setNullOnSetFrameRateListener();
      } else {
        AnimaXLog.i(TAG, "clearInternalFrameRateListenerAsync: Skipped, reference lost");
      }
    });
  }

  /**
   * Proactively clears the SurfaceTexture frame rate callback listener to avoid Handler memory
   * leaks.
   *
   * Android 15 (API 35) adds setOnSetFrameRateListener, but the OnSetFrameRateListener interface is
   * @hide and thus not accessible in third-party apps. This method uses reflection to call
   * setOnSetFrameRateListener(null, null), which unregisters the callback and handler,
   * preventing memory leaks that could be caused by the Handler holding references to
   * View/Activity.
   *
   * This method only executes on Android 15 (API 35) and above, and is safe to call on lower
   * versions.
   */
  private void setNullOnSetFrameRateListener() {
    if (android.os.Build.VERSION.SDK_INT < 35) {
      return;
    }
    if (isOnSetFrameRateHandlerNull()) {
      AnimaXLog.i(TAG, "setNullOnSetFrameRateListener fail, handler is already null.");
      return;
    }
    try {
      Class<?> listenerClass =
          Class.forName("android.graphics.SurfaceTexture$OnSetFrameRateListener");
      java.lang.reflect.Method method = android.graphics.SurfaceTexture.class.getMethod(
          "setOnSetFrameRateListener", listenerClass, Handler.class);
      method.setAccessible(true);
      method.invoke(this, null, null);
      AnimaXLog.i(TAG, "setNullOnSetFrameRateListener result: " + isOnSetFrameRateHandlerNull());
    } catch (Throwable e) {
      AnimaXLog.e(TAG, "setNullOnSetFrameRateListener fail: " + e.getMessage());
    }
  }

  /**
   * Checks whether the private field mOnSetFrameRateHandler in SurfaceTexture is currently null.
   * Returns true if the field is null, false otherwise.
   */
  private boolean isOnSetFrameRateHandlerNull() {
    try {
      Field field = SurfaceTexture.class.getDeclaredField("mOnSetFrameRateHandler");
      field.setAccessible(true);
      Object handler = field.get(this);
      return handler == null;
    } catch (Throwable e) {
      AnimaXLog.e(TAG, "isOnSetFrameRateHandlerNull fail: " + e.getMessage());
      return false;
    }
  }

  @Override
  @CalledByNative
  public synchronized void release() {
    AnimaXLog.i(TAG, "release with " + this);
    if (mHasFinalized) {
      AnimaXLog.w(TAG, "release() called after finalize(). Skip releasing SurfaceTexture. " + this);
      return;
    }

    if (mIsReleased) {
      AnimaXLog.w(TAG, "release() called multiple times. " + this);
      return;
    }
    mIsReleased = true;

    Surface surface = getSurface();
    if (surface != null && surface.isValid()) {
      surface.release();
      AnimaXLog.i(TAG, "release Surface: " + surface);
      setSurface(null);
    }

    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
      if (isReleased()) {
        AnimaXLog.w(TAG, "already released: " + this);
        return;
      }
    }
    long surfaceTextureField = getSurfaceTextureField();
    if (surfaceTextureField == 0) {
      AnimaXLog.i(TAG, "surfaceTextureField is 0, skip release");
      return;
    }
    AnimaXLog.i(TAG, "release SurfaceTexture: " + this);
    super.release();
  }

  public interface FirstFrameAvailableListener {
    void onFirstFrameAvailable();
  }

  @Override
  protected synchronized void finalize() throws Throwable {
    mHasFinalized = true;

    Surface surface = getSurface();
    if (surface != null && surface.isValid()) {
      surface.release();
      AnimaXLog.i(TAG, "release Surface in finalize: " + surface);
      setSurface(null);
    }

    AnimaXLog.i(TAG, "FirstFrameAwareSurfaceTexture finalize: " + this);
    super.finalize();
  }

  private static class OnFrameAvailableHandler extends Handler {
    private final WeakReference<FirstFrameAwareSurfaceTexture> mWeakSurfaceTexture;
    private final WeakReference<OnFrameAvailableListener> mWeakListener;

    public OnFrameAvailableHandler(Looper looper, FirstFrameAwareSurfaceTexture surfaceTexture,
        OnFrameAvailableListener listener) {
      super(looper);
      mWeakSurfaceTexture = new WeakReference<>(surfaceTexture);
      mWeakListener = new WeakReference<>(listener);
    }

    @Override
    public void handleMessage(Message msg) {
      FirstFrameAwareSurfaceTexture self = mWeakSurfaceTexture.get();
      if (self == null) {
        return;
      }
      OnFrameAvailableListener listener = mWeakListener.get();
      if (listener != null) {
        listener.onFrameAvailable(self);
      }
    }
  }
}
