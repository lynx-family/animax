// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.util;

import android.app.ActivityManager;
import android.content.Context;
import android.content.pm.ConfigurationInfo;
import android.os.Build;
import android.os.Process;
import android.text.TextUtils;
import androidx.annotation.NonNull;
import com.lynx.animax.ability.BaseAbility;
import com.lynx.animax.service.IAnimaXSettingService;
import com.lynx.animax.setting.AnimaXSettingValue;
import java.util.Arrays;
import java.util.Collection;
import java.util.concurrent.ConcurrentHashMap;

public class DeviceUtil {
  private static final String TAG = "DeviceUtil";

  private static final String sUpdateTextureWhenVisible = "ANIMAX_UPDATE_TEXTURE_WHEN_VISIBLE";
  private static final String sSettingBlockListName = "ANIMAX_MODEL_BLOCK_LIST_ANDROID";
  private static final String sSettingSoftwareListName = "ANIMAX_MODEL_SOFTWARE_LIST_ANDROID";

  private static final String sSettingVideoSurfaceDestroyTimeout =
      "ANIMAX_VIDEO_SURFACE_DESTROY_TIMEOUT";
  private static final String sSettingSyncSurfaceDestroyName = "ANIMAX_SYNC_SURFACE_DESTROY";
  private static final String sSettingLegacyFetcher = "ANIMAX_USE_LEGACY_FETCHER";
  private static final String sSettingDisableShowEventLogbox = "ANIMAX_DISABLE_SHOW_EVENT_LOGBOX";
  private static final String sSettingDisableByteVC1Decoder = "ANIMAX_DISABLE_BYTEVC1_DECODER";

  private static final String sSettingUseImageViewPrefix = "ANIMAX_USE_IMAGE_VIEW_";

  private static final String sDeviceType = Build.MODEL.toLowerCase();
  private static int sEglVersion = -1;
  private static final boolean sTextureDestroyWorkaround =
      Arrays.asList("m6 note", "m721c").contains(sDeviceType);

  private static boolean sHasInitAppBit = false;
  private static boolean sIs32BitApp = false;

  private static boolean sHasInitLowDensity = false;
  private static boolean sIsLowDensity = false;
  private static boolean sIsMiuiInited = false;
  private static boolean sIsMiui = false;

  /**
   * Determines whether the texture needs to try updating the surface texture when the view is
   * visible.
   * @param ability
   * @return
   */
  public static boolean updateTextureWhenVisible(@NonNull BaseAbility ability) {
    final String value = getStringFromExternalEnv(sUpdateTextureWhenVisible, ability);
    return isPositiveConfigValue(value);
  }

  /**
   * Determines if the device requires software rendering mode.
   * The software rendering mode is engaged when the device does not support OpenGL ES 3
   * or is specified in the online software settings list. Under these conditions,
   * AnimaX will operate in software rendering mode.
   * @param context
   * @return
   */
  public static boolean useSoftwareRender(Context context, @NonNull BaseAbility ability) {
    return !supportHardwareRender(context) || isInSettingList(sSettingSoftwareListName, ability);
  }

  /**
   * Justify whether current device type is in online setting list
   * @return
   */
  private static boolean isInSettingList(String listName, @NonNull BaseAbility ability) {
    IAnimaXSettingService service = ability.getService(IAnimaXSettingService.class);
    if (service == null) {
      return false;
    }

    try {
      AnimaXSettingValue settingValue = service.getValueByKey(listName);
      if (settingValue == null) {
        return false;
      }

      if (settingValue.isString()) {
        return isPositiveConfigValue(settingValue.getStringOrEmpty());
      } else if (settingValue.isCollection()) {
        Collection<String> list = settingValue.getCollectionOrEmpty();
        return !list.isEmpty() && sDeviceType != null && !sDeviceType.isEmpty()
            && list.contains(sDeviceType);
      }
    } catch (Throwable t) {
      AnimaXLog.e(TAG, "isInSettingList fail, message" + t.getMessage());
    }
    return false;
  }

