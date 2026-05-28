// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/composition_loader/zip_json_model_loader.h"

#include <unistd.h>

#include <cstdio>
#include <fstream>
#include <string>
#include <string_view>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "include/resource/loader_error.h"
#include "include/resource/resource_task.h"
#include "src/resource/composition_loader/composition_task.h"
#include "src/resource/composition_loader/zip_composition_task.h"

using namespace lynx::animax;
using namespace testing;

static constexpr const char* lottie_json_str =
    R"({"v":"5.6.10","fr":29.9700012207031,"ip":0,"op":60.0000024438501,"w":1200,"h":675,"nm":"RepeaterMergePaths","ddd":0,"assets":[],"layers":[{"ddd":0,"ind":1,"ty":4,"nm":"MergePaths2","sr":1,"ks":{"o":{"a":0,"k":100,"ix":11},"r":{"a":0,"k":0,"ix":10},"p":{"a":0,"k":[546,341.5,0],"ix":2},"a":{"a":0,"k":[0,0,0],"ix":1},"s":{"a":0,"k":[100,100,100],"ix":6}},"ao":0,"shapes":[{"ty":"gr","it":[{"ty":"rc","d":1,"s":{"a":0,"k":[266.812,144.995],"ix":2},"p":{"a":0,"k":[0,0],"ix":3},"r":{"a":0,"k":0,"ix":4},"nm":"Rectangle Path 1","mn":"ADBE Vector Shape - Rect","hd":false},{"ty":"sr","sy":1,"d":1,"pt":{"a":1,"k":[{"i":{"x":[0.833],"y":[0.833]},"o":{"x":[0.167],"y":[0.167]},"t":0,"s":[6]},{"t":45.0000018328876,"s":[12]}],"ix":3},"p":{"a":0,"k":[-3,0],"ix":4},"r":{"a":0,"k":0,"ix":5},"ir":{"a":0,"k":55,"ix":6},"is":{"a":0,"k":28,"ix":8},"or":{"a":0,"k":108,"ix":7},"os":{"a":0,"k":22,"ix":9},"ix":2,"nm":"Polystar Path 1","mn":"ADBE Vector Shape - Star","hd":false},{"ty":"st","c":{"a":0,"k":[1,1,1,1],"ix":3},"o":{"a":0,"k":100,"ix":4},"w":{"a":0,"k":2,"ix":5},"lc":1,"lj":1,"ml":4,"bm":0,"nm":"Stroke 1","mn":"ADBE Vector Graphic - Stroke","hd":false},{"ty":"fl","c":{"a":0,"k":[1,0,0,1],"ix":4},"o":{"a":0,"k":100,"ix":5},"r":1,"bm":0,"nm":"Fill 1","mn":"ADBE Vector Graphic - Fill","hd":false},{"ty":"tr","p":{"a":0,"k":[93.906,178.094],"ix":2},"a":{"a":0,"k":[0,0],"ix":1},"s":{"a":0,"k":[100,100],"ix":3},"r":{"a":0,"k":0,"ix":6},"o":{"a":0,"k":100,"ix":7},"sk":{"a":0,"k":0,"ix":4},"sa":{"a":0,"k":0,"ix":5},"nm":"Transform"}],"nm":"Rectangle 1","np":4,"cix":2,"bm":0,"ix":1,"mn":"ADBE Vector Group","hd":false},{"ty":"mm","mm":4,"nm":"Merge Paths 1","mn":"ADBE Vector Filter - Merge","hd":false},{"ty":"st","c":{"a":0,"k":[1,1,1,1],"ix":3},"o":{"a":0,"k":100,"ix":4},"w":{"a":0,"k":2,"ix":5},"lc":1,"lj":1,"ml":4,"bm":0,"nm":"Stroke 1","mn":"ADBE Vector Graphic - Stroke","hd":false},{"ty":"fl","c":{"a":0,"k":[1,0,0,1],"ix":4},"o":{"a":0,"k":100,"ix":5},"r":1,"bm":0,"nm":"Fill 1","mn":"ADBE Vector Graphic - Fill","hd":false}],"ip":0,"op":60.0000024438501,"st":0,"bm":0},{"ddd":0,"ind":2,"ty":4,"nm":"MergePaths1","sr":1,"ks":{"o":{"a":0,"k":100,"ix":11},"r":{"a":0,"k":0,"ix":10},"p":{"a":0,"k":[410,343.5,0],"ix":2},"a":{"a":0,"k":[0,0,0],"ix":1},"s":{"a":0,"k":[100,100,100],"ix":6}},"ao":0,"shapes":[{"ty":"gr","it":[{"d":1,"ty":"el","s":{"a":0,"k":[272.016,164.961],"ix":2},"p":{"a":0,"k":[0,0],"ix":3},"nm":"Ellipse Path 1","mn":"ADBE Vector Shape - Ellipse","hd":false},{"d":1,"ty":"el","s":{"a":1,"k":[{"i":{"x":[0.833,0.833],"y":[0.833,0.833]},"o":{"x":[0.167,0.167],"y":[0.167,0.167]},"t":0,"s":[100,100]},{"t":45.0000018328876,"s":[202,202]}],"ix":2},"p":{"a":0,"k":[0,0],"ix":3},"nm":"Ellipse Path 2","mn":"ADBE Vector Shape - Ellipse","hd":false},{"ty":"st","c":{"a":0,"k":[1,1,1,1],"ix":3},"o":{"a":0,"k":100,"ix":4},"w":{"a":0,"k":2,"ix":5},"lc":1,"lj":1,"ml":4,"bm":0,"nm":"Stroke 1","mn":"ADBE Vector Graphic - Stroke","hd":false},{"ty":"fl","c":{"a":0,"k":[1,0,0,1],"ix":4},"o":{"a":0,"k":100,"ix":5},"r":1,"bm":0,"nm":"Fill 1","mn":"ADBE Vector Graphic - Fill","hd":false},{"ty":"tr","p":{"a":0,"k":[-177.688,170.129],"ix":2},"a":{"a":0,"k":[0,0],"ix":1},"s":{"a":0,"k":[100,100],"ix":3},"r":{"a":0,"k":0,"ix":6},"o":{"a":0,"k":100,"ix":7},"sk":{"a":0,"k":0,"ix":4},"sa":{"a":0,"k":0,"ix":5},"nm":"Transform"}],"nm":"Ellipse 1","np":4,"cix":2,"bm":0,"ix":1,"mn":"ADBE Vector Group","hd":false},{"ty":"mm","mm":3,"nm":"Merge Paths 1","mn":"ADBE Vector Filter - Merge","hd":false},{"ty":"st","c":{"a":0,"k":[1,1,1,1],"ix":3},"o":{"a":0,"k":100,"ix":4},"w":{"a":0,"k":2,"ix":5},"lc":1,"lj":1,"ml":4,"bm":0,"nm":"Stroke 1","mn":"ADBE Vector Graphic - Stroke","hd":false},{"ty":"fl","c":{"a":0,"k":[1,0,0,1],"ix":4},"o":{"a":0,"k":100,"ix":5},"r":1,"bm":0,"nm":"Fill 1","mn":"ADBE Vector Graphic - Fill","hd":false}],"ip":0,"op":60.0000024438501,"st":0,"bm":0},{"ddd":0,"ind":3,"ty":4,"nm":"Repeater","sr":1,"ks":{"o":{"a":0,"k":100,"ix":11},"r":{"a":0,"k":0,"ix":10},"p":{"a":0,"k":[562,311.5,0],"ix":2},"a":{"a":0,"k":[0,0,0],"ix":1},"s":{"a":0,"k":[100,100,100],"ix":6}},"ao":0,"shapes":[{"ty":"gr","it":[{"ty":"sr","sy":2,"d":1,"pt":{"a":0,"k":5,"ix":3},"p":{"a":0,"k":[0,0],"ix":4},"r":{"a":0,"k":116.159,"ix":5},"or":{"a":0,"k":111.429,"ix":7},"os":{"a":0,"k":0,"ix":9},"ix":1,"nm":"Polystar Path 1","mn":"ADBE Vector Shape - Star","hd":false},{"ty":"st","c":{"a":0,"k":[1,1,1,1],"ix":3},"o":{"a":0,"k":100,"ix":4},"w":{"a":0,"k":2,"ix":5},"lc":1,"lj":1,"ml":4,"bm":0,"nm":"Stroke 1","mn":"ADBE Vector Graphic - Stroke","hd":false},{"ty":"fl","c":{"a":0,"k":[1,0,0,1],"ix":4},"o":{"a":0,"k":100,"ix":5},"r":1,"bm":0,"nm":"Fill 1","mn":"ADBE Vector Graphic - Fill","hd":false},{"ty":"rp","c":{"a":1,"k":[{"i":{"x":[0.833],"y":[0.833]},"o":{"x":[0.167],"y":[0.167]},"t":0,"s":[1]},{"t":45.0000018328876,"s":[5]}],"ix":1},"o":{"a":1,"k":[{"i":{"x":[0.833],"y":[0.833]},"o":{"x":[0.167],"y":[0.167]},"t":0,"s":[0]},{"t":45.0000018328876,"s":[5]}],"ix":2},"m":1,"ix":4,"tr":{"ty":"tr","p":{"a":0,"k":[100,0],"ix":2},"a":{"a":0,"k":[0,0],"ix":1},"s":{"a":0,"k":[100,100],"ix":3},"r":{"a":1,"k":[{"i":{"x":[0.833],"y":[0.833]},"o":{"x":[0.167],"y":[0.167]},"t":0,"s":[73]},{"t":45.0000018328876,"s":[540]}],"ix":4},"so":{"a":0,"k":100,"ix":5},"eo":{"a":0,"k":100,"ix":6},"nm":"Transform"},"nm":"Repeater 1","mn":"ADBE Vector Filter - Repeater","hd":false},{"ty":"tr","p":{"a":0,"k":[-390.688,-167.562],"ix":2},"a":{"a":0,"k":[0,0],"ix":1},"s":{"a":0,"k":[100,100],"ix":3},"r":{"a":0,"k":0,"ix":6},"o":{"a":0,"k":100,"ix":7},"sk":{"a":0,"k":0,"ix":4},"sa":{"a":0,"k":0,"ix":5},"nm":"Transform"}],"nm":"Polystar 1","np":4,"cix":2,"bm":0,"ix":1,"mn":"ADBE Vector Group","hd":false}],"ip":0,"op":60.0000024438501,"st":0,"bm":0}],"markers":[]})";

