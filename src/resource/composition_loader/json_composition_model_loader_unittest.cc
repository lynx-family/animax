// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/composition_loader/json_composition_model_loader.h"

#include <string_view>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "include/resource/loader_error.h"
#include "include/resource/resource_task.h"
#include "include/resource/uri_info.h"
#include "src/base/monitor/trace_event.h"
#include "src/resource/composition_loader/composition_task.h"
#include "src/resource/uri/uri_util.h"

using namespace lynx::animax;
using namespace testing;

static constexpr const char* kJSONMainURIPlaceHolder = "[[JSON]]";

static constexpr const char* kLottieJSONStr =
    R"({"v":"5.6.10","fr":29.9700012207031,"ip":0,"op":60.0000024438501,"w":1200,"h":675,"nm":"RepeaterMergePaths","ddd":0,"assets":[],"layers":[{"ddd":0,"ind":1,"ty":4,"nm":"MergePaths2","sr":1,"ks":{"o":{"a":0,"k":100,"ix":11},"r":{"a":0,"k":0,"ix":10},"p":{"a":0,"k":[546,341.5,0],"ix":2},"a":{"a":0,"k":[0,0,0],"ix":1},"s":{"a":0,"k":[100,100,100],"ix":6}},"ao":0,"shapes":[{"ty":"gr","it":[{"ty":"rc","d":1,"s":{"a":0,"k":[266.812,144.995],"ix":2},"p":{"a":0,"k":[0,0],"ix":3},"r":{"a":0,"k":0,"ix":4},"nm":"Rectangle Path 1","mn":"ADBE Vector Shape - Rect","hd":false},{"ty":"sr","sy":1,"d":1,"pt":{"a":1,"k":[{"i":{"x":[0.833],"y":[0.833]},"o":{"x":[0.167],"y":[0.167]},"t":0,"s":[6]},{"t":45.0000018328876,"s":[12]}],"ix":3},"p":{"a":0,"k":[-3,0],"ix":4},"r":{"a":0,"k":0,"ix":5},"ir":{"a":0,"k":55,"ix":6},"is":{"a":0,"k":28,"ix":8},"or":{"a":0,"k":108,"ix":7},"os":{"a":0,"k":22,"ix":9},"ix":2,"nm":"Polystar Path 1","mn":"ADBE Vector Shape - Star","hd":false},{"ty":"st","c":{"a":0,"k":[1,1,1,1],"ix":3},"o":{"a":0,"k":100,"ix":4},"w":{"a":0,"k":2,"ix":5},"lc":1,"lj":1,"ml":4,"bm":0,"nm":"Stroke 1","mn":"ADBE Vector Graphic - Stroke","hd":false},{"ty":"fl","c":{"a":0,"k":[1,0,0,1],"ix":4},"o":{"a":0,"k":100,"ix":5},"r":1,"bm":0,"nm":"Fill 1","mn":"ADBE Vector Graphic - Fill","hd":false},{"ty":"tr","p":{"a":0,"k":[93.906,178.094],"ix":2},"a":{"a":0,"k":[0,0],"ix":1},"s":{"a":0,"k":[100,100],"ix":3},"r":{"a":0,"k":0,"ix":6},"o":{"a":0,"k":100,"ix":7},"sk":{"a":0,"k":0,"ix":4},"sa":{"a":0,"k":0,"ix":5},"nm":"Transform"}],"nm":"Rectangle 1","np":4,"cix":2,"bm":0,"ix":1,"mn":"ADBE Vector Group","hd":false},{"ty":"mm","mm":4,"nm":"Merge Paths 1","mn":"ADBE Vector Filter - Merge","hd":false},{"ty":"st","c":{"a":0,"k":[1,1,1,1],"ix":3},"o":{"a":0,"k":100,"ix":4},"w":{"a":0,"k":2,"ix":5},"lc":1,"lj":1,"ml":4,"bm":0,"nm":"Stroke 1","mn":"ADBE Vector Graphic - Stroke","hd":false},{"ty":"fl","c":{"a":0,"k":[1,0,0,1],"ix":4},"o":{"a":0,"k":100,"ix":5},"r":1,"bm":0,"nm":"Fill 1","mn":"ADBE Vector Graphic - Fill","hd":false}],"ip":0,"op":60.0000024438501,"st":0,"bm":0},{"ddd":0,"ind":2,"ty":4,"nm":"MergePaths1","sr":1,"ks":{"o":{"a":0,"k":100,"ix":11},"r":{"a":0,"k":0,"ix":10},"p":{"a":0,"k":[410,343.5,0],"ix":2},"a":{"a":0,"k":[0,0,0],"ix":1},"s":{"a":0,"k":[100,100,100],"ix":6}},"ao":0,"shapes":[{"ty":"gr","it":[{"d":1,"ty":"el","s":{"a":0,"k":[272.016,164.961],"ix":2},"p":{"a":0,"k":[0,0],"ix":3},"nm":"Ellipse Path 1","mn":"ADBE Vector Shape - Ellipse","hd":false},{"d":1,"ty":"el","s":{"a":1,"k":[{"i":{"x":[0.833,0.833],"y":[0.833,0.833]},"o":{"x":[0.167,0.167],"y":[0.167,0.167]},"t":0,"s":[100,100]},{"t":45.0000018328876,"s":[202,202]}],"ix":2},"p":{"a":0,"k":[0,0],"ix":3},"nm":"Ellipse Path 2","mn":"ADBE Vector Shape - Ellipse","hd":false},{"ty":"st","c":{"a":0,"k":[1,1,1,1],"ix":3},"o":{"a":0,"k":100,"ix":4},"w":{"a":0,"k":2,"ix":5},"lc":1,"lj":1,"ml":4,"bm":0,"nm":"Stroke 1","mn":"ADBE Vector Graphic - Stroke","hd":false},{"ty":"fl","c":{"a":0,"k":[1,0,0,1],"ix":4},"o":{"a":0,"k":100,"ix":5},"r":1,"bm":0,"nm":"Fill 1","mn":"ADBE Vector Graphic - Fill","hd":false},{"ty":"tr","p":{"a":0,"k":[-177.688,170.129],"ix":2},"a":{"a":0,"k":[0,0],"ix":1},"s":{"a":0,"k":[100,100],"ix":3},"r":{"a":0,"k":0,"ix":6},"o":{"a":0,"k":100,"ix":7},"sk":{"a":0,"k":0,"ix":4},"sa":{"a":0,"k":0,"ix":5},"nm":"Transform"}],"nm":"Ellipse 1","np":4,"cix":2,"bm":0,"ix":1,"mn":"ADBE Vector Group","hd":false},{"ty":"mm","mm":3,"nm":"Merge Paths 1","mn":"ADBE Vector Filter - Merge","hd":false},{"ty":"st","c":{"a":0,"k":[1,1,1,1],"ix":3},"o":{"a":0,"k":100,"ix":4},"w":{"a":0,"k":2,"ix":5},"lc":1,"lj":1,"ml":4,"bm":0,"nm":"Stroke 1","mn":"ADBE Vector Graphic - Stroke","hd":false},{"ty":"fl","c":{"a":0,"k":[1,0,0,1],"ix":4},"o":{"a":0,"k":100,"ix":5},"r":1,"bm":0,"nm":"Fill 1","mn":"ADBE Vector Graphic - Fill","hd":false}],"ip":0,"op":60.0000024438501,"st":0,"bm":0},{"ddd":0,"ind":3,"ty":4,"nm":"Repeater","sr":1,"ks":{"o":{"a":0,"k":100,"ix":11},"r":{"a":0,"k":0,"ix":10},"p":{"a":0,"k":[562,311.5,0],"ix":2},"a":{"a":0,"k":[0,0,0],"ix":1},"s":{"a":0,"k":[100,100,100],"ix":6}},"ao":0,"shapes":[{"ty":"gr","it":[{"ty":"sr","sy":2,"d":1,"pt":{"a":0,"k":5,"ix":3},"p":{"a":0,"k":[0,0],"ix":4},"r":{"a":0,"k":116.159,"ix":5},"or":{"a":0,"k":111.429,"ix":7},"os":{"a":0,"k":0,"ix":9},"ix":1,"nm":"Polystar Path 1","mn":"ADBE Vector Shape - Star","hd":false},{"ty":"st","c":{"a":0,"k":[1,1,1,1],"ix":3},"o":{"a":0,"k":100,"ix":4},"w":{"a":0,"k":2,"ix":5},"lc":1,"lj":1,"ml":4,"bm":0,"nm":"Stroke 1","mn":"ADBE Vector Graphic - Stroke","hd":false},{"ty":"fl","c":{"a":0,"k":[1,0,0,1],"ix":4},"o":{"a":0,"k":100,"ix":5},"r":1,"bm":0,"nm":"Fill 1","mn":"ADBE Vector Graphic - Fill","hd":false},{"ty":"rp","c":{"a":1,"k":[{"i":{"x":[0.833],"y":[0.833]},"o":{"x":[0.167],"y":[0.167]},"t":0,"s":[1]},{"t":45.0000018328876,"s":[5]}],"ix":1},"o":{"a":1,"k":[{"i":{"x":[0.833],"y":[0.833]},"o":{"x":[0.167],"y":[0.167]},"t":0,"s":[0]},{"t":45.0000018328876,"s":[5]}],"ix":2},"m":1,"ix":4,"tr":{"ty":"tr","p":{"a":0,"k":[100,0],"ix":2},"a":{"a":0,"k":[0,0],"ix":1},"s":{"a":0,"k":[100,100],"ix":3},"r":{"a":1,"k":[{"i":{"x":[0.833],"y":[0.833]},"o":{"x":[0.167],"y":[0.167]},"t":0,"s":[73]},{"t":45.0000018328876,"s":[540]}],"ix":4},"so":{"a":0,"k":100,"ix":5},"eo":{"a":0,"k":100,"ix":6},"nm":"Transform"},"nm":"Repeater 1","mn":"ADBE Vector Filter - Repeater","hd":false},{"ty":"tr","p":{"a":0,"k":[-390.688,-167.562],"ix":2},"a":{"a":0,"k":[0,0],"ix":1},"s":{"a":0,"k":[100,100],"ix":3},"r":{"a":0,"k":0,"ix":6},"o":{"a":0,"k":100,"ix":7},"sk":{"a":0,"k":0,"ix":4},"sa":{"a":0,"k":0,"ix":5},"nm":"Transform"}],"nm":"Polystar 1","np":4,"cix":2,"bm":0,"ix":1,"mn":"ADBE Vector Group","hd":false}],"ip":0,"op":60.0000024438501,"st":0,"bm":0}],"markers":[]})";

