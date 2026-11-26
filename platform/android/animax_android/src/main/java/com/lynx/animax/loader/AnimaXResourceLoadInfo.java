// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.animax.loader;

import android.os.OperationCanceledException;
import android.util.Base64;
import androidx.annotation.RestrictTo;
import com.lynx.animax.monitor.AnimaXMonitorUtil;
import com.lynx.animax.service.IAnimaXMonitorService.ResourceLoadReporter;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.UUID;
import java.util.concurrent.CancellationException;
import java.util.concurrent.ConcurrentHashMap;

@RestrictTo(RestrictTo.Scope.LIBRARY_GROUP)
public final class AnimaXResourceLoadInfo {
  // Matches UriInfo::ContentType in the player.
  private static final String[] TYPES = {"json", "zip", "font", "image", "video", "audio"};

  private static final List<String> HITS =
      Arrays.asList("memory", "disk", "offline", "http_cache", "http_revalidated", "cdn_cache");

  private final ResourceLoadReporter mReporter;
  private volatile String mSource = AnimaXMonitorUtil.SRC_URL_UNKNOWN;

  public AnimaXResourceLoadInfo(ResourceLoadReporter reporter) {
    mReporter = reporter;
  }

  public void setSource(String source) {
    mSource = source;
  }

  void load(IAnimaXLoader loader, AnimaXLoaderRequest request,
      AnimaXLoaderCompletionHandler completion, int contentType) {
    final String url = request.getUri();
    final String source =
        AnimaXMonitorUtil.clearUrlQuery(contentType == 0 || contentType == 1 ? url : mSource);
    Map<AnimaXLoaderResponse<?>, String> sources = new ConcurrentHashMap<>();
    completion.setResourceLoadObserver(response -> {
      Object data = response == null ? null : response.getData();
      boolean success = (data instanceof byte[] && ((byte[]) data).length > 0)
          || (data instanceof String && !((String) data).isEmpty())
          || data instanceof IAnimaXCloseableBitmapReference;
      boolean cancelled =
          data instanceof CancellationException || data instanceof OperationCanceledException;
      String hostSource = success ? sources.get(response) : null;
      String cacheSource = success
          ? cacheSource(hostSource != null ? hostSource : response.getCacheSource())
          : "unknown";
      Map<String, Object> event = new HashMap<>();
      event.put("request_id", UUID.randomUUID().toString());
      event.put("phase", "consume");
      event.put("resource_type",
          contentType >= 0 && contentType < TYPES.length ? TYPES[contentType] : "unknown");
      event.put("resource_url_hash", urlHash(url));
      event.put("status", success ? "success" : cancelled ? "cancelled" : "failed");
      event.put("cache_source", cacheSource);
      event.put(AnimaXMonitorUtil.KEY_SRC_URL, source);
      mReporter.reportResourceLoad(event);
    });
    loader.loadWithCacheSource(request, completion, sources);
  }

  private static String cacheSource(Object source) {
    return HITS.contains(source) || "network".equals(source) || "in_flight".equals(source)
            || "builtin".equals(source)
        ? (String) source
        : "unknown";
  }

  private static String urlHash(String url) {
    if (url.isEmpty()) {
      return "";
    }
    try {
      byte[] bytes =
          MessageDigest.getInstance("SHA-256").digest(url.getBytes(StandardCharsets.UTF_8));
      return Base64.encodeToString(bytes, Base64.NO_WRAP);
    } catch (NoSuchAlgorithmException e) {
      throw new AssertionError(e);
    }
  }
}
