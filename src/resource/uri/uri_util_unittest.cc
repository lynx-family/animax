// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/uri/uri_util.h"

#include "gtest/gtest.h"
#include "include/resource/raw_data.h"

using namespace lynx::animax;

TEST(UriUtilTest, ParseUriInfoIsBase64) {
  // Valid base64 data URIs
  EXPECT_TRUE(IsBase64Uri(
      "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAUAAAABCAIAAAB"));
  EXPECT_TRUE(IsBase64Uri("data:,;base64,dGVzdCBkYXRh"));
  EXPECT_TRUE(IsBase64Uri("data:text/plain;base64,aGVsbG8gd29ybGQ="));

  // Invalid base64 data URIs (missing 'base64' marker or not starting with
  // 'data:')
  EXPECT_FALSE(
      IsBase64Uri("data:image/png;iVBORw0KGgoAAAANSUhEUgAAAAUAAAABCAIAAAB"));
  EXPECT_FALSE(IsBase64Uri("data:,Hello%2C%20World!"));
  EXPECT_FALSE(IsBase64Uri("data:text/plain;aGVsbG8gd29ybGQ="));

  // Not data URIs at all
  EXPECT_FALSE(IsBase64Uri("http://example.com"));
  EXPECT_FALSE(IsBase64Uri("https://example.com/image.png"));
  EXPECT_FALSE(IsBase64Uri("file:///path/to/file.txt"));

  // Edge cases
  EXPECT_FALSE(IsBase64Uri(""));
  EXPECT_FALSE(IsBase64Uri("base64,"));
  // There must be a semicolon ; before base64 extension.
  EXPECT_FALSE(IsBase64Uri("data:base64,"));
  // There must be a comma , after base64 extension.
  EXPECT_FALSE(IsBase64Uri("data:;base64"));
  EXPECT_TRUE(IsBase64Uri("data:;base64,"));
}

TEST(UriUtilTest, ParseUriInfoScheme) {
  EXPECT_EQ(UriInfo::Scheme::kHttp, ParseUriScheme("http://example.com"));
  EXPECT_EQ(UriInfo::Scheme::kHttp, ParseUriScheme("https://example.com"));
  EXPECT_EQ(UriInfo::Scheme::kAsset, ParseUriScheme("asset://example"));
  EXPECT_EQ(UriInfo::Scheme::kFile, ParseUriScheme("file:///example.txt"));
  EXPECT_EQ(UriInfo::Scheme::kUnknown, ParseUriScheme("ftp://example.com"));
  EXPECT_EQ(UriInfo::Scheme::kDataURL,
            ParseUriScheme("data:image/png;base64,"));
  EXPECT_EQ(UriInfo::Scheme::kUnknown, ParseUriScheme("abcd"));
  EXPECT_EQ(UriInfo::Scheme::kUnknown, ParseUriScheme(""));
}

TEST(UriUtilTest, ParseUriInfoMainResourceContentType) {
  EXPECT_EQ(UriInfo::ContentType::kJson,
            ParseUriMainResourceContentType("http://example.com/data.json"));
  EXPECT_EQ(UriInfo::ContentType::kJson,
            ParseUriMainResourceContentType(
                "http://example.com/path/data.json?query=1"));
  EXPECT_EQ(
      UriInfo::ContentType::kZip,
      ParseUriMainResourceContentType("http://example.com/path/data.zip"));
  EXPECT_EQ(UriInfo::ContentType::kZip,
            ParseUriMainResourceContentType(
                "http://example.com/path/data.zip#anchor"));
  EXPECT_EQ(
      UriInfo::ContentType::kUnknown,
      ParseUriMainResourceContentType("http://example.com/path/data.jsonx"));
  EXPECT_EQ(UriInfo::ContentType::kUnknown,
            ParseUriMainResourceContentType("http://example.com/path/"));
}

TEST(UriUtilTest, ParseUriInfoMainResourceBaseUri) {
  EXPECT_EQ("http://example.com/path/",
            ParseUriMainResourceBaseUri("http://example.com/path/file.txt"));
  EXPECT_EQ("http://example.com/",
            ParseUriMainResourceBaseUri("http://example.com/file.txt"));
  EXPECT_EQ("file:///", ParseUriMainResourceBaseUri("file:///file.txt"));
  EXPECT_EQ("/user/tmp/", ParseUriMainResourceBaseUri("/user/tmp/2.json"));
  EXPECT_EQ(
      "http://example.com/path/subpath/",
      ParseUriMainResourceBaseUri("http://example.com/path/subpath/file.txt"));
  EXPECT_EQ("", ParseUriMainResourceBaseUri(""));
}

