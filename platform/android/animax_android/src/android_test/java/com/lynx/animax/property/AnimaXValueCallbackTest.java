// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.property;

import androidx.annotation.NonNull;
import junit.framework.TestCase;
import org.junit.Test;

public class AnimaXValueCallbackTest extends TestCase {
  @Test
  public void testAnimaXFrameInfoGetters() {
    float startFrame = 10.0f;
    float endFrame = 90.0f;
    AnimaXValueParam startValue = AnimaXValueParam.fromNumber(100);
    AnimaXValueParam endValue = AnimaXValueParam.fromNumber(200);
    float linearProgress = 0.4f;
    float interpolatedProgress = 0.5f;
    float overallProgress = 0.6f;

    AnimaXFrameInfo frameInfo = new AnimaXFrameInfo(startFrame, endFrame, startValue, endValue,
        linearProgress, interpolatedProgress, overallProgress);

    assertEquals(startFrame, frameInfo.getStartFrame());
    assertEquals(endFrame, frameInfo.getEndFrame());
    assertEquals(startValue, frameInfo.getStartValue());
    assertEquals(endValue, frameInfo.getEndValue());
    assertEquals(linearProgress, frameInfo.getLinearProgress());
    assertEquals(interpolatedProgress, frameInfo.getInterpolatedProgress());
    assertEquals(overallProgress, frameInfo.getOverallProgress());
  }

  private static class TestValueCallback implements AnimaXValueCallback {
    public AnimaXFrameInfo receivedFrameInfo;
    public AnimaXValueParam valueToReturn;

    @Override
    public AnimaXValueParam getValue(@NonNull AnimaXFrameInfo frameInfo) {
      receivedFrameInfo = frameInfo;
      return valueToReturn;
    }
  }

  @Test
  public void testGetValueInternal() {
    TestValueCallback callback = new TestValueCallback();
    AnimaXValueParam expectedReturnValue = AnimaXValueParam.fromString("test_value");
    callback.valueToReturn = expectedReturnValue;

    float startFrame = 1.0f;
    float endFrame = 2.0f;
    AnimaXValueParam startValue = AnimaXValueParam.fromNumber(1);
    AnimaXValueParam endValue = AnimaXValueParam.fromNumber(2);
    float linearProgress = 0.1f;
    float interpolatedProgress = 0.2f;
    float overallProgress = 0.3f;

    Object actualReturnValue = callback.getValueInternal(startFrame, endFrame, startValue, endValue,
        linearProgress, interpolatedProgress, overallProgress);

    assertEquals(expectedReturnValue, actualReturnValue);

    AnimaXFrameInfo received = callback.receivedFrameInfo;
    assertNotNull(received);
    assertEquals(startFrame, received.getStartFrame());
    assertEquals(endFrame, received.getEndFrame());
    assertEquals(startValue, received.getStartValue());
    assertEquals(endValue, received.getEndValue());
    assertEquals(linearProgress, received.getLinearProgress());
    assertEquals(interpolatedProgress, received.getInterpolatedProgress());
    assertEquals(overallProgress, received.getOverallProgress());
  }

  @Test
  public void testGetValueInternalWithNullReturn() {
    TestValueCallback callback = new TestValueCallback();
    callback.valueToReturn = null;

    Object actualReturnValue = callback.getValueInternal(0, 0, null, null, 0, 0, 0);

    assertNull(actualReturnValue);
  }
}
