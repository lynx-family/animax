// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.animax.service;

import static org.junit.Assert.*;

import org.junit.Test;

public class ServiceScopeTest {
  @Test
  public void testDefaultScope() {
    assertEquals("default", ServiceScope.DEFAULT.toString());
  }

  @Test
  public void testCustomScope() {
    ServiceScope scope = ServiceScope.of("CUSTOM");
    assertEquals("custom", scope.toString());
  }

  @Test
  public void testEqualsWithSameValue() {
    ServiceScope scope1 = ServiceScope.of("test");
    ServiceScope scope2 = ServiceScope.of("test");
    assertEquals(scope1, scope2);
  }

  @Test
  public void testEqualsWithDifferentCase() {
    ServiceScope scope1 = ServiceScope.of("TEST");
    ServiceScope scope2 = ServiceScope.of("test");
    assertEquals(scope1, scope2);
  }

  @Test
  public void testEqualsWithDifferentValue() {
    ServiceScope scope1 = ServiceScope.of("test1");
    ServiceScope scope2 = ServiceScope.of("test2");
    assertNotEquals(scope1, scope2);
  }

  @Test
  public void testEqualsWithNull() {
    ServiceScope scope = ServiceScope.of("test");
    assertNotEquals(scope, null);
  }

  @Test
  public void testEqualsWithDifferentClass() {
    ServiceScope scope = ServiceScope.of("test");
    assertNotEquals(scope, "test");
  }

  @Test
  public void testHashCodeConsistency() {
    ServiceScope scope1 = ServiceScope.of("test");
    ServiceScope scope2 = ServiceScope.of("test");
    assertEquals(scope1.hashCode(), scope2.hashCode());
  }

  @Test
  public void testHashCodeWithDifferentCase() {
    ServiceScope scope1 = ServiceScope.of("TEST");
    ServiceScope scope2 = ServiceScope.of("test");
    assertEquals(scope1.hashCode(), scope2.hashCode());
  }

  @Test
  public void testToString() {
    ServiceScope scope = ServiceScope.of("TEST");
    assertEquals("test", scope.toString());
  }

  @Test(expected = NullPointerException.class)
  public void testOfWithNull() {
    ServiceScope.of(null);
  }

  @Test
  public void testSameInstance() {
    ServiceScope scope = ServiceScope.of("test");
    assertTrue(scope.equals(scope));
  }
}