TEST(UriUtilTest, ConcatenatesPaths) {
  ASSERT_EQ(ConcatFilePaths("/a/b/c/////", "///d/e/f.txt"), "/a/b/c/d/e/f.txt");
  ASSERT_EQ(ConcatFilePaths("/a/b/c", "/d/e/f.txt"), "/a/b/c/d/e/f.txt");
  ASSERT_EQ(ConcatFilePaths("/a/b/c/", "d/e/f.txt"), "/a/b/c/d/e/f.txt");
  ASSERT_EQ(ConcatFilePaths("", "/d/e/f.txt"), "/d/e/f.txt");
  ASSERT_EQ(ConcatFilePaths("/a/b/c", ""), "/a/b/c");
  ASSERT_EQ(ConcatFilePaths("/a/b/c/", "d/e/f"), "/a/b/c/d/e/f");
}

TEST(UriUtilTest, GetUriLocalPath) {
  // Test file:/// prefix
  EXPECT_EQ(GetUriLocalPath("file:///sdcard/test.json"), "/sdcard/test.json");
  EXPECT_EQ(GetUriLocalPath("file:///data/local/tmp/file.txt"),
            "/data/local/tmp/file.txt");

  // Test asset:/// prefix
  EXPECT_EQ(GetUriLocalPath("asset:///test/image.png"), "/test/image.png");

  // Test paths without prefix
  EXPECT_EQ(GetUriLocalPath("/simple/path.txt"), "/simple/path.txt");
  EXPECT_EQ(GetUriLocalPath("regular_file.json"), "regular_file.json");
}

TEST(UriUtilTest, IsAlphaVideoDirectory) {
  // Test with file:/// prefix
  EXPECT_FALSE(IsAlphaVideoDirectory("file:///non/existent/path/"));

  // Test with asset:/// prefix
  EXPECT_FALSE(IsAlphaVideoDirectory("asset:///test/video/"));

  // Test paths without prefix
  EXPECT_FALSE(IsAlphaVideoDirectory("/simple/path/"));
  EXPECT_FALSE(IsAlphaVideoDirectory("regular_directory/"));

  // Test empty path
  EXPECT_FALSE(IsAlphaVideoDirectory(""));
}

TEST(UriInfoFromFontAssetTest, HandleLocalFont) {
  // Font is loaded or local; expect an empty UriInfo
  FontAsset local_font(FontAssetModel{"Arial", "Regular", "Normal", 12.0,
                                      "/usr/share/fonts/Arial.ttf",
                                      FontPathOrigin::kLocal});
  MainResourceUriInfo main_info{"http://example.com/", "/images", {}};

  UriInfo result = UriInfoFromFontAsset(main_info, local_font);
  EXPECT_FALSE(result.Valid());
}

TEST(UriInfoFromFontAssetTest, HandleLoadedFont) {
  // Font is loaded or local; expect an empty UriInfo
  FontAsset loaded_font(FontAssetModel{"Arial", "Regular", "Normal", 12.0,
                                       "https://example.com/fonts/font.ttf",
                                       FontPathOrigin::kFontUrl});
  MainResourceUriInfo main_info{"http://example.com/", "/images", {}};

  std::array<char, 1> fake_data = {'1'};
  loaded_font.SetRawData(RawData::MakeRawData(
      fake_data.data(), 1, [](auto) {}, nullptr));

  UriInfo result = UriInfoFromFontAsset(main_info, loaded_font);
  EXPECT_FALSE(result.Valid());
}