static constexpr const char* kCorruptedLottieJSONStr =
    R"({"v":"5.6.10","fr":29.9700012207031,"ip":0,"op":60.0000024438501,"w":1200,"h":675,"nm":"RepeaterMergePaths","ddd":0,"assets":[],"layers":[{"ddd":0,"ind":1,"ty":4,"nm":"MergePaths2","sr":1,"ks":{"o":{"a")";

static constexpr const char* kMainURI = "https://example.com/lottie/1.json";
static constexpr const char* kBaseURI = "https://example.com/lottie/";

class MockResourceLoader : public Loader<ResourceRequest, ResourceResponse> {
 public:
  ~MockResourceLoader() override = default;
  MOCK_METHOD(void, Load, (ResourceRequest request, CallbackType callback),
              (override));
};

class MockResourceLoaderListener : public ResourceLoaderListener {
 public:
  ~MockResourceLoaderListener() override = default;
  MOCK_METHOD(void, OnResourceLoaderTraceEvent,
              (TraceEventType, std::string, std::string), (override));
};

TEST(JSONCompositionModelLoaderTest, LoadEmptyRequest) {
  auto mock_loader = std::make_shared<MockResourceLoader>();
  EXPECT_CALL(*mock_loader, Load(_, _)).Times(0);
  auto mock_listener = std::make_shared<MockResourceLoaderListener>();
  // If request is empty, no listener callback should be invoked.
  EXPECT_CALL(*mock_listener, OnResourceLoaderTraceEvent(_, _, _)).Times(0);
  auto loader =
      JSONCompositionModelLoader::MakeLoader(mock_loader, mock_listener);
  auto request = CompositionModelRequest{};
  EXPECT_NE(nullptr, loader);
  loader->Load(std::move(request),
               [](CompositionModelResponse res, LoaderError error) {
                 EXPECT_TRUE(!!error);
                 EXPECT_EQ(nullptr, res.model);
               });
}

