// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include "src/base/thread/thread_assert.h"

#include "gtest/gtest.h"
using namespace lynx::animax;

TEST(AnimaXThreadAssert, Init) {
  ThreadAssert::Init(ThreadAssert::Type::kUnknown);
  EXPECT_EQ(ThreadAssert::GetCurrent(), ThreadAssert::Type::kUnknown);
  ThreadAssert::Init(ThreadAssert::Type::kGPU);
  ThreadAssert::Assert(ThreadAssert::Type::kGPU);
  ThreadAssert::Init(ThreadAssert::Type::kResource);
  ThreadAssert::Assert(ThreadAssert::Type::kResource);
  ThreadAssert::Init(ThreadAssert::Type::kMain);
  ThreadAssert::Assert(ThreadAssert::Type::kMain);
}

TEST(AnimaXThreadAssert, WrongAssert) {
  ThreadAssert::Init(ThreadAssert::Type::kGPU);
  EXPECT_DEATH(ThreadAssert::Assert(ThreadAssert::Type::kMain), ".*");
  EXPECT_DEATH(ThreadAssert::Assert(ThreadAssert::Type::kResource), ".*");
  ThreadAssert::Init(ThreadAssert::Type::kMain);
  EXPECT_DEATH(ThreadAssert::Assert(ThreadAssert::Type::kGPU), ".*");
}
