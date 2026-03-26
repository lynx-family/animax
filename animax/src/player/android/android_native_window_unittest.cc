// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/player/android/android_native_window.h"

#include "gtest/gtest.h"
using namespace lynx::animax;

namespace {

void CheckAndroidNativeWindowInvalid(AndroidNativeWindow& window) {
  EXPECT_EQ(nullptr, static_cast<ANativeWindow*>(window));
  EXPECT_FALSE(window);
  // These operations should be noop.
  window.Resize(100, 100);
  EXPECT_EQ(nullptr, static_cast<ANativeWindow*>(window));
  EXPECT_FALSE(window);
}

}  // namespace

TEST(AndroidNativeWindowTest, DefaultConstructor) {
  auto window = AndroidNativeWindow{};
  CheckAndroidNativeWindowInvalid(window);
}

TEST(AndroidNativeWindowTest, MoveOperations) {
  auto window1 = AndroidNativeWindow{};
  auto window2 = AndroidNativeWindow{};
  window1 = std::move(window2);
  auto window3 = std::move(window1);
  CheckAndroidNativeWindowInvalid(window1);
  CheckAndroidNativeWindowInvalid(window2);
  CheckAndroidNativeWindowInvalid(window3);
}

TEST(AndroidNativeWindowTest, Release) {
  auto window = AndroidNativeWindow{};
  CheckAndroidNativeWindowInvalid(window);
  window.Release();
  CheckAndroidNativeWindowInvalid(window);
}

TEST(AndroidNativeWindowTest, Resize) {
  auto window = AndroidNativeWindow{};
  CheckAndroidNativeWindowInvalid(window);
  window.Resize(100, 100);
  CheckAndroidNativeWindowInvalid(window);
}
