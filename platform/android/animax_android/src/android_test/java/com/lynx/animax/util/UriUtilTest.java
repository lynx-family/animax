// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.util;

import android.net.Uri;
import junit.framework.TestCase;
import org.junit.Test;

public class UriUtilTest extends TestCase {
  @Test
  public void testSafeParse() {
    String url = "http://example.com";
    Uri uri = UriUtil.safeParse(url);
    assertNotNull(uri);
    assertEquals("http", uri.getScheme());
    assertNull(UriUtil.safeParse(null));
  }

  @Test
  public void testFromLocalAsset() {
    // Asset in the root directory
    String assetName = "test.png";
    String assetUrl = UriUtil.fromLocalAsset(assetName);
    assertEquals("asset:///test.png", assetUrl);
    assertEquals(assetName, UriUtil.getAssetName(UriUtil.safeParse(assetUrl)));

    // Asset in the sub-directory
    String dirAssetName = "directory1/directory2/test.json";
    String dirAssetUrl = UriUtil.fromLocalAsset(dirAssetName);
    assertEquals("asset:///directory1/directory2/test.json", dirAssetUrl);
    assertEquals(dirAssetName, UriUtil.getAssetName(UriUtil.safeParse(dirAssetUrl)));
  }

  @Test
  public void testFromLocalFile() {
    String localPath = "/sdcard/storage/emulated/0/animations/data.json";
    String result = UriUtil.fromLocalFile(localPath);
    assertTrue(UriUtil.isLocalFileUri(UriUtil.safeParse(result)));
    assertEquals("file:///sdcard/storage/emulated/0/animations/data.json", result);
    assertEquals(localPath, UriUtil.getLocalFileName(UriUtil.safeParse(localPath)));
  }

  @Test
  public void testIsNetworkUri() {
    Uri httpUri = Uri.parse("http://example.com");
    Uri httpsUri = Uri.parse("https://example.com");
    Uri fileUri = Uri.parse("file://path/to/file");
    assertTrue(UriUtil.isNetworkUri(httpUri));
    assertTrue(UriUtil.isNetworkUri(httpsUri));
    assertFalse(UriUtil.isNetworkUri(fileUri));
  }

  @Test
  public void testIsLocalAssetUri() {
    Uri assetUri = Uri.parse("asset:///path/to/asset");
    Uri httpUri = Uri.parse("http://example.com");
    assertTrue(UriUtil.isLocalAssetUri(assetUri));
    assertFalse(UriUtil.isLocalAssetUri(httpUri));
  }

  @Test
  public void testIsLocalFileUri() {
    Uri fileUri = Uri.parse("file:///sdcard/storage/emulated/0/animations/data.json");
    Uri httpUri = Uri.parse("http://example.com");
    assertTrue(UriUtil.isLocalFileUri(fileUri));
    assertFalse(UriUtil.isLocalFileUri(httpUri));
  }

  @Test
  public void testGetAssetName() {
    Uri uri = Uri.parse("asset:///directory1/directory2/test.json");
    String assetName = UriUtil.getAssetName(uri);
    assertEquals("directory1/directory2/test.json", assetName);
  }

  @Test
  public void testGetLocalFileName() {
    Uri uri = Uri.parse("file:///sdcard/storage/emulated/0/animations/data.json");
    String fileName = UriUtil.getLocalFileName(uri);
    assertEquals("/sdcard/storage/emulated/0/animations/data.json", fileName);
  }
}
