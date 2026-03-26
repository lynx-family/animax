// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.animax.service;

import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

import org.junit.Before;
import org.junit.Test;

public class AnimaXFrescoImageServiceTest {
  private AnimaXFrescoImageService service;

  @Before
  public void setUp() {
    service = new AnimaXFrescoImageService();
  }

  @Test
  public void testGetServiceClass_returnsCorrectClass() {
    assertNotNull("The service class should not be null", service.getServiceClass());
    assertTrue("The service class should be assignable from IAnimaXImageService",
        service.getServiceClass().isAssignableFrom(IAnimaXImageService.class));
  }
}
