// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.util;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicBoolean;
import junit.framework.TestCase;
import org.junit.Test;

public class UIThreadUtilsTest extends TestCase {
  @Test
  public void testIsOnUiThread() {
    // Test on UI thread
    final AtomicBoolean result = new AtomicBoolean(false);
    UIThreadUtils.runOnUiThread(() -> { result.set(UIThreadUtils.isOnUiThread()); });

    try {
      Thread.sleep(100);
    } catch (InterruptedException e) {
      Thread.currentThread().interrupt();
    }
    assertTrue(result.get());
  }

  @Test
  public void testRunOnUiThread() throws InterruptedException {
    final CountDownLatch latch = new CountDownLatch(1);
    final AtomicBoolean executed = new AtomicBoolean(false);

    UIThreadUtils.runOnUiThread(() -> {
      executed.set(true);
      latch.countDown();
    });

    assertTrue(latch.await(1, TimeUnit.SECONDS));
    assertTrue(executed.get());
  }

  @Test
  public void testRunOnUiThreadImmediately() throws InterruptedException {
    final CountDownLatch latch = new CountDownLatch(1);
    final AtomicBoolean executed = new AtomicBoolean(false);

    UIThreadUtils.runOnUiThreadImmediately(() -> {
      executed.set(true);
      latch.countDown();
    });

    assertTrue(latch.await(1, TimeUnit.SECONDS));
    assertTrue(executed.get());
  }

  @Test
  public void testPostAtFrontOfQueueOnUiThread() throws InterruptedException {
    final CountDownLatch latch = new CountDownLatch(1);
    final AtomicBoolean executed = new AtomicBoolean(false);

    UIThreadUtils.postAtFrontOfQueueOnUiThread(() -> {
      executed.set(true);
      latch.countDown();
    });

    assertTrue(latch.await(1, TimeUnit.SECONDS));
    assertTrue(executed.get());
  }

  @Test
  public void testRunOnUiThreadImmediatelyOnUiThread() {
    final AtomicBoolean executed = new AtomicBoolean(false);

    UIThreadUtils.runOnUiThread(
        () -> { UIThreadUtils.runOnUiThreadImmediately(() -> { executed.set(true); }); });

    try {
      Thread.sleep(100);
    } catch (InterruptedException e) {
      Thread.currentThread().interrupt();
    }
    assertTrue(executed.get());
  }
}