TEST(UriInfoFromFontAssetTest, HandleValidPathUrl) {
  // Font has a valid path URL; expect the UriInfo to use this path
  FontAsset valid_path_font(FontAssetModel{"Arial", "Regular", "Normal", 12.0,
                                           "http://example.com/fonts/Arial.ttf",
                                           FontPathOrigin::kFontUrl});
  MainResourceUriInfo main_info{"http://example.com/", "/images", {}};

  UriInfo result = UriInfoFromFontAsset(main_info, valid_path_font);
  EXPECT_TRUE(result.Valid());
  EXPECT_EQ(result.uri, "http://example.com/fonts/Arial.ttf");
  EXPECT_EQ(result.content_type, UriInfo::ContentType::kFont);
}

TEST(UriInfoFromFontAssetTest, ConstructFullUrlFromMainInfoAndFamily) {
  // Font asset path is unknown, construct URL from main_info and family
  FontAsset unknown_path_font(FontAssetModel{"Arial", "Regular", "Normal", 12.0,
                                             "", FontPathOrigin::kFontUrl});
  MainResourceUriInfo main_info{"http://example.com/lottie", "/images", {}};

  UriInfo result = UriInfoFromFontAsset(main_info, unknown_path_font);
  EXPECT_TRUE(result.Valid());
  EXPECT_EQ(result.uri, "http://example.com/lottie/fonts/Arial.ttf");
  EXPECT_EQ(result.content_type, UriInfo::ContentType::kFont);
}

TEST(UriInfoFromFontAssetTest, ConstructFullUrlFromMainInfoWithEmptyFamily) {
  // Font asset path is unknown, construct URL from main_info and family
  FontAsset unknown_path_font(FontAssetModel{"", "Regular", "Normal", 12.0, "",
                                             FontPathOrigin::kFontUrl});
  MainResourceUriInfo main_info{"http://example.com/lottie", "/images", {}};

  UriInfo result = UriInfoFromFontAsset(main_info, unknown_path_font);
  EXPECT_FALSE(result.Valid());
}

TEST(UriInfoFromFontAssetTest, EmptyBaseUriAndFamily) {
  // Both base_uri and family are empty; expect an empty UriInfo
  FontAsset empty_family_font(FontAssetModel{"Arial", "Regular", "Normal", 12.0,
                                             "", FontPathOrigin::kFontUrl});
  MainResourceUriInfo main_info{"", "/images", {}};

  UriInfo result = UriInfoFromFontAsset(main_info, empty_family_font);
  EXPECT_FALSE(result.Valid());
}

TEST(UriInfoFromFontAssetTest, HandleLocalFilePath) {
  // Font is on a local file path; validate the UriInfo is as expected
  FontAsset local_file_font(FontAssetModel{"Arial", "Regular", "Normal", 12.0,
                                           "/usr/share/fonts/Arial.ttf",
                                           FontPathOrigin::kFontUrl});
  MainResourceUriInfo main_info{"http://example.com/", "/images", {}};

  UriInfo result = UriInfoFromFontAsset(main_info, local_file_font);
  EXPECT_TRUE(result.Valid());
  EXPECT_EQ(result.uri, "/usr/share/fonts/Arial.ttf");
}

class UriInfoFromVideoAssetTest : public ::testing::Test {
 protected:
  std::shared_ptr<VideoAsset> make_video_asset(const std::string& file_name,
                                               const std::string& dir_name) {
    std::array<int32_t, 4> rgb_frame = {0};  // Placeholder for frame data
    std::array<int32_t, 4> a_frame = {0};    // Placeholder for alpha frame data
    return VideoAsset::Make(
        VideoAssetModel{rgb_frame, a_frame, "some_id", dir_name, file_name});
  }
};

TEST_F(UriInfoFromVideoAssetTest, VideoAlreadyLoaded) {
  auto video_asset = make_video_asset("video.mp4", "videos/");
  // Make Video Valid.
  video_asset->PrepareFrameData("");
  MainResourceUriInfo main_info{"http://example.com/", "/images", {}};

  UriInfo result = UriInfoFromVideoAsset(main_info, *video_asset);
  EXPECT_FALSE(result.Valid());
}

TEST_F(UriInfoFromVideoAssetTest, ValidUriConstructed) {
  auto video_asset =
      make_video_asset("https://example.com/videos/video.mp4", "");
  MainResourceUriInfo main_info{"http://example.com/lottie", "/images", {}};

  UriInfo result = UriInfoFromVideoAsset(main_info, *video_asset);
  EXPECT_EQ(result.uri, "https://example.com/videos/video.mp4");
  EXPECT_EQ(result.content_type, UriInfo::ContentType::kVideo);
}

