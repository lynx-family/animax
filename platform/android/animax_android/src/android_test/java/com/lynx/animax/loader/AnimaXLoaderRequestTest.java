// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.loader;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;

import com.lynx.animax.base.bridge.JavaOnlyMap;
import java.util.HashMap;
import java.util.Map;
import org.junit.Test;

public class AnimaXLoaderRequestTest {
  @Test
  public void testConstructor() {
    Map<String, Object> params = new HashMap<>();
    params.put("key", "value");
    AnimaXLoaderRequest request = new AnimaXLoaderRequest("http://example.com", params);

    assertEquals("URL should match the constructor input", "http://example.com", request.getUri());
    assertNotNull("Params should not be null", request.getParams());
    assertEquals(
        "Params should match the constructor input", "value", request.getParams().get("key"));
  }

  @Test
  public void testConstructorWithNullParams() {
    AnimaXLoaderRequest request = new AnimaXLoaderRequest("http://example.com", null);

    assertEquals("URL should match the constructor input", "http://example.com", request.getUri());
    assertNull("Params should be null", request.getParams());
  }

  @Test
  public void testCreateRequestWithParams() {
    JavaOnlyMap params = new JavaOnlyMap();
    params.put("key", "value");
    AnimaXLoaderRequest request =
        AnimaXLoaderRequest.createRequestWithParams("http://example.com", params);

    assertEquals("URL should match the input", "http://example.com", request.getUri());
    assertNotNull("Params should not be null", request.getParams());
    assertEquals("Params should contain correct values", "value", request.getParams().get("key"));
  }

  @Test
  public void testCreateRequest() {
    AnimaXLoaderRequest request = AnimaXLoaderRequest.createRequest("http://example.com");

    assertEquals("URL should match the input", "http://example.com", request.getUri());
    assertNull("Params should be null for this factory method", request.getParams());
  }
}