TEST(JSONCompositionModelLoaderTest, LoadFromJSONString) {
  auto mock_loader = std::make_shared<MockResourceLoader>();
  // When loading from JSON string, resource loader should not be used.
  EXPECT_CALL(*mock_loader, Load(_, _)).Times(0);
  auto mock_listener = std::make_shared<MockResourceLoaderListener>();
  EXPECT_CALL(*mock_listener, OnResourceLoaderTraceEvent(
                                  TraceEventType::kRequestCompositionStart,
                                  kJSONMainURIPlaceHolder, _))
      .Times(1);
  EXPECT_CALL(*mock_listener,
              OnResourceLoaderTraceEvent(TraceEventType::kRequestCompositionEnd,
                                         kJSONMainURIPlaceHolder, _))
      .Times(1);
  EXPECT_CALL(*mock_listener,
              OnResourceLoaderTraceEvent(TraceEventType::kParseCompositionStart,
                                         kJSONMainURIPlaceHolder, _))
      .Times(1);
  EXPECT_CALL(*mock_listener,
              OnResourceLoaderTraceEvent(TraceEventType::kParseCompositionEnd,
                                         kJSONMainURIPlaceHolder, _))
      .Times(1);
  auto loader =
      JSONCompositionModelLoader::MakeLoader(mock_loader, mock_listener);
  auto request = CompositionModelRequest{
      .json_str = std::string{kLottieJSONStr},
      .scale = 1.f,
  };
  EXPECT_NE(nullptr, loader);
  loader->Load(std::move(request),
               [](CompositionModelResponse res, LoaderError error) {
                 EXPECT_FALSE(!!error);
                 EXPECT_NE(nullptr, res.model);
                 EXPECT_EQ(res.model_type, CompositionModelType::kLottie);
                 EXPECT_EQ(res.base_uri, "");
               });
}

