// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.player;

import android.graphics.SurfaceTexture;
import android.os.Handler;
import android.os.Looper;
import androidx.annotation.NonNull;
import com.lynx.animax.util.AnimaXLog;
import com.lynx.animax.util.UIThreadUtils;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

public class VideoPlayerImpl extends AbsVideoPlayer
    implements SurfaceTexture.OnFrameAvailableListener, CodecManager.CodecErrorReporter {
  private static final String TAG = "VideoPlayerImpl";
  private static final long FRAME_AVAILABLE_TIMEOUT_MS = 20;
  private static final long DESTROY_TIMEOUT_MS = 1000;

  private boolean mSurfaceTextureAvailable;
  private Lock mSurfaceTextureLock = new ReentrantLock();
  private Condition mSurfaceTextureCondition = mSurfaceTextureLock.newCondition();

  // A wrapper class of MediaCodec which manages the allocation/initialization/release/decoding
  // operations of a decoder.
  private CodecManager mCodecManager;

  // Calling thread is the same as GL Thread. A different naming is for better extensibility.
  private Handler mCallingThreadHandler;

  private volatile boolean mIsDestroyed = false;

  public VideoPlayerImpl(long nativePlayer, @NonNull VideoPlayerConfig config) {
    super(nativePlayer, config);
    initCodecThread();
    postToCodecThread(new Runnable() {
      @Override
      public void run() {
        mCodecManager = new CodecManager(VideoPlayerImpl.this);
        mCodecManager.resetDecoderState();
      }
    });
    if (mConfig.getVideoFrameTimeout() <= 0) {
      mConfig.setVideoFrameTimeout(FRAME_AVAILABLE_TIMEOUT_MS);
    }
    if (mConfig.getSurfaceDestroyTimeout() <= 0) {
      mConfig.setSurfaceDestroyTimeout(DESTROY_TIMEOUT_MS);
    }
  }

  @Override
  public void attachAsset(VideoAsset asset) {
    super.attachAsset(asset);
    postToCodecThreadWhenCodecReady(new Runnable() {
      @Override
      public void run() {
        mCodecManager.attachAsset(asset);
      }
    });
  }

  @Override
  public void setSurface(int texture) {
    super.setSurface(texture);
    mSurfaceTextureAvailable = false;
    mSurfaceTexture.setOnFrameAvailableListener(this, UIThreadUtils.getUiThreadHandler());

    if (null == mAsset) {
      AnimaXLog.e(TAG, "setSurface error: mAsset is null");
      return;
    }

    postToCodecThreadWhenCodecReady(new Runnable() {
      @Override
      public void run() {
        mCodecManager.attachSurface(mSurface);
        mCodecManager.initDecoder();
      }
    });
  }

  // main thread
  @Override
  public void onFrameAvailable(SurfaceTexture surfaceTexture) {
    mSurfaceTextureLock.lock();
    mSurfaceTextureAvailable = true;
    try {
      mSurfaceTextureCondition.signalAll();
    } finally {
      mSurfaceTextureLock.unlock();
    }
  }

  @Override
  public boolean updateSurface(int toFrame) {
    if (mIsDestroyed) {
      return false;
    }

    postToCodecThreadWhenCodecReady(new Runnable() {
      @Override
      public void run() {
        mCodecManager.decodeAndUploadFrame(toFrame);
      }
    });
    return updateTexImageIfNecessary();
  }

  // called from external
  // GPU Thread
  @Override
  public void destroy() {
    if (mIsDestroyed) {
      return;
    }
    mIsDestroyed = true;

    final CountDownLatch latch = new CountDownLatch(1);
    CodecThreadManager.getInstance().postAtFrontOfQueue(new Runnable() {
      @Override
      public void run() {
        if (mCodecManager != null) {
          mCodecManager.setReporter(null);
          mCodecManager.releaseDecoder();
        }
        latch.countDown();
      }
    });

    try {
      // Wait for the codec thread tasks to complete
      long timeout = mConfig.getSurfaceDestroyTimeout();
      AnimaXLog.i(
          TAG, "destroy: wait for codec thread to complete destroy tasks with " + timeout + "ms");
      if (!latch.await(timeout, TimeUnit.MILLISECONDS)) {
        AnimaXLog.w(TAG, "Timeout waiting for codec thread to complete destroy tasks");
      }
    } catch (InterruptedException e) {
      AnimaXLog.w(TAG, "Interrupted while waiting for codec thread to complete destroy tasks");
      Thread.currentThread().interrupt();
    }

    super.destroy();
  }

  @Override
  public void reportError(@NonNull String errMsg) {
    runNowOrPostToGLThread(new Runnable() {
      @Override
      public void run() {
        reportErrorOnce(errMsg);
      }
    });
  }

  @Override
  public void markDrawnOnce() {
    runNowOrPostToGLThread(new Runnable() {
      @Override
      public void run() {
        hasDrewOnce();
      }
    });
  }

  // GPU Thread
  private boolean updateTexImageIfNecessary() {
    ensureCurrentThreadIsGLThread();
    boolean surfaceTextureAvailable = false;
    mSurfaceTextureLock.lock();
    if (!mSurfaceTextureAvailable) {
      try {
        mSurfaceTextureCondition.await(mConfig.getVideoFrameTimeout(), TimeUnit.MILLISECONDS);
      } catch (InterruptedException e) {
        AnimaXLog.e(TAG, "await onFrameAvailable error: " + e);
      } finally {
        surfaceTextureAvailable = mSurfaceTextureAvailable;
        mSurfaceTextureAvailable = false;
        mSurfaceTextureLock.unlock();
      }
    } else {
      surfaceTextureAvailable = true;
      mSurfaceTextureAvailable = false;
      mSurfaceTextureLock.unlock();
    }

    if (!surfaceTextureAvailable) {
      AnimaXLog.e(TAG, "await onFrameAvailable time out");
      return false;
    }
    mSurfaceTexture.updateTexImage();
    return true;
  }

  // Called in the GPU thread
  private void initCodecThread() {
    Looper callingThreadLooper = Looper.myLooper();
    if (callingThreadLooper != null) {
      mCallingThreadHandler = new Handler(callingThreadLooper);
    }
  }

  // Utility method to safely post to the codec thread handler
  private void postToCodecThread(Runnable r) {
    if (mIsDestroyed) {
      return;
    }
    CodecThreadManager.getInstance().runNowOrPostToCodecThread(r);
  }

  private void postToCodecThreadWhenCodecReady(Runnable r) {
    postToCodecThread(new Runnable() {
      @Override
      public void run() {
        if (mCodecManager != null) {
          r.run();
        }
      }
    });
  }

  // Utility method to safely post to the gl thread handler
  private void runNowOrPostToGLThread(Runnable r) {
    if (mCallingThreadHandler != null) {
      if (Looper.myLooper() == mCallingThreadHandler.getLooper()) {
        r.run();
      } else {
        mCallingThreadHandler.post(r);
      }
    } else {
      AnimaXLog.e(TAG, "Attempted to post to GL thread after it was released or not alive.");
    }
  }

  private void ensureCurrentThreadIsGLThread() {
    if (mCallingThreadHandler == null || Looper.myLooper() != mCallingThreadHandler.getLooper()) {
      throw new IllegalStateException(
          "Must be called on GL thread, current: " + Thread.currentThread().getName());
    }
  }

  private void ensureCurrentThreadIsCodecThread() {
    CodecThreadManager.getInstance().ensureOnCodecThread();
  }
}
