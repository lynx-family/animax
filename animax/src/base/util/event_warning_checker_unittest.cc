// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include "src/base/util/event_warning_checker.h"

#include "gtest/gtest.h"

using namespace lynx::animax;

class EventWarningCheckerTest : public ::testing::Test {
 protected:
  EventWarningCheckerTest() = default;
  ~EventWarningCheckerTest() override = default;
};

TEST_F(EventWarningCheckerTest, CheckStartAndEndFrame) {
  bool valid = EventWarningChecker::CheckStartAndEndFrame(10, 5, 0, nullptr);
  EXPECT_EQ(valid, false);
}

TEST_F(EventWarningCheckerTest, CheckAssetCountOverLimit) {
  bool valid =
      EventWarningChecker::CheckAssetCountOverLimit(30, 20, 5, nullptr);
  EXPECT_EQ(valid, false);
}

TEST_F(EventWarningCheckerTest, CheckLottieFormat) {
  bool valid =
      EventWarningChecker::CheckLottieFormat("https://example.lottie", nullptr);
  EXPECT_EQ(valid, false);
}

TEST_F(EventWarningCheckerTest, CheckDynamicResourceLoadable) {
  bool valid =
      EventWarningChecker::CheckDynamicResourceLoadable(true, true, nullptr);
  EXPECT_EQ(valid, false);
}

TEST_F(EventWarningCheckerTest, CheckExecuteBeforeReady) {
  bool valid =
      EventWarningChecker::CheckExecuteBeforeReady(false, "pause", nullptr);
  EXPECT_EQ(valid, false);
}

TEST_F(EventWarningCheckerTest, CheckIllegalAlphaVideoSize) {
  bool valid = EventWarningChecker::CheckIllegalAlphaVideoSize(17, 17, nullptr);
  EXPECT_EQ(valid, false);
}

TEST_F(EventWarningCheckerTest, CheckIllegalSubscribedStartFrame) {
  bool valid =
      EventWarningChecker::CheckIllegalSubscribedStartFrame(10, nullptr);
  EXPECT_EQ(valid, false);
}
