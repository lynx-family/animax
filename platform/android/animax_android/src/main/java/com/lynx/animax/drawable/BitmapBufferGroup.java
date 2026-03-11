// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.animax.drawable;

import android.view.View;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import com.lynx.animax.base.CalledByNative;
import java.lang.ref.WeakReference;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.atomic.AtomicReference;

/**
 * Manages a pair of buffers for smooth, tear-free animation rendering.
 * Implements a producer-consumer buffer strategy for animation rendering.
 * The GPU thread (producer) writes frames into a fixed-size buffer queue.
 * The UI thread (consumer) displays the latest buffer.
 * With triple buffering (queue size = 3), rendering and display can proceed independently,
 * avoiding blocking between producer and consumer for smoother animations.
 *
 * <p><b>Native Interaction:</b>
 * The buffer swapping logic is driven by calls from the native C++ rendering engine.
 * The native side writes pixel data into the current back buffer. When a frame is complete, it
 * calls
 * {@link #onBufferUpdate()}, which flips the buffers and posts an invalidation request to the
 * attached {@link View}, triggering a redraw with the newly rendered content.
 */
public class BitmapBufferGroup {
  private final AtomicReference<BitmapBuffer> mFrontBitmapBufferRef = new AtomicReference<>();
  private final ConcurrentLinkedQueue<BitmapBuffer> mBackBitmapBufferQueue;
  private final int mMaxCapacity;

  // Target view to be invalidated when buffer updates
  @NonNull private final WeakReference<View> mAttachedView;

  private final boolean mEnableSoftwareRendering;

  public BitmapBufferGroup(@NonNull View attachedView, boolean enableSoftwareRendering) {
    this(attachedView, enableSoftwareRendering, 3);
  }

  public BitmapBufferGroup(
      @NonNull View attachedView, boolean enableSoftwareRendering, int bufferQueueSize) {
    mAttachedView = new WeakReference<>(attachedView);
    mEnableSoftwareRendering = enableSoftwareRendering;
    mBackBitmapBufferQueue = new ConcurrentLinkedQueue<>();
    mMaxCapacity = bufferQueueSize;
  }

  /**
   * Invoked on the GPU/producer thread after a frame has been rendered by native code.
   * It promotes the next Ready buffer from the back queue to the front reference, demotes
   * the previous front to NotReady and enqueues it back for reuse, and finally posts an
   * invalidate on the attached View so the UI thread schedules onDraw.
   * <p>
   * This method may interleave with swapBitmapBuffer and release on the UI thread.
   * The swap path is safe because a triple-buffer arrangement and
   * lock-free, thread-safe primitives (AtomicReference and ConcurrentLinkedQueue) decouple
   * producer and consumer without blocking. A concurrent release may temporarily prevent
   * the queue from being fully drained at that moment, but it will not cause use-after-free
   * or crashes.
   * <p>
   * The implementation follows an optimistic, lock-free execute-then-correct pattern. It
   * first peeks and promotes a candidate front buffer, then re-validates its state. If a
   * race invalidates the candidate (for example it is no longer Ready), the code rolls back
   * by removing stale entries, releasing resources as needed, and restoring a consistent
   * state before returning.
   * <p>
   * If a Ready buffer exists, it becomes the new front and the UI is asked to redraw. If no
   * Ready buffer is available, the method exits without side effects.
   */
  @CalledByNative
  public void onBufferUpdate() {
    BitmapBuffer newFront = mBackBitmapBufferQueue.peek();
    if (newFront == null || newFront.getState() != BitmapBuffer.BufferState.Ready) {
      // It seems that after the GPU thread peeked and rendered the current queue head,
      // something happened on the UI thread, making this Update invalid.
      return;
    }
    // else: The internal state of BitmapBuffer matches our expectation! Proceed with triple-buffer
    // Update. Copy a reference to FrontBitmapBuffer. Separate the pollFromBack and setToFront
    // operations, postponing side-effect-free parts.
    BitmapBuffer oldFront = mFrontBitmapBufferRef.getAndSet(newFront);
    if (oldFront != null) {
      // Return the used BitmapBuffer back to the queue.
      oldFront.setState(BitmapBuffer.BufferState.NotReady);
      mBackBitmapBufferQueue.offer(oldFront);
    }
    // Done! From the last check to now, does the queue still meet my expectations?
    if (newFront.getState() != BitmapBuffer.BufferState.Ready) {
      // Oops, looks like a release (currently the only operation that can affect expectations)
      // happened at the same time, so now we need to roll back the previous side effects.
      if (oldFront != null) {
        // Side effect 1: I recycled the used BitmapBuffer, but this recycle might have happened
        // after the queue was released!
        mBackBitmapBufferQueue.remove(oldFront);
        oldFront.release();
      }
      // Side effect 2: I updated FrontBitmapBuffer, which might have happened after it was
      // released!
      oldFront = mFrontBitmapBufferRef.getAndSet(null);
      if (oldFront != null) {
        oldFront.release();
      }
      // The rollback of side effects is thread-safe because we use thread-safe data structures
      // and there is no combined atomic operation.
      // No need to worry about falling behind the main thread by one release + one update,
      // because after release, updates rely on the GPU thread. Even if there is a leftover
      // postInvalidate, it cannot get a valid BitmapBuffer.
      return;
    }
    // Before this point, release has not started, so all previous changes are visible to the UI
    // thread. Remove the duplicate from the queue head; it should only exist in FrontBitmapBuffer,
    // which is thread-safe.
    mBackBitmapBufferQueue.remove(newFront);
    // Regardless of any issues, postInvalidate has no side effects.
    View view = mAttachedView.get();
    if (view != null) {
      view.postInvalidate();
    }
  }

