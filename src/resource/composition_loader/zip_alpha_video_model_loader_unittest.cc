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
  auto task =
      ZipCompositionModelTask{.request = CompositionModelRequest{.scale = 1.f},
                              .response = CompositionModelResponse{
                                  .model_type = CompositionModelType::kLottie,
                                  .base_uri = base_uri_str}};
  EXPECT_NE(nullptr, loader);
  loader->Load(std::move(task),
               [](CompositionModelResponse res, LoaderError error) {
                 EXPECT_FALSE(!!error);
                 EXPECT_NE(nullptr, res.model);
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
               });
}