class MockResourceLoader : public Loader<ResourceRequest, ResourceResponse> {
 public:
  ~MockResourceLoader() override = default;
  MOCK_METHOD(void, Load, (ResourceRequest request, CallbackType callback),
              (override));
};

class ZipJSONModelLoaderTest : public ::testing::Test {
 protected:
  std::string unzip_dir_path;
  bool is_unzip_dir_setup = false;

  void SetUp() override {
    char temp_dir_template[] =
        "/tmp/animax_test_zip_json_model_loader_unittest.XXXXXX";
    auto* created_dir = mkdtemp(temp_dir_template);
    ASSERT_NE(nullptr, created_dir);
    unzip_dir_path = created_dir;
  }

  void TearDown() override {
    const auto config_file_path = unzip_dir_path + "/config.json";
    std::remove(config_file_path.c_str());
    rmdir(unzip_dir_path.c_str());
  }

  void SetupUnzipDir() {
    if (is_unzip_dir_setup) {
      return;
    }

    // Create a JSON file in unzip dir
    const auto config_file_path = unzip_dir_path + "/config.json";
    std::ofstream out(config_file_path.c_str());
    out << lottie_json_str;
    out.close();
    is_unzip_dir_setup = true;
  }

  std::string UnzipDirPath() { return unzip_dir_path; }
};

