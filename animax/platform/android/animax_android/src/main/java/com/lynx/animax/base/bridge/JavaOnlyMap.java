// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.animax.base.bridge;

import com.lynx.animax.base.CalledByNative;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

public class JavaOnlyMap extends HashMap<String, Object> implements ReadableMap {
  public static JavaOnlyMap from(Map map) {
    return new JavaOnlyMap(map);
  }

  private JavaOnlyMap(Map map) {
    super(map);
  }

  public JavaOnlyMap() {
    super();
  }

  @Override
  public int getInt(String name, int defaultValue) {
    Object result = get(name);
    if (result instanceof Number) {
      return ((Number) result).intValue();
    }

    if (result instanceof Boolean) {
      return ((Boolean) result) ? 1 : 0;
    }

    if (result instanceof String) {
      try {
        return Integer.parseInt(((String) result));
      } catch (NumberFormatException e) {
        e.printStackTrace();
      }
    }

    return defaultValue;
  }

  @Override
  public long getLong(String name, long defaultValue) {
    Object result = get(name);
    if (result instanceof Number) {
      return ((Number) result).longValue();
    }

    if (result instanceof Boolean) {
      return ((Boolean) result) ? 1 : 0;
    }

    if (result instanceof String) {
      try {
        return Long.parseLong(((String) result));
      } catch (NumberFormatException e) {
        e.printStackTrace();
      }
    }

    return defaultValue;
  }

  @Override
  public double getDouble(String name, double defaultValue) {
    Object result = get(name);
    if (result instanceof Number) {
      return ((Number) result).doubleValue();
    }

    if (result instanceof Boolean) {
      return ((Boolean) result) ? 1 : 0;
    }

    if (result instanceof String) {
      try {
        return Double.parseDouble(((String) result));
      } catch (NumberFormatException e) {
        e.printStackTrace();
      }
    }

    return defaultValue;
  }

  @CalledByNative
  public void putDouble(String key, double value) {
    put(key, value);
  }

  @CalledByNative
  public void putInt(String key, int value) {
    put(key, value);
  }

  @CalledByNative
  public void putString(String key, String value) {
    put(key, value);
  }

  @CalledByNative
  public void putByteArrayAsString(byte[] key, byte[] value) {
    put(new String(key), new String(value));
  }

  @CalledByNative
  public void putArray(String key, JavaOnlyArray value) {
    put(key, value);
  }

  @Override
  public HashMap<String, Object> asHashMap() {
    return this;
  }

  @CalledByNative
  private static JavaOnlyMap create() {
    return new JavaOnlyMap();
  }

  @CalledByNative
  public ArrayList<String> getKeys() {
    return new ArrayList<>(keySet());
  }

  @CalledByNative
  public int getTypeIndex(String name) {
    return getType(name).ordinal();
  }

  @Override
  @CalledByNative
  public String getString(String name) {
    Object obj = get(name);
    if (obj instanceof String) {
      return (String) obj;
    }
    if (obj instanceof Boolean) {
      return ((Boolean) obj) ? "true" : "false";
    }

    if (obj instanceof Number) {
      return ((Number) obj).toString();
    }

    if (obj == null) {
      return null;
    }
    throw new ClassCastException(obj.getClass().getName()
        + " cannot be cast to java.lang.String, key: " + name + ", value: " + obj);
  }

  @Override
  @CalledByNative
  public ReadableArray getArray(String name) {
    Object obj = get(name);
    if (obj instanceof JavaOnlyArray) {
      return (JavaOnlyArray) obj;
    }
    throw new ClassCastException(obj.getClass().getName() + " cannot be cast to "
        + ReadableArray.class.getName() + ", key: " + name + ", value: " + obj);
  }

  @Override
  public ReadableType getType(String name) {
    Object value = get(name);
    if (value == null) {
      return ReadableType.Null;
    } else if (value instanceof Integer) {
      return ReadableType.Int;
    } else if (value instanceof Long) {
      return ReadableType.Long;
    } else if (value instanceof Number || value instanceof Character) {
      return ReadableType.Number;
    } else if (value instanceof String) {
      return ReadableType.String;
    } else if (value instanceof Boolean) {
      return ReadableType.Boolean;
    } else if (value instanceof ReadableMap) {
      return ReadableType.Map;
    } else if (value instanceof ReadableArray) {
      return ReadableType.Array;
    } else {
      throw new IllegalArgumentException(
          "Invalid value " + value.toString() + " for key " + name + "contained in JavaOnlyMap");
    }
  }
}
