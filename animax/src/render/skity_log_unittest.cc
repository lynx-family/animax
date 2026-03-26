// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/render/skity_log.h"

#include <gtest/gtest.h>

using namespace lynx::animax;

void TestALogFunction(AnimaXLogLevel level, const char* tag, const char* text) {
}

TEST(SkityLogTest, RegisterLogFunction) {
  SkityLog::RegisterLogFunction(TestALogFunction);
  EXPECT_EQ(SkityLog::GetLogFunction(), TestALogFunction);
}
