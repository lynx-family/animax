// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>

#include "base/include/fml/message_loop.h"
#include "base/include/fml/thread.h"
#include "base/include/lynx_actor.h"
#include "base/include/no_destructor.h"
#include "include/resource/loader_error.h"
#include "include/resource/resource_task.h"
#include "include/resource/unzip_task.h"
#include "include/resource/uri_info.h"
#include "src/base/thread/task_runner.h"
#include "src/parser/composition_parser.h"
#include "src/player/animax_composition_loader.h"
#include "src/resource/composition_loader/composition_asset_task.h"
#include "testing/gtest_expectation.h"

using namespace lynx::animax;
using namespace ::testing;

namespace {

class MockResourceLoader : public Loader<ResourceRequest, ResourceResponse> {
 public:
  ~MockResourceLoader() override = default;
  MOCK_METHOD(void, Load, (ResourceRequest, CallbackType), (override));
};

class MockUnzipLoader : public Loader<UnzipRequest, UnzipResponse> {
 public:
  ~MockUnzipLoader() override = default;
  MOCK_METHOD(void, Load, (UnzipRequest, CallbackType), (override));
};

static constexpr std::array<uint8_t, 4> kBitmap =
    std::array<uint8_t, 4>{1, 1, 1, 1};
static constexpr std::array<uint8_t, 1> kRawData = std::array<uint8_t, 1>{1};
static constexpr const char* kVideoFilePath = "/tmp/video.mp4";
static constexpr const char* kFailImageAssetURI =
    "https://example.com/fail_img_1.44fdfe24.png";
static constexpr const char* kFailImageID = "image_1";
static constexpr const char* kErrorMessage = "mock error";

static constexpr const char* kCompositionModelJSON =
    R"({"v":"5.6.1","fr":60,"ip":0,"op":181,"w":1125,"h":1657,"nm":"lottie","ddd":0,"assets":[{"id":"image_0","w":1019,"h":1019,"u":"https://example.com/","p":"img_0.c65d636c.png","e":0},{"id":"image_1","w":1092,"h":1072,"u":"https://example.com/","p":"fail_img_1.44fdfe24.png","e":0},{"id":"image_2","w":1010,"h":1127,"u":"https://example.com/","p":"img_2.a424ceaa.png","e":0},{"id":"image_3","w":994,"h":1176,"u":"https://example.com/","p":"img_3.6e276d42.png","e":0},{"id":"comp_0","layers":[{"ddd":0,"ind":1,"ty":4,"nm":"xingzhuangtuceng 1","sr":1,"ks":{"o":{"a":0,"k":100,"ix":11},"r":{"a":0,"k":0,"ix":10},"p":{"a":0,"k":[562.5,845,0],"ix":2},"a":{"a":0,"k":[0,0,0],"ix":1},"s":{"a":0,"k":[100,100,100],"ix":6}},"ao":0,"shapes":[],"ip":5,"op":245,"st":5,"bm":0},{"ddd":0,"ind":2,"ty":0,"nm":"guangquan","refId":"comp_1","sr":1,"ks":{"o":{"a":0,"k":100,"ix":11},"r":{"a":0,"k":0,"ix":10},"p":{"a":0,"k":[562.508,844.211,0],"ix":2},"a":{"a":0,"k":[562.5,1218,0],"ix":1},"s":{"a":0,"k":[101.576,101.576,100],"ix":6}},"ao":0,"w":1125,"h":2436,"ip":9,"op":129,"st":9,"bm":0},{"ddd":0,"ind":3,"ty":2,"nm":"xiaoguang.png","cl":"png","refId":"image_2","sr":1,"ks":{"o":{"a":1,"k":[{"i":{"x":[0.833],"y":[0.833]},"o":{"x":[0.167],"y":[0.167]},"t":2,"s":[0],"e":[100]},{"i":{"x":[0.833],"y":[1]},"o":{"x":[0.167],"y":[0]},"t":26,"s":[100],"e":[100]},{"i":{"x":[0.833],"y":[0.833]},"o":{"x":[0.167],"y":[0.167]},"t":75,"s":[100],"e":[0]},{"t":125}],"ix":11},"r":{"a":0,"k":0,"ix":10},"p":{"a":0,"k":[562.508,917.346,0],"ix":2},"a":{"a":0,"k":[505,563.5,0],"ix":1},"s":{"a":1,"k":[{"i":{"x":[0.58,0.58,0.58],"y":[1,1,1]},"o":{"x":[0.42,0.42,0.42],"y":[0,0,0]},"t":7,"s":[110.718,110.718,100],"e":[149.469,149.469,100]},{"i":{"x":[0.58,0.58,0.58],"y":[1,1,1]},"o":{"x":[0.42,0.42,0.42],"y":[0,0,0]},"t":23.65,"s":[149.469,149.469,100],"e":[110.718,110.718,100]},{"t":98}],"ix":6}},"ao":0,"ip":2,"op":242,"st":2,"bm":0},{"ddd":0,"ind":4,"ty":2,"nm":"daguang.png","cl":"png","parent":3,"refId":"image_3","sr":1,"ks":{"o":{"a":1,"k":[{"i":{"x":[0.833],"y":[0.833]},"o":{"x":[0.167],"y":[0.167]},"t":75,"s":[100],"e":[0]},{"t":125}],"ix":11},"r":{"a":0,"k":0,"ix":10},"p":{"a":0,"k":[505,485.06,0],"ix":2},"a":{"a":0,"k":[497,588,0],"ix":1},"s":{"a":1,"k":[{"i":{"x":[0.3,0.3,0.3],"y":[1,1,1]},"o":{"x":[0.3,0.3,0.3],"y":[1.181,1.181,0]},"t":20,"s":[0,0,100],"e":[105,105,100]},{"t":50}],"ix":6}},"ao":0,"ip":20,"op":260,"st":20,"bm":0}]},{"id":"comp_1","layers":[{"ddd":0,"ind":1,"ty":2,"nm":"daguangquan.png","cl":"png","refId":"image_0","sr":1,"ks":{"o":{"a":1,"k":[{"i":{"x":[0.833],"y":[0.833]},"o":{"x":[0.167],"y":[0.167]},"t":61,"s":[100],"e":[0]},{"t":70}],"ix":11},"r":{"a":0,"k":0,"ix":10},"p":{"a":0,"k":[562,1218,0],"ix":2},"a":{"a":0,"k":[509.5,509.5,0],"ix":1},"s":{"a":1,"k":[{"i":{"x":[0.833,0.833,0.833],"y":[0.833,0.833,1]},"o":{"x":[0.167,0.167,0.167],"y":[0.167,0.167,0]},"t":34,"s":[73.083,73.083,100],"e":[123.083,123.083,100]},{"t":70}],"ix":6}},"ao":0,"ip":34,"op":71,"st":34,"bm":0},{"ddd":0,"ind":2,"ty":2,"nm":"daguangquan.png","cl":"png","refId":"image_0","sr":1,"ks":{"o":{"a":1,"k":[{"i":{"x":[0.833],"y":[0.833]},"o":{"x":[0.167],"y":[0.167]},"t":45,"s":[100],"e":[0]},{"t":54}],"ix":11},"r":{"a":0,"k":0,"ix":10},"p":{"a":0,"k":[562,1218,0],"ix":2},"a":{"a":0,"k":[509.5,509.5,0],"ix":1},"s":{"a":1,"k":[{"i":{"x":[0.833,0.833,0.833],"y":[0.833,0.833,1]},"o":{"x":[0.167,0.167,0.167],"y":[0.167,0.167,0]},"t":18,"s":[73.083,73.083,100],"e":[123.083,123.083,100]},{"t":54}],"ix":6}},"ao":0,"ip":18,"op":55,"st":18,"bm":0},{"ddd":0,"ind":3,"ty":2,"nm":"daguangquan.png","cl":"png","refId":"image_0","sr":1,"ks":{"o":{"a":1,"k":[{"i":{"x":[0.833],"y":[0.833]},"o":{"x":[0.167],"y":[0.167]},"t":27,"s":[100],"e":[0]},{"t":36}],"ix":11},"r":{"a":0,"k":0,"ix":10},"p":{"a":0,"k":[562,1218,0],"ix":2},"a":{"a":0,"k":[509.5,509.5,0],"ix":1},"s":{"a":1,"k":[{"i":{"x":[0.833,0.833,0.833],"y":[0.833,0.833,1]},"o":{"x":[0.167,0.167,0.167],"y":[0.167,0.167,0]},"t":0,"s":[73.083,73.083,100],"e":[123.083,123.083,100]},{"t":36}],"ix":6}},"ao":0,"ip":0,"op":37,"st":0,"bm":0},{"ddd":0,"ind":4,"ty":2,"nm":"guangshu.png","cl":"png","refId":"image_1","sr":1,"ks":{"o":{"a":1,"k":[{"i":{"x":[0.833],"y":[0.833]},"o":{"x":[0.167],"y":[0.167]},"t":47,"s":[100],"e":[0]},{"t":57}],"ix":11},"r":{"a":0,"k":189.6,"ix":10},"p":{"a":0,"k":[562,1218,0],"ix":2},"a":{"a":0,"k":[547,537,0],"ix":1},"s":{"a":1,"k":[{"i":{"x":[0.6,0.6,0.6],"y":[1,1,1]},"o":{"x":[0.32,0.32,0.32],"y":[0.94,0.94,0]},"t":22,"s":[0,0,100],"e":[130,130,100]},{"t":57}],"ix":6}},"ao":0,"ip":22,"op":58,"st":22,"bm":0},{"ddd":0,"ind":5,"ty":2,"nm":"guangshu.png","cl":"png","refId":"image_1","sr":1,"ks":{"o":{"a":1,"k":[{"i":{"x":[0.833],"y":[0.833]},"o":{"x":[0.167],"y":[0.167]},"t":25,"s":[100],"e":[0]},{"t":35}],"ix":11},"r":{"a":0,"k":0,"ix":10},"p":{"a":0,"k":[562,1218,0],"ix":2},"a":{"a":0,"k":[547,537,0],"ix":1},"s":{"a":1,"k":[{"i":{"x":[0.6,0.6,0.6],"y":[1,1,1]},"o":{"x":[0.32,0.32,0.32],"y":[0.94,0.94,0]},"t":0,"s":[0,0,100],"e":[130,130,100]},{"t":35}],"ix":6}},"ao":0,"ip":0,"op":36,"st":0,"bm":0}]}],"videos":[{"id":"video_0","x":750,"y":0,"w":750,"h":400,"ax":0,"ay":0,"aw":750,"ah":400,"u":"https://example.com/video","p":"animax_video.947daa7e74fcf28605507f635591b479.mp4","sz":862391}],"fonts":{"list":[{"origin": 3,"fName":"Lato-Black","fFamily":"Lato","fStyle":"Black","ascent":74.8992919921875,"fPath": "https://example.com/font/a.ttf"}]},"layers":[{"ddd":0,"ind":1,"ty":0,"nm":"wanchengrenwutanchuang","refId":"comp_0","sr":1,"ks":{"o":{"a":0,"k":100,"ix":11},"r":{"a":0,"k":0,"ix":10},"p":{"a":0,"k":[562.5,838,0],"ix":2},"a":{"a":0,"k":[562,845,0],"ix":1},"s":{"a":0,"k":[100,100,100],"ix":6}},"ao":0,"w":1125,"h":1660,"ip":63,"op":213,"st":63,"bm":0},{"ddd":0,"ind":2,"ty":0,"nm":"wanchengrenwutanchuang","refId":"comp_0","sr":1,"ks":{"o":{"a":0,"k":100,"ix":11},"r":{"a":0,"k":0,"ix":10},"p":{"a":0,"k":[562.5,838,0],"ix":2},"a":{"a":0,"k":[562,845,0],"ix":1},"s":{"a":0,"k":[100,100,100],"ix":6}},"ao":0,"w":1125,"h":1660,"ip":0,"op":150,"st":0,"bm":0}],"markers":[]})";

std::shared_ptr<CompositionModel> GetCompositionModel() {
  return CompositionParser::Parse(
      kCompositionModelJSON, std::string_view{kCompositionModelJSON}.length(),
      1.0f);
}
}  // namespace

