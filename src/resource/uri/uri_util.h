// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_URI_URI_UTIL_H_
#define ANIMAX_SRC_RESOURCE_URI_URI_UTIL_H_

#include "include/resource/uri_info.h"
#include "src/resource/asset/font_asset.h"
#include "src/resource/asset/image_asset.h"
#include "src/resource/asset/video_asset.h"

namespace lynx {
namespace animax {

#ifdef OS_WIN
static constexpr const char* kPathSeparator = "\\";
#else
static constexpr const char* kPathSeparator = "/";
#endif

class FontAsset;
class ImageAsset;
class VideoAsset;

bool IsBase64Uri(const std::string& uri);
bool IsPolyfillUri(const std::string& uri);

UriInfo::Scheme ParseUriScheme(const std::string& uri);
UriInfo::ContentType ParseUriMainResourceContentType(
    const std::string& uri, bool allow_extensionless_json = false);
std::string ParseUriMainResourceBaseUri(const std::string& uri);

std::string ConcatFilePaths(const std::string& base_path,
                            const std::string& relative_path);

UriInfo UriInfoFromFontAsset(const MainResourceUriInfo& main_uri_info,
                             const FontAsset& font_asset);
UriInfo UriInfoFromImageAsset(const MainResourceUriInfo& main_uri_info,
                              const ImageAsset& image_asset);
UriInfo UriInfoFromVideoAsset(const MainResourceUriInfo& main_uri_info,
                              const VideoAsset& video_asset);
UriInfo UriInfoFromAudioAsset(const MainResourceUriInfo& main_uri_info,
                              const AudioAsset& audio_asset);

bool IsRegularFileExists(const std::string& filePath);

std::string FindFirstMatchingFile(const std::string& directoryPath,
                                  const std::string& fileExtension);

std::string GetUriLocalPath(const std::string& uri);

bool IsAlphaVideoDirectory(const std::string& path);
}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_URI_URI_UTIL_H_
