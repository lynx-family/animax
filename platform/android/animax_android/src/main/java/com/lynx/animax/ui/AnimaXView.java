// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.ui;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.TypedArray;
import android.graphics.SurfaceTexture;
import android.graphics.drawable.Drawable;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.TextureView;
import android.view.View;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.RestrictTo;
import com.lynx.animax.AnimaXPlayer;
import com.lynx.animax.R;
import com.lynx.animax.ability.BaseAbility;
import com.lynx.animax.ability.NativeAbility;
import com.lynx.animax.base.VisibilityState;
import com.lynx.animax.drawable.AnimaXSurfaceDrawable;
import com.lynx.animax.drawable.FirstFrameAwareSurfaceTexture;
import com.lynx.animax.util.AnimaXLog;
import com.lynx.animax.util.DeviceUtil;
import com.lynx.animax.util.UIPropertyUtil;
import com.lynx.animax.util.UIThreadUtils;

/**
 * AnimaXView is a TextureView based implementation for playing lottie animations.
 * It implements IAnimaXView and need set lottie json source to play animation.
 *
 * <p> Example usage:
 * <pre>
 * AnimaXView animaxView = new AnimaXView(context);
 * animaxView.setAutoPlay(true);
 * animaxView.setLoop(true);
 *
 * // Add an animation listener to handle animation events
 * animaxView.addAnimationListener(new AnimationListenerAdapter() {
 *     &#64;Override
 *     public void onReady(AnimaXParam param) {
 *         Log.i(TAG, "onReady");
 *     }
 * });
 *
 * // Set the lottie json https link
 * animaxView.setSrc("https://xxx");
 * </pre>
 * </p>
 */