TEST(CompositionModelPipelineTest, LoadBeforeIntialized_ShouldReturnError) {
  auto exp1 = GTestExpectation::Make();
  auto loader = AnimaXCompositionLoader::Create();
  loader->Act([exp1](auto& loader_impl) {
    loader_impl->LoadCompositionModelFromURI(
        "test", 1.0, [exp1](CompositionAssetResponse res, LoaderError error) {
          EXPECT_TRUE(!!error);
          exp1->Fulfill();
        });
  });

  EXPECT_FALSE(exp1->Wait());

  auto exp2 = GTestExpectation::Make();
  loader->Act([exp2](auto& loader_impl) {
    loader_impl->LoadCompositionModelFromJSONString(
        "{}", 1.0, [exp2](CompositionAssetResponse res, LoaderError error) {
          EXPECT_TRUE(!!error);
          exp2->Fulfill();
        });
  });

  EXPECT_FALSE(exp2->Wait());

  auto exp3 = GTestExpectation::Make();
  loader->Act([exp3, model = GetCompositionModel()](auto& loader_impl) {
    loader_impl->LoadCompositionModelAsset(
        model, [exp3](CompositionAssetResponse res, LoaderError error) {
          EXPECT_TRUE(!!error);
          exp3->Fulfill();
        });
  });

  EXPECT_FALSE(exp3->Wait());
}

