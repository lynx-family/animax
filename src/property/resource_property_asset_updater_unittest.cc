// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/property/resource_property_asset_updater.h"

#include "gtest/gtest.h"
#include "src/property/property_update_context.h"
#include "src/resource/asset/audio_asset.h"
#include "src/resource/asset/font_asset.h"
#include "src/resource/asset/image_asset.h"
#include "src/resource/asset/video_asset.h"
#include "src/resource/asset/video_asset_model.h"

using namespace lynx::animax;

class MockVideoAsset : public VideoAsset {
 public:
  using VideoAsset::VideoAsset;  // Inherit constructors
  static std::shared_ptr<VideoAsset> Make(VideoAssetModel model) {
    return std::shared_ptr<VideoAsset>(new MockVideoAsset(std::move(model)));
  }
  bool PrepareFrameData(const std::string& video_path) override {
    is_valid_ = true;
    return true;
  }
  int32_t GetFrameCount() const override { return 1; }
};

class MockAudioAsset : public AudioAsset {
 public:
  using AudioAsset::AudioAsset;
  static std::shared_ptr<AudioAsset> Make(AudioAssetModel model) {
    return std::shared_ptr<AudioAsset>(new MockAudioAsset(std::move(model)));
  }
  void LoadLocal(const std::string& file_path) override { is_valid_ = true; }
};

TEST(ResourcePropertyAssetUpdatorTest, NullParamsTest) {
  auto request = ResourceUpdateRequest::Make(
      ResourcePropertyType::kUnknown, "font1",
      std::make_unique<AnimaXValueParam>("Arial"), nullptr);
  PropertyUpdateContext context(std::move(request));
  ResourcePropertyAssetUpdator updator(context);

  // Test FontAsset
  {
    FontAssetModel fontModel;
    fontModel.family = "Arial";
    fontModel.style = "Regular";
    fontModel.path = "/fonts/Arial.ttf";
    fontModel.ascent = 71.5f;
    FontAsset fontAsset(fontModel);

    fontAsset.AcceptVisitor(updator);

    EXPECT_FALSE(context.GetResponse().IsSuccess());
    EXPECT_EQ(fontAsset.Model().family, "Arial");
    EXPECT_EQ(fontAsset.Model().style, "Regular");
    EXPECT_EQ(fontAsset.Model().path, "/fonts/Arial.ttf");
    EXPECT_FLOAT_EQ(fontAsset.Model().ascent, 71.5f);
  }

  // Test ImageAsset
  {
    ImageAssetModel imageModel;
    imageModel.dir_name = "/images";
    imageModel.file_name = "test.png";
    imageModel.width = 100;
    imageModel.height = 200;
    ImageAsset imageAsset(imageModel);

    imageAsset.AcceptVisitor(updator);

    EXPECT_FALSE(context.GetResponse().IsSuccess());
    EXPECT_EQ(imageAsset.Model().dir_name, "/images");
    EXPECT_EQ(imageAsset.Model().file_name, "test.png");
    EXPECT_EQ(imageAsset.Model().width, 100);
    EXPECT_EQ(imageAsset.Model().height, 200);
  }

  // Test MockVideoAsset
  {
    std::array<int32_t, 4> rgb_frame = {0, 0, 100, 100};
    std::array<int32_t, 4> a_frame = {0, 0, 100, 100};
    auto mockVideoAsset = MockVideoAsset::Make(
        VideoAssetModel{rgb_frame, a_frame, "video1", "/videos", "test.mp4"});

    mockVideoAsset->AcceptVisitor(updator);

    EXPECT_FALSE(context.GetResponse().IsSuccess());
    EXPECT_EQ(mockVideoAsset->Model().file_name, "test.mp4");
    EXPECT_EQ(mockVideoAsset->Model().dir_name, "/videos");
    EXPECT_EQ(mockVideoAsset->Model().id, "video1");
    EXPECT_EQ(mockVideoAsset->GetFrameCount(), 1);
    EXPECT_TRUE(mockVideoAsset->PrepareFrameData("/videos/test.mp4"));
  }

  // Test AudioAsset
  {
    auto model = AudioAssetModel{"audio1", "/audios", "test.mp3"};
    auto asset = MockAudioAsset::Make(std::move(model));
    asset->LoadLocal("some file");
    asset->AcceptVisitor(updator);

    EXPECT_FALSE(context.GetResponse().IsSuccess());
    EXPECT_TRUE(asset->IsValid());
    EXPECT_EQ(asset->Model().file_name, "test.mp3");
    EXPECT_EQ(asset->Model().dir_name, "/audios");
    EXPECT_EQ(asset->Model().id, "audio1");
  }
}

