// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.lottieadapter.lottie;
public class LottieResult<T> {
  private T value;
  private Exception exception;

  public LottieResult(T value) {
    this.value = value;
  }

  public LottieResult(Exception exception) {
    this.exception = exception;
  }

  public T getValue() {
    return value;
  }

  public Exception getException() {
    return exception;
  }

  public boolean hasError() {
    return exception != null;
  }
}
