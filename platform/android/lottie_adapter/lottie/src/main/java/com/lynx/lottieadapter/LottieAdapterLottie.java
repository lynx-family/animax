// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.lottieadapter;

import android.animation.Animator;
import android.animation.ValueAnimator;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.ColorFilter;
import android.graphics.Typeface;
import android.graphics.drawable.Drawable;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import android.widget.ImageView;
import androidx.annotation.FloatRange;
import androidx.annotation.Keep;
import androidx.annotation.MainThread;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.RawRes;
import com.airbnb.lottie.LottieAnimationView;
import com.lynx.lottieadapter.lottie.FontAssetDelegate;
import com.lynx.lottieadapter.lottie.ImageAssetDelegate;
import com.lynx.lottieadapter.lottie.LottieComposition;
import com.lynx.lottieadapter.lottie.LottieImageAsset;
import com.lynx.lottieadapter.lottie.LottieProperty;
import com.lynx.lottieadapter.lottie.model.KeyPath;
import com.lynx.lottieadapter.lottie.value.LottieFrameInfo;
import com.lynx.lottieadapter.lottie.value.LottieValueCallback;
import com.lynx.lottieadapter.lottie.value.SimpleLottieValueCallback;
import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.List;
import org.json.JSONObject;

@Keep
public class LottieAdapterLottie implements ILottieAdapter {
  private final LottieAnimationView mLottieView;

  public LottieAdapterLottie(Context context) {
    mLottieView = new LottieAnimationView(context);
  }

  public LottieAdapterLottie(Context context, LottieAdapterOption option) {
    this(context);
  }

  @Override
  public void setParentView(ViewGroup parentView) {
    FrameLayout.LayoutParams layoutParams = new FrameLayout.LayoutParams(
        FrameLayout.LayoutParams.MATCH_PARENT, FrameLayout.LayoutParams.MATCH_PARENT);
    parentView.addView(mLottieView, layoutParams);
  }

  @Override
  public void setComposition(@NonNull LottieComposition composition) {
    Object object = composition.getComposition();
    if (object instanceof com.airbnb.lottie.LottieComposition) {
      mLottieView.setComposition((com.airbnb.lottie.LottieComposition) object);
    }
  }

  @Nullable
  @Override
  public LottieComposition getComposition() {
    if (mLottieView.getComposition() == null) {
      return null;
    }
    return new LottieComposition(mLottieView.getComposition());
  }

  @Override
  public void setAnimation(@RawRes int rawRes) {
    mLottieView.setAnimation(rawRes);
  }

  @Override
  public void setAnimation(String assetName) {
    mLottieView.setAnimation(assetName);
  }

  @Override
  public void setAnimation(JSONObject json) {
    mLottieView.setAnimation(json);
  }

  @Override
  public void setAnimationFromJson(String jsonString) {
    mLottieView.setAnimationFromJson(jsonString);
  }

  @Override
  public void setAnimationFromUrl(String url) {
    mLottieView.setAnimationFromUrl(url);
  }

  @Override
  public void setImageAssetsFolder(String imageAssetsFolder) {
    mLottieView.setImageAssetsFolder(imageAssetsFolder);
  }

  @Override
  public String getImageAssetsFolder() {
    return mLottieView.getImageAssetsFolder();
  }

  @Override
  public void setImageAssetDelegate(ImageAssetDelegate delegate) {
    if (delegate == null) {
      mLottieView.setImageAssetDelegate(null);
      return;
    }

    mLottieView.setImageAssetDelegate(new com.airbnb.lottie.ImageAssetDelegate() {
      @Override
      public Bitmap fetchBitmap(com.airbnb.lottie.LottieImageAsset asset) {
        return delegate.fetchBitmap(new LottieImageAsset(asset.getWidth(), asset.getHeight(),
            asset.getId(), asset.getFileName(), asset.getDirName(), asset.isHasAlpha()));
      }
    });
  }

  @Override
  public void setFontAssetDelegate(@NonNull FontAssetDelegate delegate) {
    mLottieView.setFontAssetDelegate(new com.airbnb.lottie.FontAssetDelegate() {
      @Override
      public Typeface fetchFont(String fontFamily) {
        return delegate.fetchFont(fontFamily);
      }

      @Override
      public String getFontPath(String fontFamily) {
        return delegate.getFontPath(fontFamily);
      }
    });
  }

