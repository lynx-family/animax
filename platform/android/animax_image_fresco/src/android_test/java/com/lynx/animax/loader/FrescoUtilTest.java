// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.animax.loader;

import static org.junit.Assert.assertFalse;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.graphics.Bitmap;
import java.lang.reflect.Method;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

public class FrescoUtilTest {
  @Mock private IAnimaXLoaderRequest mockRequest;

  @Mock private IAnimaXLoaderCompletionHandler mockCompletionHandler;

  @Before
  public void setUp() {
    MockitoAnnotations.initMocks(this);
  }

  @Test
  public void testTryHandleLoaderRequestWithFresco_withNullImageInfo_returnsFalse() {
    when(mockRequest.getImageInfo()).thenReturn(null);

    boolean result =
        FrescoUtil.tryHandleLoaderRequestWithFresco(mockRequest, mockCompletionHandler);

    assertFalse("The method should return false when image info is null", result);
    verify(mockCompletionHandler, never()).onComplete(any());
  }

  /**
   * Helper method to call the private checkBitmapRGBA8888 method using reflection.
   *
   * @param bitmap The bitmap to check
   * @return The result of the checkBitmapRGBA8888 method
   * @throws Exception if reflection fails
   */
  private boolean callCheckBitmapRGBA8888(Bitmap bitmap) throws Exception {
    Method method = FrescoUtil.class.getDeclaredMethod("checkBitmapRGBA8888", Bitmap.class);
    method.setAccessible(true);
    return (boolean) method.invoke(null, bitmap);
  }

  @Test
  public void testCheckBitmapRGBA8888_withNullBitmap_returnsFalse() throws Exception {
    boolean result = callCheckBitmapRGBA8888(null);

    assertFalse("The method should return false for null bitmaps", result);
  }
}
