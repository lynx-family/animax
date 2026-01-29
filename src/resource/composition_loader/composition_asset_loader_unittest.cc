// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/composition_loader/composition_asset_loader.h"

#include <cstdint>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "include/resource/loader_error.h"
#include "include/resource/resource_task.h"
#include "src/base/monitor/trace_event.h"
#include "src/model/composition_model.h"
#include "src/parser/composition_parser.h"
#include "src/resource/loader/exec_loader.h"
#include "src/resource/log_util.h"
#include "src/resource/resource_loader_listener.h"

using namespace ::testing;
using namespace lynx::animax;

namespace {

class MockResourceLoader : public Loader<ResourceRequest, ResourceResponse> {
 public:
  ~MockResourceLoader() override = default;
  MOCK_METHOD(void, Load, (ResourceRequest, CallbackType), (override));
};

class MockResourceLoaderListener : public ResourceLoaderListener {
 public:
  ~MockResourceLoaderListener() override = default;
  MOCK_METHOD(void, OnResourceLoaderTraceEvent,
              (TraceEventType trace_event, std::string arg1, std::string arg2),
              (override));
  MOCK_METHOD(void, OnBeforeAssetsLoad,
              (const std::vector<std::shared_ptr<Asset>>& assets,
               const std::vector<std::shared_ptr<Asset>>& invalid_assets),
              (override));
};

static constexpr std::array<uint8_t, 4> kBitmap =
    std::array<uint8_t, 4>{1, 1, 1, 1};
static constexpr std::array<uint8_t, 1> kRawData = std::array<uint8_t, 1>{1};
static constexpr const char* kVideoFilePath = "/tmp/video.mp4";
static constexpr const char* kFailImageAssetURI =
    "https://example.com/fail_img_1.44fdfe24.png";
static constexpr const char* kFailImageID = "image_1";
static constexpr const char* kErrorMessage = "mock error";
static constexpr const char* kMainURI =
    "https://example.com/lottie/lottie.json";
static constexpr const char* kBaseURI = "https://example.com/lottie";

static constexpr const char* kImageAsset0ID = "image_0";
static constexpr const char* kImageAsset1ID = "image_1";
static constexpr const char* kImageAsset2ID = "image_2";
static constexpr const char* kImageAsset3ID = "image_3";
static constexpr const char* kFontAssetID = "Lato-Black";
static constexpr const char* kVideoAssetID = "video_0";

static constexpr const char* kCompositionModelJSON =
    R"({"v":"5.6.1","fr":60,"ip":0,"op":181,"w":1125,"h":1657,"nm":"lottie","ddd":0,"assets":[{"id":"image_0","w":1019,"h":1019,"u":"https://example.com/","p":"img_0.c65d636c.png","e":0},{"id":"image_1","w":1092,"h":1072,"u":"https://example.com/","p":"fail_img_1.44fdfe24.png","e":0},{"id":"image_2","w":1010,"h":1127,"u":"https://example.com/","p":"img_2.a424ceaa.png","e":0},{"id":"image_3","w":994,"h":1176,"u":"https://example.com/","p":"img_3.6e276d42.png","e":0},{"id":"comp_0","layers":[{"ddd":0,"ind":1,"ty":4,"nm":"xingzhuangtuceng 1","sr":1,"ks":{"o":{"a":0,"k":100,"ix":11},"r":{"a":0,"k":0,"ix":10},"p":{"a":0,"k":[562.5,845,0],"ix":2},"a":{"a":0,"k":[0,0,0],"ix":1},"s":{"a":0,"k":[100,100,100],"ix":6}},"ao":0,"shapes":[],"ip":5,"op":245,"st":5,"bm":0},{"ddd":0,"ind":2,"ty":0,"nm":"guangquan","refId":"comp_1","sr":1,"ks":{"o":{"a":0,"k":100,"ix":11},"r":{"a":0,"k":0,"ix":10},"p":{"a":0,"k":[562.508,844.211,0],"ix":2},"a":{"a":0,"k":[562.5,1218,0],"ix":1},"s":{"a":0,"k":[101.576,101.576,100],"ix":6}},"ao":0,"w":1125,"h":2436,"ip":9,"op":129,"st":9,"bm":0},{"ddd":0,"ind":3,"ty":2,"nm":"xiaoguang.png","cl":"png","refId":"image_2","sr":1,"ks":{"o":{"a":1,"k":[{"i":{"x":[0.833],"y":[0.833]},"o":{"x":[0.167],"y":[0.167]},"t":2,"s":[0],"e":[100]},{"i":{"x":[0.833],"y":[1]},"o":{"x":[0.167],"y":[0]},"t":26,"s":[100],"e":[100]},{"i":{"x":[0.833],"y":[0.833]},"o":{"x":[0.167],"y":[0.167]},"t":75,"s":[100],"e":[0]},{"t":125}],"ix":11},"r":{"a":0,"k":0,"ix":10},"p":{"a":0,"k":[562.508,917.346,0],"ix":2},"a":{"a":0,"k":[505,563.5,0],"ix":1},"s":{"a":1,"k":[{"i":{"x":[0.58,0.58,0.58],"y":[1,1,1]},"o":{"x":[0.42,0.42,0.42],"y":[0,0,0]},"t":7,"s":[110.718,110.718,100],"e":[149.469,149.469,100]},{"i":{"x":[0.58,0.58,0.58],"y":[1,1,1]},"o":{"x":[0.42,0.42,0.42],"y":[0,0,0]},"t":23.65,"s":[149.469,149.469,100],"e":[110.718,110.718,100]},{"t":98}],"ix":6}},"ao":0,"ip":2,"op":242,"st":2,"bm":0},{"ddd":0,"ind":4,"ty":2,"nm":"daguang.png","cl":"png","parent":3,"refId":"image_3","sr":1,"ks":{"o":{"a":1,"k":[{"i":{"x":[0.833],"y":[0.833]},"o":{"x":[0.167],"y":[0.167]},"t":75,"s":[100],"e":[0]},{"t":125}],"ix":11},"r":{"a":0,"k":0,"ix":10},"p":{"a":0,"k":[505,485.06,0],"ix":2},"a":{"a":0,"k":[497,588,0],"ix":1},"s":{"a":1,"k":[{"i":{"x":[0.3,0.3,0.3],"y":[1,1,1]},"o":{"x":[0.3,0.3,0.3],"y":[1.181,1.181,0]},"t":20,"s":[0,0,100],"e":[105,105,100]},{"t":50}],"ix":6}},"ao":0,"ip":20,"op":260,"st":20,"bm":0}]},{"id":"comp_1","layers":[{"ddd":0,"ind":1,"ty":2,"nm":"daguangquan.png","cl":"png","refId":"image_0","sr":1,"ks":{"o":{"a":1,"k":[{"i":{"x":[0.833],"y":[0.833]},"o":{"x":[0.167],"y":[0.167]},"t":61,"s":[100],"e":[0]},{"t":70}],"ix":11},"r":{"a":0,"k":0,"ix":10},"p":{"a":0,"k":[562,1218,0],"ix":2},"a":{"a":0,"k":[509.5,509.5,0],"ix":1},"s":{"a":1,"k":[{"i":{"x":[0.833,0.833,0.833],"y":[0.833,0.833,1]},"o":{"x":[0.167,0.167,0.167],"y":[0.167,0.167,0]},"t":34,"s":[73.083,73.083,100],"e":[123.083,123.083,100]},{"t":70}],"ix":6}},"ao":0,"ip":34,"op":71,"st":34,"bm":0},{"ddd":0,"ind":2,"ty":2,"nm":"daguangquan.png","cl":"png","refId":"image_0","sr":1,"ks":{"o":{"a":1,"k":[{"i":{"x":[0.833],"y":[0.833]},"o":{"x":[0.167],"y":[0.167]},"t":45,"s":[100],"e":[0]},{"t":54}],"ix":11},"r":{"a":0,"k":0,"ix":10},"p":{"a":0,"k":[562,1218,0],"ix":2},"a":{"a":0,"k":[509.5,509.5,0],"ix":1},"s":{"a":1,"k":[{"i":{"x":[0.833,0.833,0.833],"y":[0.833,0.833,1]},"o":{"x":[0.167,0.167,0.167],"y":[0.167,0.167,0]},"t":18,"s":[73.083,73.083,100],"e":[123.083,123.083,100]},{"t":54}],"ix":6}},"ao":0,"ip":18,"op":55,"st":18,"bm":0},{"ddd":0,"ind":3,"ty":2,"nm":"daguangquan.png","cl":"png","refId":"image_0","sr":1,"ks":{"o":{"a":1,"k":[{"i":{"x":[0.833],"y":[0.833]},"o":{"x":[0.167],"y":[0.167]},"t":27,"s":[100],"e":[0]},{"t":36}],"ix":11},"r":{"a":0,"k":0,"ix":10},"p":{"a":0,"k":[562,1218,0],"ix":2},"a":{"a":0,"k":[509.5,509.5,0],"ix":1},"s":{"a":1,"k":[{"i":{"x":[0.833,0.833,0.833],"y":[0.833,0.833,1]},"o":{"x":[0.167,0.167,0.167],"y":[0.167,0.167,0]},"t":0,"s":[73.083,73.083,100],"e":[123.083,123.083,100]},{"t":36}],"ix":6}},"ao":0,"ip":0,"op":37,"st":0,"bm":0},{"ddd":0,"ind":4,"ty":2,"nm":"guangshu.png","cl":"png","refId":"image_1","sr":1,"ks":{"o":{"a":1,"k":[{"i":{"x":[0.833],"y":[0.833]},"o":{"x":[0.167],"y":[0.167]},"t":47,"s":[100],"e":[0]},{"t":57}],"ix":11},"r":{"a":0,"k":189.6,"ix":10},"p":{"a":0,"k":[562,1218,0],"ix":2},"a":{"a":0,"k":[547,537,0],"ix":1},"s":{"a":1,"k":[{"i":{"x":[0.6,0.6,0.6],"y":[1,1,1]},"o":{"x":[0.32,0.32,0.32],"y":[0.94,0.94,0]},"t":22,"s":[0,0,100],"e":[130,130,100]},{"t":57}],"ix":6}},"ao":0,"ip":22,"op":58,"st":22,"bm":0},{"ddd":0,"ind":5,"ty":2,"nm":"guangshu.png","cl":"png","refId":"image_1","sr":1,"ks":{"o":{"a":1,"k":[{"i":{"x":[0.833],"y":[0.833]},"o":{"x":[0.167],"y":[0.167]},"t":25,"s":[100],"e":[0]},{"t":35}],"ix":11},"r":{"a":0,"k":0,"ix":10},"p":{"a":0,"k":[562,1218,0],"ix":2},"a":{"a":0,"k":[547,537,0],"ix":1},"s":{"a":1,"k":[{"i":{"x":[0.6,0.6,0.6],"y":[1,1,1]},"o":{"x":[0.32,0.32,0.32],"y":[0.94,0.94,0]},"t":0,"s":[0,0,100],"e":[130,130,100]},{"t":35}],"ix":6}},"ao":0,"ip":0,"op":36,"st":0,"bm":0}]}],"videos":[{"id":"video_0","x":750,"y":0,"w":750,"h":400,"ax":0,"ay":0,"aw":750,"ah":400,"u":"https://example.com/video","p":"animax_video.947daa7e74fcf28605507f635591b479.mp4","sz":862391}],"fonts":{"list":[{"origin": 3,"fName":"Lato-Black","fFamily":"Lato","fStyle":"Black","ascent":74.8992919921875,"fPath": "https://example.com/font/a.ttf"}]},"layers":[{"ddd":0,"ind":1,"ty":0,"nm":"wanchengrenwutanchuang","refId":"comp_0","sr":1,"ks":{"o":{"a":0,"k":100,"ix":11},"r":{"a":0,"k":0,"ix":10},"p":{"a":0,"k":[562.5,838,0],"ix":2},"a":{"a":0,"k":[562,845,0],"ix":1},"s":{"a":0,"k":[100,100,100],"ix":6}},"ao":0,"w":1125,"h":1660,"ip":63,"op":213,"st":63,"bm":0},{"ddd":0,"ind":2,"ty":0,"nm":"wanchengrenwutanchuang","refId":"comp_0","sr":1,"ks":{"o":{"a":0,"k":100,"ix":11},"r":{"a":0,"k":0,"ix":10},"p":{"a":0,"k":[562.5,838,0],"ix":2},"a":{"a":0,"k":[562,845,0],"ix":1},"s":{"a":0,"k":[100,100,100],"ix":6}},"ao":0,"w":1125,"h":1660,"ip":0,"op":150,"st":0,"bm":0}],"markers":[]})";

static constexpr const char* kCompositionModelJSONNoAsset =
    R"({"v":"5.6.10","fr":29.9700012207031,"ip":0,"op":61.0000024845809,"w":80,"h":80,"nm":"Shapes","ddd":0,"assets":[],"layers":[{"ddd":0,"ind":1,"ty":4,"nm":"shapes","sr":1,"ks":{"o":{"a":0,"k":100,"ix":11},"r":{"a":0,"k":0,"ix":10},"p":{"a":0,"k":[40,40,0],"ix":2},"a":{"a":0,"k":[0,0,0],"ix":1},"s":{"a":0,"k":[100,100,100],"ix":6}},"ao":0,"shapes":[{"ty":"gr","it":[{"d":1,"ty":"el","s":{"a":0,"k":[20,20],"ix":2},"p":{"a":0,"k":[0,0],"ix":3},"nm":"Ellipse Path 1","mn":"ADBE Vector Shape - Ellipse","hd":false},{"ty":"tm","s":{"a":1,"k":[{"i":{"x":[0.833],"y":[0.833]},"o":{"x":[0.167],"y":[0.167]},"t":12,"s":[0]},{"t":27.0000010997325,"s":[50]}],"ix":1},"e":{"a":0,"k":100,"ix":2},"o":{"a":0,"k":0,"ix":3},"m":1,"ix":2,"nm":"Trim Paths 1","mn":"ADBE Vector Filter - Trim","hd":false},{"ty":"st","c":{"a":0,"k":[0.894117647059,0.278431372549,0.274509803922,1],"ix":3},"o":{"a":0,"k":100,"ix":4},"w":{"a":0,"k":1,"ix":5},"lc":2,"lj":2,"bm":0,"nm":"Stroke 1","mn":"ADBE Vector Graphic - Stroke","hd":false},{"ty":"tr","p":{"a":0,"k":[0,0],"ix":2},"a":{"a":0,"k":[0,0],"ix":1},"s":{"a":0,"k":[100,100],"ix":3},"r":{"a":0,"k":0,"ix":6},"o":{"a":0,"k":100,"ix":7},"sk":{"a":0,"k":0,"ix":4},"sa":{"a":0,"k":0,"ix":5},"nm":"Transform"}],"nm":"Ellipse","np":3,"cix":2,"bm":0,"ix":1,"mn":"ADBE Vector Group","hd":false},{"ty":"gr","it":[{"ty":"rc","d":1,"s":{"a":0,"k":[60,60],"ix":2},"p":{"a":0,"k":[0,0],"ix":3},"r":{"a":0,"k":0,"ix":4},"nm":"Rectangle Path 1","mn":"ADBE Vector Shape - Rect","hd":false},{"ty":"tm","s":{"a":1,"k":[{"i":{"x":[0.833],"y":[0.833]},"o":{"x":[0.167],"y":[0.167]},"t":12,"s":[0]},{"t":27.0000010997325,"s":[50]}],"ix":1},"e":{"a":0,"k":100,"ix":2},"o":{"a":0,"k":0,"ix":3},"m":1,"ix":2,"nm":"Trim Paths 1","mn":"ADBE Vector Filter - Trim","hd":false},{"ty":"st","c":{"a":0,"k":[0.894117647059,0.278431372549,0.274509803922,1],"ix":3},"o":{"a":0,"k":100,"ix":4},"w":{"a":0,"k":1,"ix":5},"lc":2,"lj":2,"bm":0,"nm":"Stroke 1","mn":"ADBE Vector Graphic - Stroke","hd":false},{"ty":"tr","p":{"a":0,"k":[0,-0.062],"ix":2},"a":{"a":0,"k":[0,0],"ix":1},"s":{"a":0,"k":[100,100],"ix":3},"r":{"a":0,"k":0,"ix":6},"o":{"a":0,"k":100,"ix":7},"sk":{"a":0,"k":0,"ix":4},"sa":{"a":0,"k":0,"ix":5},"nm":"Transform"}],"nm":"Rectangle","np":3,"cix":2,"bm":0,"ix":2,"mn":"ADBE Vector Group","hd":false},{"ty":"gr","it":[{"ty":"sr","sy":1,"d":1,"pt":{"a":0,"k":5,"ix":3},"p":{"a":0,"k":[0,0],"ix":4},"r":{"a":0,"k":0,"ix":5},"ir":{"a":0,"k":10,"ix":6},"is":{"a":0,"k":0,"ix":8},"or":{"a":0,"k":25,"ix":7},"os":{"a":0,"k":0,"ix":9},"ix":1,"nm":"Polystar Path 1","mn":"ADBE Vector Shape - Star","hd":false},{"ty":"tm","s":{"a":1,"k":[{"i":{"x":[0.833],"y":[0.833]},"o":{"x":[0.167],"y":[0.167]},"t":12,"s":[0]},{"t":27.0000010997325,"s":[50]}],"ix":1},"e":{"a":0,"k":100,"ix":2},"o":{"a":0,"k":0,"ix":3},"m":1,"ix":2,"nm":"Trim Paths 1","mn":"ADBE Vector Filter - Trim","hd":false},{"ty":"st","c":{"a":0,"k":[0.894117647059,0.278431372549,0.274509803922,1],"ix":3},"o":{"a":0,"k":100,"ix":4},"w":{"a":0,"k":1,"ix":5},"lc":2,"lj":2,"bm":0,"nm":"Stroke 1","mn":"ADBE Vector Graphic - Stroke","hd":false},{"ty":"tr","p":{"a":0,"k":[0,-0.062],"ix":2},"a":{"a":0,"k":[0,0],"ix":1},"s":{"a":0,"k":[100,100],"ix":3},"r":{"a":0,"k":0,"ix":6},"o":{"a":0,"k":100,"ix":7},"sk":{"a":0,"k":0,"ix":4},"sa":{"a":0,"k":0,"ix":5},"nm":"Transform"}],"nm":"Polystar","np":3,"cix":2,"bm":0,"ix":3,"mn":"ADBE Vector Group","hd":false},{"ty":"gr","it":[{"ind":0,"ty":"sh","ix":1,"ks":{"a":0,"k":{"i":[[0,0],[0,0],[0,0],[0,0]],"o":[[0,0],[0,0],[0,0],[0,0]],"v":[[35,-35],[35,35],[-35,35],[-35,-35]],"c":true},"ix":2},"nm":"Path 1","mn":"ADBE Vector Shape - Group","hd":false},{"ty":"tm","s":{"a":1,"k":[{"i":{"x":[0.833],"y":[0.833]},"o":{"x":[0.167],"y":[0.167]},"t":0,"s":[0]},{"t":15.0000006109625,"s":[50]}],"ix":1},"e":{"a":0,"k":100,"ix":2},"o":{"a":0,"k":0,"ix":3},"m":1,"ix":2,"nm":"Trim Paths 1","mn":"ADBE Vector Filter - Trim","hd":false},{"ty":"st","c":{"a":0,"k":[0.162837296724,0.110633164644,0.730024516582,1],"ix":3},"o":{"a":0,"k":100,"ix":4},"w":{"a":0,"k":1,"ix":5},"lc":2,"lj":2,"bm":0,"nm":"Stroke 1","mn":"ADBE Vector Graphic - Stroke","hd":false},{"ty":"tr","p":{"a":0,"k":[0,-0.062],"ix":2},"a":{"a":0,"k":[0,0],"ix":1},"s":{"a":0,"k":[100,100],"ix":3},"r":{"a":0,"k":0,"ix":6},"o":{"a":0,"k":100,"ix":7},"sk":{"a":0,"k":0,"ix":4},"sa":{"a":0,"k":0,"ix":5},"nm":"Transform"}],"nm":"Shape","np":3,"cix":2,"bm":0,"ix":4,"mn":"ADBE Vector Group","hd":false},{"ty":"tm","s":{"a":0,"k":0,"ix":1},"e":{"a":1,"k":[{"i":{"x":[0.833],"y":[0.833]},"o":{"x":[0.333],"y":[0]},"t":12,"s":[0]},{"t":27.0000010997325,"s":[100]}],"ix":2},"o":{"a":0,"k":0,"ix":3},"m":1,"ix":5,"nm":"Trim Paths 1","mn":"ADBE Vector Filter - Trim","hd":false}],"ip":0,"op":60.0000024438501,"st":-30.0000012219251,"bm":0}],"markers":[]})";

std::shared_ptr<CompositionModel> GetCompositionModel() {
  return CompositionParser::Parse(
      kCompositionModelJSON, std::string_view{kCompositionModelJSON}.length(),
      1.0f);
}

std::shared_ptr<CompositionModel> GetNoAssetCompositionModel() {
  return CompositionParser::Parse(
      kCompositionModelJSONNoAsset,
      std::string_view{kCompositionModelJSONNoAsset}.length(), 1.0f);
}

}  // namespace

