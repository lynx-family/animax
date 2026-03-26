// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.listener;

/**
 * Interface for an AnimaXView animation play lifecycle listener.
 * This interface allows implementation of callback methods
 * to respond to different stages and params during an animation.
 */
public interface IAnimationListener {
  /**
   * Called when the animation starts.
   * @param param An instance of AnimaXParam containing details about the animation start event.
   */
  void onStart(AnimaXParam param);

  /**
   * Called when the animation is ready to be played.
   * @param param An instance of AnimaXParam containing details about the animation readiness.
   */
  void onReady(AnimaXParam param);

  /**
   * Called when the animation completes.
   * @param param An instance of AnimaXParam containing details about the animation completion
   *     event.
   */
  void onComplete(AnimaXParam param);

  /**
   * Called each time the animation repeats.
   * @param param An instance of AnimaXParam containing details about the animation repeat event.
   */
  void onRepeat(AnimaXParam param);

  /**
   * Called when the animation is cancelled.
   * @param param An instance of AnimaXParam containing details about the animation cancellation
   *     event.
   */
  void onCancel(AnimaXParam param);

  /**
   * Called when an error occurs during the animation.
   * @param param An instance of AnimaXErrorParam containing details about the error that occurred.
   */
  void onError(AnimaXErrorParam param);

  /**
   * Called when the animation frame has been updated.
   * @param param An instance of AnimaXParam containing details about the animation frame update.
   */
  void onUpdate(AnimaXParam param);

  /**
   * Called to report the current frames per second (FPS) of the animation.
   * @param param An instance of AnimaXFPSParam containing details about the animation's FPS.
   */
  void onFPS(AnimaXFPSParam param);

  /**
   * Called when click on the layers.
   * @param param An instance of AnimaXFPSParam containing details about the animation's FPS.
   */
  void onTapLayers(AnimaXTapParam param);

  /**
   * Called when the first frame is flushed to the surface.
   * @param param An instance of AnimaXParam containing details about the animation frame update.
   */
  void onFirstFrame(AnimaXParam param);

  /**
   * Called when the composition is ready.
   * @param param An instance of AnimaXParam containing details about the animation frame update.
   */
  void onCompositionReady(AnimaXParam param);

  /**
   * Called when a warning occurs during the animation.
   * @param param An instance of AnimaXErrorParam containing details about the warning that
   *     occurred.
   */
  void onWarning(AnimaXErrorParam param);
}
