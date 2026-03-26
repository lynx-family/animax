// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.lottieadapter;

import android.animation.Animator;
import android.animation.ValueAnimator;
import android.content.Context;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.ColorFilter;
import android.graphics.PointF;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffColorFilter;
import android.graphics.drawable.Drawable;
import android.os.Handler;
import android.os.Looper;
import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import android.widget.ImageView;
import androidx.annotation.FloatRange;
import androidx.annotation.Keep;
import androidx.annotation.MainThread;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.RawRes;
import androidx.appcompat.content.res.AppCompatResources;
import androidx.appcompat.widget.AppCompatImageView;
import androidx.core.math.MathUtils;
import androidx.core.util.Consumer;
import com.lynx.animax.ability.NativeAbility;
import com.lynx.animax.composition.AnimaXComposition;
import com.lynx.animax.listener.AnimaXParam;
import com.lynx.animax.listener.AnimationListenerAdapter;
import com.lynx.animax.property.AnimaXFrameInfo;
import com.lynx.animax.property.AnimaXKeyPath;
import com.lynx.animax.property.AnimaXKeyPathListCallback;
import com.lynx.animax.property.AnimaXPropertyCallback;
import com.lynx.animax.property.AnimaXValueCallback;
import com.lynx.animax.property.AnimaXValueParam;
import com.lynx.animax.property.LayerPropertyType;
import com.lynx.animax.ui.AnimaXContext;
import com.lynx.animax.ui.AnimaXImageView;
import com.lynx.animax.ui.AnimaXView;
import com.lynx.animax.ui.IAnimaXPlayerDelegate;
import com.lynx.animax.ui.ObjectFit;
import com.lynx.animax.util.AnimaXLog;
import com.lynx.animax.util.UriUtil;
import com.lynx.lottieadapter.lottie.FontAssetDelegate;
import com.lynx.lottieadapter.lottie.ImageAssetDelegate;
import com.lynx.lottieadapter.lottie.LottieComposition;
import com.lynx.lottieadapter.lottie.LottieDrawable;
import com.lynx.lottieadapter.lottie.LottieProperty;
import com.lynx.lottieadapter.lottie.model.KeyPath;
import com.lynx.lottieadapter.lottie.value.LottieFrameInfo;
import com.lynx.lottieadapter.lottie.value.LottieValueCallback;
import com.lynx.lottieadapter.lottie.value.ScaleXY;
import com.lynx.lottieadapter.lottie.value.SimpleLottieValueCallback;
import java.io.InputStream;
import java.lang.ref.WeakReference;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Objects;
import java.util.Set;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import org.json.JSONObject;

@Keep
public class LottieAdapterAnimaX implements ILottieAdapter {
  private static final String TAG = "LottieAdapterAnimaX";
  private static final double EPSILON = 1e-6;
  @NonNull private final Context mContext;
  @Nullable private volatile IAnimaXPlayerDelegate mAnimaXView;
  @Nullable private volatile AppCompatImageView mImageDrawableView;
  @Nullable private ColorFilter mColorFilter;
  private WeakReference<ViewGroup> mParentViewRef;
  private int mRepeatCount, mRepeatMode;
  private int mStartFrame = 0, mEndFrame = -1;
  private float mSpeed = 1.0f;
  private float mModelStartFrame, mModelEndFrame;
  private float mMaxFrameRate = 60.0f;
  @Nullable private String mImageAssetsFolder;
  @Nullable private LottieComposition mComposition;
  private final Handler mMainThreadHandler = new Handler(Looper.getMainLooper());
  public final Set mExternalListeners =
      Collections.synchronizedSet(new HashSet<>()); // regardless of type
  private final List<Runnable> mOperationQueue = new CopyOnWriteArrayList<>();
  private boolean mAutoPlay = false;
  private boolean mIsAnimationReady = false;
  private boolean mUseImageMode = false;
  private boolean mMultiThreadAccelerate = false;
  @Nullable private String mCurrentUrl;
  @Nullable private ImageView.ScaleType mScaleType;

  public LottieAdapterAnimaX(@NonNull Context context) {
    mContext = context.getApplicationContext();
  }

  @Override
  public void setOption(@NonNull LottieAdapterOption option) {
    mUseImageMode = option.isUseImageMode();
    mMultiThreadAccelerate = option.isMultiThreadAccelerate();
  }

  @Override
  public void setParentView(@NonNull ViewGroup parentView) {
    mParentViewRef = new WeakReference<>(parentView);
    runOnUIThread(new Runnable() {
      @Override
      public void run() {
        onSetParentView();
      }
    });
  }

  @Override
  public void setComposition(@NonNull LottieComposition composition) {
    mComposition = composition;
    if (mAnimaXView != null) {
      setCompositionToAnimaXView(mAnimaXView, composition);
    }
  }

