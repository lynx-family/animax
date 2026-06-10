// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/render/image_gl_factory.h"

#include <GL/osmesa.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <memory>

#include "skity/gpu/texture.hpp"
#include "skity/graphic/image.hpp"
#include "skity/io/data.hpp"
#include "skity/io/pixmap.hpp"
#include "src/base/gl/gl_include.h"
#include "src/render/real_context.h"
#include "src/render/surface_gl.h"
#include "src/render/texture_info_gl.h"

namespace lynx {
namespace animax {
namespace {

class ImageGLFactoryTest : public ::testing::Test {
 protected:
  void SetUp() override {
    osmesa_context_ = OSMesaCreateContextExt(OSMESA_RGBA, 0, 0, 0, nullptr);
    buffer_ = std::make_unique<uint8_t[]>(kWidth * kHeight * 4);
    ASSERT_NE(osmesa_context_, nullptr);
    ASSERT_TRUE(OSMesaMakeCurrent(osmesa_context_, buffer_.get(),
                                  GL_UNSIGNED_BYTE, kWidth, kHeight));
  }

  void TearDown() override {
    if (texture_id_ != 0) {
      glDeleteTextures(1, &texture_id_);
      texture_id_ = 0;
    }
    gpu_context_.reset();
    if (osmesa_context_) {
      OSMesaDestroyContext(osmesa_context_);
      osmesa_context_ = nullptr;
    }
  }

  std::shared_ptr<skity::Pixmap> MakePixmap(uint32_t width = kWidth,
                                            uint32_t height = kHeight) {
    const uint8_t pixels[] = {
        0xFF, 0x00, 0x00, 0xFF,  //
        0x00, 0xFF, 0x00, 0xFF,  //
        0x00, 0x00, 0xFF, 0xFF,  //
        0xFF, 0xFF, 0xFF, 0xFF,  //
    };
    auto data = skity::Data::MakeWithCopy(pixels, sizeof(pixels));
    return std::make_shared<skity::Pixmap>(data, width * 4, width, height,
                                           skity::AlphaType::kPremul_AlphaType,
                                           skity::ColorType::kRGBA);
  }

  RealContext MakeRealContext() {
    gpu_context_ = CreateGPUContext();
    return RealContext(gpu_context_.get());
  }

  TextureInfoGL MakeGLTextureInfo(uint32_t width = kWidth,
                                  uint32_t height = kHeight) {
    glGenTextures(1, &texture_id_);
    glBindTexture(GL_TEXTURE_2D, texture_id_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    return TextureInfoGL(texture_id_, width, height, GL_TEXTURE_2D);
  }

  static constexpr uint32_t kWidth = 2;
  static constexpr uint32_t kHeight = 2;

  OSMesaContext osmesa_context_ = nullptr;
  std::unique_ptr<uint8_t[]> buffer_;
  std::shared_ptr<skity::GPUContext> gpu_context_;
  GLuint texture_id_ = 0;
};

TEST_F(ImageGLFactoryTest, MakeTextureFromGLTextureReturnsNullWithoutContext) {
  TextureInfoGL texture_info(1, kWidth, kHeight, GL_TEXTURE_2D);

  auto texture = MakeTextureFromGLTexture(nullptr, texture_info);

  EXPECT_EQ(texture, nullptr);
}

TEST_F(ImageGLFactoryTest, MakeTextureFromGLTextureWrapsExternalTexture) {
  auto real_context = MakeRealContext();
  auto texture_info = MakeGLTextureInfo();

  auto texture = MakeTextureFromGLTexture(real_context.Get(), texture_info);

  ASSERT_NE(texture, nullptr);
  EXPECT_EQ(texture->Width(), kWidth);
  EXPECT_EQ(texture->Height(), kHeight);
}

TEST_F(ImageGLFactoryTest,
       CreateGLPixmapImageCreatesPixmapImageWithoutContext) {
  auto image = CreateGLPixmapImage(MakePixmap(), nullptr);

  ASSERT_NE(image, nullptr);
  EXPECT_EQ(image->GetImageType(), skity::ImageType::kPixmap);
  EXPECT_FALSE(image->IsTextureBackend());
  EXPECT_EQ(image->Width(), kWidth);
  EXPECT_EQ(image->Height(), kHeight);
  EXPECT_EQ(image->GetAlphaType(), skity::AlphaType::kPremul_AlphaType);
}

TEST_F(ImageGLFactoryTest, CreateGLPixmapImageCreatesTextureImageWithContext) {
  auto real_context = MakeRealContext();

  auto image = CreateGLPixmapImage(MakePixmap(), &real_context);

  ASSERT_NE(image, nullptr);
  EXPECT_EQ(image->GetImageType(), skity::ImageType::kTexture);
  EXPECT_TRUE(image->IsTextureBackend());
  EXPECT_EQ(image->Width(), kWidth);
  EXPECT_EQ(image->Height(), kHeight);
  EXPECT_EQ(image->GetAlphaType(), skity::AlphaType::kPremul_AlphaType);
}

TEST_F(ImageGLFactoryTest, CreateGLTextureImageRejectsInvalidInputs) {
  auto texture_info = MakeGLTextureInfo();
  auto real_context = MakeRealContext();

  EXPECT_EQ(CreateGLTextureImage(nullptr, &real_context), nullptr);
  EXPECT_EQ(CreateGLTextureImage(&texture_info, nullptr), nullptr);
}

TEST_F(ImageGLFactoryTest, CreateGLTextureImageCreatesHardwareImage) {
  auto real_context = MakeRealContext();
  auto texture_info = MakeGLTextureInfo();

  auto image = CreateGLTextureImage(&texture_info, &real_context);

  ASSERT_NE(image, nullptr);
  EXPECT_EQ(image->GetImageType(), skity::ImageType::kTexture);
  EXPECT_TRUE(image->IsTextureBackend());
  EXPECT_EQ(image->Width(), kWidth);
  EXPECT_EQ(image->Height(), kHeight);
  EXPECT_EQ(image->GetAlphaType(), skity::AlphaType::kPremul_AlphaType);
}

}  // namespace
}  // namespace animax
}  // namespace lynx