TEST_F(UriInfoFromVideoAssetTest, ConstructFullUrlFromMainInfo) {
  auto video_asset = make_video_asset("video.mp4", "videos/");
  MainResourceUriInfo main_info{"http://example.com/lottie", "/images", {}};

  UriInfo result = UriInfoFromVideoAsset(main_info, *video_asset);
  EXPECT_EQ(result.uri, "http://example.com/lottie/videos/video.mp4");
  EXPECT_EQ(result.content_type, UriInfo::ContentType::kVideo);
}

TEST_F(UriInfoFromVideoAssetTest, EmptyBaseUri) {
  auto video_asset = make_video_asset("video.mp4", "videos/");
  MainResourceUriInfo main_info{
      "", "/images", {}};  // Empty base_uri should lead to relative URL

  UriInfo result = UriInfoFromVideoAsset(main_info, *video_asset);
  EXPECT_FALSE(result.Valid());
}

class UriInfoFromImageAssetTest : public ::testing::Test {
 protected:
  ImageAsset create_image_asset(std::string id, std::string file_name,
                                std::string dir_name) {
    return ImageAsset(ImageAssetModel{1, 1, id, dir_name, file_name});
  }
};

TEST_F(UriInfoFromImageAssetTest, ImageAlreadyLoaded) {
  ImageAsset image_asset = create_image_asset("123", "image.png", "/images/");
  std::array<char, 1> fake_data = {1};
  image_asset.SetBitmap(Bitmap::MakeRGBA(
      1, 1, fake_data.data(), [](auto) {}, nullptr));  // Mock the loaded state
  MainResourceUriInfo main_info{"http://example.com/lottie", "/images", {}};

  UriInfo result = UriInfoFromImageAsset(main_info, image_asset);
  EXPECT_FALSE(result.Valid());
  EXPECT_TRUE(result.uri.empty());
  EXPECT_EQ(result.content_type, UriInfo::ContentType::kUnknown);
}

TEST_F(UriInfoFromImageAssetTest, ImageBase64) {
  ImageAsset image_asset =
      create_image_asset("123", "data:image/png;base64,abc123==", "");
  MainResourceUriInfo main_info{"http://example.com/lottie", "/images", {}};

  UriInfo result = UriInfoFromImageAsset(main_info, image_asset);
  EXPECT_EQ(result.uri, "data:image/png;base64,abc123==");
  EXPECT_EQ(result.content_type, UriInfo::ContentType::kImage);
  EXPECT_EQ(result.scheme, UriInfo::Scheme::kDataURL);
}

TEST_F(UriInfoFromImageAssetTest, ImageNeedsPolyfillFullUrl) {
  ImageAsset image_asset = create_image_asset("123", "%s", "/images/");
  EXPECT_TRUE(IsPolyfillUri(image_asset.Model().file_name));
  std::unordered_map<std::string, std::string> polyfill = {
      {"123", "http://example.com/images/image.png"}};
  MainResourceUriInfo main_info{"http://example.com/lottie", "/images",
                                polyfill};

  UriInfo result = UriInfoFromImageAsset(main_info, image_asset);
  EXPECT_EQ(result.uri, "http://example.com/images/image.png");
  EXPECT_EQ(result.content_type, UriInfo::ContentType::kImage);
}

TEST_F(UriInfoFromImageAssetTest, ImageNeedsPolyfillWithEmptyPolyfill) {
  ImageAsset image_asset = create_image_asset("123", "%s", "/images/");
  EXPECT_TRUE(IsPolyfillUri(image_asset.Model().file_name));
  MainResourceUriInfo main_info{"http://example.com/lottie", "/images", {}};

  UriInfo result = UriInfoFromImageAsset(main_info, image_asset);
  EXPECT_FALSE(result.Valid());
}

TEST_F(UriInfoFromImageAssetTest, ImageNeedsPolyfillNotFullPath) {
  ImageAsset image_asset = create_image_asset("123", "%s", "images/");
  EXPECT_TRUE(IsPolyfillUri(image_asset.Model().file_name));
  std::unordered_map<std::string, std::string> polyfill = {
      {"123", "image.png"}};
  MainResourceUriInfo main_info{"http://example.com/lottie", "images_new/",
                                polyfill};

  UriInfo result = UriInfoFromImageAsset(main_info, image_asset);
  EXPECT_TRUE(result.Valid());
  EXPECT_EQ(result.uri, "http://example.com/lottie/images_new/image.png");
}

