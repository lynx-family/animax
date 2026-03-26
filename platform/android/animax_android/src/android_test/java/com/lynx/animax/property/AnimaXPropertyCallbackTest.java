// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.property;

import androidx.annotation.NonNull;
import com.lynx.animax.base.bridge.JavaOnlyArray;
import java.lang.reflect.Method;
import java.util.List;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicReference;
import junit.framework.TestCase;
import org.junit.Test;

public class AnimaXPropertyCallbackTest extends TestCase {
  private static class TestPropertyCallback implements AnimaXPropertyCallback {
    private final AtomicBoolean mSuccessCalled = new AtomicBoolean(false);
    private final AtomicReference<List<String>> mErrorMessages = new AtomicReference<>();

    @Override
    public void onSuccess() {
      mSuccessCalled.set(true);
    }

    @Override
    public void onError(@NonNull List<String> errorMessageList) {
      mErrorMessages.set(errorMessageList);
    }

    public boolean wasSuccessCalled() {
      return mSuccessCalled.get();
    }

    public List<String> getErrorMessages() {
      return mErrorMessages.get();
    }
  }

  private JavaOnlyArray createJavaOnlyArray() {
    try {
      Method method = JavaOnlyArray.class.getDeclaredMethod("create");
      method.setAccessible(true);
      return (JavaOnlyArray) method.invoke(null);
    } catch (Exception e) {
      throw new RuntimeException(e);
    }
  }

  @Test
  public void testOnPropertyCallback_Success() {
    TestPropertyCallback callback = new TestPropertyCallback();
    JavaOnlyArray emptyArray = createJavaOnlyArray();

    callback.onPropertyCallback(emptyArray);

    assertTrue(
        "onSuccess should have been called for an empty error array", callback.wasSuccessCalled());
    assertNull("onError should not have been called", callback.getErrorMessages());
  }

  @Test
  public void testOnPropertyCallback_Error() {
    TestPropertyCallback callback = new TestPropertyCallback();
    JavaOnlyArray errorArray = createJavaOnlyArray();
    String errorMessage1 = "Error message 1";
    String errorMessage2 = "Error message 2";
    errorArray.pushString(errorMessage1);
    errorArray.pushString(errorMessage2);

    callback.onPropertyCallback(errorArray);

    assertFalse("onSuccess should not have been called", callback.wasSuccessCalled());
    List<String> receivedErrors = callback.getErrorMessages();
    assertNotNull("onError should have been called", receivedErrors);
    assertEquals("Should have received 2 error messages", 2, receivedErrors.size());
    assertEquals(errorMessage1, receivedErrors.get(0));
    assertEquals(errorMessage2, receivedErrors.get(1));
  }
}
