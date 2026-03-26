// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.lottieadapter;

import android.animation.Animator;
import android.animation.ValueAnimator;
import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.graphics.ColorFilter;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffColorFilter;
import android.graphics.drawable.Drawable;
import android.util.AttributeSet;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import android.widget.ImageView;
import androidx.annotation.FloatRange;
import androidx.annotation.MainThread;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.RawRes;
import com.lynx.lottieadapter.base.R;
import com.lynx.lottieadapter.lottie.FontAssetDelegate;
import com.lynx.lottieadapter.lottie.ImageAssetDelegate;
import com.lynx.lottieadapter.lottie.LottieComposition;
import com.lynx.lottieadapter.lottie.LottieDrawable;
import com.lynx.lottieadapter.lottie.LottieProperty;
import com.lynx.lottieadapter.lottie.model.KeyPath;
import com.lynx.lottieadapter.lottie.value.LottieFrameInfo;
import com.lynx.lottieadapter.lottie.value.LottieValueCallback;
import com.lynx.lottieadapter.lottie.value.SimpleLottieValueCallback;
import java.util.List;
import org.json.JSONObject;

public class LottieAdapterView extends FrameLayout implements ILottieAdapter {
  private final ILottieAdapter mAdapter;

  public LottieAdapterView(Context context) {
    this(context, LottieAdapterFactory.getGlobalOption());
  }

  public LottieAdapterView(Context context, LottieAdapterOption option) {
    super(context);
    mAdapter = initAdapter(context, option);
  }

  public LottieAdapterView(Context context, AttributeSet attrs) {
    super(context, attrs);
    if (attrs != null) {
      TypedArray ta = context.obtainStyledAttributes(attrs, R.styleable.LottieAdapterView);
      try {
        LottieAdapterOption option = parseAttributesForOption(ta);
        mAdapter = initAdapter(context, option);
        parseAttributes(ta);
      } finally {
        ta.recycle();
      }
    } else {
      mAdapter = initAdapter(context);
    }
  }

  public LottieAdapterView(Context context, AttributeSet attrs, int defStyleAttr) {
    super(context, attrs, defStyleAttr);
    if (attrs != null) {
      TypedArray ta = context.obtainStyledAttributes(attrs, R.styleable.LottieAdapterView);
      try {
        LottieAdapterOption option = parseAttributesForOption(ta);
        mAdapter = initAdapter(context, option);
        parseAttributes(ta);
      } finally {
        ta.recycle();
      }
    } else {
      mAdapter = initAdapter(context);
    }
  }

  private ILottieAdapter initAdapter(Context context, LottieAdapterOption option) {
    ILottieAdapter adapter = LottieAdapterFactory.create(context, option);
    adapter.setParentView(this);
    return adapter;
  }

  private ILottieAdapter initAdapter(Context context) {
    ILottieAdapter adapter = LottieAdapterFactory.create(context);
    adapter.setParentView(this);
    return adapter;
  }

  private LottieAdapterOption parseAttributesForOption(TypedArray ta) {
    LottieAdapterOption option = LottieAdapterFactory.getGlobalOption();
    int typeNumber = ta.getInt(R.styleable.LottieAdapterView_lottie_adapter_type,
        LottieAdapterFactory.getDefaultType().ordinal());
    AnimationType type = AnimationType.values()[typeNumber];
    option.setType(type);
    option.setUseImageMode(
        ta.getBoolean(R.styleable.LottieAdapterView_lottie_adapter_useImageMode, false));
    option.setMultiThreadAccelerate(
        ta.getBoolean(R.styleable.LottieAdapterView_lottie_adapter_multiThreadAccelerate, false));
    return option;
  }

