// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.animax.ui;

import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.util.AttributeSet;
import android.view.Choreographer;
import android.view.MotionEvent;
import android.view.View;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.RestrictTo;
import com.lynx.animax.AnimaXPlayer;
import com.lynx.animax.R;
import com.lynx.animax.ability.NativeAbility;
import com.lynx.animax.base.VisibilityState;
import com.lynx.animax.drawable.AnimaXSurfaceDrawable;
import com.lynx.animax.drawable.BitmapBuffer;
import com.lynx.animax.drawable.BitmapBufferGroup;
import com.lynx.animax.util.AnimaXLog;
import com.lynx.animax.util.UIPropertyUtil;

/**
 * AnimaXImageView is a custom View based implementation for playing animations.
 * It renders the animation frames onto a Bitmap, which is then drawn to the View's Canvas.
 * This approach is suitable for simple animations and environments where a TextureView is not
 * ideal.
 *
 * <p> Example usage:
 * <pre>
 * AnimaXImageView animaxImageView = new AnimaXImageView(context);
 * animaxImageView.setAutoPlay(true);
 * animaxImageView.setLoop(true);
 *
 * // Add an animation listener to handle animation events
 * animaxImageView.addAnimationListener(new AnimationListenerAdapter() {
 *     &#64;Override
 *     public void onReady(AnimaXParam param) {
 *         Log.i(TAG, "Animation is ready to play!");
 *     }
 * });
 *
 * // Set the lottie animation from a URL
 * animaxImageView.setSrc("https://example.com/animation.json");
 * </pre>
 * </p>
 */
public class AnimaXImageView extends View implements IAnimaXView, IAnimaXPlayerDelegate {
  private static final String TAG = "AnimaXImageView";

  private final @NonNull BitmapBufferGroup mBufferGroup;
  private final @NonNull Paint mPaint;
  private @Nullable AnimaXSurfaceDrawable mSurfaceDrawable;

  private final @NonNull IAnimaXPlayer mPlayer;

  private boolean mEnableNativeTapLayerEvent = false;
  private boolean mIgnoreAttachStatus = false;

  private @Nullable BitmapBuffer mRenderBitmapBuffer;

  public AnimaXImageView(Context context) {
    super(context);
    AnimaXContext animaXContext = new AnimaXContext.Builder(new NativeAbility(), context).build();
    mPlayer = new AnimaXPlayer(animaXContext);
    mPaint = new Paint(Paint.FILTER_BITMAP_FLAG | Paint.DITHER_FLAG);
    boolean enableSoftRender = true;
    mPlayer.enableSoftwareRender(enableSoftRender);
    mBufferGroup = new BitmapBufferGroup(this, enableSoftRender);
    initView(null);
  }

  public AnimaXImageView(Context context, AttributeSet attrs) {
    super(context, attrs);
    AnimaXContext animaXContext = new AnimaXContext.Builder(new NativeAbility(), context).build();
    mPlayer = new AnimaXPlayer(animaXContext);
    mPaint = new Paint(Paint.FILTER_BITMAP_FLAG | Paint.DITHER_FLAG);
    boolean enableSoftRender = true;
    mPlayer.enableSoftwareRender(enableSoftRender);
    mBufferGroup = new BitmapBufferGroup(this, enableSoftRender);
    initView(attrs);
  }

  @RestrictTo(RestrictTo.Scope.LIBRARY)
  public AnimaXImageView(@NonNull AnimaXContext animaXContext) {
    super(animaXContext.getContext());
    mPlayer = new AnimaXPlayer(animaXContext);
    mPaint = new Paint(Paint.FILTER_BITMAP_FLAG | Paint.DITHER_FLAG);
    boolean enable = true;
    mPlayer.enableSoftwareRender(enable);
    mBufferGroup = new BitmapBufferGroup(this, enable);
    initView(null);
  }

  @RestrictTo(RestrictTo.Scope.LIBRARY)
  public AnimaXImageView(@NonNull AnimaXPlayer animaXPlayer) {
    super(animaXPlayer.getAnimaXContext().getContext());
    mPlayer = animaXPlayer;
    mPaint = new Paint(Paint.FILTER_BITMAP_FLAG | Paint.DITHER_FLAG);
    boolean enable = true;
    mPlayer.enableSoftwareRender(enable);
    mBufferGroup = new BitmapBufferGroup(this, enable);
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
    animaXContext.getAbility().setCompositionReadyListener(this);
    animaXContext.getAbility().markShouldSendFirstFrame();

    // Some host contexts may have abnormal states that prevent normal attribute retrieval,
    // potentially causing NullPointerException when calling getContext() or accessing theme.
    // This exception typically doesn't impact core functionality and can be safely caught
    // without affecting business logic or user experience.
    try {
      initAttributes(animaXContext.getContext(), attrs);
    } catch (NullPointerException e) {
      AnimaXLog.e(TAG, "initAttributes fail, context's theme is null.");
    }
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

    TypedArray ta = context.obtainStyledAttributes(attrs, R.styleable.AnimaXImageView);

    String animationUrl = ta.getString(R.styleable.AnimaXImageView_animax_image_animationUrl);
    if (animationUrl != null) {
      mPlayer.setSrc(animationUrl);
    }

    mPlayer.setAutoPlay(ta.getBoolean(R.styleable.AnimaXImageView_animax_image_autoPlay, true));
    mPlayer.setLoop(ta.getBoolean(R.styleable.AnimaXImageView_animax_image_loop, false));

    String objectFit = ta.getString(R.styleable.AnimaXImageView_animax_image_objectFit);
    if (objectFit != null) {
      mPlayer.setObjectFit(UIPropertyUtil.convertStringToObjectFit(objectFit));
    }

    ta.recycle();
  }

