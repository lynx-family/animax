// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "include/resource/bitmap.h"

#include <functional>
#include <memory>
#include <vector>

#include "gtest/gtest.h"

using namespace lynx::animax;

class BitmapTest : public ::testing::Test {
 protected:
  BitmapTest() = default;
  ~BitmapTest() override = default;
};

namespace {
void DummyDeleter(const void* context) {
  // Do nothing, just for testing
}
int32_t& GetDeleterCalledCountRef() {
  thread_local int32_t deleter_called_count = 0;
  return deleter_called_count;
}
void DeleterWithCount(const void* /*pixels*/) { GetDeleterCalledCountRef()++; }
}  // namespace

TEST_F(BitmapTest, MakeRGBA) {
  const uint32_t width = 10;
  const uint32_t height = 10;
  std::vector<uint8_t> pixels(width * height * 4, 255);  // RGBA with all 255
  auto bitmap =
      Bitmap::MakeRGBA(width, height, pixels.data(), DummyDeleter, nullptr);
  EXPECT_EQ(bitmap->Width(), width);
  EXPECT_EQ(bitmap->Height(), height);
  EXPECT_EQ(bitmap->Pixels(), pixels.data());
  EXPECT_EQ(bitmap->Format(), BitmapFormat::kRGBA);
  EXPECT_EQ(bitmap->AlphaType(), BitmapAlphaType::kPremul_AlphaType);
  EXPECT_EQ(bitmap->BytesPerPixel(), 4);
  EXPECT_EQ(bitmap->PixelsLength(), width * height * 4);
  EXPECT_EQ(bitmap->BytesPerRow(), width * 4);
}

TEST_F(BitmapTest, MakeWithFormat) {
  const uint32_t width = 20;
  const uint32_t height = 20;
  std::vector<uint8_t> rgba_pixels(width * height * 4, 128);
  std::vector<uint16_t> rgb565_pixels(width * height, 0xFFFF);
  // Test RGBA format
  auto rgba_bitmap =
      Bitmap::Make(width, height, rgba_pixels.data(), DummyDeleter, nullptr,
                   BitmapFormat::kRGBA, BitmapAlphaType::kUnpremul_AlphaType);
  EXPECT_EQ(rgba_bitmap->Width(), width);
  EXPECT_EQ(rgba_bitmap->Height(), height);
  EXPECT_EQ(rgba_bitmap->Format(), BitmapFormat::kRGBA);
  EXPECT_EQ(rgba_bitmap->AlphaType(), BitmapAlphaType::kUnpremul_AlphaType);
  EXPECT_EQ(rgba_bitmap->BytesPerPixel(), 4);
  // Test RGB565 format
  auto rgb565_bitmap =
      Bitmap::Make(width, height, rgb565_pixels.data(), DummyDeleter, nullptr,
                   BitmapFormat::kRGB565, BitmapAlphaType::kPremul_AlphaType);
  EXPECT_EQ(rgb565_bitmap->Width(), width);
  EXPECT_EQ(rgb565_bitmap->Height(), height);
  EXPECT_EQ(rgb565_bitmap->Format(), BitmapFormat::kRGB565);
  EXPECT_EQ(rgb565_bitmap->BytesPerPixel(), 2);
  EXPECT_EQ(rgb565_bitmap->PixelsLength(), width * height * 2);
}

TEST_F(BitmapTest, MakeFromTexture_HasTextureAndID) {
  const uint32_t width = 5, height = 5;
  const uint64_t texture = 12345;
  auto bitmap =
      Bitmap::MakeFromTexture(width, height, texture, BitmapFormat::kRGBA,
                              BitmapAlphaType::kUnpremul_AlphaType);
  EXPECT_EQ(bitmap->Width(), width);
  EXPECT_EQ(bitmap->Height(), height);
  EXPECT_TRUE(bitmap->HasTexture());
  EXPECT_EQ(bitmap->TextureID(), texture);
  EXPECT_EQ(bitmap->Pixels(), nullptr);
  EXPECT_EQ(bitmap->Format(), BitmapFormat::kRGBA);
  EXPECT_EQ(bitmap->AlphaType(), BitmapAlphaType::kUnpremul_AlphaType);

  const uint32_t width2 = 10, height2 = 10;
  const uint64_t texture2 = 4321;
  auto bitmap2 =
      Bitmap::MakeFromTexture(width2, height2, texture2, BitmapFormat::kRGB565,
                              BitmapAlphaType::kPremul_AlphaType);
  EXPECT_EQ(bitmap2->Width(), width2);
  EXPECT_EQ(bitmap2->Height(), height2);
  EXPECT_TRUE(bitmap2->HasTexture());
  EXPECT_EQ(bitmap2->TextureID(), texture2);
  EXPECT_EQ(bitmap2->Pixels(), nullptr);
  EXPECT_EQ(bitmap2->Format(), BitmapFormat::kRGB565);
  EXPECT_EQ(bitmap2->AlphaType(), BitmapAlphaType::kPremul_AlphaType);
}

TEST_F(BitmapTest, DestructorCallsDeleter) {
  const uint32_t width = 3;
  const uint32_t height = 3;
  std::vector<uint8_t> pixels(width * height * 4, 0);
  auto called_count = GetDeleterCalledCountRef();
  {
    auto bitmap = Bitmap::MakeRGBA(width, height, pixels.data(),
                                   DeleterWithCount, nullptr);
  }
  // Destructor should have been called when bitmap went out of scope
  EXPECT_EQ(GetDeleterCalledCountRef(), called_count + 1);
}