  private void parseAttributes(TypedArray ta) {
    // Handle rawRes and fileName
    if (ta.hasValue(R.styleable.LottieAdapterView_lottie_rawRes)
        && ta.hasValue(R.styleable.LottieAdapterView_lottie_fileName)) {
      throw new IllegalArgumentException("lottie_rawRes and lottie_fileName cannot be used at "
          + "the same time. Please use only one at once.");
    } else if (ta.hasValue(R.styleable.LottieAdapterView_lottie_rawRes)) {
      int rawResId = ta.getResourceId(R.styleable.LottieAdapterView_lottie_rawRes, 0);
      if (rawResId != 0) {
        setAnimation(rawResId);
      }
    } else if (ta.hasValue(R.styleable.LottieAdapterView_lottie_fileName)) {
      String fileName = ta.getString(R.styleable.LottieAdapterView_lottie_fileName);
      if (fileName != null) {
        setAnimation(fileName);
      }
    } else if (ta.hasValue(R.styleable.LottieAdapterView_lottie_url)) {
      String url = ta.getString(R.styleable.LottieAdapterView_lottie_url);
      if (url != null) {
        setAnimationFromUrl(url);
      }
    }

    // Handle autoPlay
    if (ta.getBoolean(R.styleable.LottieAdapterView_lottie_autoPlay, false)) {
      setAutoPlay(true);
    }

    // Handle loop
    if (ta.getBoolean(R.styleable.LottieAdapterView_lottie_loop, false)) {
      setRepeatCount(ValueAnimator.INFINITE);
    }

    // Handle repeatMode
    if (ta.hasValue(R.styleable.LottieAdapterView_lottie_adapter_repeatMode)) {
      setRepeatMode(ta.getInt(
          R.styleable.LottieAdapterView_lottie_adapter_repeatMode, ValueAnimator.RESTART));
    }

    // Handle repeatCount
    if (ta.hasValue(R.styleable.LottieAdapterView_lottie_repeatCount)) {
      setRepeatCount(
          ta.getInt(R.styleable.LottieAdapterView_lottie_repeatCount, ValueAnimator.INFINITE));
    }

    // Handle imageAssetsFolder
    String imageAssetsFolder = ta.getString(R.styleable.LottieAdapterView_lottie_imageAssetsFolder);
    setImageAssetsFolder(imageAssetsFolder);

    // Handle progress
    setProgress(ta.getFloat(R.styleable.LottieAdapterView_lottie_progress, 0));

    // Handle colorFilter
    if (ta.hasValue(R.styleable.LottieAdapterView_lottie_colorFilter)) {
      PorterDuffColorFilter filter = new PorterDuffColorFilter(
          ta.getColor(R.styleable.LottieAdapterView_lottie_colorFilter, Color.TRANSPARENT),
          PorterDuff.Mode.SRC_ATOP);
      setColorFilter(filter);
      KeyPath keyPath = new KeyPath("**");
      addValueCallback(
          keyPath, LottieProperty.COLOR_FILTER, new SimpleLottieValueCallback<ColorFilter>() {
            @Override
            public ColorFilter getValue(LottieFrameInfo<ColorFilter> frameInfo) {
              return filter;
            }
          });
    }

    // Handle scale
    if (ta.hasValue(R.styleable.LottieAdapterView_lottie_scale)) {
      setScale(ta.getFloat(R.styleable.LottieAdapterView_lottie_scale, 1f));
    }

    if (ta.hasValue(R.styleable.LottieAdapterView_android_scaleType)) {
      int scaleTypeInt = ta.getInt(R.styleable.LottieAdapterView_android_scaleType,
          ImageView.ScaleType.FIT_CENTER.ordinal());
      setScaleType(ImageView.ScaleType.values()[scaleTypeInt]);
    }
  }

  @Override
  public void setParentView(@NonNull ViewGroup parentView) {
    throw new UnsupportedOperationException("setParentView should not be called here");
  }

  @Override
  public void setAnimation(@RawRes int rawRes) {
    mAdapter.setAnimation(rawRes);
  }

  @Override
  public void setAnimation(String assetName) {
    mAdapter.setAnimation(assetName);
  }

  @Override
  public void setAnimation(JSONObject json) {
    mAdapter.setAnimation(json);
  }

  @Override
  public void setAnimationFromJson(String jsonString) {
    mAdapter.setAnimationFromJson(jsonString);
  }

  @Override
  public void setAnimationFromUrl(String url) {
    mAdapter.setAnimationFromUrl(url);
  }

  @Override
  public void setComposition(@NonNull LottieComposition composition) {
    mAdapter.setComposition(composition);
  }

  @Nullable
  @Override
  public LottieComposition getComposition() {
    return mAdapter.getComposition();
  }

  @Override
  public void setImageAssetsFolder(String imageAssetsFolder) {
    mAdapter.setImageAssetsFolder(imageAssetsFolder);
  }

  @Override
  public String getImageAssetsFolder() {
    return mAdapter.getImageAssetsFolder();
  }

  @Override
  public void setImageAssetDelegate(ImageAssetDelegate delegate) {
    mAdapter.setImageAssetDelegate(delegate);
  }

  @Override
  public void setFontAssetDelegate(@NonNull FontAssetDelegate delegate) {
    mAdapter.setFontAssetDelegate(delegate);
  }

  @MainThread
  @Override
  public void playAnimation() {
    mAdapter.playAnimation();
  }

  @MainThread
  @Override
  public void resumeAnimation() {
    mAdapter.resumeAnimation();
  }

  @MainThread
  @Override
  public void cancelAnimation() {
    mAdapter.cancelAnimation();
  }

  @MainThread
  @Override
  public void pauseAnimation() {
    mAdapter.pauseAnimation();
  }

  @Override
  public void setProgress(@FloatRange(from = 0f, to = 1f) float progress) {
    mAdapter.setProgress(progress);
  }
  @Override
  public float getProgress() {
    return mAdapter.getProgress();
  }

  @Override
  public void setSpeed(float speed) {
    mAdapter.setSpeed(speed);
  }

  @Override
  public float getSpeed() {
    return mAdapter.getSpeed();
  }

