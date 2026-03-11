// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.ui;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.view.View;
import android.widget.FrameLayout;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.RestrictTo;

public class AnimaXContainerView extends FrameLayout {
  public interface IDispatchDrawHook {
    void beforeDispatchDraw(Canvas canvas);
    boolean useBitmapOnDraw();
  }
  @Nullable private IDispatchDrawHook mDispatchDrawHook;
  @Nullable private AnimaXView mAnimaXView;
  @Nullable private AnimaXImageView mAnimaXImageView;

  @RestrictTo(RestrictTo.Scope.LIBRARY)
  public AnimaXContainerView(Context context) {
    super(context);
  }

  @RestrictTo(RestrictTo.Scope.LIBRARY)
  public void addChildAnimaXView(
      @NonNull View childView, @Nullable IDispatchDrawHook dispatchDrawHook) {
    if (childView instanceof AnimaXView) {
      mAnimaXView = (AnimaXView) childView;
    } else if (childView instanceof AnimaXImageView) {
      mAnimaXImageView = (AnimaXImageView) childView;
    } else {
      return;
    }
    childView.setLayoutParams(new FrameLayout.LayoutParams(
        FrameLayout.LayoutParams.MATCH_PARENT, FrameLayout.LayoutParams.MATCH_PARENT));
    addView(childView);
    mDispatchDrawHook = dispatchDrawHook;
  }

  @Nullable
  public AnimaXView getAnimaXView() {
    return mAnimaXView;
  }

  @Nullable
  public AnimaXImageView getAnimaXImageView() {
    return mAnimaXImageView;
  }

  @Override
  protected void dispatchDraw(final Canvas canvas) {
    if (mDispatchDrawHook != null) {
      mDispatchDrawHook.beforeDispatchDraw(canvas);
    }
    super.dispatchDraw(canvas);
    final boolean toDrawBitmap = !canvas.isHardwareAccelerated() && mDispatchDrawHook != null
        && mDispatchDrawHook.useBitmapOnDraw();
    if (toDrawBitmap) {
      Bitmap bitmap = null;
      if (mAnimaXView != null) {
        bitmap = mAnimaXView.getBitmap();
      } else if (mAnimaXImageView != null) {
        bitmap = mAnimaXImageView.getCurrentBitmap();
      }
      if (bitmap != null) {
        canvas.drawBitmap(bitmap, getPaddingLeft(), getPaddingTop(), null);
      }
    }
  }
}
