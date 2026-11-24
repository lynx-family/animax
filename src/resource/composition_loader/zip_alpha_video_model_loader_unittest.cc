// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/composition_loader/zip_alpha_video_model_loader.h"

#include <string_view>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "include/resource/loader_error.h"
#include "include/resource/resource_task.h"
#include "include/resource/uri_info.h"
#include "src/model/composition_model.h"
#include "src/parser/alpha_player_asset/alpha_player_asset_parser.h"
#include "src/resource/composition_loader/composition_task.h"
#include "src/resource/composition_loader/zip_composition_task.h"
#include "src/resource/uri/uri_util.h"

using namespace lynx::animax;
using namespace testing;

static constexpr const char* config_json =
    R"({"portrait":{"videoW":1136,"f":226,"h":1624,"align":2,"aFrame":[0,0,375,812],"rgbFrame":[375,0,750,1624],"w":750,"v":1,"path":"output.mp4","videoH":1632}})";

static constexpr const char* corrupt_config_json =
    R"({"portrait":{"videoW":1136,"f":226,"h":1624,"align":2,"aFrame":[0,0,375,812],"rgbFrame":[375,0,750,1624],"w":750,"v":1,"path":"output.mp4","videoH)";

static constexpr const char* base_uri_str = "/temp/";

class MockResourceLoader : public Loader<ResourceRequest, ResourceResponse> {
 public:
  ~MockResourceLoader() override = default;
  MOCK_METHOD(void, Load, (ResourceRequest request, CallbackType callback),
              (override));
};

static void LoadModelOnlyConfig(
    const char* config, ZipAlphaVideoModelLoader::CallbackType callback) {
  auto mock_loader = std::make_shared<StrictMock<MockResourceLoader>>();
  EXPECT_CALL(*mock_loader, Load(_, _))
      .WillOnce([config](ResourceRequest request, auto callback) mutable {
        EXPECT_EQ(ResourceRequestType::kLoadRawData, request.type);
        EXPECT_EQ(ConcatFilePaths(base_uri_str, "config.json"),
                  request.uri_info.uri);
        auto payload = MakeRawDataResourcePayload(RawData::MakeRawData(
            config, std::string_view{config}.length(), nullptr, nullptr));
        callback(ResourceResponse{.payload = std::move(payload)}, {});
      });
  auto loader = ZipAlphaVideoModelLoader::MakeLoader(mock_loader);
  auto task = ZipCompositionModelTask{
      .request = CompositionModelRequest{.model_only = true},
      .response = CompositionModelResponse{
          .model_type = CompositionModelType::kAlphaVideo,
          .base_uri = base_uri_str,
          .main_uri = "file:///animation.zip"}};
  bool completed = false;
  loader->Load(std::move(task),
               [&completed, &callback](CompositionModelResponse response,
                                       LoaderError error) {
                 completed = true;
                 callback(std::move(response), std::move(error));
               });
  EXPECT_TRUE(completed);
}

TEST(ZipAlphaVideoModelLoaderTest, LoadFromJSONString) {
  auto mock_loader = std::make_shared<MockResourceLoader>();
  auto loader = ZipAlphaVideoModelLoader::MakeLoader(mock_loader);
  EXPECT_CALL(*mock_loader, Load(_, _))
      .WillOnce([](ResourceRequest request, auto callback) mutable {
        EXPECT_EQ(request.type, ResourceRequestType::kLoadRawData);
        auto raw_data_payload = MakeRawDataResourcePayload(RawData::MakeRawData(
            config_json, std::string_view{config_json}.length(), nullptr,
            nullptr));
        callback(ResourceResponse{.payload = std::move(raw_data_payload)}, {});
      });
  auto task = ZipCompositionModelTask{
      .request = CompositionModelRequest{.scale = 1.f, .enable_audio = true},
      .response =
          CompositionModelResponse{.model_type = CompositionModelType::kLottie,
                                   .base_uri = base_uri_str}};
  EXPECT_NE(nullptr, loader);
  loader->Load(std::move(task),
               [](CompositionModelResponse res, LoaderError error) {
                 EXPECT_FALSE(!!error);
                 ASSERT_NE(nullptr, res.model);
                 EXPECT_EQ(nullptr, res.alpha_player_config);
                 EXPECT_EQ(1u, res.model->GetVideos().count("0"));
                 EXPECT_EQ(2u, res.model->GetLayers().size());
               });
}