TEST(ResourcePropertyAssetUpdatorTest, FontAssetUpdate) {
  FontAssetModel initial_model;
  initial_model.family = "Arial";
  initial_model.name = "ArialMT";
  initial_model.style = "Regular";
  initial_model.ascent = 71.5f;
  initial_model.path = "/fonts/Arial.ttf";
  initial_model.path_origin = FontPathOrigin::kLocal;

  FontAsset asset(initial_model);

  // Test each possible ResourcePropertyType for FontAsset
  {
    std::string new_family = "Helvetica";
    auto request = ResourceUpdateRequest::Make(
        ResourcePropertyType::kFontFamily, "font1",
        std::make_unique<AnimaXValueParam>(new_family), nullptr);
    PropertyUpdateContext context(std::move(request));

    ResourcePropertyAssetUpdator updator(context);
    asset.AcceptVisitor(updator);

    EXPECT_TRUE(context.GetResponse().IsSuccess());
    EXPECT_EQ(asset.Model().family, "Helvetica");
  }

  {
    std::string new_style = "Bold";
    auto request = ResourceUpdateRequest::Make(
        ResourcePropertyType::kFontStyle, "font1",
        std::make_unique<AnimaXValueParam>(new_style), nullptr);
    PropertyUpdateContext context(std::move(request));

    ResourcePropertyAssetUpdator updator(context);
    asset.AcceptVisitor(updator);

    EXPECT_TRUE(context.GetResponse().IsSuccess());
    EXPECT_EQ(asset.Model().style, "Bold");
  }

  {
    std::string new_path = "/fonts/Helvetica.ttf";
    auto request = ResourceUpdateRequest::Make(
        ResourcePropertyType::kFontPath, "font1",
        std::make_unique<AnimaXValueParam>(new_path), nullptr);
    PropertyUpdateContext context(std::move(request));

    ResourcePropertyAssetUpdator updator(context);
    asset.AcceptVisitor(updator);

    EXPECT_TRUE(context.GetResponse().IsSuccess());
    EXPECT_EQ(asset.Model().path, "/fonts/Helvetica.ttf");
  }

  {
    double new_ascent = 75.0f;
    auto request = ResourceUpdateRequest::Make(
        ResourcePropertyType::kFontAscent, "font1",
        std::make_unique<AnimaXValueParam>(new_ascent), nullptr);
    PropertyUpdateContext context(std::move(request));

    ResourcePropertyAssetUpdator updator(context);
    asset.AcceptVisitor(updator);

    EXPECT_TRUE(context.GetResponse().IsSuccess());
    EXPECT_FLOAT_EQ(asset.Model().ascent, 75.0f);
  }

  // Test an unsupported property type for FontAsset
  {
    int32_t dummy_value = 100;
    auto request = ResourceUpdateRequest::Make(
        ResourcePropertyType::kImageWidth, "font1",
        std::make_unique<AnimaXValueParam>(dummy_value), nullptr);
    PropertyUpdateContext context(std::move(request));

    ResourcePropertyAssetUpdator updator(context);
    asset.AcceptVisitor(updator);

    EXPECT_FALSE(context.GetResponse().IsSuccess());
    EXPECT_EQ(context.GetResponse().GetErrorType(),
              PropertyUpdateResult::kPropertyNotImplemented);
  }
}