  @MainThread
  @Override
  public void playAnimation() {
    mLottieView.playAnimation();
  }

  @MainThread
  @Override
  public void resumeAnimation() {
    mLottieView.resumeAnimation();
  }

  @MainThread
  @Override
  public void cancelAnimation() {
    mLottieView.cancelAnimation();
  }

  @MainThread
  @Override
  public void pauseAnimation() {
    mLottieView.pauseAnimation();
  }

  @Override
  public void setProgress(@FloatRange(from = 0f, to = 1f) float progress) {
    mLottieView.setProgress(progress);
  }

  @Override
  public float getProgress() {
    return mLottieView.getProgress();
  }

  @Override
  public void setSpeed(float speed) {
    mLottieView.setSpeed(speed);
  }

  @Override
  public float getSpeed() {
    return mLottieView.getSpeed();
  }

  @Override
  public void setRepeatCount(int count) {
    mLottieView.setRepeatCount(count);
  }

  @Override
  public int getRepeatCount() {
    return mLottieView.getRepeatCount();
  }

  @Override
  public int getRepeatMode() {
    return mLottieView.getRepeatMode();
  }

  @Override
  public boolean isAnimating() {
    return mLottieView.isAnimating();
  }

  @Override
  public void setScale(float scale) {
    mLottieView.setScale(scale);
  }

  @Override
  public float getScale() {
    return mLottieView.getScale();
  }

  @Override
  public void addAnimatorListener(Animator.AnimatorListener listener) {
    mLottieView.addAnimatorListener(listener);
  }

  @Override
  public void removeAnimatorListener(Animator.AnimatorListener listener) {
    mLottieView.removeAnimatorListener(listener);
  }

  @Override
  public void removeAllAnimatorListeners() {
    mLottieView.removeAllAnimatorListeners();
  }

  @Override
  public void addAnimatorUpdateListener(ValueAnimator.AnimatorUpdateListener listener) {
    mLottieView.addAnimatorUpdateListener(listener);
  }

  @Override
  public void removeUpdateListener(ValueAnimator.AnimatorUpdateListener listener) {
    mLottieView.removeUpdateListener(listener);
  }

  @Override
  public void removeAllUpdateListeners() {
    mLottieView.removeAllUpdateListeners();
  }

  @Override
  public List<KeyPath> resolveKeyPath(KeyPath keyPath) {
    List<com.airbnb.lottie.model.KeyPath> lottieKeyPaths =
        mLottieView.resolveKeyPath(new com.airbnb.lottie.model.KeyPath(keyPath.getKeysArray()));
    List<KeyPath> keyPaths = new ArrayList<>();
    for (com.airbnb.lottie.model.KeyPath lottieKeyPath : lottieKeyPaths) {
      try {
        Field keysField = com.airbnb.lottie.model.KeyPath.class.getDeclaredField("keys");
        keysField.setAccessible(true);
        List<String> keys = (List<String>) keysField.get(lottieKeyPath);
        keyPaths.add(new KeyPath(keys.toArray(new String[0])));
      } catch (NoSuchFieldException e) {
        e.printStackTrace();
      } catch (IllegalAccessException e) {
        e.printStackTrace();
      }
    }
    return keyPaths;
  }

  @Override
  public <T> void addValueCallback(KeyPath keyPath, T property, LottieValueCallback<T> callback) {
    Object lottieProperty = toLottieProperty(property);
    if (lottieProperty == null) {
      return;
    }
    mLottieView.addValueCallback(
        new com.airbnb.lottie.model.KeyPath(keyPath.getKeysArray()), lottieProperty, frameInfo -> {
          LottieFrameInfo newInfo = new LottieFrameInfo();
          newInfo.set(frameInfo.getStartFrame(), frameInfo.getEndFrame(), frameInfo.getStartValue(),
              frameInfo.getEndValue(), frameInfo.getLinearKeyframeProgress(),
              frameInfo.getInterpolatedKeyframeProgress(), frameInfo.getOverallProgress());
          return callback.getValue(newInfo);
        });
  }