TEST(CompositionAssetLoaderTest, CompositionModelAssetCheck) {
  // This test is intented to show you how many assets are in
  // kCompositionModelJSON
  auto model = GetCompositionModel();
  EXPECT_NE(nullptr, model);
  // There are 4 images
  EXPECT_EQ(4, model->GetImages().size());
  // There are 1 fonts
  EXPECT_EQ(1, model->GetFonts().size());
  // There is one video
  EXPECT_EQ(1, model->GetVideos().size());

  auto model2 = GetNoAssetCompositionModel();
  EXPECT_NE(nullptr, model2);
  // There are 0 images
  EXPECT_EQ(0, model2->GetImages().size());
  // There are 0 fonts
  EXPECT_EQ(0, model2->GetFonts().size());
  // There is 0 video
  EXPECT_EQ(0, model2->GetVideos().size());
}

TEST(CompositionAssetLoaderTest, LoadCompositionAssetNoAsset) {
  auto model = GetNoAssetCompositionModel();
  EXPECT_NE(nullptr, model);
  auto resource_loader = std::make_shared<MockResourceLoader>();
  EXPECT_CALL(*resource_loader, Load(_, _)).Times(0);
  auto listener = std::make_shared<MockResourceLoaderListener>();
  Sequence s;
  {
    InSequence in_seq;
    // When there is no assets, PrepareAssetsStart and PrepareAssetsEnd should
    // not be called.
    EXPECT_CALL(*listener, OnResourceLoaderTraceEvent(
                               TraceEventType::kPrepareAssetsStart, _, _))
        .Times(0)
        .InSequence(s);
    EXPECT_CALL(*listener, OnResourceLoaderTraceEvent(
                               TraceEventType::kPrepareAssetsEnd, _, _))
        .Times(0)
        .InSequence(s);
  }
  auto composition_asset_loader =
      CompositionAssetLoader::Make<CompositionAssetLoader>(resource_loader,
                                                           listener);
  composition_asset_loader->Load(
      CompositionAssetRequest{.main_uri_info =
                                  MainResourceUriInfo{
                                      .base_uri = kBaseURI,
                                  },
                              .model = model},
      [](CompositionAssetResponse res, LoaderError error) {
        EXPECT_FALSE(!!error);
        EXPECT_NE(nullptr, res.model);
        EXPECT_TRUE(res.asset_responses.empty());

        auto asset = AssetResponse();
        asset.type = ResourceType::kImage;
        asset.id = "image_0";
        asset.error.code = LoaderErrorCode::kInvalidLoaderRequest;
        asset.error.message = "error msg";
        res.asset_responses.push_back(asset);
        std::ostringstream oss;
        oss << res;
        EXPECT_FALSE(oss.str().empty());
      });
}