  @Nullable
  @Override
  public LottieComposition getComposition() {
    return mComposition;
  }

  @Override
  public void setAnimation(@RawRes int rawRes) {
    Resources resources = mContext.getResources();
    try (InputStream inputStream = resources.openRawResource(rawRes)) {
      byte[] buffer = new byte[inputStream.available()];
      inputStream.read(buffer);
      String json = new String(buffer, "UTF-8");
      setAnimationFromJson(json);
    } catch (Exception e) {
      AnimaXLog.e(TAG, "Failed to load animation from raw resource: " + e.getMessage());
    }
  }

  @Override
  public void setAnimation(String assetName) {
    setAnimationFromUrl(UriUtil.fromLocalAsset(assetName));
  }

  @Override
  public void setAnimation(JSONObject json) {
    setAnimationFromJson(json != null ? json.toString() : null);
  }

  @Override
  public void setAnimationFromJson(final String jsonString) {
    // The logic for returning here needs to align with the underlying implementation of AnimaX.
    if (jsonString == null || jsonString.isEmpty()) {
      AnimaXLog.w(TAG, "setAnimationFromJson with empty jsonString, do nothing");
      return;
    }
    runOnUIThread(new Runnable() {
      @Override
      public void run() {
        onSetAnimationFromJson(jsonString);
      }
    });
  }

  @Override
  public void setAnimationFromUrl(final String url) {
    // The logic for returning here needs to align with the underlying implementation of AnimaX.
    if (url == null || url.isEmpty()) {
      AnimaXLog.w(TAG, "setAnimationFromUrl with empty url, do nothing");
      return;
    }

    runOnUIThread(new Runnable() {
      @Override
      public void run() {
        onSetAnimationFromUrl(url);
      }
    });
  }

  @Override
  public void setImageAssetsFolder(String imageAssetsFolder) {
    mImageAssetsFolder = imageAssetsFolder;
    if (mAnimaXView != null) {
      if (imageAssetsFolder != null && !imageAssetsFolder.isEmpty()) {
        mAnimaXView.setImageFolder(imageAssetsFolder);
      } else {
        mAnimaXView.setImageFolder(null);
      }
    }
  }

  @Override
  public String getImageAssetsFolder() {
    return mImageAssetsFolder;
  }

  @Override
  public void setImageAssetDelegate(ImageAssetDelegate delegate) {
    noticeNotImplemented("setImageAssetDelegate");
  }

  @Override
  public void setFontAssetDelegate(@NonNull FontAssetDelegate delegate) {
    noticeNotImplemented("setFontAssetDelegate");
  }

  @MainThread
  @Override
  public void playAnimation() {
    enqueueOperation(new Runnable() {
      @Override
      public void run() {
        Objects.requireNonNull(mAnimaXView).play();
      }
    });
  }

  @MainThread
  @Override
  public void resumeAnimation() {
    enqueueOperation(new Runnable() {
      @Override
      public void run() {
        Objects.requireNonNull(mAnimaXView).resume();
      }
    });
  }

  @MainThread
  @Override
  public void cancelAnimation() {
    enqueueOperation(new Runnable() {
      @Override
      public void run() {
        Objects.requireNonNull(mAnimaXView).cancel();
      }
    });
  }

  @MainThread
  @Override
  public void pauseAnimation() {
    enqueueOperation(new Runnable() {
      @Override
      public void run() {
        Objects.requireNonNull(mAnimaXView).pause();
      }
    });
  }

  @Override
  public void setProgress(@FloatRange(from = 0f, to = 1f) float progress) {
    enqueueOperation(new Runnable() {
      @Override
      public void run() {
        Objects.requireNonNull(mAnimaXView).setProgress(progress);
      }
    });
  }

  @Override
  public float getProgress() {
    if (!mIsAnimationReady) {
      return 0;
    }
    return (float) progressForFrame(Objects.requireNonNull(mAnimaXView).getCurrentFrame());
  }

  @Override
  public void setSpeed(float speed) {
    mSpeed = speed;
    if (mAnimaXView != null) {
      mAnimaXView.setSpeed(speed);
    }
  }

  @Override
  public float getSpeed() {
    return mSpeed;
  }

  @Override
  public void setRepeatCount(int count) {
    mRepeatCount = count;
    if (mAnimaXView != null) {
      setRepeatCountToAnimaXView(mAnimaXView, count);
    }
  }

  @Override
  public int getRepeatCount() {
    return mRepeatCount;
  }

  @Override
  public void setRepeatMode(int mode) {
    mRepeatMode = mode;
    if (mAnimaXView != null) {
      setRepeatModeToAnimaXView(mAnimaXView, mode);
    }
  }