  /**
   * Justify whether current egl version is greater or equals to 3.0
   * @param context
   * @return
   */
  private static boolean supportHardwareRender(Context context) {
    if (sEglVersion < 0) {
      try {
        ActivityManager am = (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
        ConfigurationInfo info = am.getDeviceConfigurationInfo();
        sEglVersion = info.reqGlEsVersion;
      } catch (Throwable t) {
        AnimaXLog.e(TAG, "isES3Supported fail, message" + t.getMessage());
      }
    }
    return sEglVersion >= 0x30000;
  }

  /**
   * Checks if the device is supported by the capability settings list.
   * If the device is not supported, it will be unable to init AnimaXView.
   * @return boolean indicating whether the device is capable or not.
   */
  public static boolean checkCapability(@NonNull BaseAbility ability) {
    return !isInSettingList(sSettingBlockListName, ability);
  }

  public static boolean needTextureDestroyWorkaround() {
    return sTextureDestroyWorkaround;
  }

  public static synchronized boolean isMiui() {
    if (!sIsMiuiInited) {
      try {
        Class.forName("miui.os.Build");
        sIsMiui = true;
      } catch (Throwable e) {
        // ignore
      }
      sIsMiuiInited = true;
    }
    return sIsMiui;
  }

  /**
   * Device with android 9 and below need this workaround
   * @return boolean indicating whether the device requires an alpha workaround.
   */
  public static boolean needAlphaWorkaround() {
    return Build.VERSION.SDK_INT <= Build.VERSION_CODES.P;
  }

  private static boolean is64BitProcess() {
    try {
      if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
        return Process.is64Bit();
      } else {
        Class<?> cls = Class.forName("dalvik.system.VMRuntime");
        Object runtime = cls.getDeclaredMethod("getRuntime").invoke(null);
        Boolean is64Bit = (Boolean) cls.getDeclaredMethod("is64Bit").invoke(runtime);
        return is64Bit != null ? is64Bit : true;
      }
    } catch (Exception e) {
      return true;
    }
  }

  private static boolean is32BitApp() {
    if (!sHasInitAppBit) {
      sIs32BitApp = !is64BitProcess();
      sHasInitAppBit = true;
    }
    return sIs32BitApp;
  }

  public static boolean needLimitFrameRate() {
    return Build.VERSION.SDK_INT < Build.VERSION_CODES.O || is32BitApp();
  }

  public static void initDeviceLowDensity(float density) {
    if (!sHasInitLowDensity) {
      sIsLowDensity = density < 2.0f && density > 0.01f;
      sHasInitLowDensity = true;
    }
  }

  public static boolean needSampleImage(int width, int height) {
    // Do not sample image if the size is smaller than 32
    if (width < 32 || height < 32) {
      return false;
    }

    return is32BitApp() || sIsLowDensity;
  }

  public static long getVideoSurfaceDestroyTimeout(@NonNull BaseAbility ability) {
    final String result = getStringFromExternalEnv(sSettingVideoSurfaceDestroyTimeout, ability);
    try {
      return TextUtils.isEmpty(result) ? 0 : Long.parseLong(result);
    } catch (NumberFormatException e) {
      AnimaXLog.e(TAG, "getVideoSurfaceDestroyTimeout fail, message" + e.getMessage());
    }
    return 0;
  }

  // Use legacy fetcher, default is false
  public static boolean useLegacyFetcher(@NonNull BaseAbility ability) {
    final String result = getStringFromExternalEnv(sSettingLegacyFetcher, ability);
    return isPositiveConfigValue(result);
  }

  public static boolean disableShowEventLogbox(@NonNull BaseAbility ability) {
    final String result = getStringFromExternalEnv(sSettingDisableShowEventLogbox, ability);
    return isPositiveConfigValue(result);
  }

  public static boolean shouldUseImageViewByTag(@NonNull BaseAbility ability, String tag) {
    if (tag == null || tag.isEmpty()) {
      return false;
    }
    final String result = getStringFromExternalEnv(sSettingUseImageViewPrefix + tag, ability);
    return isPositiveConfigValue(result);
  }
  public static boolean disableByteVC1Decoder(@NonNull BaseAbility ability) {
    final String result = getStringFromExternalEnv(sSettingDisableByteVC1Decoder, ability);
    return isPositiveConfigValue(result);
  }
  private static String getStringFromExternalEnv(String key, @NonNull BaseAbility ability) {
    IAnimaXSettingService service = ability.getService(IAnimaXSettingService.class);
    if (service != null) {
      AnimaXSettingValue value = service.getValueByKey(key);
      return value != null ? value.getStringOrEmpty() : "";
    } else {
      return "";
    }
  }

  public static boolean needAutoDestroyEGLContext() {
    return false;
  }

  public static boolean needScreenLockWorkaround() {
    return Build.VERSION.SDK_INT >= Build.VERSION_CODES.N
        && Build.VERSION.SDK_INT <= Build.VERSION_CODES.Q;
  }

  private static boolean isPositiveConfigValue(String stringValue) {
    return stringValue != null && !stringValue.isEmpty()
        && ("1".equals(stringValue) || "true".equalsIgnoreCase(stringValue));
  }
}