TEST(JSONCompositionModelLoaderTest, LoadFromURI) {
  auto mock_loader = std::make_shared<MockResourceLoader>();
  EXPECT_CALL(*mock_loader, Load(_, _))
      .WillOnce([](ResourceRequest request, auto callback) mutable {
        EXPECT_EQ(request.type, ResourceRequestType::kLoadRawData);
        EXPECT_EQ(request.uri_info.uri, kMainURI);
        auto raw_data_payload = MakeRawDataResourcePayload(RawData::MakeRawData(
            kLottieJSONStr, std::string_view{kLottieJSONStr}.length(), nullptr,
            nullptr));
        callback(ResourceResponse{.payload = std::move(raw_data_payload)}, {});
      });
  auto mock_listener = std::make_shared<MockResourceLoaderListener>();
  EXPECT_CALL(*mock_listener,
              OnResourceLoaderTraceEvent(
                  TraceEventType::kRequestCompositionStart, kMainURI, _))
      .Times(1);
  EXPECT_CALL(*mock_listener,
              OnResourceLoaderTraceEvent(TraceEventType::kRequestCompositionEnd,
                                         kMainURI, _))
      .Times(1);
  EXPECT_CALL(*mock_listener,
              OnResourceLoaderTraceEvent(TraceEventType::kParseCompositionStart,
                                         kMainURI, _))
      .Times(1);
  EXPECT_CALL(*mock_listener,
              OnResourceLoaderTraceEvent(TraceEventType::kParseCompositionEnd,
                                         kMainURI, _))
      .Times(1);
  auto loader =
      JSONCompositionModelLoader::MakeLoader(mock_loader, mock_listener);
  auto request = CompositionModelRequest{
      .uri_info =
          UriInfo{
              .scheme = ParseUriScheme(kMainURI),
              .content_type = ParseUriMainResourceContentType(kMainURI),
              .uri = kMainURI,
          },
      .scale = 1.f,
  };
  EXPECT_NE(nullptr, loader);
  loader->Load(std::move(request),
               [](CompositionModelResponse res, LoaderError error) {
                 EXPECT_FALSE(!!error);
                 EXPECT_NE(nullptr, res.model);
                 EXPECT_EQ(res.model_type, CompositionModelType::kLottie);
                 EXPECT_EQ(res.base_uri, kBaseURI);
               });
}

