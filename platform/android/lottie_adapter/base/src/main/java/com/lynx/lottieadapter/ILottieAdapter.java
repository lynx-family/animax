// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.lottieadapter;

import android.animation.Animator;
import android.animation.ValueAnimator;
import android.graphics.Bitmap;
import android.graphics.ColorFilter;
import android.graphics.drawable.Drawable;
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
import com.lynx.lottieadapter.lottie.LottieDrawable;
import com.lynx.lottieadapter.lottie.model.KeyPath;
import com.lynx.lottieadapter.lottie.value.LottieValueCallback;
import com.lynx.lottieadapter.lottie.value.SimpleLottieValueCallback;
import java.util.List;
import org.json.JSONObject;

public interface ILottieAdapter {
  // Lottie interface BEGIN

  default void setOption(LottieAdapterOption option) {}
  void setAnimation(@RawRes int rawRes);
  void setAnimation(String assetName);
  void setAnimation(JSONObject json);
  void setAnimationFromJson(String jsonString);
  void setAnimationFromUrl(String url);
  void setComposition(@NonNull LottieComposition composition);
  @Nullable LottieComposition getComposition();
  void setImageAssetsFolder(String imageAssetsFolder);
  String getImageAssetsFolder();
  void setImageAssetDelegate(ImageAssetDelegate delegate);
  void setFontAssetDelegate(@NonNull FontAssetDelegate delegate);

  @MainThread void playAnimation();
  @MainThread void resumeAnimation();
  @MainThread void cancelAnimation();
  @MainThread void pauseAnimation();

  void setProgress(@FloatRange(from = 0f, to = 1f) float progress);
  float getProgress();
  void setSpeed(float speed);
  float getSpeed();
  void setRepeatCount(int count);
  int getRepeatCount();
  void setRepeatMode(@LottieDrawable.RepeatMode int mode);
  int getRepeatMode();
  boolean isAnimating();
  void setScale(float scale);
  float getScale();

  void addAnimatorListener(Animator.AnimatorListener listener);
  void removeAnimatorListener(Animator.AnimatorListener listener);
  void removeAllAnimatorListeners();
  void addAnimatorUpdateListener(ValueAnimator.AnimatorUpdateListener listener);
  void removeUpdateListener(ValueAnimator.AnimatorUpdateListener listener);
  void removeAllUpdateListeners();
  // TODO(aiyongbiao.rick): These methods are not used for now, implement them later.
  //    void setTextDelegate(TextDelegate delegate);
  //    void setRenderMode(RenderMode renderMode);
  //    boolean addLottieOnCompositionLoadedListener(@NonNull LottieOnCompositionLoadedListener
  //    listener); boolean removeLottieOnCompositionLoadedListener(@NonNull
  //    LottieOnCompositionLoadedListener listener);

  List<KeyPath> resolveKeyPath(KeyPath keyPath);
  <T> void addValueCallback(KeyPath keyPath, T property, LottieValueCallback<T> callback);
  <T> void addValueCallback(KeyPath keyPath, T property, SimpleLottieValueCallback<T> callback);

  void removeAllLottieOnCompositionLoadedListener();

  void setMinFrame(int startFrame);
  float getMinFrame();
  void setMaxFrame(int endFrame);
  float getMaxFrame();
  void setMinAndMaxFrame(int minFrame, int maxFrame);
  void setMinProgress(float startProgress);
  void setMaxProgress(@FloatRange(from = 0f, to = 1f) float endProgress);
  void setMinAndMaxProgress(@FloatRange(from = 0f, to = 1f) float minProgress,
      @FloatRange(from = 0f, to = 1f) float maxProgress);

  @Nullable Bitmap updateBitmap(String id, @Nullable Bitmap bitmap);
  void setFrame(int frame);
  int getFrame();
  long getDuration();

  void setImageResource(int resId);
  void setImageDrawable(Drawable drawable);
  void setColorFilter(ColorFilter filter);
  @Nullable ColorFilter getColorFilter();

  // Lottie interface END

  void setParentView(@NonNull ViewGroup parentView);
  void setAutoPlay(boolean autoPlay);
  AnimationType getContentType();
  void setMaxFrameRate(float maxFrameRate);
  void setScaleType(ScaleType scaleType);
}
