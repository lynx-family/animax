// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.property;

import com.lynx.animax.base.bridge.JavaOnlyArray;
import java.util.List;
import junit.framework.TestCase;
import org.junit.Test;

public class AnimaXKeyPathTest extends TestCase {
  @Test
  public void testConstructorSingleKey() {
    AnimaXKeyPath keyPath = new AnimaXKeyPath("layer1");

    JavaOnlyArray keys = keyPath.getKeys();
    assertEquals(1, keys.size());
    assertEquals("layer1", keys.getString(0));
  }

  @Test
  public void testConstructorMultipleKeys() {
    AnimaXKeyPath keyPath = new AnimaXKeyPath("layer1", "group1", "shape1");

    JavaOnlyArray keys = keyPath.getKeys();
    assertEquals(3, keys.size());
    assertEquals("layer1", keys.getString(0));
    assertEquals("group1", keys.getString(1));
    assertEquals("shape1", keys.getString(2));
  }

  @Test
  public void testConstructorEmptyKeys() {
    AnimaXKeyPath keyPath = new AnimaXKeyPath();

    JavaOnlyArray keys = keyPath.getKeys();
    assertEquals(0, keys.size());
  }

  @Test
  public void testConstructorWithWildcard() {
    AnimaXKeyPath keyPath = new AnimaXKeyPath("layer1", "*", "shape1");

    JavaOnlyArray keys = keyPath.getKeys();
    assertEquals(3, keys.size());
    assertEquals("layer1", keys.getString(0));
    assertEquals("*", keys.getString(1));
    assertEquals("shape1", keys.getString(2));
  }

  @Test
  public void testConstructorWithGlobstar() {
    AnimaXKeyPath keyPath = new AnimaXKeyPath("layer1", "**", "fill");

    JavaOnlyArray keys = keyPath.getKeys();
    assertEquals(3, keys.size());
    assertEquals("layer1", keys.getString(0));
    assertEquals("**", keys.getString(1));
    assertEquals("fill", keys.getString(2));
  }

  @Test
  public void testComplexHierarchyExample() {
    AnimaXKeyPath keyPath = new AnimaXKeyPath("Character", "Body", "Fill");

    JavaOnlyArray keys = keyPath.getKeys();
    assertEquals(3, keys.size());
    assertEquals("Character", keys.getString(0));
    assertEquals("Body", keys.getString(1));
    assertEquals("Fill", keys.getString(2));
  }

  @Test
  public void testWildcardToTargetAllFills() {
    AnimaXKeyPath keyPath = new AnimaXKeyPath("Character", "**", "Fill");

    JavaOnlyArray keys = keyPath.getKeys();
    assertEquals(3, keys.size());
    assertEquals("Character", keys.getString(0));
    assertEquals("**", keys.getString(1));
    assertEquals("Fill", keys.getString(2));
  }

  @Test
  public void testGlobalWildcard() {
    AnimaXKeyPath keyPath = new AnimaXKeyPath("**", "Fill");

    JavaOnlyArray keys = keyPath.getKeys();
    assertEquals(2, keys.size());
    assertEquals("**", keys.getString(0));
    assertEquals("Fill", keys.getString(1));
  }

  @Test
  public void testSingleGlobalKey() {
    AnimaXKeyPath keyPath = new AnimaXKeyPath("Fill");

    JavaOnlyArray keys = keyPath.getKeys();
    assertEquals(1, keys.size());
    assertEquals("Fill", keys.getString(0));
  }

  @Test
  public void testEmptyStringKey() {
    AnimaXKeyPath keyPath = new AnimaXKeyPath("");

    JavaOnlyArray keys = keyPath.getKeys();
    assertEquals(0, keys.size());
  }