  @Override
  public void setRepeatCount(int count) {
    mAdapter.setRepeatCount(count);
  }

  @Override
  public int getRepeatCount() {
    return mAdapter.getRepeatCount();
  }

  @Override
  public void setRepeatMode(@LottieDrawable.RepeatMode int mode) {
    mAdapter.setRepeatMode(mode);
  }

  @Override
  public int getRepeatMode() {
    return mAdapter.getRepeatMode();
  }

  @Override
  public boolean isAnimating() {
    return mAdapter.isAnimating();
  }

  @Override
  public void setScale(float scale) {
    mAdapter.setScale(scale);
  }

  @Override
  public float getScale() {
    return mAdapter.getScale();
  }

  @Override
  public void addAnimatorListener(Animator.AnimatorListener listener) {
    mAdapter.addAnimatorListener(listener);
  }

  @Override
  public void removeAnimatorListener(Animator.AnimatorListener listener) {
    mAdapter.removeAnimatorListener(listener);
  }

  @Override
  public void removeAllAnimatorListeners() {
    mAdapter.removeAllAnimatorListeners();
  }

  @Override
  public void addAnimatorUpdateListener(ValueAnimator.AnimatorUpdateListener listener) {
    mAdapter.addAnimatorUpdateListener(listener);
  }

  @Override
  public void removeUpdateListener(ValueAnimator.AnimatorUpdateListener listener) {
    mAdapter.removeUpdateListener(listener);
  }

  @Override
  public void removeAllUpdateListeners() {
    mAdapter.removeAllUpdateListeners();
  }

  @Override
  public <T> void addValueCallback(
      KeyPath keyPath, T property, SimpleLottieValueCallback<T> callback) {
    mAdapter.addValueCallback(keyPath, property, callback);
  }

  @Override
  public List<KeyPath> resolveKeyPath(KeyPath keyPath) {
    return mAdapter.resolveKeyPath(keyPath);
  }

  @Override
  public <T> void addValueCallback(KeyPath keyPath, T property, LottieValueCallback<T> callback) {
    mAdapter.addValueCallback(keyPath, property, callback);
  }

  @Override
  public void removeAllLottieOnCompositionLoadedListener() {
    mAdapter.removeAllLottieOnCompositionLoadedListener();
  }

  @Override
  public void setMinFrame(int startFrame) {
    mAdapter.setMinFrame(startFrame);
  }

  @Override
  public float getMinFrame() {
    return mAdapter.getMinFrame();
  }

  @Override
  public void setMaxFrame(int endFrame) {
    mAdapter.setMaxFrame(endFrame);
  }

  @Override
  public float getMaxFrame() {
    return mAdapter.getMaxFrame();
  }

  @Override
  public void setMinAndMaxFrame(int minFrame, int maxFrame) {
    mAdapter.setMinAndMaxFrame(minFrame, maxFrame);
  }

  @Override
  public void setMinProgress(float startProgress) {
    mAdapter.setMinProgress(startProgress);
  }

  @Override
  public void setMaxProgress(@FloatRange(from = 0f, to = 1f) float endProgress) {
    mAdapter.setMaxProgress(endProgress);
  }

  @Override
  public void setMinAndMaxProgress(@FloatRange(from = 0f, to = 1f) float minProgress,
      @FloatRange(from = 0f, to = 1f) float maxProgress) {
    mAdapter.setMinAndMaxProgress(minProgress, maxProgress);
  }

  @Override
  @Nullable
  public Bitmap updateBitmap(String id, @Nullable Bitmap bitmap) {
    return mAdapter.updateBitmap(id, bitmap);
  }

  @Override
  public void setFrame(int frame) {
    mAdapter.setFrame(frame);
  }

  @Override
  public int getFrame() {
    return mAdapter.getFrame();
  }

  @Override
  public long getDuration() {
    return mAdapter.getDuration();
  }

  @Override
  public void setImageResource(int resId) {
    mAdapter.setImageResource(resId);
  }

  @Override
  public void setImageDrawable(Drawable drawable) {
    mAdapter.setImageDrawable(drawable);
  }

  @Override
  public void setAutoPlay(boolean autoPlay) {
    mAdapter.setAutoPlay(autoPlay);
  }

  @Override
  public AnimationType getContentType() {
    return mAdapter.getContentType();
  }

  @Override
  public void setColorFilter(ColorFilter filter) {
    mAdapter.setColorFilter(filter);
  }

  @Nullable
  @Override
  public ColorFilter getColorFilter() {
    return mAdapter.getColorFilter();
  }

  @Override
  public void setMaxFrameRate(float maxFrameRate) {
    if (mAdapter != null) {
      mAdapter.setMaxFrameRate(maxFrameRate);
    }
  }

  @Override
  public void setScaleType(ImageView.ScaleType scaleType) {
    if (mAdapter != null) {
      mAdapter.setScaleType(scaleType);
    }
  }
}
