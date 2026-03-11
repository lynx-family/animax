// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.util;

import android.net.Uri;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

public class UriUtil {
  // Defined several common URI schemes
  public static final String HTTP_SCHEME = "http";
  public static final String HTTPS_SCHEME = "https";
  public static final String LOCAL_ASSET_SCHEME = "asset";
  public static final String LOCAL_FILE_SCHEME = "file";

  /*
   * Safely parses a URL to a Uri object
   * @param url The URL string to be parsed
   * @return The parsed Uri object, or null if url is null
   */
  public static Uri safeParse(@Nullable String url) {
    return url == null ? null : Uri.parse(url);
  }

  /*
   * Creates a "asset://" type URI from a local asset name
   * @param assetName The name of the local asset
   * @return The URI string of type asset
   */
  public static String fromLocalAsset(@NonNull String assetName) {
    return new Uri.Builder()
        .scheme(LOCAL_ASSET_SCHEME)
        .authority("")
        .path(assetName)
        .build()
        .toString();
  }

  /*
   * Creates a "file://" type URI from a local file path
   * @param localPath The path of the local file
   * @return The URI string of type file
   */
  public static String fromLocalFile(@NonNull String localPath) {
    return new Uri.Builder()
        .scheme(LOCAL_FILE_SCHEME)
        .authority("")
        .path(localPath)
        .build()
        .toString();
  }

  /*
   * Gets the asset name from an asset URI
   * @param uri The Uri of type asset
   * @return The name of the asset, i.e., the path part of the URI without the leading "/"
   */
  public static String getAssetName(@NonNull Uri uri) {
    return uri.getPath().substring(1);
  }

  /*
   * Gets the local file path from a file URI
   * @param uri The Uri of type file
   * @return The path of the local file, i.e., the path part of the URI
   */
  public static String getLocalFileName(@NonNull Uri uri) {
    return uri.getPath();
  }

  /**
   * Check if uri represents network resource
   *
   * @param uri uri to check
   * @return true if uri's scheme is equal to "http" or "https"
   */
  public static boolean isNetworkUri(@Nullable Uri uri) {
    final String scheme = getSchemeOrNull(uri);
    return HTTPS_SCHEME.equals(scheme) || HTTP_SCHEME.equals(scheme);
  }

  /**
   * Check if uri represents local asset
   *
   * @param uri uri to check
   * @return true if uri's scheme is equal to "asset"
   */
  public static boolean isLocalAssetUri(@Nullable Uri uri) {
    final String scheme = getSchemeOrNull(uri);
    return LOCAL_ASSET_SCHEME.equals(scheme);
  }

  /**
   * Check if uri represents local file
   *
   * @param uri uri to check
   * @return true if uri's scheme is equal to "file"
   */
  public static boolean isLocalFileUri(@Nullable Uri uri) {
    final String scheme = getSchemeOrNull(uri);
    return LOCAL_FILE_SCHEME.equals(scheme);
  }

  /**
   * @param uri uri to extract scheme from, possibly null
   * @return null if uri is null, result of uri.getScheme() otherwise
   */
  @Nullable
  private static String getSchemeOrNull(@Nullable Uri uri) {
    return uri == null ? null : uri.getScheme();
  }
}