  @Override
  public int getRepeatMode() {
    return mRepeatMode;
  }

  @Override
  public boolean isAnimating() {
    return mAnimaXView != null ? mAnimaXView.isAnimating() : false;
  }

  @Override
  public void setScale(float scale) {
    // AnimaX get scale from context.getResources().getDisplayMetrics().density;
    noticeNotImplemented("setScale");
  }

  @Override
  public float getScale() {
    // return the currrent scale
    return mContext.getResources().getDisplayMetrics().density;
  }

  @Override
  public void addAnimatorListener(Animator.AnimatorListener listener) {
    if (listener == null) {
      return;
    }
    mExternalListeners.add(listener);
  }

  @Override
  public void removeAnimatorListener(Animator.AnimatorListener listener) {
    if (listener == null) {
      return;
    }
    mExternalListeners.remove(listener);
  }

  @Override
  public void removeAllAnimatorListeners() {
    removeExternalListeners(Animator.AnimatorListener.class);
  }

  @Override
  public void addAnimatorUpdateListener(ValueAnimator.AnimatorUpdateListener listener) {
    if (listener == null) {
      return;
    }
    if (isUpdateListenersEmpty()) {
      subscribeAllUpdateEvents(true);
    }
    mExternalListeners.add(listener);
  }

  private boolean isUpdateListenersEmpty() {
    for (Object listener : mExternalListeners) {
      if (listener instanceof ValueAnimator.AnimatorUpdateListener) {
        return false;
      }
    }
    return true;
  }

  private void subscribeAllUpdateEvents(boolean subscribe) {
    enqueueOperation(new Runnable() {
      @Override
      public void run() {
        if (mModelEndFrame > mModelStartFrame) {
          int startFrame = (int) mModelStartFrame;
          int endFrame = (int) mModelEndFrame;
          int[] frameRange = new int[endFrame - startFrame + 1];
          for (int i = 0; i < frameRange.length; i++) {
            frameRange[i] = startFrame;
          }
          mAnimaXView.subscribeUpdateEvents(frameRange, subscribe);
        }
      }
    });
  }

  @Override
  public void removeUpdateListener(ValueAnimator.AnimatorUpdateListener listener) {
    if (listener == null) {
      return;
    }
    mExternalListeners.remove(listener);
    if (isUpdateListenersEmpty()) {
      subscribeAllUpdateEvents(false);
    }
  }

  @Override
  public void removeAllUpdateListeners() {
    removeExternalListeners(ValueAnimator.AnimatorUpdateListener.class);
  }

  @Override
  public List<KeyPath> resolveKeyPath(KeyPath keyPath) {
    final List<KeyPath> keyPaths = new ArrayList<>();
    if (keyPath == null) {
      return keyPaths;
    }

    AnimaXKeyPath animaXKeyPath = new AnimaXKeyPath(keyPath.getKeysArray());
    final CountDownLatch latch = new CountDownLatch(1);
    mAnimaXView.getPlayer().getKeysForKeyPath(animaXKeyPath, new AnimaXKeyPathListCallback() {
      @Override
      public void onCallback(@NonNull List<AnimaXKeyPath> result) {
        for (AnimaXKeyPath path : result) {
          keyPaths.add(new KeyPath(path.getKeysList().toArray(new String[0])));
        }
        latch.countDown();
      }
    });
    try {
      latch.await(1000, TimeUnit.MILLISECONDS);
    } catch (InterruptedException e) {
      Thread.currentThread().interrupt();
    }
    return keyPaths;
  }

