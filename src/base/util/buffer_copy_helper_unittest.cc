// Copyright 2025 The Lynx Authors
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include "src/base/util/buffer_copy_helper.h"

#include "gtest/gtest.h"
using namespace lynx::animax;

TEST(BufferCopyHelper, CopyBuffer) {
  auto src = new uint8_t[100 * 100 * 4];
  auto dst = new uint8_t[100 * 100 * 4];
  for (uint32_t i = 0; i < 10000; ++i) {
    reinterpret_cast<uint32_t*>(src)[i] = i;
  }
  BufferCopyHelper::CopyBuffer(src, dst, 100, 100 * 4, 100 * 4);
  for (uint32_t i = 0; i < 10000; ++i) {
    EXPECT_EQ(reinterpret_cast<uint32_t*>(dst)[i], i);
  }
}

TEST(BufferCopyHelper, CopyBufferWithStride) {
  auto src = new uint8_t[50 * 100 * 4];
  auto dst = new uint8_t[100 * 100 * 4];
  for (uint32_t i = 0; i < 5000; ++i) {
    reinterpret_cast<uint32_t*>(src)[i] = i;
  }
  for (uint32_t i = 0; i < 10000; ++i) {
    reinterpret_cast<uint32_t*>(dst)[i] = 0;
  }
  BufferCopyHelper::CopyBuffer(src, dst, 100, 50 * 4, 100 * 4);
  for (uint32_t i = 0; i < 10000; ++i) {
    int res = i % 100 < 50 ? i / 100 * 50 + i % 100 : 0;
    EXPECT_EQ(reinterpret_cast<uint32_t*>(dst)[i], res);
  }
}

TEST(BufferCopyHelper, CopyBufferWithInvalidParams) {
  EXPECT_TRUE(BufferCopyHelper::IsSizeMatch(100, 100, 100, 100));
  EXPECT_FALSE(BufferCopyHelper::IsSizeMatch(100, 50, 100, 100));
  EXPECT_FALSE(BufferCopyHelper::IsSizeMatch(50, 100, 100, 100));
  uint8_t src = 1;
  uint8_t dst = 0;
  BufferCopyHelper::CopyBuffer(nullptr, &dst, 1, 4, 4);
  EXPECT_EQ(dst, 0);
  BufferCopyHelper::CopyBuffer(&src, nullptr, 1, 4, 4);
  EXPECT_EQ(dst, 0);
  BufferCopyHelper::CopyBuffer(&src, &dst, 0, 1, 4);
  EXPECT_EQ(dst, 0);
  BufferCopyHelper::CopyBuffer(&src, &dst, 1, 0, 4);
  EXPECT_EQ(dst, 0);
}