TEST_F(UriInfoFromImageAssetTest, ImagePathConcatenation) {
  ImageAsset image_asset =
      create_image_asset("123", "image.png", "https://example.com/images/");
  MainResourceUriInfo main_info{"http://example.com/lottie", "", {}};

  UriInfo result = UriInfoFromImageAsset(main_info, image_asset);
  EXPECT_EQ(result.uri, "https://example.com/images/image.png");
  EXPECT_EQ(result.content_type, UriInfo::ContentType::kImage);
}

TEST_F(UriInfoFromImageAssetTest, ImagePathWithBaseUri) {
  ImageAsset image_asset = create_image_asset("123", "image.png", "images/");
  MainResourceUriInfo main_info{"http://example.com/", "media/", {}};

  UriInfo result = UriInfoFromImageAsset(main_info, image_asset);
  EXPECT_EQ(result.uri, "http://example.com/media/image.png");
  EXPECT_EQ(result.content_type, UriInfo::ContentType::kImage);
}

TEST_F(UriInfoFromImageAssetTest, ImageInAssets) {
  ImageAsset image_asset = create_image_asset("123", "image.png", "images/");
  MainResourceUriInfo main_info{"asset:///", "asset_images", {}};

  UriInfo result = UriInfoFromImageAsset(main_info, image_asset);
  EXPECT_EQ(result.uri, "asset:///asset_images/image.png");
}

TEST_F(UriInfoFromImageAssetTest, ImageInAssetsRoot) {
  ImageAsset image_asset = create_image_asset("123", "image.png", "");
  MainResourceUriInfo main_info{"asset:///", "", {}};

  UriInfo result = UriInfoFromImageAsset(main_info, image_asset);
  EXPECT_EQ(result.uri, "asset:///image.png");
}

TEST_F(UriInfoFromImageAssetTest, ImageInFiles) {
  ImageAsset image_asset = create_image_asset("123", "image.png", "images/");
  MainResourceUriInfo main_info{"file:///", "asset_images", {}};

  UriInfo result = UriInfoFromImageAsset(main_info, image_asset);
  EXPECT_EQ(result.uri, "file:///asset_images/image.png");
}

TEST_F(UriInfoFromImageAssetTest, ImageInFilesRoot) {
  ImageAsset image_asset = create_image_asset("123", "image.png", "");
  MainResourceUriInfo main_info{"file:///", "", {}};

  UriInfo result = UriInfoFromImageAsset(main_info, image_asset);
  EXPECT_EQ(result.uri, "file:///image.png");
}

TEST_F(UriInfoFromImageAssetTest, ImageOverHttps_ImageFolderNoTrailingSlash) {
  ImageAsset image_asset =
      create_image_asset("789", "secure_image.png", "secure_path/");
  // The base URI does NOT end with a slash. Some code may forget to insert one.
  MainResourceUriInfo main_info{"https://secure.example.com", "imgs", {}};

  // Proper code should insert the slash if missing:
  // Expected: https://secure.example.com/imgs/secure_image.png
  UriInfo result = UriInfoFromImageAsset(main_info, image_asset);
  EXPECT_EQ(result.uri, "https://secure.example.com/imgs/secure_image.png");
}

TEST_F(UriInfoFromImageAssetTest, ImageOverHttps_DirNameNoTrailingSlash) {
  ImageAsset image_asset =
      create_image_asset("789", "secure_image.png", "secure_path");
  // The base URI does NOT end with a slash. Some code may forget to insert one.
  MainResourceUriInfo main_info{"https://secure.example.com", "", {}};

  // Proper code should insert the slash if missing:
  // Expected: https://secure.example.com/imgs/secure_image.png
  UriInfo result = UriInfoFromImageAsset(main_info, image_asset);
  EXPECT_EQ(result.uri,
            "https://secure.example.com/secure_path/secure_image.png");
}