TEST(ZipAlphaVideoModelLoaderTest, LoadFromCorruptedData) {
  auto mock_loader = std::make_shared<MockResourceLoader>();
  EXPECT_CALL(*mock_loader, Load(_, _))
      .WillOnce([](ResourceRequest request, auto callback) mutable {
        EXPECT_EQ(request.type, ResourceRequestType::kLoadRawData);
        auto raw_data_payload = MakeRawDataResourcePayload(RawData::MakeRawData(
            corrupt_config_json, std::string_view{corrupt_config_json}.length(),
            nullptr, nullptr));
        callback(ResourceResponse{.payload = std::move(raw_data_payload)}, {});
      });
  auto loader = ZipAlphaVideoModelLoader::MakeLoader(mock_loader);
  auto task =
      ZipCompositionModelTask{.request = CompositionModelRequest{.scale = 1.f},
                              .response = CompositionModelResponse{
                                  .model_type = CompositionModelType::kLottie,
                                  .base_uri = base_uri_str}};
  EXPECT_NE(nullptr, loader);
  loader->Load(std::move(task),
               [](CompositionModelResponse res, LoaderError error) {
                 EXPECT_TRUE(!!error);
                 EXPECT_EQ(nullptr, res.model);
                 EXPECT_EQ(nullptr, res.alpha_player_config);
               });
}

TEST(ZipAlphaVideoModelLoaderTest, ModelOnlyReturnsConfigWithoutModel) {
  LoadModelOnlyConfig(
      config_json, [](CompositionModelResponse response, LoaderError error) {
        EXPECT_FALSE(!!error);
        EXPECT_EQ(nullptr, response.model);
        ASSERT_NE(nullptr, response.alpha_player_config);
        EXPECT_EQ(CompositionModelType::kAlphaVideo, response.model_type);
        EXPECT_EQ(base_uri_str, response.base_uri);
        EXPECT_EQ("file:///animation.zip", response.main_uri);
        EXPECT_EQ(ConcatFilePaths(base_uri_str, "output.mp4"),
                  response.alpha_player_config->video_path);
        EXPECT_THAT(response.alpha_player_config->rgb_frame,
                    ElementsAre(375, 0, 750, 1624));
        EXPECT_THAT(response.alpha_player_config->alpha_frame,
                    ElementsAre(0, 0, 375, 812));
      });
}

TEST(ZipAlphaVideoModelLoaderTest,
     ModelOnlyPreservesMissingCropForVideoRecovery) {
  const char* configs[] = {
      R"({"portrait":{"path":"missing.mp4"}})",
      R"({"portrait":{"path":"missing.mp4","rgbFrame":[0,0,0,0],"aFrame":[0,0,0,0]}})",
  };
  for (const auto* config : configs) {
    SCOPED_TRACE(config);
    LoadModelOnlyConfig(
        config, [](CompositionModelResponse response, LoaderError error) {
          EXPECT_FALSE(!!error);
          EXPECT_EQ(nullptr, response.model);
          ASSERT_NE(nullptr, response.alpha_player_config);
          EXPECT_EQ(ConcatFilePaths(base_uri_str, "missing.mp4"),
                    response.alpha_player_config->video_path);
          EXPECT_THAT(response.alpha_player_config->rgb_frame,
                      ElementsAre(0, 0, 0, 0));
          EXPECT_THAT(response.alpha_player_config->alpha_frame,
                      ElementsAre(0, 0, 0, 0));
        });
  }
}

TEST(ZipAlphaVideoModelLoaderTest, ModelOnlyRejectsInvalidConfig) {
  const char* configs[] = {
      corrupt_config_json,
      R"([])",
      R"({})",
      R"({"portrait":null})",
      R"({"portrait":{}})",
      R"({"portrait":{"path":17}})",
      R"({"portrait":{"path":""}})",
      R"({"portrait":{"path":"video.mp4","rgbFrame":{}}})",
      R"({"portrait":{"path":"video.mp4","rgbFrame":[0,0,1]}})",
      R"({"portrait":{"path":"video.mp4","rgbFrame":[0,0,1,1,0]}})",
      R"({"portrait":{"path":"video.mp4","rgbFrame":[0,0,1.5,1]}})",
      R"({"portrait":{"path":"video.mp4","rgbFrame":[0,0,true,1]}})",
      R"({"portrait":{"path":"video.mp4","rgbFrame":[0,0,2147483648,1]}})",
      R"({"portrait":{"path":"video.mp4","rgbFrame":[-1,0,1,1]}})",
      R"({"portrait":{"path":"video.mp4","rgbFrame":[0,0,0,1]}})",
      R"({"portrait":{"path":"video.mp4","rgbFrame":[0,0,1,1]}})",
      R"({"portrait":{"path":"video.mp4","rgbFrame":[0,0,1,1],"aFrame":[0,0,0,0]}})",
      R"({"portrait":{"path":"video.mp4","aFrame":[0,0,1,1]}})",
      R"({"portrait":{"path":"video.mp4","aFrame":"invalid"}})",
  };
  for (const auto* config : configs) {
    SCOPED_TRACE(config);
    LoadModelOnlyConfig(
        config, [](CompositionModelResponse response, LoaderError error) {
          EXPECT_EQ(kCompositionParserError, error.code);
          EXPECT_EQ(nullptr, response.model);
          EXPECT_EQ(nullptr, response.alpha_player_config);
        });
  }
}