  @Test
  public void testMixedNullAndValidKeys() {
    String nullKey = null;
    AnimaXKeyPath keyPath = new AnimaXKeyPath("layer1", nullKey, "", "shape1");

    JavaOnlyArray keys = keyPath.getKeys();
    assertEquals(2, keys.size());
    assertEquals("layer1", keys.getString(0));
    assertEquals("shape1", keys.getString(1));
  }

  @Test
  public void testNullKey() {
    String nullKey = null;
    AnimaXKeyPath keyPath = new AnimaXKeyPath(nullKey);
    JavaOnlyArray keys = keyPath.getKeys();
    assertEquals(0, keys.size());
  }

  @Test
  public void testLongKeyPath() {
    AnimaXKeyPath keyPath =
        new AnimaXKeyPath("MainScene", "Background", "Sky", "CloudGroup", "Cloud1", "Fill");

    JavaOnlyArray keys = keyPath.getKeys();
    assertEquals(6, keys.size());
    assertEquals("MainScene", keys.getString(0));
    assertEquals("Background", keys.getString(1));
    assertEquals("Sky", keys.getString(2));
    assertEquals("CloudGroup", keys.getString(3));
    assertEquals("Cloud1", keys.getString(4));
    assertEquals("Fill", keys.getString(5));
  }

  @Test
  public void testSpecialCharacters() {
    AnimaXKeyPath keyPath = new AnimaXKeyPath("layer-1", "group_2", "shape.3");

    JavaOnlyArray keys = keyPath.getKeys();
    assertEquals(3, keys.size());
    assertEquals("layer-1", keys.getString(0));
    assertEquals("group_2", keys.getString(1));
    assertEquals("shape.3", keys.getString(2));
  }

  @Test
  public void testSpacesInKeys() {
    AnimaXKeyPath keyPath = new AnimaXKeyPath("Layer 1", "Group 2", "Shape 3");

    JavaOnlyArray keys = keyPath.getKeys();
    assertEquals(3, keys.size());
    assertEquals("Layer 1", keys.getString(0));
    assertEquals("Group 2", keys.getString(1));
    assertEquals("Shape 3", keys.getString(2));
  }

  @Test
  public void testComplexPatternWithMultipleWildcards() {
    AnimaXKeyPath keyPath = new AnimaXKeyPath("MainScene", "*", "**", "Fill");

    JavaOnlyArray keys = keyPath.getKeys();
    assertEquals(4, keys.size());
    assertEquals("MainScene", keys.getString(0));
    assertEquals("*", keys.getString(1));
    assertEquals("**", keys.getString(2));
    assertEquals("Fill", keys.getString(3));
  }

  @Test
  public void testLayerPropertyTargeting() {
    AnimaXKeyPath transformPath = new AnimaXKeyPath("Character", "Transform");
    JavaOnlyArray transformKeys = transformPath.getKeys();
    assertEquals(2, transformKeys.size());
    assertEquals("Character", transformKeys.getString(0));
    assertEquals("Transform", transformKeys.getString(1));

    AnimaXKeyPath fillPath = new AnimaXKeyPath("Background", "**", "Fill");
    JavaOnlyArray fillKeys = fillPath.getKeys();
    assertEquals(3, fillKeys.size());
    assertEquals("Background", fillKeys.getString(0));
    assertEquals("**", fillKeys.getString(1));
    assertEquals("Fill", fillKeys.getString(2));

    AnimaXKeyPath strokePath = new AnimaXKeyPath("**", "Stroke");
    JavaOnlyArray strokeKeys = strokePath.getKeys();
    assertEquals(2, strokeKeys.size());
    assertEquals("**", strokeKeys.getString(0));
    assertEquals("Stroke", strokeKeys.getString(1));
  }

  @Test
  public void testToStringFormat() {
    AnimaXKeyPath keyPath = new AnimaXKeyPath("layer1", "group1");

    String result = keyPath.toString();

    assertTrue(result.startsWith("KeyPath"));
    assertTrue(result.contains("keys="));
    assertTrue(result.contains("{") && result.contains("}"));
  }