TEST(CompositionAssetLoaderTest, LoadCompositionAssetAllFailure) {
  auto model = GetCompositionModel();
  EXPECT_NE(nullptr, model);
  auto resource_loader = std::make_shared<MockResourceLoader>();
  EXPECT_CALL(*resource_loader, Load(_, _))
      .WillRepeatedly(Invoke([](auto req, auto callback) {
        callback({}, LoaderError{.code = kUserDefined, kErrorMessage});
      }));
  auto listener = std::make_shared<MockResourceLoaderListener>();
  EXPECT_CALL(*listener, OnResourceLoaderTraceEvent(
                             TraceEventType::kPrepareAssetsStart, kMainURI, _))
      .Times(1);
  EXPECT_CALL(*listener, OnResourceLoaderTraceEvent(
                             TraceEventType::kPrepareAssetsEnd, kMainURI, _))
      .Times(1);

  EXPECT_CALL(*listener, OnResourceLoaderTraceEvent(
                             TraceEventType::kPrepareSingleAssetStart, kMainURI,
                             kImageAsset0ID))
      .Times(1);
  EXPECT_CALL(*listener,
              OnResourceLoaderTraceEvent(TraceEventType::kPrepareSingleAssetEnd,
                                         kMainURI, kImageAsset0ID))
      .Times(1);

  EXPECT_CALL(*listener, OnResourceLoaderTraceEvent(
                             TraceEventType::kPrepareSingleAssetStart, kMainURI,
                             kImageAsset1ID))
      .Times(1);
  EXPECT_CALL(*listener,
              OnResourceLoaderTraceEvent(TraceEventType::kPrepareSingleAssetEnd,
                                         kMainURI, kImageAsset1ID))
      .Times(1);

  EXPECT_CALL(*listener, OnResourceLoaderTraceEvent(
                             TraceEventType::kPrepareSingleAssetStart, kMainURI,
                             kImageAsset2ID))
      .Times(1);
  EXPECT_CALL(*listener,
              OnResourceLoaderTraceEvent(TraceEventType::kPrepareSingleAssetEnd,
                                         kMainURI, kImageAsset2ID))
      .Times(1);

  EXPECT_CALL(*listener, OnResourceLoaderTraceEvent(
                             TraceEventType::kPrepareSingleAssetStart, kMainURI,
                             kImageAsset3ID))
      .Times(1);
  EXPECT_CALL(*listener,
              OnResourceLoaderTraceEvent(TraceEventType::kPrepareSingleAssetEnd,
                                         kMainURI, kImageAsset3ID))
      .Times(1);

  EXPECT_CALL(*listener, OnResourceLoaderTraceEvent(
                             TraceEventType::kPrepareSingleAssetStart, kMainURI,
                             kFontAssetID))
      .Times(1);
  EXPECT_CALL(*listener,
              OnResourceLoaderTraceEvent(TraceEventType::kPrepareSingleAssetEnd,
                                         kMainURI, kFontAssetID))
      .Times(1);

  EXPECT_CALL(*listener, OnResourceLoaderTraceEvent(
                             TraceEventType::kPrepareSingleAssetStart, kMainURI,
                             kVideoAssetID))
      .Times(1);
  EXPECT_CALL(*listener,
              OnResourceLoaderTraceEvent(TraceEventType::kPrepareSingleAssetEnd,
                                         kMainURI, kVideoAssetID))
      .Times(1);

  auto composition_asset_loader =
      CompositionAssetLoader::Make<CompositionAssetLoader>(resource_loader,
                                                           listener);
  composition_asset_loader->Load(
      CompositionAssetRequest{.main_uri_info =
                                  MainResourceUriInfo{

                                      .base_uri = kBaseURI,
                                      .main_uri = kMainURI,
                                  },
                              .model = model},
      [](CompositionAssetResponse res, LoaderError error) {
        for (auto& [type, id, error] : res.asset_responses) {
          EXPECT_TRUE(!!error);
          EXPECT_EQ(kErrorMessage, error.message);
        }
        EXPECT_FALSE(!!error);
      });
}

