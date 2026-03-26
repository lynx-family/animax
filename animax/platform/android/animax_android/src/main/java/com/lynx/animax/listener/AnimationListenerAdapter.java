// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.listener;

/**
 * An empty implementation of the IAnimationListener interface.
 * This class does nothing in each callback.
 */
public class AnimationListenerAdapter implements IAnimationListener {
  @Override
  public void onStart(AnimaXParam param) {}

  @Override
  public void onReady(AnimaXParam param) {}

  @Override
  public void onComplete(AnimaXParam param) {}

  @Override
  public void onRepeat(AnimaXParam param) {}

  @Override
  public void onCancel(AnimaXParam param) {}

  @Override
  public void onError(AnimaXErrorParam param) {}

  @Override
  public void onUpdate(AnimaXParam param) {}

  @Override
  public void onFPS(AnimaXFPSParam param) {}

  @Override
  public void onTapLayers(AnimaXTapParam param) {}

  @Override
  public void onFirstFrame(AnimaXParam param) {}

  @Override
  public void onCompositionReady(AnimaXParam param) {}

  @Override
  public void onWarning(AnimaXErrorParam param) {}
}
