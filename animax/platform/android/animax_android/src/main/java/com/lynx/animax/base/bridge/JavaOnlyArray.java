// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.animax.base.bridge;

import com.lynx.animax.base.CalledByNative;
import java.util.ArrayList;

public class JavaOnlyArray extends ArrayList<Object> implements ReadableArray {
  public JavaOnlyArray() {
    super();
  }

  public ArrayList<Object> asArrayList() {
    return this;
  }

  @CalledByNative
  public void pushArray(JavaOnlyArray array) {
    this.add(array);
  }

  @CalledByNative
  public void pushString(String value) {
    this.add(value);
  }

  @CalledByNative
  public void pushByteArrayAsString(byte[] array) {
    this.add(new String(array));
  }

  @Override
  @CalledByNative
  public int size() {
    return super.size();
  }

  @Override
  @CalledByNative
  public String getString(int index) {
    return (String) this.get(index);
  }

  public ReadableType getType(int index) {
    Object object = this.get(index);

    if (object == null) {
      return ReadableType.Null;
    } else if (object instanceof Boolean) {
      return ReadableType.Boolean;
    } else if (object instanceof Integer) {
      return ReadableType.Int;
    } else if (object instanceof Long) {
      return ReadableType.Long;
    } else if (object instanceof Number || object instanceof Character) {
      return ReadableType.Number;
    } else if (object instanceof String) {
      return ReadableType.String;
    } else if (object instanceof ReadableMap) {
      return ReadableType.Map;
    } else if (object instanceof byte[]) {
      return ReadableType.ByteArray;
    } else {
      throw new IllegalArgumentException(
          "unsupported type " + object.getClass() + " contained in JavaOnlyArray");
    }
  }

  @CalledByNative
  public int getTypeIndex(int index) {
    return getType(index).ordinal();
  }

  @CalledByNative
  private static JavaOnlyArray create() {
    return new JavaOnlyArray();
  }
}
