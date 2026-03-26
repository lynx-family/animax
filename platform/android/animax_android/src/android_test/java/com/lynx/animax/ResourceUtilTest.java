// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax;

import static junit.framework.TestCase.assertNotNull;
import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

import android.content.Context;
import androidx.test.ext.junit.runners.AndroidJUnit4;
import androidx.test.platform.app.InstrumentationRegistry;
import com.lynx.animax.util.ResourceUtil;
import java.io.FileOutputStream;
import java.io.IOException;
import org.junit.Test;
import org.junit.runner.RunWith;

@RunWith(AndroidJUnit4.class)
public class ResourceUtilTest {
  private static final String ASSET_NAME = "example_asset.txt";

  @Test
  public void testGetByteArrayFromAsset_Success() {
    Context appContext = InstrumentationRegistry.getInstrumentation().getTargetContext();

    byte[] result = ResourceUtil.getByteArrayFromAsset(ASSET_NAME, appContext.getAssets());

    assertNotNull("Result should not be null", result);

    assertTrue("Array should contain data", result.length > 0);

    String expectedString = "Hello, World!\n";
    byte[] expectedBytes = expectedString.getBytes();

    assertArrayEquals("Byte array does not match expected content", expectedBytes, result);
  }

  @Test
  public void testGetByteArrayFromAsset_Failed() {
    Context appContext = InstrumentationRegistry.getInstrumentation().getTargetContext();

    byte[] result = ResourceUtil.getByteArrayFromAsset("no_such_asset.txt", appContext.getAssets());

    assertNull("Result should be null", result);
  }

  @Test
  public void testGetByteArrayFromFile_Success() {
    Context appContext = InstrumentationRegistry.getInstrumentation().getTargetContext();

    String filePath = appContext.getFilesDir() + "/testfile.txt";

    String expectedString = "Hello, World!\n";
    byte[] expectedBytes = expectedString.getBytes();
    try (FileOutputStream fos = appContext.openFileOutput("testfile.txt", Context.MODE_PRIVATE)) {
      fos.write(expectedBytes);
    } catch (IOException e) {
      e.printStackTrace();
      return;
    }

    byte[] result = ResourceUtil.getByteArrayFromFile(filePath);

    assertNotNull("Result should not be null", result);

    assertTrue("Array should contain data", result.length > 0);

    assertArrayEquals("Byte array does not match expected content", expectedBytes, result);
  }

  @Test
  public void testGetByteArrayFromFile_Failed() {
    String filePath = "/path/to/non/existent/file.txt";

    byte[] result = ResourceUtil.getByteArrayFromFile(filePath);

    assertNull("Result should be null", result);
  }
}