TEST(CompositionAssetLoaderTest, LoadCompositionAssetAllFailureNoListener) {
  auto model = GetCompositionModel();
  EXPECT_NE(nullptr, model);
  auto resource_loader = std::make_shared<MockResourceLoader>();
  EXPECT_CALL(*resource_loader, Load(_, _))
      .WillRepeatedly(Invoke([](auto req, auto callback) {
        callback({}, LoaderError{.code = kUserDefined, kErrorMessage});
      }));
  auto composition_asset_loader =
      CompositionAssetLoader::Make<CompositionAssetLoader>(resource_loader);
  composition_asset_loader->Load(
      CompositionAssetRequest{.main_uri_info =
                                  MainResourceUriInfo{
                                      .base_uri = kBaseURI,
                                  },
                              .model = model},
      [](CompositionAssetResponse res, LoaderError error) {
        for (auto& [type, id, error] : res.asset_responses) {
          EXPECT_TRUE(!!error);
          EXPECT_EQ(kErrorMessage, error.message);
        }
        EXPECT_FALSE(!!error);
      });
}

TEST(CompositionAssetLoaderTest,
     LoadCompositionAssetSuccessAndOneImageFailure) {
  auto model = GetCompositionModel();
  EXPECT_NE(nullptr, model);
  auto resource_loader = std::make_shared<MockResourceLoader>();
  EXPECT_CALL(*resource_loader, Load(_, _))
      .WillRepeatedly(Invoke([](ResourceRequest req, auto callback) {
        switch (req.type) {
          case lynx::animax::ResourceRequestType::kInvalid: {
            FAIL();
            break;
          }
          case lynx::animax::ResourceRequestType::kLoadBitmap: {
            if (req.uri_info.uri == kFailImageAssetURI) {
              callback({}, LoaderError{.code = kInvalidImage, kErrorMessage});
            } else {
              callback(
                  ResourceResponse{
                      .payload = MakeBitmapResourcePayload(Bitmap::MakeRGBA(
                          1, 1, kBitmap.data(), nullptr, nullptr))},
                  {});
            }
            break;
          }
          case lynx::animax::ResourceRequestType::kLoadRawData: {
            callback(
                ResourceResponse{
                    .payload = MakeRawDataResourcePayload(RawData::MakeRawData(
                        kBitmap.data(), kBitmap.size(), nullptr, nullptr))},
                {});
            break;
          }
          case lynx::animax::ResourceRequestType::kDownloadToLocal: {
            callback(ResourceResponse{.payload = MakePathResourcePayload(
                                          kVideoFilePath)},
                     {});
            break;
          }
          default: {
            FAIL();
          }
        };
      }));
  auto listener = std::make_shared<MockResourceLoaderListener>();
  EXPECT_CALL(*listener, OnResourceLoaderTraceEvent(
                             TraceEventType::kPrepareAssetsStart, kMainURI, _))
      .Times(1);
  EXPECT_CALL(*listener, OnResourceLoaderTraceEvent(
                             TraceEventType::kPrepareAssetsEnd, kMainURI, _))
      .Times(1);

  EXPECT_CALL(*listener, OnResourceLoaderTraceEvent(
                             TraceEventType::kPrepareSingleAssetStart, kMainURI,
                             kImageAsset0ID))
      .Times(1);
  EXPECT_CALL(*listener,
              OnResourceLoaderTraceEvent(TraceEventType::kPrepareSingleAssetEnd,
                                         kMainURI, kImageAsset0ID))
      .Times(1);

  EXPECT_CALL(*listener, OnResourceLoaderTraceEvent(
                             TraceEventType::kPrepareSingleAssetStart, kMainURI,
                             kImageAsset1ID))
      .Times(1);
  EXPECT_CALL(*listener,
              OnResourceLoaderTraceEvent(TraceEventType::kPrepareSingleAssetEnd,
                                         kMainURI, kImageAsset1ID))
      .Times(1);

  EXPECT_CALL(*listener, OnResourceLoaderTraceEvent(
                             TraceEventType::kPrepareSingleAssetStart, kMainURI,
                             kImageAsset2ID))
      .Times(1);
  EXPECT_CALL(*listener,
              OnResourceLoaderTraceEvent(TraceEventType::kPrepareSingleAssetEnd,
                                         kMainURI, kImageAsset2ID))
      .Times(1);

  EXPECT_CALL(*listener, OnResourceLoaderTraceEvent(
                             TraceEventType::kPrepareSingleAssetStart, kMainURI,
                             kImageAsset3ID))
      .Times(1);
  EXPECT_CALL(*listener,
              OnResourceLoaderTraceEvent(TraceEventType::kPrepareSingleAssetEnd,
                                         kMainURI, kImageAsset3ID))
      .Times(1);

  EXPECT_CALL(*listener, OnResourceLoaderTraceEvent(
                             TraceEventType::kPrepareSingleAssetStart, kMainURI,
                             kFontAssetID))
      .Times(1);
  EXPECT_CALL(*listener,
              OnResourceLoaderTraceEvent(TraceEventType::kPrepareSingleAssetEnd,
                                         kMainURI, kFontAssetID))
      .Times(1);

  EXPECT_CALL(*listener, OnResourceLoaderTraceEvent(
                             TraceEventType::kPrepareSingleAssetStart, kMainURI,
                             kVideoAssetID))
      .Times(1);
  EXPECT_CALL(*listener,
              OnResourceLoaderTraceEvent(TraceEventType::kPrepareSingleAssetEnd,
                                         kMainURI, kVideoAssetID))
      .Times(1);
  auto composition_asset_loader =
      CompositionAssetLoader::Make<CompositionAssetLoader>(resource_loader,
                                                           listener);
  composition_asset_loader->Load(
      CompositionAssetRequest{
          .main_uri_info =
              MainResourceUriInfo{.base_uri = kBaseURI, .main_uri = kMainURI},
          .model = model},
      [](CompositionAssetResponse res, LoaderError error) {
        for (auto& [type, id, error] : res.asset_responses) {
          if (id == kFailImageID) {
            EXPECT_TRUE(!!error);
            EXPECT_EQ(kErrorMessage, error.message);
          } else {
            EXPECT_FALSE(!!error);
          }
        }
        EXPECT_FALSE(!!error);
      });

  for (auto [id, asset] : model->GetImages()) {
    if (id != kFailImageID) {
      EXPECT_TRUE(asset->IsLoaded());
    } else {
      EXPECT_FALSE(asset->IsLoaded());
    }
  }

  for (auto [id, asset] : model->GetFonts()) {
    EXPECT_TRUE(asset->IsLoaded());
  }

  for (auto [id, asset] : model->GetVideos()) {
    EXPECT_TRUE(asset->IsValid());
  }
}