TEST_F(UriInfoFromImageAssetTest, ImagePathWithInvalidBaseUri) {
  ImageAsset image_asset = create_image_asset("123", "image.png", "images/");
  MainResourceUriInfo main_info{"", "media/", {}};

  UriInfo result = UriInfoFromImageAsset(main_info, image_asset);
  EXPECT_FALSE(result.Valid());
}

TEST_F(UriInfoFromImageAssetTest, HandleLocalImageFile) {
  // Image is stored locally; validate the UriInfo is as expected
  ImageAsset local_image_asset =
      create_image_asset("124", "image.png", "/local/path/");
  MainResourceUriInfo main_info{"http://example.com/lottie", "images/", {}};

  UriInfo result = UriInfoFromImageAsset(main_info, local_image_asset);
  EXPECT_TRUE(result.Valid());
  EXPECT_EQ(result.uri, "http://example.com/lottie/images/image.png");
}

TEST_F(UriInfoFromImageAssetTest, AssetSchemeWithSpecifiedImageFolder) {
  ImageAsset image_asset =
      create_image_asset("456", "test_image.jpg", "images/");
  // Base URI uses asset scheme and image folder is specified (not empty)
  MainResourceUriInfo main_info{
      "asset:///first_folder", "first_image_folder/second_image_folder/", {}};
  // When base URI has asset scheme and image folder is specified,
  // the URI should use asset:/// prefix directly rather than concatenating with
  // base URI
  UriInfo result = UriInfoFromImageAsset(main_info, image_asset);
  EXPECT_EQ(result.uri,
            "asset:///first_image_folder/second_image_folder/test_image.jpg");
  EXPECT_EQ(result.scheme, UriInfo::Scheme::kAsset);

  // When image folder is empty, the URI should use asset:/// prefix directly
  MainResourceUriInfo main_info_empty_image_folder{
      "asset:///first_folder", "", {}};
  UriInfo result_empty_image_folder =
      UriInfoFromImageAsset(main_info_empty_image_folder, image_asset);
  EXPECT_EQ(result_empty_image_folder.uri,
            "asset:///first_folder/images/test_image.jpg");
  EXPECT_EQ(result_empty_image_folder.scheme, UriInfo::Scheme::kAsset);
}

TEST_F(UriInfoFromImageAssetTest, HttpSchemeWithSpecifiedImageFolder) {
  ImageAsset image_asset = create_image_asset("789", "product.jpg", "catalog/");

  // HTTP scheme with specified image folder
  MainResourceUriInfo main_info{"https://example.com/api", "media/images", {}};
  UriInfo result = UriInfoFromImageAsset(main_info, image_asset);
  EXPECT_EQ(result.uri, "https://example.com/api/media/images/product.jpg");
  EXPECT_EQ(result.scheme, UriInfo::Scheme::kHttp);

  // HTTP scheme with empty image folder
  MainResourceUriInfo main_info_empty_folder{
      "https://cdn.example.org/content", "", {}};
  UriInfo result_empty_folder =
      UriInfoFromImageAsset(main_info_empty_folder, image_asset);
  EXPECT_EQ(result_empty_folder.uri,
            "https://cdn.example.org/content/catalog/product.jpg");
  EXPECT_EQ(result_empty_folder.scheme, UriInfo::Scheme::kHttp);
}

TEST_F(UriInfoFromImageAssetTest, FileSchemeWithSpecifiedImageFolder) {
  ImageAsset image_asset = create_image_asset("abc", "document.png", "docs/");

  // File scheme with specified image folder
  MainResourceUriInfo main_info{"file:///user/local", "resources/assets", {}};
  UriInfo result = UriInfoFromImageAsset(main_info, image_asset);
  EXPECT_EQ(result.uri, "file:///user/local/resources/assets/document.png");
  EXPECT_EQ(result.scheme, UriInfo::Scheme::kFile);

  // File scheme with empty image folder
  MainResourceUriInfo main_info_empty_folder{"file:///home/user/data", "", {}};
  UriInfo result_empty_folder =
      UriInfoFromImageAsset(main_info_empty_folder, image_asset);
  EXPECT_EQ(result_empty_folder.uri,
            "file:///home/user/data/docs/document.png");
  EXPECT_EQ(result_empty_folder.scheme, UriInfo::Scheme::kFile);
}