  @Override
  public <T> void addValueCallback(
      KeyPath keyPath, T property, SimpleLottieValueCallback<T> callback) {
    addValueCallback(keyPath, property, new LottieValueCallback<T>() {
      @Override
      public T getValue(LottieFrameInfo<T> frameInfo) {
        return callback.getValue(frameInfo);
      }
    });
  }

  private Object toLottieProperty(Object property) {
    if (property == LottieProperty.COLOR) {
      return com.airbnb.lottie.LottieProperty.COLOR;
    } else if (property == LottieProperty.STROKE_COLOR) {
      return com.airbnb.lottie.LottieProperty.STROKE_COLOR;
    } else if (property == LottieProperty.TRANSFORM_OPACITY) {
      return com.airbnb.lottie.LottieProperty.TRANSFORM_OPACITY;
    } else if (property == LottieProperty.OPACITY) {
      return com.airbnb.lottie.LottieProperty.OPACITY;
    } else if (property == LottieProperty.TRANSFORM_ANCHOR_POINT) {
      return com.airbnb.lottie.LottieProperty.TRANSFORM_ANCHOR_POINT;
    } else if (property == LottieProperty.TRANSFORM_POSITION) {
      return com.airbnb.lottie.LottieProperty.TRANSFORM_POSITION;
    } else if (property == LottieProperty.ELLIPSE_SIZE) {
      return com.airbnb.lottie.LottieProperty.ELLIPSE_SIZE;
    } else if (property == LottieProperty.RECTANGLE_SIZE) {
      return com.airbnb.lottie.LottieProperty.RECTANGLE_SIZE;
    } else if (property == LottieProperty.CORNER_RADIUS) {
      return com.airbnb.lottie.LottieProperty.CORNER_RADIUS;
    } else if (property == LottieProperty.POSITION) {
      return com.airbnb.lottie.LottieProperty.POSITION;
    } else if (property == LottieProperty.TRANSFORM_SCALE) {
      return com.airbnb.lottie.LottieProperty.TRANSFORM_SCALE;
    } else if (property == LottieProperty.TRANSFORM_ROTATION) {
      return com.airbnb.lottie.LottieProperty.TRANSFORM_ROTATION;
    } else if (property == LottieProperty.TRANSFORM_SKEW) {
      return com.airbnb.lottie.LottieProperty.TRANSFORM_SKEW;
    } else if (property == LottieProperty.TRANSFORM_SKEW_ANGLE) {
      return com.airbnb.lottie.LottieProperty.TRANSFORM_SKEW_ANGLE;
    } else if (property == LottieProperty.STROKE_WIDTH) {
      return com.airbnb.lottie.LottieProperty.STROKE_WIDTH;
    } else if (property == LottieProperty.TEXT_TRACKING) {
      return com.airbnb.lottie.LottieProperty.TEXT_TRACKING;
    } else if (property == LottieProperty.REPEATER_COPIES) {
      return com.airbnb.lottie.LottieProperty.REPEATER_COPIES;
    } else if (property == LottieProperty.REPEATER_OFFSET) {
      return com.airbnb.lottie.LottieProperty.REPEATER_OFFSET;
    } else if (property == LottieProperty.POLYSTAR_POINTS) {
      return com.airbnb.lottie.LottieProperty.POLYSTAR_POINTS;
    } else if (property == LottieProperty.POLYSTAR_ROTATION) {
      return com.airbnb.lottie.LottieProperty.POLYSTAR_ROTATION;
    } else if (property == LottieProperty.POLYSTAR_INNER_RADIUS) {
      return com.airbnb.lottie.LottieProperty.POLYSTAR_INNER_RADIUS;
    } else if (property == LottieProperty.POLYSTAR_OUTER_RADIUS) {
      return com.airbnb.lottie.LottieProperty.POLYSTAR_OUTER_RADIUS;
    } else if (property == LottieProperty.POLYSTAR_INNER_ROUNDEDNESS) {
      return com.airbnb.lottie.LottieProperty.POLYSTAR_INNER_ROUNDEDNESS;
    } else if (property == LottieProperty.POLYSTAR_OUTER_ROUNDEDNESS) {
      return com.airbnb.lottie.LottieProperty.POLYSTAR_OUTER_ROUNDEDNESS;
    } else if (property == LottieProperty.TRANSFORM_START_OPACITY) {
      return com.airbnb.lottie.LottieProperty.TRANSFORM_START_OPACITY;
    } else if (property == LottieProperty.TRANSFORM_END_OPACITY) {
      return com.airbnb.lottie.LottieProperty.TRANSFORM_END_OPACITY;
    } else if (property == LottieProperty.TIME_REMAP) {
      return com.airbnb.lottie.LottieProperty.TIME_REMAP;
    } else if (property == LottieProperty.TEXT_SIZE) {
      return com.airbnb.lottie.LottieProperty.TEXT_SIZE;
    } else if (property == LottieProperty.COLOR_FILTER) {
      return com.airbnb.lottie.LottieProperty.COLOR_FILTER;
    } else if (property == LottieProperty.GRADIENT_COLOR) {
      return com.airbnb.lottie.LottieProperty.GRADIENT_COLOR;
    }
    return null;
  }

