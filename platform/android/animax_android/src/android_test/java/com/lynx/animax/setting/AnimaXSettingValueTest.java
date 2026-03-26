// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.animax.setting;

import static org.junit.Assert.*;

import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import org.junit.Test;

public class AnimaXSettingValueTest {
  @Test
  public void testCreateFromString() {
    // Arrange & Act
    AnimaXSettingValue value = AnimaXSettingValue.fromString("test");

    // Assert
    assertTrue("Should be string type", value.isString());
    assertEquals("Should return correct string value", "test", value.getStringOrEmpty());
  }

  @Test
  public void testCreateFromNullString() {
    // Arrange & Act
    AnimaXSettingValue value = AnimaXSettingValue.fromString(null);

    // Assert
    assertTrue("Should be string type", value.isString());
    assertEquals("Should return empty string for null", "", value.getStringOrEmpty());
  }

  @Test
  public void testCreateFromCollection() {
    // Arrange
    Collection<String> collection = Arrays.asList("item1", "item2");

    // Act
    AnimaXSettingValue value = AnimaXSettingValue.fromCollection(collection);

    // Assert
    assertTrue("Should be collection type", value.isCollection());
    assertEquals("Should return correct collection", collection, value.getCollectionOrEmpty());
  }

  @Test
  public void testCreateFromNullCollection() {
    // Arrange & Act
    AnimaXSettingValue value = AnimaXSettingValue.fromCollection(null);

    // Assert
    assertTrue("Should be collection type", value.isCollection());
    assertEquals("Should return empty collection for null", Collections.emptyList(),
        value.getCollectionOrEmpty());
  }

  @Test
  public void testCreateFromBoolean() {
    // Arrange & Act
    AnimaXSettingValue value = AnimaXSettingValue.fromBoolean(true);

    // Assert
    assertTrue("Should be boolean type", value.isBoolean());
    assertTrue("Should return correct boolean value", value.getBooleanOrFalse());
  }

  @Test
  public void testCreateFromLong() {
    // Arrange & Act
    AnimaXSettingValue value = AnimaXSettingValue.fromLong(123L);

    // Assert
    assertTrue("Should be long type", value.isLong());
    assertEquals("Should return correct long value", 123L, value.getLongOrZero());
  }

  @Test
  public void testCreateFromDouble() {
    // Arrange & Act
    AnimaXSettingValue value = AnimaXSettingValue.fromDouble(123.45);

    // Assert
    assertTrue("Should be double type", value.isDouble());
    assertEquals("Should return correct double value", 123.45, value.getDoubleOrZero(), 0.001);
  }

  @Test
  public void testEmpty() {
    // Arrange & Act
    AnimaXSettingValue value = AnimaXSettingValue.empty();

    // Assert
    assertTrue("Should be string type", value.isString());
    assertEquals("Should return empty string", "", value.getStringOrEmpty());
  }

  @Test
  public void testTypeChecks() {
    // Arrange
    AnimaXSettingValue stringValue = AnimaXSettingValue.fromString("test");
    AnimaXSettingValue collectionValue = AnimaXSettingValue.fromCollection(Collections.emptyList());
    AnimaXSettingValue booleanValue = AnimaXSettingValue.fromBoolean(true);
    AnimaXSettingValue longValue = AnimaXSettingValue.fromLong(123L);
    AnimaXSettingValue doubleValue = AnimaXSettingValue.fromDouble(123.45);

    // Assert
    assertTrue(stringValue.isString());
    assertFalse(stringValue.isCollection());
    assertFalse(stringValue.isBoolean());
    assertFalse(stringValue.isLong());
    assertFalse(stringValue.isDouble());

    assertTrue(collectionValue.isCollection());
    assertFalse(collectionValue.isString());

    assertTrue(booleanValue.isBoolean());
    assertFalse(booleanValue.isString());

    assertTrue(longValue.isLong());
    assertFalse(longValue.isString());

    assertTrue(doubleValue.isDouble());
    assertFalse(doubleValue.isString());
  }

  @Test
  public void testDefaultValues() {
    // Arrange
    AnimaXSettingValue stringValue = AnimaXSettingValue.fromString("test");

    // Assert - testing get methods on wrong types
    assertEquals("", stringValue.getCollectionOrEmpty().size(), 0);
    assertFalse(stringValue.getBooleanOrFalse());
    assertEquals(0L, stringValue.getLongOrZero());
    assertEquals(0.0, stringValue.getDoubleOrZero(), 0.001);
  }

  @Test
  public void testToString() {
    // Arrange & Act & Assert
    assertEquals("{type=STRING, value=test}", AnimaXSettingValue.fromString("test").toString());

    assertEquals("{type=BOOLEAN, value=true}", AnimaXSettingValue.fromBoolean(true).toString());

    assertEquals("{type=LONG, value=123}", AnimaXSettingValue.fromLong(123L).toString());

    assertEquals("{type=DOUBLE, value=123.45}", AnimaXSettingValue.fromDouble(123.45).toString());

    assertEquals("{type=COLLECTION, value=[item1, item2]}",
        AnimaXSettingValue.fromCollection(Arrays.asList("item1", "item2")).toString());
  }
}