  /**
   * Called during OnProgress.
   */
  @Nullable
  @CalledByNative
  public BitmapBuffer getBitmapBufferForOffscreenRendering() {
    return mBackBitmapBufferQueue.peek();
  }

  /**
   * Exchanges the provided buffer (which has just finished being displayed) with the next buffer to
   * be displayed.
   *
   * <p>
   * The buffer parameter is a buffer that was previously displayed on screen and is now ready to
   * be recycled. This method resets its offscreen rendering state and places it back into the back
   * buffer queue for future reuse. It then retrieves and returns the next buffer to be displayed
   * (the current front buffer), or buffer if no front buffer exists.
   *
   * <p>
   * This ensures that buffers are efficiently rotated between display and rendering roles,
   * maintaining smooth and tear-free animation.
   *
   * @param buffer The buffer that has just finished being displayed and is ready for recycling.
   * @return The buffer that is about to be displayed, or buffer if no front buffer exists.
   */
  public BitmapBuffer swapBitmapBuffer(BitmapBuffer buffer) {
    BitmapBuffer frontBuffer = mFrontBitmapBufferRef.get();
    if (frontBuffer == null || frontBuffer.getState() != BitmapBuffer.BufferState.Ready) {
      return buffer;
    }
    if (buffer != null) {
      buffer.setState(BitmapBuffer.BufferState.NotReady);
      mBackBitmapBufferQueue.offer(buffer);
    }
    return mFrontBitmapBufferRef.getAndSet(null);
  }

  /**
   * Set buffer dimensions
   */
  public void setBufferSize(int width, int height) {
    if (width <= 0 || height <= 0) {
      return;
    }

    release();
    initBuffers(width, height);
  }

  private void initBuffers(int width, int height) {
    if (width <= 0 || height <= 0) {
      return;
    }
    for (int i = 0; i < mMaxCapacity; ++i) {
      mBackBitmapBufferQueue.offer(BitmapBuffer.create(width, height, mEnableSoftwareRendering));
    }
  }

  /**
   * Release all buffers
   */
  public void release() {
    BitmapBuffer buffer;
    while ((buffer = mBackBitmapBufferQueue.poll()) != null) {
      buffer.release();
    }
    BitmapBuffer frontBuffer = mFrontBitmapBufferRef.getAndSet(null);
    if (frontBuffer != null) {
      frontBuffer.release();
    }
  }
}