TEST(ResourcePropertyAssetUpdatorTest, ImageAssetUpdate) {
  ImageAssetModel initial_model;
  initial_model.dir_name = "/images";
  initial_model.file_name = "test.png";
  initial_model.width = 100;
  initial_model.height = 200;

  ImageAsset asset(initial_model);

  // Test each possible ResourcePropertyType for ImageAsset
  {
    std::string new_dir_name = "/new_images";
    auto request = ResourceUpdateRequest::Make(
        ResourcePropertyType::kImageDirName, "image1",
        std::make_unique<AnimaXValueParam>(new_dir_name), nullptr);
    PropertyUpdateContext context(std::move(request));

    ResourcePropertyAssetUpdator updator(context);
    asset.AcceptVisitor(updator);

    EXPECT_TRUE(context.GetResponse().IsSuccess());
    EXPECT_EQ(asset.Model().dir_name, "/new_images");
  }

  {
    std::string new_file_name = "new_test.png";
    auto request = ResourceUpdateRequest::Make(
        ResourcePropertyType::kImageFileName, "image1",
        std::make_unique<AnimaXValueParam>(new_file_name), nullptr);
    PropertyUpdateContext context(std::move(request));

    ResourcePropertyAssetUpdator updator(context);
    asset.AcceptVisitor(updator);

    EXPECT_TRUE(context.GetResponse().IsSuccess());
    EXPECT_EQ(asset.Model().file_name, "new_test.png");
  }

  {
    double new_width = 150;
    auto request = ResourceUpdateRequest::Make(
        ResourcePropertyType::kImageWidth, "image1",
        std::make_unique<AnimaXValueParam>(new_width), nullptr);
    PropertyUpdateContext context(std::move(request));

    ResourcePropertyAssetUpdator updator(context);
    asset.AcceptVisitor(updator);

    EXPECT_TRUE(context.GetResponse().IsSuccess());
    EXPECT_EQ(asset.Model().width, 150);
  }

  {
    double new_height = 250;
    auto request = ResourceUpdateRequest::Make(
        ResourcePropertyType::kImageHeight, "image1",
        std::make_unique<AnimaXValueParam>(new_height), nullptr);
    PropertyUpdateContext context(std::move(request));

    ResourcePropertyAssetUpdator updator(context);
    asset.AcceptVisitor(updator);

    EXPECT_TRUE(context.GetResponse().IsSuccess());
    EXPECT_EQ(asset.Model().height, 250);
  }

  // Test an unsupported property type for ImageAsset
  {
    std::string dummy_value = "Unsupported";
    auto request = ResourceUpdateRequest::Make(
        ResourcePropertyType::kFontFamily, "image1",
        std::make_unique<AnimaXValueParam>(dummy_value), nullptr);
    PropertyUpdateContext context(std::move(request));

    ResourcePropertyAssetUpdator updator(context);
    asset.AcceptVisitor(updator);

    EXPECT_FALSE(context.GetResponse().IsSuccess());
    EXPECT_EQ(context.GetResponse().GetErrorType(),
              PropertyUpdateResult::kPropertyNotImplemented);
  }
}