TEST_F(ZipJSONModelLoaderTest, LoadFromUnzipDirWithConfigJSON) {
  auto mock_loader = std::make_shared<MockResourceLoader>();
  auto loader = ZipJSONModelLoader::MakeLoader(mock_loader);
  EXPECT_CALL(*mock_loader, Load(_, _))
      .WillOnce([](ResourceRequest request, auto callback) mutable {
        EXPECT_EQ(request.type, ResourceRequestType::kLoadRawData);
        auto raw_data_payload = MakeRawDataResourcePayload(RawData::MakeRawData(
            lottie_json_str, std::string_view{lottie_json_str}.length(),
            nullptr, nullptr));
        callback(ResourceResponse{.payload = std::move(raw_data_payload)}, {});
      });
  // setup the unzip dir
  SetupUnzipDir();
  auto task =
      ZipCompositionModelTask{.request = CompositionModelRequest{.scale = 1.f},
                              .response = CompositionModelResponse{
                                  .model_type = CompositionModelType::kLottie,
                                  .base_uri = UnzipDirPath()}};
  EXPECT_NE(nullptr, loader);
  loader->Load(std::move(task),
               [](CompositionModelResponse res, LoaderError error) {
                 EXPECT_FALSE(!!error);
                 EXPECT_NE(nullptr, res.model);
               });
}

TEST_F(ZipJSONModelLoaderTest, LoadFromUnzipDirWithoutJSONFile) {
  auto mock_loader = std::make_shared<MockResourceLoader>();
  auto loader = ZipJSONModelLoader::MakeLoader(mock_loader);
  // Resource loader should not be used, since there is no JSON file in the
  // unzip folder.
  EXPECT_CALL(*mock_loader, Load(_, _)).Times(0);
  // do not setup the unzip dir, there is no JSON file in the unzip dir, which
  // will leads to error.
  auto task =
      ZipCompositionModelTask{.request = CompositionModelRequest{.scale = 1.f},
                              .response = CompositionModelResponse{
                                  .model_type = CompositionModelType::kLottie,
                                  .base_uri = UnzipDirPath()}};
  EXPECT_NE(nullptr, loader);
  loader->Load(std::move(task),
               [](CompositionModelResponse res, LoaderError error) {
                 EXPECT_TRUE(!!error);
                 EXPECT_EQ(nullptr, res.model);
               });
}