TEST(JSONCompositionModelLoaderTest, LoadFromURIResourceLoaderError) {
  auto mock_loader = std::make_shared<MockResourceLoader>();
  EXPECT_CALL(*mock_loader, Load(_, _))
      .WillOnce([](ResourceRequest request, auto callback) mutable {
        EXPECT_EQ(request.type, ResourceRequestType::kLoadRawData);
        EXPECT_EQ(request.uri_info.uri, kMainURI);
        callback({}, LoaderError{.code = kNoSuchLoader});
      });
  auto mock_listener = std::make_shared<MockResourceLoaderListener>();
  EXPECT_CALL(*mock_listener,
              OnResourceLoaderTraceEvent(
                  TraceEventType::kRequestCompositionStart, kMainURI, _))
      .Times(1);
  EXPECT_CALL(*mock_listener,
              OnResourceLoaderTraceEvent(TraceEventType::kRequestCompositionEnd,
                                         kMainURI, _))
      .Times(1);
  EXPECT_CALL(*mock_listener,
              OnResourceLoaderTraceEvent(TraceEventType::kParseCompositionStart,
                                         kMainURI, _))
      .Times(0);
  EXPECT_CALL(*mock_listener,
              OnResourceLoaderTraceEvent(TraceEventType::kParseCompositionEnd,
                                         kMainURI, _))
      .Times(0);
  auto loader =
      JSONCompositionModelLoader::MakeLoader(mock_loader, mock_listener);
  auto request = CompositionModelRequest{
      .uri_info =
          UriInfo{
              .scheme = ParseUriScheme(kMainURI),
              .content_type = ParseUriMainResourceContentType(kMainURI),
              .uri = kMainURI,
          },
      .scale = 1.f,
  };
  EXPECT_NE(nullptr, loader);
  loader->Load(std::move(request),
               [](CompositionModelResponse res, LoaderError error) {
                 EXPECT_TRUE(!!error);
                 EXPECT_EQ(nullptr, res.model);
               });
}

TEST(JSONCompositionModelLoaderTest, LoadFromCorruptedData) {
  auto mock_loader = std::make_shared<MockResourceLoader>();
  EXPECT_CALL(*mock_loader, Load(_, _))
      .WillOnce([](ResourceRequest request, auto callback) mutable {
        EXPECT_EQ(request.type, ResourceRequestType::kLoadRawData);
        EXPECT_EQ(request.uri_info.uri, kMainURI);
        auto raw_data_payload = MakeRawDataResourcePayload(RawData::MakeRawData(
            kCorruptedLottieJSONStr,
            std::string_view{kCorruptedLottieJSONStr}.length(), nullptr,
            nullptr));
        callback(ResourceResponse{.payload = std::move(raw_data_payload)}, {});
      });
  auto mock_listener = std::make_shared<MockResourceLoaderListener>();
  EXPECT_CALL(*mock_listener,
              OnResourceLoaderTraceEvent(
                  TraceEventType::kRequestCompositionStart, kMainURI, _))
      .Times(1);
  EXPECT_CALL(*mock_listener,
              OnResourceLoaderTraceEvent(TraceEventType::kRequestCompositionEnd,
                                         kMainURI, _))
      .Times(1);
  EXPECT_CALL(*mock_listener,
              OnResourceLoaderTraceEvent(TraceEventType::kParseCompositionStart,
                                         kMainURI, _))
      .Times(1);
  EXPECT_CALL(*mock_listener,
              OnResourceLoaderTraceEvent(TraceEventType::kParseCompositionEnd,
                                         kMainURI, _))
      .Times(1);
  auto loader =
      JSONCompositionModelLoader::MakeLoader(mock_loader, mock_listener);
  auto request = CompositionModelRequest{
      .uri_info =
          UriInfo{
              .scheme = ParseUriScheme(kMainURI),
              .content_type = ParseUriMainResourceContentType(kMainURI),
              .uri = kMainURI,
          },
      .scale = 1.f,
  };
  EXPECT_NE(nullptr, loader);
  loader->Load(std::move(request),
               [](CompositionModelResponse res, LoaderError error) {
                 EXPECT_TRUE(!!error);
                 EXPECT_EQ(nullptr, res.model);
               });
}