TEST(ResourcePropertyAssetUpdatorTest, VideoAssetUpdate) {
  std::array<int32_t, 4> initial_rgb_frame = {0, 0, 100, 100};
  std::array<int32_t, 4> initial_a_frame = {0, 0, 100, 100};
  auto asset = MockVideoAsset::Make(VideoAssetModel{
      initial_rgb_frame, initial_a_frame, "video1", "/videos", "test.mp4"});

  // Test each possible ResourcePropertyType for VideoAsset
  {
    std::string new_dir_name = "/new_videos";
    auto request = ResourceUpdateRequest::Make(
        ResourcePropertyType::kVideoDirName, "video1",
        std::make_unique<AnimaXValueParam>(new_dir_name), nullptr);
    PropertyUpdateContext context(std::move(request));

    ResourcePropertyAssetUpdator updator(context);
    asset->AcceptVisitor(updator);

    EXPECT_TRUE(context.GetResponse().IsSuccess());
    EXPECT_EQ(asset->Model().dir_name, "/new_videos");
  }

  {
    std::string new_file_name = "new_test.mp4";
    auto request = ResourceUpdateRequest::Make(
        ResourcePropertyType::kVideoFileName, "video1",
        std::make_unique<AnimaXValueParam>(new_file_name), nullptr);
    PropertyUpdateContext context(std::move(request));

    ResourcePropertyAssetUpdator updator(context);
    asset->AcceptVisitor(updator);

    EXPECT_TRUE(context.GetResponse().IsSuccess());
    EXPECT_EQ(asset->Model().file_name, "new_test.mp4");
  }

  // Test an unsupported property type for VideoAsset
  {
    std::string dummy_value = "Unsupported";
    auto request = ResourceUpdateRequest::Make(
        ResourcePropertyType::kFontFamily, "video1",
        std::make_unique<AnimaXValueParam>(dummy_value), nullptr);
    PropertyUpdateContext context(std::move(request));

    ResourcePropertyAssetUpdator updator(context);
    asset->AcceptVisitor(updator);

    EXPECT_FALSE(context.GetResponse().IsSuccess());
    EXPECT_EQ(context.GetResponse().GetErrorType(),
              PropertyUpdateResult::kPropertyNotImplemented);
  }

  // Verify that other properties of the VideoAsset remain unchanged
  EXPECT_EQ(asset->Model().id, "video1");
  EXPECT_EQ(asset->GetFrameCount(), 1);
  EXPECT_TRUE(asset->PrepareFrameData("/new_videos/new_test.mp4"));
}

TEST(ResourcePropertyAssetUpdatorTest, AudioAssetUpdate) {
  auto asset = MockAudioAsset::Make({"audio1", "/audios", "test.mp3"});

  // Test each possible ResourcePropertyType for VideoAsset
  {
    std::string new_dir_name = "/new_audios";
    auto request = ResourceUpdateRequest::Make(
        ResourcePropertyType::kAudioDirName, "audio1",
        std::make_unique<AnimaXValueParam>(new_dir_name), nullptr);
    PropertyUpdateContext context(std::move(request));

    ResourcePropertyAssetUpdator updator(context);
    asset->AcceptVisitor(updator);

    EXPECT_TRUE(context.GetResponse().IsSuccess());
    EXPECT_EQ(asset->Model().dir_name, "/new_audios");
  }

  {
    std::string new_file_name = "new_test.mp3";
    auto request = ResourceUpdateRequest::Make(
        ResourcePropertyType::kAudioFileName, "audio1",
        std::make_unique<AnimaXValueParam>(new_file_name), nullptr);
    PropertyUpdateContext context(std::move(request));

    ResourcePropertyAssetUpdator updator(context);
    asset->AcceptVisitor(updator);

    EXPECT_TRUE(context.GetResponse().IsSuccess());
    EXPECT_EQ(asset->Model().file_name, "new_test.mp3");
  }

  // Test an unsupported property type for VideoAsset
  {
    std::string dummy_value = "Unsupported";
    auto request = ResourceUpdateRequest::Make(
        ResourcePropertyType::kFontFamily, "audio1",
        std::make_unique<AnimaXValueParam>(dummy_value), nullptr);
    PropertyUpdateContext context(std::move(request));

    ResourcePropertyAssetUpdator updator(context);
    asset->AcceptVisitor(updator);

    EXPECT_FALSE(context.GetResponse().IsSuccess());
    EXPECT_EQ(context.GetResponse().GetErrorType(),
              PropertyUpdateResult::kPropertyNotImplemented);
  }

  // Verify that other properties of the VideoAsset remain unchanged
  EXPECT_EQ(asset->Model().id, "audio1");
}