  private <T> LayerPropertyType toLayerPropertyType(T property) {
    if (property == LottieProperty.COLOR) {
      return LayerPropertyType.COLOR;
    } else if (property == LottieProperty.STROKE_COLOR) {
      return LayerPropertyType.STROKE_COLOR;
    } else if (property == LottieProperty.TRANSFORM_OPACITY) {
      return LayerPropertyType.TRANSFORM_OPACITY;
    } else if (property == LottieProperty.OPACITY) {
      return LayerPropertyType.OPACITY;
    } else if (property == LottieProperty.TRANSFORM_ANCHOR_POINT) {
      return LayerPropertyType.TRANSFORM_ANCHOR;
    } else if (property == LottieProperty.TRANSFORM_POSITION) {
      return LayerPropertyType.TRANSFORM_POSITION;
    } else if (property == LottieProperty.ELLIPSE_SIZE) {
      return LayerPropertyType.ELLIPSE_SIZE;
    } else if (property == LottieProperty.RECTANGLE_SIZE) {
      return LayerPropertyType.RECTANGLE_SIZE;
    } else if (property == LottieProperty.CORNER_RADIUS) {
      return LayerPropertyType.CORNER_RADIUS;
    } else if (property == LottieProperty.POSITION) {
      return LayerPropertyType.POSITION;
    } else if (property == LottieProperty.TRANSFORM_SCALE) {
      return LayerPropertyType.TRANSFORM_SCALE;
    } else if (property == LottieProperty.TRANSFORM_ROTATION) {
      return LayerPropertyType.TRANSFORM_ROTATION;
    } else if (property == LottieProperty.TRANSFORM_SKEW) {
      return LayerPropertyType.TRANSFORM_SKEW;
    } else if (property == LottieProperty.TRANSFORM_SKEW_ANGLE) {
      return LayerPropertyType.TRANSFORM_SKEW_ANGLE;
    } else if (property == LottieProperty.STROKE_WIDTH) {
      return LayerPropertyType.STROKE_WIDTH;
    } else if (property == LottieProperty.TEXT_TRACKING) {
      return LayerPropertyType.TEXT_TRACKING;
    } else if (property == LottieProperty.REPEATER_COPIES) {
      return LayerPropertyType.REPEATER_COPIES;
    } else if (property == LottieProperty.POLYSTAR_POINTS) {
      return LayerPropertyType.POLYSTAR_POINTS;
    } else if (property == LottieProperty.POLYSTAR_ROTATION) {
      return LayerPropertyType.POLYSTAR_ROTATION;
    } else if (property == LottieProperty.POLYSTAR_INNER_RADIUS) {
      return LayerPropertyType.POLYSTAR_INNER_RADIUS;
    } else if (property == LottieProperty.POLYSTAR_OUTER_RADIUS) {
      return LayerPropertyType.POLYSTAR_OUTER_RADIUS;
    } else if (property == LottieProperty.POLYSTAR_INNER_ROUNDEDNESS) {
      return LayerPropertyType.POLYSTAR_INNER_ROUNDED;
    } else if (property == LottieProperty.POLYSTAR_OUTER_ROUNDEDNESS) {
      return LayerPropertyType.POLYSTAR_OUTER_ROUNDED;
    } else if (property == LottieProperty.TRANSFORM_START_OPACITY) {
      return LayerPropertyType.TRANSFORM_START_OPACITY;
    } else if (property == LottieProperty.TRANSFORM_END_OPACITY) {
      return LayerPropertyType.TRANSFORM_END_OPACITY;
    } else if (property == LottieProperty.TEXT_SIZE) {
      return LayerPropertyType.TEXT_SIZE;
    } else if (property == LottieProperty.COLOR_FILTER) {
      return LayerPropertyType.COLOR_FILTER;
    }
    return null;
  }

  private LottieFrameInfo toLottieFrameInfo(AnimaXFrameInfo frameInfo) {
    return new LottieFrameInfo<>().set(frameInfo.getStartFrame(), frameInfo.getEndFrame(),
        frameInfo.getStartValue(), frameInfo.getEndValue(), frameInfo.getLinearProgress(),
        frameInfo.getInterpolatedProgress(), frameInfo.getOverallProgress());
  }

  private AnimaXValueParam toAnimaXValueParam(Object value) {
    if (value instanceof Integer) {
      return AnimaXValueParam.fromNumber(((Integer) value).doubleValue());
    } else if (value instanceof PointF) {
      return AnimaXValueParam.fromCoordinate(((PointF) value).x, ((PointF) value).y);
    } else if (value instanceof Float) {
      return AnimaXValueParam.fromNumber(((Float) value).doubleValue());
    } else if (value instanceof String) {
      return AnimaXValueParam.fromString((String) value);
    } else if (value instanceof ScaleXY) {
      return AnimaXValueParam.fromCoordinate(
          ((ScaleXY) value).getScaleX(), ((ScaleXY) value).getScaleY());
    } else if (value instanceof PorterDuffColorFilter) {
      return getColorFilterParam((PorterDuffColorFilter) value);
    }
    return null;
  }

  private AnimaXValueParam getColorFilterParam(PorterDuffColorFilter colorFilter) {
    try {
      Method getColorMethod = PorterDuffColorFilter.class.getDeclaredMethod("getColor");
      getColorMethod.setAccessible(true);
      int color = (int) getColorMethod.invoke(colorFilter);

      Method getModeMethod = PorterDuffColorFilter.class.getDeclaredMethod("getMode");
      getModeMethod.setAccessible(true);
      PorterDuff.Mode mode = (PorterDuff.Mode) getModeMethod.invoke(colorFilter);

      return AnimaXValueParam.fromColorFilter(color, mode.ordinal());
    } catch (Exception e) {
      e.printStackTrace();
      return null;
    }
  }

