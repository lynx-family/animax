// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.lottieadapter;

import android.animation.Animator;
import android.animation.ValueAnimator;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.ColorFilter;
import android.graphics.drawable.Drawable;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView.ScaleType;
import androidx.annotation.FloatRange;
import androidx.annotation.MainThread;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.RawRes;
import com.lynx.lottieadapter.lottie.FontAssetDelegate;
import com.lynx.lottieadapter.lottie.ImageAssetDelegate;
import com.lynx.lottieadapter.lottie.LottieComposition;
import com.lynx.lottieadapter.lottie.model.KeyPath;
import com.lynx.lottieadapter.lottie.value.LottieValueCallback;
import com.lynx.lottieadapter.lottie.value.SimpleLottieValueCallback;
import java.util.Collections;
import java.util.List;
import org.json.JSONObject;

public class EmptyLottieAdapter implements ILottieAdapter {
  public EmptyLottieAdapter(Context context) {}

  @Override
  public void setParentView(@NonNull ViewGroup parentView) {}

  @Override
  public void setComposition(@NonNull LottieComposition composition) {}

  @Nullable
  @Override
  public LottieComposition getComposition() {
    return null;
  }

  @Override
  public void setAnimation(@RawRes int rawRes) {}

  @Override
  public void setAnimation(String assetName) {}

  @Override
  public void setAnimation(JSONObject json) {}

  @Override
  public void setAnimationFromJson(String jsonString) {}

  @Override
  public void setAnimationFromUrl(String url) {}

  @Override
  public void setImageAssetsFolder(String imageAssetsFolder) {}

  @Override
  public String getImageAssetsFolder() {
    return "";
  }

  @Override
  public void setImageAssetDelegate(ImageAssetDelegate delegate) {}

  @Override
  public void setFontAssetDelegate(@NonNull FontAssetDelegate delegate) {}

  @MainThread
  @Override
  public void playAnimation() {}

  @MainThread
  @Override
  public void resumeAnimation() {}

  @MainThread
  @Override
  public void cancelAnimation() {}

  @MainThread
  @Override
  public void pauseAnimation() {}

  @Override
  public void setProgress(@FloatRange(from = 0f, to = 1f) float progress) {}

  @Override
  public float getProgress() {
    return 0f;
  }

  @Override
  public void setSpeed(float speed) {}

  @Override
  public float getSpeed() {
    return 0f;
  }

  @Override
  public void setRepeatCount(int count) {}

  @Override
  public int getRepeatCount() {
    return 0;
  }

  @Override
  public int getRepeatMode() {
    return 0;
  }

  @Override
  public boolean isAnimating() {
    return false;
  }

  @Override
  public void setScale(float scale) {}

  @Override
  public float getScale() {
    return 1f;
  }

  @Override
  public void addAnimatorListener(Animator.AnimatorListener listener) {}

  @Override
  public void removeAnimatorListener(Animator.AnimatorListener listener) {}

  @Override
  public void removeAllAnimatorListeners() {}

  @Override
  public void addAnimatorUpdateListener(ValueAnimator.AnimatorUpdateListener listener) {}

  @Override
  public void removeUpdateListener(ValueAnimator.AnimatorUpdateListener listener) {}

  @Override
  public void removeAllUpdateListeners() {}

  @Override
  public <T> void addValueCallback(
      KeyPath keyPath, T property, SimpleLottieValueCallback<T> callback) {}

  @Override
  public List<KeyPath> resolveKeyPath(KeyPath keyPath) {
    return Collections.emptyList();
  }

  @Override
  public <T> void addValueCallback(KeyPath keyPath, T property, LottieValueCallback<T> callback) {}

  @Override
  public void removeAllLottieOnCompositionLoadedListener() {}

  @Override
  public void setMinFrame(int startFrame) {}

  @Override
  public float getMinFrame() {
    return 0f;
  }

  @Override
  public void setMaxFrame(int endFrame) {}

  @Override
  public float getMaxFrame() {
    return 0f;
  }

  @Override
  public void setMinAndMaxFrame(int minFrame, int maxFrame) {}

  @Override
  public void setMinProgress(float startProgress) {}

  @Override
  public void setMaxProgress(@FloatRange(from = 0f, to = 1f) float endProgress) {}

  @Override
  public void setMinAndMaxProgress(@FloatRange(from = 0f, to = 1f) float minProgress,
      @FloatRange(from = 0f, to = 1f) float maxProgress) {}

  @Override
  @Nullable
  public Bitmap updateBitmap(String id, @Nullable Bitmap bitmap) {
    return null;
  }

  @Override
  public void setFrame(int frame) {}

  @Override
  public int getFrame() {
    return 0;
  }

  @Override
  public long getDuration() {
    return 0L;
  }

  @Override
  public void setAutoPlay(boolean autoPlay) {}

  @Override
  public void setImageResource(int resId) {}

  @Override
  public void setImageDrawable(Drawable drawable) {}

  @Override
  public AnimationType getContentType() {
    return AnimationType.EMPTY;
  }

  @Override
  public void setColorFilter(ColorFilter filter) {}

  @Nullable
  @Override
  public ColorFilter getColorFilter() {
    return null;
  }

  @Override
  public void setRepeatMode(int mode) {}

  @Override
  public void setMaxFrameRate(float maxFrameRate) {}

  @Override
  public void setScaleType(ScaleType scaleType) {}
}