public class AnimaXView
    extends TextureView implements TextureView.SurfaceTextureListener,
                                   FirstFrameAwareSurfaceTexture.FirstFrameAvailableListener,
                                   IAnimaXView, IAnimaXPlayerDelegate {
  private static final String TAG = "AnimaXView";

  private FirstFrameAwareSurfaceTexture mSurfaceTexture;
  private Surface mSurface;

  private boolean mSelfIsDestroyed = false;
  private AnimaXSurfaceDrawable mSurfaceDrawable;

  private boolean mIgnoreAttachStatus = false;

  private BroadcastReceiver mBoardCastReceiver;
  private boolean mScreenLockWorkaroundInited;
  private float mAlphaSet = 1.0f;
  private boolean mNeedAlphaWorkaround;
  private boolean mNeedVisibilityWorkaround = false;
  private int mLastVisibility = View.VISIBLE;

  private final @NonNull IAnimaXPlayer mPlayer;

  // Allow AnimaXView send touch event to element.
  private boolean mEnableNativeTapLayerEvent = false;

  private boolean mEnableTextureUpdateWorkaround = false;

  public AnimaXView(Context context) {
    super(context);
    AnimaXContext animaXContext = new AnimaXContext.Builder(new NativeAbility(), context).build();
    mPlayer = new AnimaXPlayer(animaXContext);
    initView(null);
  }

  public AnimaXView(Context context, AttributeSet attrs) {
    super(context, attrs);
    AnimaXContext animaXContext = new AnimaXContext.Builder(new NativeAbility(), context).build();
    mPlayer = new AnimaXPlayer(animaXContext);
    initView(attrs);
  }

  @RestrictTo(RestrictTo.Scope.LIBRARY)
  public AnimaXView(@NonNull AnimaXContext animaXContext) {
    super(animaXContext.getContext());
    mPlayer = new AnimaXPlayer(animaXContext);
    initView(null);
  }

  @RestrictTo(RestrictTo.Scope.LIBRARY)
  public AnimaXView(@NonNull AnimaXPlayer animaXPlayer) {
    super(animaXPlayer.getAnimaXContext().getContext());
    mPlayer = animaXPlayer;
    initView(null);
  }

  private void initView(@Nullable AttributeSet attrs) {
    // Check that both context & ability are valid
    AnimaXContext animaXContext = mPlayer.getAnimaXContext();
    if (animaXContext == null || animaXContext.getAbility() == null) {
      AnimaXLog.e(TAG, "initView fail, missing AnimaXContext or Ability.");
      return;
    }

    // Additional setup
    BaseAbility ability = animaXContext.getAbility();
    ability.setCompositionReadyListener(this);
    mEnableTextureUpdateWorkaround = DeviceUtil.updateTextureWhenVisible(ability);

    // Some host contexts may have abnormal states that prevent normal attribute retrieval,
    // potentially causing NullPointerException when calling getContext() or accessing theme.
    // This exception typically doesn't impact core functionality and can be safely caught
    // without affecting business logic or user experience.
    try {
      initAttributes(animaXContext.getContext(), attrs);
    } catch (NullPointerException e) {
      AnimaXLog.e(TAG, "initAttributes fail, context's theme is null.");
    }
    initTexture();
  }

  private void initAttributes(@NonNull Context context, @Nullable AttributeSet attrs) {
    // If attrs is null, the attribute will be null, so return directly.
    if (attrs == null) {
      return;
    }

    if (context.getTheme() == null) {
      AnimaXLog.e(TAG, "initAttributes fail, context's theme is null.");
      return;
    }

    TypedArray ta = context.obtainStyledAttributes(attrs, R.styleable.AnimaXView);

    String animationUrl = ta.getString(R.styleable.AnimaXView_animax_animationUrl);
    if (animationUrl != null) {
      mPlayer.setSrc(animationUrl);
    }

    mPlayer.setAutoPlay(ta.getBoolean(R.styleable.AnimaXView_animax_autoPlay, true));
    mPlayer.setLoop(ta.getBoolean(R.styleable.AnimaXView_animax_loop, false));

    String objectFit = ta.getString(R.styleable.AnimaXView_animax_objectFit);
    if (objectFit != null) {
      mPlayer.setObjectFit(UIPropertyUtil.convertStringToObjectFit(objectFit));
    }

    ta.recycle();
  }

  private void initTexture() {
    // need to blend TextureView with background view contents
    setOpaque(false);
    setNeedAlphaWorkaround(true);

    initSurfaceTexture();
    updateSurfaceTexture();
  }

  private void initSurfaceTexture() {
    setSurfaceTextureListener(this);

    mSurfaceTexture = new FirstFrameAwareSurfaceTexture();
    mSurface = new Surface(mSurfaceTexture);
    AnimaXLog.i(TAG, "create Surface: " + mSurface);
    mSurfaceTexture.setSurface(mSurface);
    mSurfaceTexture.detachFromGLContext();
    mSurfaceTexture.setFirstFrameListener(this);
    mSurfaceTexture.setTextureView(this);
  }

  private void updateSurfaceTexture() {
    SurfaceTexture originSurfaceTexture = getSurfaceTexture();
    if (mSurfaceTexture == null || mSurfaceTexture.equals(originSurfaceTexture)) {
      return;
    }

    if (originSurfaceTexture != null) {
      AnimaXLog.i(TAG, "Init TextureView but it has already another st.");
    }

    setSurfaceTexture(mSurfaceTexture);
  }

  @Override
  protected void onAttachedToWindow() {
    initScreenLockWorkaround();

    if (mIgnoreAttachStatus) {
      return;
    }

    super.onAttachedToWindow();
    mPlayer.updateVisibilityState(true, VisibilityState.ATTACH);
  }

  private void initScreenLockWorkaround() {
    if (!DeviceUtil.needScreenLockWorkaround()) {
      return;
    }
    if (mScreenLockWorkaroundInited) {
      return;
    }
    if (mBoardCastReceiver == null) {
      mBoardCastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
          // in some devices, it will clear TextureView content when screen locked
          // need trick like this to avoid blank screen after unlock.
          // listen ACTION_SCREEN_ON may to late for this, use can see blank screen for some
          // seconds.
          AnimaXLog.i(TAG, "detect screen unlock, force redraw " + this);
          // on some host app, broadcast received be triggered on other thread by hook.
          UIThreadUtils.runOnUiThread(new Runnable() {
            @Override
            public void run() {
              AnimaXLog.i(TAG, "detect screen unlock, force redraw " + this + " run");
              if (getVisibility() == VISIBLE) {
                setVisibility(INVISIBLE);
                setVisibility(VISIBLE);
              }
            }
          });
        }
      };
    }

    IntentFilter filter = new IntentFilter();
    filter.addAction(Intent.ACTION_SCREEN_OFF);
    try {
      getContext().registerReceiver(mBoardCastReceiver, filter);
    } catch (Exception e) {
      AnimaXLog.e(TAG, e.getMessage());
      AnimaXLog.e(TAG, "register BoardCastReceiver: " + mBoardCastReceiver);
    }
    mScreenLockWorkaroundInited = true;
  }

  @Override
  protected void onDetachedFromWindow() {
    deInitScreenLockWorkaround();

    if (mIgnoreAttachStatus) {
      return;
    }

    super.onDetachedFromWindow();
    mPlayer.updateVisibilityState(false, VisibilityState.ATTACH);
  }

  private void deInitScreenLockWorkaround() {
    if (!DeviceUtil.needScreenLockWorkaround()) {
      return;
    }
    if (!mScreenLockWorkaroundInited) {
      return;
    }
    mScreenLockWorkaroundInited = false;
    try {
      getContext().unregisterReceiver(mBoardCastReceiver);
    } catch (Exception e) {
      AnimaXLog.e(TAG, e.getMessage());
      AnimaXLog.e(TAG, "unregister BoardCastReceiver: " + mBoardCastReceiver);
    }
  }

  @NonNull
  @Override
  public IAnimaXPlayer getPlayer() {
    return mPlayer;
  }

  @Override
  public void setIgnoreAttachStatus(boolean ignore) {
    mIgnoreAttachStatus = ignore;
  }

  @Override
  public void onSurfaceTextureAvailable(SurfaceTexture texture, int width, int height) {
    AnimaXLog.i(TAG,
        "onSurfaceTextureAvailable: " + texture + ", width: " + width + ", height: " + height
            + ", this: " + this);
    if (mSelfIsDestroyed) {
      AnimaXLog.i(TAG, "onSurfaceTextureAvailable failed as view is destroyed.");
      return;
    }

    updateSurfaceTexture();
    onSurfaceChanged(width, height);
  }

  @Override
  public void onSurfaceTextureSizeChanged(SurfaceTexture texture, int width, int height) {
    AnimaXLog.i(TAG,
        "onSurfaceTextureSizeChanged: " + texture + ", width: " + width + ", height: " + height
            + ", this: " + this);
    if (mSelfIsDestroyed) {
      AnimaXLog.i(TAG, "onSurfaceTextureSizeChanged failed as view is destroyed.");
      return;
    }

    onSurfaceChanged(width, height);
  }

  private void setNeedAlphaWorkaround(boolean needAlphaWorkaround) {
    if (!DeviceUtil.needAlphaWorkaround()) {
      return;
    }
    mNeedAlphaWorkaround = needAlphaWorkaround;
    if (needAlphaWorkaround) {
      super.setAlpha(0f);
    } else {
      super.setAlpha(mAlphaSet);
    }
  }

  /**
   * Listen the click event when mEnableLayerEvent is true
   * @param event
   * @return
   */
  @Override
  public boolean onTouchEvent(MotionEvent event) {
    if (mEnableNativeTapLayerEvent) {
      handleTouchEvent(event);
    }
    return super.onTouchEvent(event);
  }

  private void onSurfaceChanged(int width, int height) {
    mPlayer.updateVisibilityState(width > 0 && height > 0, VisibilityState.SIZE);

    if (width <= 0 || height <= 0) {
      AnimaXLog.i(TAG, "Skip invalid size: " + width + " / " + height);
      return;
    }

    if (mSurfaceDrawable == null) {
      mSurfaceDrawable = AnimaXSurfaceDrawable.createFromFirstFrameAwareSurfaceTexture(
          mSurfaceTexture, width, height);
      mPlayer.createAnimaXSurface(mSurfaceDrawable);
      return;
    }

    if (mSurfaceDrawable.updateSize(width, height)) {
      mPlayer.updateAnimaXSurface(mSurfaceDrawable);
    }
  }

  @Override
  public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
    AnimaXLog.i(TAG,
        "onSurfaceTextureDestroyed: " + surface + ", mHasDestroyed: " + mSelfIsDestroyed
            + ", this: " + this);
    // Post a task at the head of the main thread queue.
    // Ensuring it is triggered before the call to TextureView#draw.
    if (DeviceUtil.needTextureDestroyWorkaround() && !mSelfIsDestroyed) {
      AnimaXLog.i(TAG, "needTextureDestroyWorkaround, try to postAtFrontOnUI.");
      mLastVisibility = getVisibility();
      // Ensure TextureView#draw not called before updateSurfaceTexture
      mNeedVisibilityWorkaround = true;
      setVisibility(View.INVISIBLE);
      UIThreadUtils.postAsynchronousAtFrontOfQueueOnUiThread(new Runnable() {
        @Override
        public void run() {
          AnimaXLog.i(TAG, "Try to updateSurfaceTexture, mHasDestroyed: " + mSelfIsDestroyed);
          if (!mSelfIsDestroyed) {
            mNeedVisibilityWorkaround = false;
            setVisibility(mLastVisibility);
            // getSurfaceTexture is NULL at this point.
            // Call setSurfaceTexture to update surface texture to avoid NPE.
            updateSurfaceTexture();
          }
        }
      });
    }
    return false;
  }

  @Override
  public void onSurfaceTextureUpdated(SurfaceTexture surface) {}

  @Override
  public void release() {
    if (mSelfIsDestroyed) {
      return;
    }

    AnimaXLog.i(TAG, "release: " + this);

    mSelfIsDestroyed = true;

    mPlayer.release();
  }

  @Override
  protected void onSizeChanged(int w, int h, int oldW, int oldH) {
    try {
      super.onSizeChanged(w, h, oldW, oldH);
    } catch (NullPointerException e) {
      e.printStackTrace();
      // some roms need textureview is added to view tree before layout. see issues 7077 for
      // details.
      // https://t.wtturl.cn/DradgWq/
      AnimaXLog.e(TAG, "onSizeChanged before added to view tree, may produce npe on some devices");
    }
  }

  @Override
  public void onFirstFrameAvailable() {
    setNeedAlphaWorkaround(false);
  }

  @Override
  public void setForeground(Drawable foreground) {
    try {
      super.setForeground(foreground);
    } catch (UnsupportedOperationException e) {
      AnimaXLog.e(TAG, "setForeground fail, message: " + e.getMessage());
    }
  }

  @Override
  public void setBackgroundDrawable(Drawable background) {
    try {
      super.setBackgroundDrawable(background);
    } catch (UnsupportedOperationException e) {
      AnimaXLog.e(TAG, "setBackgroundDrawable fail, message: " + e.getMessage());
    }
  }

  @Override
  public void setBackground(Drawable background) {
    try {
      super.setBackground(background);
    } catch (UnsupportedOperationException e) {
      AnimaXLog.e(TAG, "setBackground fail, message: " + e.getMessage());
    }
  }

  /**
   * Override the View's setVisibility, when the ui is visible, will call with
   * VisibilityState.VISIBLE param
   * @param visibility
   */
  @Override
  public void setVisibility(int visibility) {
    if (mNeedVisibilityWorkaround) {
      mLastVisibility = visibility;
      if (visibility == View.VISIBLE) {
        AnimaXLog.i(TAG, "Force visiblity to INVISIBLE due to workaround.");
        visibility = View.INVISIBLE;
      }
    }
    super.setVisibility(visibility);
    mPlayer.updateVisibilityState(visibility == View.VISIBLE, VisibilityState.VISIBLE);
  }

  /**
   * Override the View's setAlpha, when the ui is visible, will call onShow with
   * VisibilityState.OPACITY param
   * @param alpha
   */
  @Override
  public void setAlpha(float alpha) {
    float oldAlpha = getAlpha();
    mAlphaSet = alpha;
    if (!mNeedAlphaWorkaround) {
      super.setAlpha(alpha);
    }

    boolean wasVisible = oldAlpha > 0;
    boolean isVisible = alpha > 0;
    if (wasVisible != isVisible) {
      mPlayer.updateVisibilityState(isVisible, VisibilityState.OPACITY);
    }
  }

  @Override
  public void onVisibilityAggregated(boolean isVisible) {
    super.onVisibilityAggregated(isVisible);
    if (mIgnoreAttachStatus) {
      return;
    }
    mPlayer.updateVisibilityState(isVisible, VisibilityState.AGGREGATED);
  }

  @Override
  protected void onVisibilityChanged(View changedView, int visibility) {
    super.onVisibilityChanged(changedView, visibility);
    if (mEnableTextureUpdateWorkaround && changedView == this && visibility == View.VISIBLE) {
      updateSurfaceTexture();
    }
  }

  @Override
  protected void onWindowVisibilityChanged(int visibility) {
    super.onWindowVisibilityChanged(visibility);
    if (mEnableTextureUpdateWorkaround && getVisibility() == View.VISIBLE
        && visibility == View.VISIBLE) {
      updateSurfaceTexture();
    }
  }

  @Override
  public void onCompositionReady() {
    if (mSurfaceDrawable != null || !mIgnoreAttachStatus) {
      return;
    }
    mPlayer.markPlatformSurfaceAsInvalid(true);
    // Since the surface is already marked as invalid, the temporary surface will not be rendered.
    // Therefore, we no longer need to set the ensured size to match the real view's dimensions.
    // This prevents the temporary surface size from matching the subsequent real size.
    AnimaXLog.i(TAG, "The Surface has not been created yet; Creating a temporary one.");
    onSurfaceTextureAvailable(null, 1, 1);
  }

  @Override
  public void setEnableTapLayerEvent(boolean enable) {
    mEnableNativeTapLayerEvent = enable;
  }
}
