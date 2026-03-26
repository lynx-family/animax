// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.loader;

import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNotSame;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

import org.junit.Before;
import org.junit.Test;
import org.mockito.*;

public class AnimaXLoaderTest {
  @Mock private IAnimaXLoader mockLoader;

  private AnimaXLoader animaXLoader;

  @Before
  public void setUp() {
    MockitoAnnotations.initMocks(this);
    animaXLoader = new AnimaXLoader(mockLoader);
  }

  @Test
  public void testCreate() {
    IAnimaXLoader newLoader = mock(IAnimaXLoader.class);
    AnimaXLoader createdLoader = animaXLoader.create(newLoader);
    assertNotNull("The created AnimaXLoader should not be null", createdLoader);
    assertNotSame("The created AnimaXLoader should be a new instance", animaXLoader, createdLoader);
  }

  @Test
  public void testLoad() {
    AnimaXLoaderRequest mockRequest = mock(AnimaXLoaderRequest.class);
    AnimaXLoaderCompletionHandler mockHandler = mock(AnimaXLoaderCompletionHandler.class);

    animaXLoader.load(mockRequest, mockHandler);

    // Verify that the load method is called on the mock IAnimaXLoader with correct parameters.
    verify(mockLoader, times(1)).load(mockRequest, mockHandler);
  }
}