  @RestrictTo(RestrictTo.Scope.LIBRARY)
  public Bitmap getCurrentBitmap() {
    if (mRenderBitmapBuffer != null) {
      return mRenderBitmapBuffer.getBitmap();
    } else {
      return null;
    }
  }

  @Override
  public void release() {
    AnimaXLog.i(TAG, "release: " + this);
    if (mRenderBitmapBuffer != null) {
      mRenderBitmapBuffer.release();
    }
    mBufferGroup.release();
    mPlayer.release();
  }

  @NonNull
  @Override
  public IAnimaXPlayer getPlayer() {
    return mPlayer;
  }

  @Override
  public void setEnableTapLayerEvent(boolean enable) {
    mEnableNativeTapLayerEvent = enable;
  }

  public void setIgnoreAttachStatus(boolean ignore) {
    mIgnoreAttachStatus = ignore;
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
    onSizeChanged(1, 1, 0, 0);
  }

  /**
   * Listen the click event when mEnableLayerEvent is true.
   */
  @Override
  public boolean onTouchEvent(MotionEvent event) {
    if (mEnableNativeTapLayerEvent) {
      handleTouchEvent(event);
    }
    if (event != null) {
      return super.onTouchEvent(event);
    }
    return false;
  }

  @Override
  protected void onAttachedToWindow() {
    if (mIgnoreAttachStatus) {
      return;
    }

    super.onAttachedToWindow();
    mPlayer.updateVisibilityState(true, VisibilityState.ATTACH);
  }

  @Override
  protected void onDetachedFromWindow() {
    if (mIgnoreAttachStatus) {
      return;
    }

    super.onDetachedFromWindow();
    mPlayer.updateVisibilityState(false, VisibilityState.ATTACH);
  }

  /**
   * Override the View's setVisibility, when the ui is visible, will call with
   * VisibilityState.VISIBLE param
   * @param visibility visibility state.
   */
  @Override
  public void setVisibility(int visibility) {
    super.setVisibility(visibility);
    mPlayer.updateVisibilityState(visibility == View.VISIBLE, VisibilityState.VISIBLE);
  }

  /**
   * Override the View's setAlpha, when the ui is visible, will call onShow with
   * VisibilityState.OPACITY param
   * @param alpha alpha value.
   */
  @Override
  public void setAlpha(float alpha) {
    super.setAlpha(alpha);
    float oldAlpha = getAlpha();
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
  protected void onSizeChanged(int w, int h, int oldW, int oldH) {
    super.onSizeChanged(w, h, oldW, oldH);
    mPlayer.updateVisibilityState(w > 0 && h > 0, VisibilityState.SIZE);

    if (w <= 0 || h <= 0) {
      AnimaXLog.i(TAG, "Skip invalid size: " + w + " / " + h);
      return;
    }

    if (mRenderBitmapBuffer != null) {
      mRenderBitmapBuffer.release();
      mRenderBitmapBuffer = null;
    }

    if (mSurfaceDrawable == null) {
      mSurfaceDrawable = AnimaXSurfaceDrawable.createFromBitmapBufferGroup(mBufferGroup, w, h);
      mPlayer.createAnimaXSurface(mSurfaceDrawable);
      return;
    }

    if (mSurfaceDrawable.updateSize(w, h)) {
      mPlayer.updateAnimaXSurface(mSurfaceDrawable);
    }
  }

  protected void onDraw(Canvas canvas) {
    super.onDraw(canvas);
    // Step 1: If ImageView didn't hold any bitmap(after initializing or resizing), it acquires a
    // bitmap from bufferGroup and recycle nothing.
    if (mRenderBitmapBuffer == null) {
      mRenderBitmapBuffer = mBufferGroup.swapBitmapBuffer(null);
    }

    // Step 2: If native renderer have not produced any bitmap or there is a racing situation(which
    // turns out that ImageView is holding an expired buffer), skip onDraw this time.
    if (mRenderBitmapBuffer == null) {
      return;
    }
    if (mRenderBitmapBuffer.getState() != BitmapBuffer.BufferState.Ready) {
      mRenderBitmapBuffer.release();
      mRenderBitmapBuffer = null;
      return;
    }
    // Step 3: Post the bitmap to system rendering thread, which will be consumed and flush to the
    // screen later.
    Bitmap bitmap = mRenderBitmapBuffer.getBitmap();
    if (bitmap == null || bitmap.isRecycled()) {
      return;
    }
    int saveCount = canvas.save();
    canvas.drawBitmap(bitmap, 0, 0, mPaint);
    canvas.restoreToCount(saveCount);
    // Step 4: Assert that on the next Vsync, the previous bitmap has been consumed and ready to
    // recycle.
    Choreographer.getInstance().postFrameCallback(frameTimeNanos -> {
      BitmapBuffer lastRenderBitmapBuffer = mRenderBitmapBuffer;
      // Recycle the consumed buffer and acquire for a new one.
      mRenderBitmapBuffer = mBufferGroup.swapBitmapBuffer(lastRenderBitmapBuffer);
      // Step 5: A new bitmap suggests that there is at least one frame available and the ImageView
      // is ready to consume it on the next tick.
      if (mRenderBitmapBuffer != null && lastRenderBitmapBuffer != mRenderBitmapBuffer) {
        invalidate();
      }
    });
  }
}