  @Override
  public void removeAllLottieOnCompositionLoadedListener() {
    mLottieView.removeAllLottieOnCompositionLoadedListener();
  }

  @Override
  public void setMinFrame(int startFrame) {
    mLottieView.setMinFrame(startFrame);
  }

  @Override
  public float getMinFrame() {
    return mLottieView.getMinFrame();
  }

  @Override
  public void setMaxFrame(int endFrame) {
    mLottieView.setMaxFrame(endFrame);
  }

  @Override
  public float getMaxFrame() {
    return mLottieView.getMaxFrame();
  }

  @Override
  public void setMinAndMaxFrame(int minFrame, int maxFrame) {
    mLottieView.setMinAndMaxFrame(minFrame, maxFrame);
  }

  @Override
  public void setMinProgress(float startProgress) {
    mLottieView.setMinProgress(startProgress);
  }

  @Override
  public void setMaxProgress(@FloatRange(from = 0f, to = 1f) float endProgress) {
    mLottieView.setMaxProgress(endProgress);
  }

  @Override
  public void setMinAndMaxProgress(@FloatRange(from = 0f, to = 1f) float minProgress,
      @FloatRange(from = 0f, to = 1f) float maxProgress) {
    mLottieView.setMinAndMaxProgress(minProgress, maxProgress);
  }

  @Override
  @Nullable
  public Bitmap updateBitmap(String id, @Nullable Bitmap bitmap) {
    return mLottieView.updateBitmap(id, bitmap);
  }

  @Override
  public void setFrame(int frame) {
    mLottieView.setFrame(frame);
  }

  @Override
  public int getFrame() {
    return mLottieView.getFrame();
  }

  @Override
  public long getDuration() {
    return mLottieView.getDuration();
  }

  @Override
  public void setImageResource(int resId) {
    mLottieView.setImageResource(resId);
  }

  @Override
  public void setImageDrawable(Drawable drawable) {
    mLottieView.setImageDrawable(drawable);
  }

  @Override
  public void setAutoPlay(boolean autoPlay) {
    try {
      Field field = LottieAnimationView.class.getDeclaredField("autoPlay");
      field.setAccessible(true);
      field.setBoolean(mLottieView, autoPlay);

      Field detachFiled = LottieAnimationView.class.getDeclaredField("wasAnimatingWhenDetached");
      detachFiled.setAccessible(true);
      detachFiled.setBoolean(mLottieView, autoPlay);
    } catch (Exception e) {
      e.printStackTrace();
    }
  }

  @Override
  public AnimationType getContentType() {
    return AnimationType.LOTTIE;
  }

  @Override
  public void setColorFilter(ColorFilter filter) {
    mLottieView.setColorFilter(filter);
  }

  @Nullable
  @Override
  public ColorFilter getColorFilter() {
    return mLottieView.getColorFilter();
  }

  @Override
  public void setRepeatMode(int mode) {
    mLottieView.setRepeatMode(mode);
  }

  @Override
  public void setMaxFrameRate(float maxFrameRate) {
    // do not supported
  }

  @Override
  public void setScaleType(ImageView.ScaleType scaleType) {
    if (mLottieView != null) {
      mLottieView.setScaleType(scaleType);
    }
  }
}