  @Test
  public void testToStringNotNull() {
    AnimaXKeyPath emptyPath = new AnimaXKeyPath();
    AnimaXKeyPath singlePath = new AnimaXKeyPath("test");
    AnimaXKeyPath multiPath = new AnimaXKeyPath("test1", "test2", "test3");

    assertNotNull(emptyPath.toString());
    assertNotNull(singlePath.toString());
    assertNotNull(multiPath.toString());

    assertFalse(emptyPath.toString().isEmpty());
    assertFalse(singlePath.toString().isEmpty());
    assertFalse(multiPath.toString().isEmpty());
  }

  @Test
  public void testVeryLongKey() {
    StringBuilder longKey = new StringBuilder();
    for (int i = 0; i < 1000; i++) {
      longKey.append("a");
    }

    AnimaXKeyPath keyPath = new AnimaXKeyPath(longKey.toString());
    JavaOnlyArray keys = keyPath.getKeys();

    assertEquals(1, keys.size());
    assertEquals(longKey.toString(), keys.getString(0));
  }

  @Test
  public void testManyKeys() {
    String[] manyKeys = new String[100];
    for (int i = 0; i < 100; i++) {
      manyKeys[i] = "key" + i;
    }

    AnimaXKeyPath keyPath = new AnimaXKeyPath(manyKeys);
    JavaOnlyArray keys = keyPath.getKeys();

    assertEquals(100, keys.size());
    assertEquals("key0", keys.getString(0));
    assertEquals("key99", keys.getString(99));

    for (int i = 0; i < 100; i++) {
      assertEquals("key" + i, keys.getString(i));
    }
  }

  @Test
  public void testOnlyWildcards() {
    AnimaXKeyPath keyPath = new AnimaXKeyPath("*", "**", "*");

    JavaOnlyArray keys = keyPath.getKeys();
    assertEquals(3, keys.size());
    assertEquals("*", keys.getString(0));
    assertEquals("**", keys.getString(1));
    assertEquals("*", keys.getString(2));
  }

  @Test
  public void testRealisticAnimationTargeting() {
    AnimaXKeyPath opacityPath = new AnimaXKeyPath("**", "Transform", "Opacity");
    JavaOnlyArray opacityKeys = opacityPath.getKeys();
    assertEquals(3, opacityKeys.size());
    assertEquals("Opacity", opacityKeys.getString(2));

    AnimaXKeyPath colorPath = new AnimaXKeyPath("Character", "Body", "Fill", "Color");
    JavaOnlyArray colorKeys = colorPath.getKeys();
    assertEquals(4, colorKeys.size());
    assertEquals("Character", colorKeys.getString(0));
    assertEquals("Color", colorKeys.getString(3));

    AnimaXKeyPath strokePath = new AnimaXKeyPath("**", "Stroke", "Width");
    JavaOnlyArray strokeKeys = strokePath.getKeys();
    assertEquals(3, strokeKeys.size());
    assertEquals("Width", strokeKeys.getString(2));

    AnimaXKeyPath textPath = new AnimaXKeyPath("**", "Text");
    JavaOnlyArray textKeys = textPath.getKeys();
    assertEquals(2, textKeys.size());
    assertEquals("Text", textKeys.getString(1));
  }

  @Test
  public void testGetKeysList() {
    AnimaXKeyPath keyPath = new AnimaXKeyPath("a", "b", "c");
    List<String> keysList = keyPath.getKeysList();
    assertEquals(3, keysList.size());
    assertEquals("a", keysList.get(0));
    assertEquals("b", keysList.get(1));
    assertEquals("c", keysList.get(2));
  }

  @Test
  public void testGetKeysListEmpty() {
    AnimaXKeyPath keyPath = new AnimaXKeyPath();
    List<String> keysList = keyPath.getKeysList();
    assertNotNull(keysList);
    assertTrue(keysList.isEmpty());
  }
}