TEST(CompositionModelPipelineTest,
     LoadCompositionModelAsset_WithNullptr_ShouldReturnError) {
  auto loader = AnimaXCompositionLoader::Create();
  auto exp1 = GTestExpectation::Make();
  loader->Act([exp1](auto& loader_impl) {
    loader_impl->LoadCompositionModelAsset(
        nullptr, [exp1](CompositionAssetResponse res, LoaderError error) {
          EXPECT_TRUE(!!error);
          exp1->Fulfill();
        });
  });
  EXPECT_FALSE(exp1->Wait());
}

TEST(CompositionModelPipelineTest,
     LoadCompositionModel_HasDynamicResource_ShouldNotLoadAsset) {
  std::cout << "TEST4" << std::endl;
  auto loader = AnimaXCompositionLoader::Create();
  auto mock_resource_loader = std::make_shared<MockResourceLoader>();
  auto mock_unzip_loader = std::make_shared<MockUnzipLoader>();
  loader->Act([mock_resource_loader, mock_unzip_loader](auto& loader_impl) {
    loader_impl->Init(mock_resource_loader, mock_unzip_loader);
  });

  EXPECT_CALL(*mock_resource_loader, Load(_, _))
      .Times(AtLeast(1))
      .WillOnce([&](ResourceRequest request,
                    MockResourceLoader::CallbackType callback) {
        EXPECT_EQ(request.uri_info.uri, "https://example.com/test/test.json");
        callback(
            ResourceResponse{
                .payload = MakeRawDataResourcePayload(RawData::MakeRawData(
                    kCompositionModelJSON,
                    std::string_view{kCompositionModelJSON}.length(), nullptr,
                    nullptr)),
            },
            {});
      });

  loader->Act(
      [](auto& loader_impl) { loader_impl->SetHasDynamicResource(true); });
  auto exp1 = GTestExpectation::Make();
  loader->Act([exp1](auto& loader_impl) {
    loader_impl->LoadCompositionModelFromURI(
        "https://example.com/test/test.json", 1.0,
        [exp1](CompositionAssetResponse response, LoaderError error) {
          EXPECT_NE(nullptr, response.model);
          EXPECT_FALSE(!!error);
          // We do not load composition asset if the resource is marked as a
          // dynamic
          EXPECT_EQ(0, response.asset_responses.size());
          for (auto& [id, asset] : response.model->GetImages()) {
            EXPECT_FALSE(asset->IsLoaded());
          }
          for (auto& [id, asset] : response.model->GetFonts()) {
            EXPECT_FALSE(asset->IsLoaded());
          }
          for (auto& [id, asset] : response.model->GetVideos()) {
            EXPECT_FALSE(asset->IsValid());
          }
          exp1->Fulfill();
        });
  });
  EXPECT_FALSE(exp1->Wait());
}

