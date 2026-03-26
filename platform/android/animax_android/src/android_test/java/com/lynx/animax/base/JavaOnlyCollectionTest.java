// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.base;

import com.lynx.animax.base.bridge.JavaOnlyArray;
import com.lynx.animax.base.bridge.JavaOnlyMap;
import com.lynx.animax.base.bridge.ReadableType;
import java.util.ArrayList;
import java.util.HashMap;
import junit.framework.TestCase;
import org.junit.Before;
import org.junit.Test;

public class JavaOnlyCollectionTest extends TestCase {
  private JavaOnlyArray javaOnlyArray;
  private JavaOnlyMap javaOnlyMap;

  @Before
  public void setUp() {
    javaOnlyArray = new JavaOnlyArray();
    javaOnlyMap = new JavaOnlyMap();
  }

  @Test
  public void testJavaOnlyArrayPushString() {
    javaOnlyArray.pushString("test");
    assertEquals("test", javaOnlyArray.get(0));
  }

  @Test
  public void testJavaOnlyArrayPushByteArrayAsString() {
    byte[] byteArray = "hello".getBytes();
    javaOnlyArray.pushByteArrayAsString(byteArray);
    assertEquals("hello", javaOnlyArray.get(0));
  }

  @Test
  public void testJavaOnlyArrayAsArrayList() {
    javaOnlyArray.pushString("test1");
    javaOnlyArray.pushString("test2");
    ArrayList<Object> arrayList = javaOnlyArray.asArrayList();
    assertEquals(2, arrayList.size());
    assertEquals("test1", arrayList.get(0));
    assertEquals("test2", arrayList.get(1));
  }

  @Test
  public void testJavaOnlyMapPutAndGetMethods() {
    javaOnlyMap.putString("stringKey", "stringValue");
    javaOnlyMap.putInt("intKey", 42);
    javaOnlyMap.putDouble("doubleKey", 3.14);

    assertEquals("stringValue", javaOnlyMap.getString("stringKey"));
    assertEquals(42, javaOnlyMap.getInt("intKey", 0));
    assertEquals(3.14, javaOnlyMap.getDouble("doubleKey", 0.0), 0.001);
  }

  @Test
  public void testJavaOnlyMapPutByteArrayAsString() {
    byte[] key = "key".getBytes();
    byte[] value = "value".getBytes();
    javaOnlyMap.putByteArrayAsString(key, value);
    assertEquals("value", javaOnlyMap.getString("key"));
  }

  @Test
  public void testJavaOnlyMapPutArray() {
    JavaOnlyArray array = new JavaOnlyArray();
    array.pushString("item1");
    array.pushString("item2");
    javaOnlyMap.putArray("arrayKey", array);

    Object retrievedArray = javaOnlyMap.get("arrayKey");
    assertTrue(retrievedArray instanceof JavaOnlyArray);
    assertEquals(2, ((JavaOnlyArray) retrievedArray).size());
  }

  @Test
  public void testJavaOnlyMapGetKeys() {
    javaOnlyMap.putString("key1", "value1");
    javaOnlyMap.putString("key2", "value2");
    ArrayList<String> keys = javaOnlyMap.getKeys();
    assertEquals(2, keys.size());
    assertTrue(keys.contains("key1"));
    assertTrue(keys.contains("key2"));
  }

  @Test
  public void testJavaOnlyMapGetTypeIndex() {
    javaOnlyMap.putString("stringKey", "stringValue");
    javaOnlyMap.putInt("intKey", 42);
    javaOnlyMap.putDouble("doubleKey", 3.14);

    assertEquals(ReadableType.String.ordinal(), javaOnlyMap.getTypeIndex("stringKey"));
    assertEquals(ReadableType.Int.ordinal(), javaOnlyMap.getTypeIndex("intKey"));
    assertEquals(ReadableType.Number.ordinal(), javaOnlyMap.getTypeIndex("doubleKey"));
  }

  @Test
  public void testJavaOnlyMapAsHashMap() {
    javaOnlyMap.putString("key1", "value1");
    javaOnlyMap.putInt("key2", 42);
    HashMap<String, Object> hashMap = javaOnlyMap.asHashMap();
    assertEquals(2, hashMap.size());
    assertEquals("value1", hashMap.get("key1"));
    assertEquals(42, hashMap.get("key2"));
  }
}
