// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/composition_loader/zip_composition_unzip_loader.h"

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

static constexpr const char* zip_uri = "https://example.com/v.zip";
static constexpr const char* unzip_file_path = "/tmp/v.zip";
static constexpr const char* unzip_folder_path = "/tmp/unzip_folder";

class MockResourceLoader : public Loader<ResourceRequest, ResourceResponse> {
 public:
  ~MockResourceLoader() override = default;
  MOCK_METHOD(void, Load, (ResourceRequest request, CallbackType callback),
              (override));
};

class MockUnzipLoader : public Loader<UnzipRequest, UnzipResponse> {
 public:
  ~MockUnzipLoader() override = default;
  MOCK_METHOD(void, Load, (UnzipRequest request, CallbackType callback),
              (override));
};

TEST(ZipCompositionUnzipLoaderTest, LoadFromUri) {
  auto resource_loader = std::make_shared<MockResourceLoader>();
  auto unzip_loader = std::make_shared<MockUnzipLoader>();
  auto loader =
      ZipCompositionUnzipLoader::MakeLoader(resource_loader, unzip_loader);
  Sequence s;
  {
    InSequence in_seq;
    EXPECT_CALL(*resource_loader, Load(_, _))
        .InSequence(s)
        .WillOnce([](ResourceRequest request, auto callback) {
          callback(ResourceResponse{.payload = MakePathResourcePayload(
                                        unzip_file_path)},
                   {});
        });
    EXPECT_CALL(*unzip_loader, Load(_, _))
        .InSequence(s)
        .WillOnce([](UnzipRequest request, auto callback) {
          EXPECT_EQ(unzip_file_path, request.zip_file_path);
          callback(UnzipResponse{.path = unzip_folder_path}, {});
        });
  }
  loader->Load(
      CompositionModelRequest{
          .uri_info =
              UriInfo{
                  .scheme = ParseUriScheme(zip_uri),
                  .content_type = UriInfo::ContentType::kZip,
                  .uri = zip_uri,
              },
          .scale = 1.f,
      },
      [](ZipCompositionModelTask task, LoaderError err) {
        EXPECT_FALSE(!!err);
        EXPECT_EQ(unzip_folder_path, task.response.base_uri);
      });
}

TEST(ZipCompositionUnzipLoaderTest, LoadInvalidUri) {
  auto resource_loader = std::make_shared<MockResourceLoader>();
  auto unzip_loader = std::make_shared<MockUnzipLoader>();
  auto loader =
      ZipCompositionUnzipLoader::MakeLoader(resource_loader, unzip_loader);
  EXPECT_CALL(*resource_loader, Load(_, _)).Times(0);
  EXPECT_CALL(*unzip_loader, Load(_, _)).Times(0);
  // invalid content_type
  const auto* json_uri = "https://a.com/b.json";
  loader->Load(
      CompositionModelRequest{
          .uri_info =
              UriInfo{
                  .scheme = ParseUriScheme(json_uri),
                  .content_type = ParseUriMainResourceContentType(json_uri),
                  .uri = json_uri,
              },
          .scale = 1.f,
      },
      [](ZipCompositionModelTask res, LoaderError err) { EXPECT_TRUE(!!err); });
  // invalid scheme and content_type
  const auto* invalid_uri = "images/2.png";
  loader->Load(
      CompositionModelRequest{
          .uri_info =
              UriInfo{
                  .scheme = ParseUriScheme(invalid_uri),
                  .content_type = ParseUriMainResourceContentType(invalid_uri),
                  .uri = invalid_uri,
              },
          .scale = 1.f,
      },
      [](ZipCompositionModelTask res, LoaderError err) { EXPECT_TRUE(!!err); });
  // empty uri
  loader->Load(
      CompositionModelRequest{
          .uri_info = UriInfo{},
          .scale = 1.f,
      },
      [](ZipCompositionModelTask res, LoaderError err) { EXPECT_TRUE(!!err); });
  // invalid URI
  loader->Load(
      CompositionModelRequest{
          .uri_info = UriInfo{.scheme = UriInfo::Scheme::kUnknown},
          .scale = 1.f,
      },
      [](ZipCompositionModelTask res, LoaderError err) { EXPECT_TRUE(!!err); });
  // json string
  loader->Load(
      CompositionModelRequest{
          .json_str = "{}",
          .scale = 1.f,
      },
      [](ZipCompositionModelTask res, LoaderError err) { EXPECT_TRUE(!!err); });
}

TEST(ZipCompositionUnzipLoaderTest, BadResourceLoader) {
  auto resource_loader = std::make_shared<MockResourceLoader>();
  auto unzip_loader = std::make_shared<MockUnzipLoader>();
  auto loader =
      ZipCompositionUnzipLoader::MakeLoader(resource_loader, unzip_loader);
  Sequence s;
  {
    InSequence in_seq;
    EXPECT_CALL(*resource_loader, Load(_, _))
        .InSequence(s)
        .WillOnce([](ResourceRequest request, auto callback) {
          // Resource Loader is broken
          callback({}, LoaderError{.code = kInvalidLoader});
        });
    // should not call unzip
    EXPECT_CALL(*unzip_loader, Load(_, _)).Times(0);
  }
  loader->Load(
      CompositionModelRequest{
          .uri_info =
              UriInfo{
                  .scheme = ParseUriScheme(zip_uri),
                  .content_type = UriInfo::ContentType::kZip,
                  .uri = zip_uri,
              },
          .scale = 1.f,
      },
      [](ZipCompositionModelTask res, LoaderError err) { EXPECT_TRUE(!!err); });
}