  @Override
  public <T> void addValueCallback(
      KeyPath keyPath, T property, final LottieValueCallback<T> callback) {
    enqueueOperation(new Runnable() {
      @Override
      public void run() {
        if (mAnimaXView == null || keyPath == null || property == null || callback == null) {
          return;
        }

        LayerPropertyType layerPropertyType = toLayerPropertyType(property);
        if (layerPropertyType == null) {
          return;
        }

        AnimaXKeyPath animaXKeyPath = new AnimaXKeyPath(keyPath.getKeysArray());
        mAnimaXView.getPlayer().addLayerPropertyCallback(layerPropertyType, animaXKeyPath,
            new AnimaXValueCallback() {
              @Nullable
              @Override
              public AnimaXValueParam getValue(@NonNull AnimaXFrameInfo frameInfo) {
                Object value = callback.getValue(toLottieFrameInfo(frameInfo));
                return toAnimaXValueParam(value);
              }
            },
            new AnimaXPropertyCallback() {
              @Override
              public void onSuccess() {
                AnimaXLog.i(TAG, "addValueCallback onSuccess");
              }

              @Override
              public void onError(@NonNull List<String> errorMessageList) {
                AnimaXLog.e(TAG, "addValueCallback onError: " + errorMessageList);
              }
            });
      }
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

  @Override
  public void removeAllLottieOnCompositionLoadedListener() {
    noticeNotImplemented("removeAllLottieOnCompositionLoadedListener");
  }

  @Override
  public void setMinFrame(int minFrame) {
    mStartFrame = minFrame;
    enqueueOperation(new Runnable() {
      @Override
      public void run() {
        mStartFrame = adjustStartFrame(minFrame);
        mAnimaXView.setStartFrame(mStartFrame);
      }
    });
  }

  @Override
  public float getMinFrame() {
    return mStartFrame == 0 ? mModelStartFrame : mStartFrame;
  }

  @Override
  public void setMaxFrame(int maxFrame) {
    mEndFrame = maxFrame;
    enqueueOperation(new Runnable() {
      @Override
      public void run() {
        mEndFrame = adjustEndFrame(mEndFrame);
        mAnimaXView.setEndFrame(mEndFrame);
      }
    });
  }

  @Override
  public float getMaxFrame() {
    return mEndFrame == -1 ? mModelEndFrame : mEndFrame;
  }

  @Override
  public void setMinAndMaxFrame(int minFrame, int maxFrame) {
    mStartFrame = minFrame;
    mEndFrame = maxFrame;
    enqueueOperation(new Runnable() {
      @Override
      public void run() {
        mStartFrame = adjustStartFrame(minFrame);
        mAnimaXView.setStartFrame(mStartFrame);
        mEndFrame = adjustEndFrame(maxFrame);
        mAnimaXView.setEndFrame(mEndFrame);
      }
    });
  }

  @Override
  public void setMinProgress(float minProgress) {
    enqueueOperation(new Runnable() {
      @Override
      public void run() {
        Objects.requireNonNull(mAnimaXView).setStartFrame((int) frameForProgress(minProgress));
      }
    });
  }

  @Override
  public void setMaxProgress(@FloatRange(from = 0f, to = 1f) float maxProgress) {
    enqueueOperation(new Runnable() {
      @Override
      public void run() {
        Objects.requireNonNull(mAnimaXView).setEndFrame((int) frameForProgress(maxProgress));
      }
    });
  }

  @Override
  public void setMinAndMaxProgress(@FloatRange(from = 0f, to = 1f) float minProgress,
      @FloatRange(from = 0f, to = 1f) float maxProgress) {
    enqueueOperation(new Runnable() {
      @Override
      public void run() {
        int startFrame = (int) frameForProgress(minProgress);
        int endFrame = (int) frameForProgress(maxProgress);
        if (startFrame == endFrame) {
          startFrame = startFrame - 1;
        }
        mAnimaXView.setStartFrame(adjustStartFrame(startFrame));
        mAnimaXView.setEndFrame(adjustEndFrame(endFrame));
      }
    });
  }

  @Override
  @Nullable
  public Bitmap updateBitmap(String id, @Nullable Bitmap bitmap) {
    noticeNotImplemented("updateBitmap");
    return null;
  }

  @Override
  public void setFrame(int frame) {
    enqueueOperation(new Runnable() {
      @Override
      public void run() {
        Objects.requireNonNull(mAnimaXView).seek(frame);
      }
    });
  }

  @Override
  public int getFrame() {
    return mAnimaXView != null ? (int) mAnimaXView.getCurrentFrame() : 0;
  }

  @Override
  public long getDuration() {
    return mAnimaXView != null ? (long) mAnimaXView.getDurationMs() : 0;
  }

  @Override
  public void setImageResource(int resId) {
    if (resId == 0) {
      AnimaXLog.w(TAG, "Image resource id is 0");
      return;
    }
    runOnUIThread(new Runnable() {
      @Override
      public void run() {
        try {
          Drawable drawable = AppCompatResources.getDrawable(mContext, resId);
          if (drawable != null) {
            internalSetImageDrawable(drawable);
          }
        } catch (Resources.NotFoundException e) {
          AnimaXLog.w(TAG, "Image resource not found: " + resId);
        }
      }
    });
  }

  @Override
  public void setImageDrawable(Drawable drawable) {
    runOnUIThread(new Runnable() {
      @Override
      public void run() {
        internalSetImageDrawable(drawable);
      }
    });
  }

  private void internalSetImageDrawable(Drawable drawable) {
    if (drawable == null) {
      hideImageDrawableView();
      if (mAnimaXView != null) {
        ((View) mAnimaXView).setVisibility(View.VISIBLE);
      }
    } else if (showImageDrawableView()) {
      // mImageDrawableView must not be null
      mImageDrawableView.setImageDrawable(drawable);
      if (mAnimaXView != null) {
        ((View) mAnimaXView).setVisibility(View.GONE);
      }
    }
  }

  @Override
  public void setAutoPlay(boolean autoPlay) {
    mAutoPlay = autoPlay;
    if (mAnimaXView != null) {
      mAnimaXView.setAutoPlay(autoPlay);
    }
  }

  @Override
  public AnimationType getContentType() {
    return AnimationType.ANIMAX;
  }

  @Override
  public void setColorFilter(ColorFilter filter) {
    mColorFilter = filter;
    if (mImageDrawableView != null) {
      mImageDrawableView.setColorFilter(filter);
    }
  }

  @Nullable
  @Override
  public ColorFilter getColorFilter() {
    return mColorFilter;
  }

  @Override
  public void setMaxFrameRate(float maxFrameRate) {
    if (maxFrameRate < 1.f) {
      return;
    }
    mMaxFrameRate = maxFrameRate;
    if (mAnimaXView != null) {
      mAnimaXView.setMaxFrameRate(mMaxFrameRate);
    }
  }

  @Override
  public void setScaleType(ImageView.ScaleType scaleType) {
    mScaleType = scaleType;
    if (mImageDrawableView != null) {
      mImageDrawableView.setScaleType(scaleType);
    }
    setScaleTypeToAnimaXView(mAnimaXView, scaleType);
  }

  private void noticeNotImplemented(String funcName) {
    AnimaXLog.w(TAG, "!!!! " + funcName + " not implemented !!!!");
  }

  private void enqueueOperation(Runnable runnable) {
    if (mIsAnimationReady) {
      runnable.run();
    } else {
      mOperationQueue.add(runnable);
    }
  }
  private void flushOperationQueue() {
    List<Runnable> tasksToRun = new ArrayList<>(mOperationQueue);
    mOperationQueue.clear();
    for (Runnable task : tasksToRun) {
      if (task != null) {
        task.run();
      }
    }
  }

  private void removeExternalListeners(Class<?> clazz) {
    Iterator<Object> iterator = mExternalListeners.iterator();
    while (iterator.hasNext()) {
      Object obj = iterator.next();
      if (clazz.isInstance(obj)) {
        iterator.remove();
      }
    }
  }

  private <T> void executeExternalListeners(Class<T> type, Consumer<T> listenerConsumer) {
    Iterator<Object> it = new ArrayList<>(mExternalListeners).iterator();
    while (it.hasNext()) {
      Object t = it.next();
      if (type.isInstance(t)) {
        listenerConsumer.accept((T) t);
      }
    }
  }

  private void runOnUIThread(Runnable runnable) {
    if (Looper.myLooper() == Looper.getMainLooper()) {
      runnable.run();
    } else {
      mMainThreadHandler.post(runnable);
    }
  }

  private void onSetParentView() {
    ViewGroup parentView = mParentViewRef.get();
    if (parentView == null) {
      AnimaXLog.w(TAG, "no parent view");
      return;
    }

    if (mAnimaXView != null) {
      addViewToParent((View) mAnimaXView, parentView, mImageDrawableView);
    }
    if (mImageDrawableView != null) {
      addViewToParent(mImageDrawableView, parentView, null);
    }
  }

  private void onSetAnimationFromJson(@NonNull String jsonString) {
    if (!ensureAnimaXView()) {
      return;
    }
    onSetAnimationWithNewData();
    mCurrentUrl = null;
    Objects.requireNonNull(mAnimaXView).setJson(jsonString);
  }

  private void onSetAnimationFromUrl(@NonNull String url) {
    if (!ensureAnimaXView()) {
      return;
    }
    if (url.equals(mCurrentUrl)) {
      AnimaXLog.w(TAG, "setAnimationFromUrl url not changed, do nothing");
      Objects.requireNonNull((View) mAnimaXView).setVisibility(View.VISIBLE);
      if (mIsAnimationReady) {
        hideImageDrawableView();
      }
      return;
    }
    onSetAnimationWithNewData();
    mCurrentUrl = url;
    Objects.requireNonNull(mAnimaXView).setSrc(url);
  }

  private static void setRepeatCountToAnimaXView(
      @NonNull IAnimaXPlayerDelegate animaXView, int count) {
    if (count == ValueAnimator.INFINITE || count < 0) {
      animaXView.setLoop(true);
    } else {
      animaXView.setLoop(false);
      // For AnimaX: 0 = loop for ever, 1 = no loop
      animaXView.setLoopCount(count > 0 ? count : 1);
    }
  }

  private static void setRepeatModeToAnimaXView(
      @NonNull IAnimaXPlayerDelegate animaXView, int mode) {
    switch (mode) {
      case LottieDrawable.RESTART:
        animaXView.setAutoReverse(false);
        break;
      case LottieDrawable.REVERSE:
        animaXView.setAutoReverse(true);
        break;
      default:
        // no other value
        break;
    }
  }

  private static void setCompositionToAnimaXView(
      @NonNull IAnimaXPlayerDelegate animaXView, LottieComposition composition) {
    if (composition != null) {
      Object object = composition.getComposition();
      if (object instanceof AnimaXComposition) {
        animaXView.setComposition((AnimaXComposition) object);
      }
    }
  }

  private static void setScaleTypeToAnimaXView(
      IAnimaXPlayerDelegate animaXView, ImageView.ScaleType scaleType) {
    if (animaXView == null || scaleType == null) {
      return;
    }
    ObjectFit objectFit;
    switch (scaleType) {
      case CENTER:
        objectFit = ObjectFit.CENTER;
        break;
      case CENTER_CROP:
        objectFit = ObjectFit.COVER;
        break;
      case CENTER_INSIDE:
        objectFit = ObjectFit.SCALE_DOWN;
        break;
      case FIT_CENTER:
      case FIT_START:
      case FIT_END:
        objectFit = ObjectFit.CONTAIN;
        break;
      case FIT_XY:
        objectFit = ObjectFit.FILL;
        break;
      case MATRIX:
      default:
        objectFit = ObjectFit.CONTAIN;
        break;
    }
    animaXView.setObjectFit(objectFit);
  }

  private double progressForFrame(double frame) {
    if (mModelEndFrame - mModelStartFrame < EPSILON) {
      return frame <= mModelStartFrame ? 0 : 1;
    }
    final double result = (frame - mModelStartFrame) / (mModelEndFrame - mModelStartFrame);
    return MathUtils.clamp(result, 0f, 1f);
  }

  private double frameForProgress(double progress) {
    final long resultFrame =
        Math.round(mModelStartFrame + (mModelEndFrame - mModelStartFrame) * progress);
    return MathUtils.clamp(resultFrame, mModelStartFrame, mModelEndFrame);
  }

  private int adjustEndFrame(int endFrame) {
    if (endFrame < 0) {
      return (int) mModelEndFrame;
    }
    return (int) MathUtils.clamp(endFrame, mModelStartFrame, mModelEndFrame);
  }

  private int adjustStartFrame(int startFrame) {
    return (int) MathUtils.clamp(startFrame, mModelStartFrame, mModelEndFrame);
  }

  private void onSetAnimationWithNewData() {
    mIsAnimationReady = false;
    mModelStartFrame = mModelEndFrame = 0.0f;
    // AnimaXView must be visible for the content to load.
    Objects.requireNonNull((View) mAnimaXView).setVisibility(View.VISIBLE);
    // hideImageDrawableView will be called on ready
  }

  private boolean ensureAnimaXView() {
    if (mAnimaXView != null) {
      return true;
    }

    ViewGroup parentView = mParentViewRef.get();
    if (parentView == null) {
      AnimaXLog.w(TAG, "no parent view, call setParentView first");
      return false;
    }

    AnimaXContext animaXContext = new AnimaXContext.Builder(new NativeAbility(), mContext)
                                      .multiThreadAccelerate(mMultiThreadAccelerate)
                                      .build();
    IAnimaXPlayerDelegate animaXView;
    if (mUseImageMode) {
      animaXView = new AnimaXImageView(animaXContext);
    } else {
      animaXView = new AnimaXView(animaXContext);
    }
    animaXView.setAutoPlay(mAutoPlay);
    animaXView.addAnimationListener(new AnimationListenerImpl(this));
    if (mImageAssetsFolder != null && !mImageAssetsFolder.isEmpty()) {
      animaXView.setImageFolder(UriUtil.fromLocalAsset(mImageAssetsFolder));
    }
    animaXView.setSpeed(mSpeed);
    animaXView.setMaxFrameRate(mMaxFrameRate);

    setCompositionToAnimaXView(animaXView, mComposition);
    setRepeatCountToAnimaXView(animaXView, mRepeatCount);
    setRepeatModeToAnimaXView(animaXView, mRepeatMode);
    setScaleTypeToAnimaXView(animaXView, mScaleType);

    addViewToParent((View) animaXView, parentView, mImageDrawableView);

    mAnimaXView = animaXView;
    return true;
  }

  private static void addViewToParent(
      @NonNull View targetView, @NonNull ViewGroup parentView, @Nullable View viewAboveTarget) {
    ViewGroup oldParent = (ViewGroup) targetView.getParent();
    if (oldParent == parentView) {
      return;
    }
    if (oldParent != null) {
      oldParent.removeView(targetView);
    }
    FrameLayout.LayoutParams layoutParams = new FrameLayout.LayoutParams(
        FrameLayout.LayoutParams.MATCH_PARENT, FrameLayout.LayoutParams.MATCH_PARENT);
    final int image_index =
        (viewAboveTarget != null ? parentView.indexOfChild(viewAboveTarget) : -1);
    if (image_index >= 0) {
      // mAnimaXView is below mImageDrawableView
      parentView.addView(targetView, image_index, layoutParams);
    } else {
      parentView.addView(targetView, layoutParams);
    }
  }

  private boolean ensureImageDrawableView() {
    if (mImageDrawableView != null) {
      return true;
    }

    ViewGroup parentView = mParentViewRef.get();
    if (parentView == null) {
      return false;
    }

    AppCompatImageView imageDrawableView = new AppCompatImageView(mContext);
    if (mColorFilter != null) {
      imageDrawableView.setColorFilter(mColorFilter);
    }

    addViewToParent(imageDrawableView, parentView, null);

    mImageDrawableView = imageDrawableView;
    return true;
  }

  private void onAnimationReady(float modelStartFrame, float modelEndFrame) {
    mIsAnimationReady = true;
    mModelStartFrame = modelStartFrame;
    mModelEndFrame = modelEndFrame;
    flushOperationQueue();
    hideImageDrawableView();
  }

  private boolean showImageDrawableView() {
    if (!ensureImageDrawableView()) {
      return false;
    }
    // mImageDrawableView must not be null here
    mImageDrawableView.setVisibility(View.VISIBLE);
    return true;
  }

  private void hideImageDrawableView() {
    if (mImageDrawableView != null) {
      mImageDrawableView.setVisibility(View.GONE);
    }
  }

  static class AnimationListenerImpl extends AnimationListenerAdapter {
    private WeakReference<LottieAdapterAnimaX> mAdapterWeakRef;

    public AnimationListenerImpl(@NonNull LottieAdapterAnimaX adapter) {
      mAdapterWeakRef = new WeakReference<>(adapter);
    }

    @Override
    public void onStart(AnimaXParam param) {
      executeExternalListeners(Animator.AnimatorListener.class,
          (Animator.AnimatorListener listener) -> listener.onAnimationStart(null));
    }

    @Override
    public void onUpdate(AnimaXParam param) {
      executeExternalListeners(ValueAnimator.AnimatorUpdateListener.class,
          (ValueAnimator.AnimatorUpdateListener listener) -> listener.onAnimationUpdate(null));
    }

    @Override
    public void onComplete(AnimaXParam param) {
      executeExternalListeners(Animator.AnimatorListener.class,
          (Animator.AnimatorListener listener) -> listener.onAnimationEnd(null));
    }

    @Override
    public void onRepeat(AnimaXParam param) {
      executeExternalListeners(Animator.AnimatorListener.class,
          (Animator.AnimatorListener listener) -> listener.onAnimationRepeat(null));
    }

    @Override
    public void onCancel(AnimaXParam param) {
      executeExternalListeners(Animator.AnimatorListener.class,
          (Animator.AnimatorListener listener) -> listener.onAnimationCancel(null));
    }

    @Override
    public void onReady(AnimaXParam param) {
      LottieAdapterAnimaX adapter = mAdapterWeakRef.get();
      if (adapter != null) {
        adapter.onAnimationReady(
            param.getCurrentFrame(), param.getCurrentFrame() + param.getTotalFrame());
      }
    }

    private <T> void executeExternalListeners(Class<T> type, Consumer<T> listenerConsumer) {
      LottieAdapterAnimaX adapter = mAdapterWeakRef.get();
      if (adapter != null) {
        adapter.executeExternalListeners(type, listenerConsumer);
      }
    }
  }
}
