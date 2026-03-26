// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax;

import static org.junit.Assert.assertEquals;

import android.Manifest;
import android.app.Application;
import android.content.Context;
import androidx.test.ext.junit.runners.AndroidJUnit4;
import androidx.test.platform.app.InstrumentationRegistry;
import androidx.test.rule.GrantPermissionRule;
import com.lynx.animax.base.CalledByNative;
import com.lynx.animax.util.AnimaX;
import com.lynx.animax.util.AnimaXLog;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;

@RunWith(AndroidJUnit4.class)
public class GTestRunnerTest {
  @Rule
  public GrantPermissionRule mRuntimePermissionRule = GrantPermissionRule.grant(
      Manifest.permission.WRITE_EXTERNAL_STORAGE, Manifest.permission.READ_EXTERNAL_STORAGE);
  private String testMessage;

  private String[] testSuites;

  @Before
  public void setUp() {
    Context context =
        InstrumentationRegistry.getInstrumentation().getTargetContext().getApplicationContext();
    AnimaX.inst().init();
    AnimaX.inst().setAppContextIfUnset(context.getApplicationContext());
    registerJNI();
    testMessage = "";
    testSuites = new String[] {"MiscUtilTest", "AnimaXLoaderCompletionHandlerAndroidTest",
        "AnimaXLoaderResponseAndroidTest", "AndroidTransformLoadersTest"};
  }

  private String gtestFilter() {
    StringBuilder builder = new StringBuilder();
    builder.append("--gtest_filter=");
    for (String suite : testSuites) {
      builder.append(suite);
      builder.append(".*");
      builder.append(":");
    }
    return builder.toString();
  }

  @Test
  public void runAllGTest() {
    int result = nativeRunGTestsNative(new String[] {gtestFilter()});
    AnimaXLog.i("AnimaXGTestRunnerTest", testMessage);
    assertEquals(testMessage, 0, result);
  }

  @CalledByNative
  public void onTestMessage(String message) {
    testMessage = message;
  }
  public native int nativeRunGTestsNative(String[] argv);

  // use static register defined at testing/lynx/android/gtest_runner_android.cc
  private static native void registerJNI();
}