TEST(CompositionModelPipelineTest, LoadCompositionModel_ShouldLoadAllAssets) {
  auto loader = AnimaXCompositionLoader::Create();
  auto mock_resource_loader = std::make_shared<MockResourceLoader>();
  auto mock_unzip_loader = std::make_shared<MockUnzipLoader>();
  loader->Act([mock_resource_loader, mock_unzip_loader](auto& loader_impl) {
    loader_impl->Init(mock_resource_loader, mock_unzip_loader);
  });

  EXPECT_CALL(*mock_resource_loader, Load(_, _))
      .Times(AtLeast(1))
      .WillRepeatedly([&](ResourceRequest request,
                          MockResourceLoader::CallbackType callback) {
        if (request.type == ResourceRequestType::kLoadRawData &&
            request.uri_info.content_type == UriInfo::ContentType::kJson) {
          callback(
              ResourceResponse{
                  .payload = MakeRawDataResourcePayload(RawData::MakeRawData(
                      kCompositionModelJSON,
                      std::string_view{kCompositionModelJSON}.length(), nullptr,
                      nullptr)),
              },
              {});
        } else if (request.type == ResourceRequestType::kLoadBitmap) {
          callback(ResourceResponse{.payload = MakeBitmapResourcePayload(
                                        Bitmap::MakeRGBA(1, 1, kBitmap.data(),
                                                         nullptr, nullptr))},
                   {});
        } else if (request.type == ResourceRequestType::kLoadRawData &&
                   request.uri_info.content_type ==
                       UriInfo::ContentType::kFont) {
          callback(
              ResourceResponse{
                  .payload = MakeRawDataResourcePayload(RawData::MakeRawData(
                      kBitmap.data(), kBitmap.size(), nullptr, nullptr))},
              {});
        } else if (request.type ==
                       lynx::animax::ResourceRequestType::kDownloadToLocal &&
                   request.uri_info.content_type ==
                       lynx::animax::UriInfo::ContentType::kVideo) {
          callback(ResourceResponse{.payload = MakePathResourcePayload(
                                        "/usr/tmp/test.mp4")},
                   {});
        } else {
          callback({}, LoaderError{.code = kUserDefined,
                                   .message = "unexpected resource requedt"});
        }
      });
  auto exp = GTestExpectation::Make();
  loader->Act([exp](auto& loader_impl) {
    loader_impl->LoadCompositionModelFromURI(
        "https://example.com/test/test.json", 1.0,
        [exp](CompositionAssetResponse response, LoaderError error) {
          EXPECT_NE(nullptr, response.model);
          EXPECT_FALSE(!!error);
          for (auto& [id, asset] : response.model->GetImages()) {
            EXPECT_TRUE(asset->IsLoaded());
          }
          for (auto& [id, asset] : response.model->GetFonts()) {
            EXPECT_TRUE(asset->IsLoaded());
          }
          for (auto& [id, asset] : response.model->GetVideos()) {
            EXPECT_TRUE(asset->IsValid());
          }
          exp->Fulfill();
        });
  });
  EXPECT_FALSE(exp->Wait());
}
