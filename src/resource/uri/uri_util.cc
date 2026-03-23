// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/uri/uri_util.h"

#ifdef OS_WIN
#include <windows.h>
#define S_ISREG(m) (!(m & FILE_ATTRIBUTE_DIRECTORY))
#else
#include <dirent.h>
#endif
#include <sys/stat.h>

#include "base/include/string/string_utils.h"
#include "include/resource/uri_info.h"
#include "src/resource/asset/audio_asset.h"
#include "src/resource/asset/font_asset.h"
#include "src/resource/asset/image_asset.h"
#include "src/resource/asset/video_asset.h"

namespace {
// Strip URL resource query parameters and fragments
static constexpr const char* kStripChars = "?#";

// Extensions
static constexpr const char* kJson = ".json";
static constexpr const char* kZip = ".zip";
static constexpr const char* kTrueType = ".ttf";

// Alpha video config.json file name
static constexpr const char* kConfigJSONFileName = "config.json";

// Lengths of extensions
static constexpr auto kJsonLength = 5;
static constexpr auto kZipLength = 4;

struct URIScheme {
  const char* const base;
  const char* const slash;
};

static constexpr URIScheme kHttp{"http:", "http://"};
static constexpr URIScheme kHttps{"https:", "https://"};
static constexpr URIScheme kAsset{"asset:", "asset:///"};
static constexpr URIScheme kFile{"file:", "file:///"};

static constexpr const char* kPolyfillPlaceHolder = "%s";

// Base64
static constexpr const char* kDataURLScheme = "data:";
static constexpr const char* kDataURLBase64Extension = ";base64,";

static constexpr const char* kFontFolderName = "fonts";
}  // namespace

namespace lynx {
namespace animax {

bool IsBase64Uri(const std::string& uri) {
  return base::BeginsWith(uri, kDataURLScheme) &&
         uri.find(kDataURLBase64Extension) != std::string::npos;
}

bool IsPolyfillUri(const std::string& uri) {
  return uri == kPolyfillPlaceHolder;
}

UriInfo::Scheme ParseUriScheme(const std::string& uri) {
  if (IsBase64Uri(uri)) {
    return UriInfo::Scheme::kDataURL;
  } else if (lynx::base::BeginsWith(uri, kHttp.base) ||
             lynx::base::BeginsWith(uri, kHttps.base)) {
    return UriInfo::Scheme::kHttp;
  } else if (lynx::base::BeginsWith(uri, kAsset.base)) {
    return UriInfo::Scheme::kAsset;
  } else if (lynx::base::BeginsWith(uri, kPathSeparator) ||
             lynx::base::BeginsWith(uri, kFile.base)) {
    return UriInfo::Scheme::kFile;
  }

#if OS_WIN
  if (uri.size() >= 3 && std::isalpha(uri[0]) && uri[1] == ':' &&
      (uri[2] == '\\' || uri[2] == '/')) {
    return UriInfo::Scheme::kFile;
  }
#endif

  return UriInfo::Scheme::kUnknown;
}

UriInfo::ContentType ParseUriMainResourceContentType(const std::string& uri) {
  auto length = uri.length();
  auto position = uri.find_first_of(kStripChars);
  if (position != std::string::npos) {
    length = position;
  }

  // Check if the URL ends with .json or .zip
  if (length >= kJsonLength &&
      uri.compare(length - kJsonLength, kJsonLength, kJson) == 0) {
    return UriInfo::ContentType::kJson;
  }

  if (length >= kZipLength &&
      uri.compare(length - kZipLength, kZipLength, kZip) == 0) {
    return UriInfo::ContentType::kZip;
  }

  return UriInfo::ContentType::kUnknown;
}

std::string ParseUriMainResourceBaseUri(const std::string& uri) {
  if (auto pos = uri.rfind("/"); std::string::npos != pos) {
    return uri.substr(0, pos + 1);
  } else {
    return std::string{};
  }
}

namespace {
std::string NormalizePath(const std::string& path) {
  bool path_starts_with_slash = !path.empty() && path[0] == '/';
  auto split_result = std::vector<std::string>{};
  base::SplitString(path, '/', split_result);
  split_result.erase(
      std::remove_if(split_result.begin(), split_result.end(),
                     [](const auto& str) { return str.empty(); }),
      split_result.end());
  if (split_result.empty()) {
    return std::string{};
  }

  auto first = split_result.front();
  bool first_is_scheme = true;
  if (first == kAsset.base) {
    first = kAsset.slash;
  } else if (first == kFile.base) {
    first = kFile.slash;
  } else if (first == kHttp.base) {
    first = kHttp.slash;
  } else if (first == kHttps.base) {
    first = kHttps.slash;
  } else {
    first_is_scheme = false;
  }

  auto head = std::string{};

  if (path_starts_with_slash) {
    head = kPathSeparator;
  } else if (first_is_scheme) {
    head = first;
    split_result.erase(split_result.begin());
  }

  auto result = head + base::Join(split_result, kPathSeparator);
  return result;
}
}  // namespace

std::string ConcatFilePaths(const std::string& base_path,
                            const std::string& relative_path) {
  if (base_path.empty()) {
    return relative_path;
  } else if (relative_path.empty()) {
    return base_path;
  } else {
    return NormalizePath(base_path + kPathSeparator + relative_path);
  }
}

UriInfo UriInfoFromFontAsset(const MainResourceUriInfo& main_uri_info,
                             const FontAsset& font_asset) {
  if (font_asset.IsLoaded() ||
      font_asset.Model().path_origin == FontPathOrigin::kLocal) {
    // Font was loaded or system font will be used, there is no resource need to
    // be loaded. Return an empty UriInfo.
    return UriInfo{};
  }

  auto uri = font_asset.Model().path;
  auto scheme = ParseUriScheme(uri);
  const auto content_type = UriInfo::ContentType::kFont;
  do {
    if (scheme != UriInfo::Scheme::kUnknown) {
      break;
    }

    // use BASE_URI/fonts/FAMILY_NAME.ttf
    if (font_asset.Model().family.empty()) {
      break;
    }

    uri = ConcatFilePaths(
        ConcatFilePaths(main_uri_info.base_uri, kFontFolderName),
        font_asset.Model().family + kTrueType);
    scheme = ParseUriScheme(uri);
  } while (false);

  if (scheme != UriInfo::Scheme::kUnknown) {
    return UriInfo{
        .scheme = scheme, .content_type = content_type, .uri = std::move(uri)};
  } else {
    return UriInfo{};
  }
}

UriInfo UriInfoFromImageAsset(const MainResourceUriInfo& main_uri_info,
                              const ImageAsset& image_asset) {
  if (image_asset.IsLoaded()) {
    return UriInfo{};
  }

  auto uri = image_asset.Model().file_name;
  auto scheme = ParseUriScheme(uri);
  const auto content_type = UriInfo::ContentType::kImage;
  do {
    if (scheme != UriInfo::Scheme::kUnknown) {
      break;
    }

    if (IsPolyfillUri(uri)) {
      auto it = main_uri_info.polyfill.find(image_asset.Model().id);
      if (main_uri_info.polyfill.end() != it) {
        uri = it->second;
        scheme = ParseUriScheme(uri);
      } else {
        break;
      }
    }

    if (scheme != UriInfo::Scheme::kUnknown) {
      break;
    }

    // use image_folder if specificed, which will override
    // image_asset.Model().dir_name
    auto is_image_folder_empty = main_uri_info.image_folder.empty();
    const auto dir_name = is_image_folder_empty ? image_asset.Model().dir_name
                                                : main_uri_info.image_folder;
    uri = ConcatFilePaths(dir_name, uri);

    // If image folder is specified and the base uri is asset scheme, use asset
    // scheme for the uri.
    if (!is_image_folder_empty &&
        ParseUriScheme(main_uri_info.base_uri) == UriInfo::Scheme::kAsset) {
      uri = ConcatFilePaths(kAsset.slash, uri);
    }

    scheme = ParseUriScheme(uri);
    if (scheme != UriInfo::Scheme::kUnknown) {
      break;
    }

    uri = ConcatFilePaths(main_uri_info.base_uri, uri);
    scheme = ParseUriScheme(uri);
  } while (false);

  if (scheme != UriInfo::Scheme::kUnknown) {
    return UriInfo{
        .scheme = scheme, .content_type = content_type, .uri = std::move(uri)};
  } else {
    return UriInfo{};
  }
}

UriInfo UriInfoFromVideoAsset(const MainResourceUriInfo& main_uri_info,
                              const VideoAsset& video_asset) {
  // The video asset has stored valid decode result, means the video has been
  // loaded.
  if (video_asset.IsValid()) {
    return UriInfo{};
  }

  auto uri = video_asset.Model().file_name;
  auto scheme = ParseUriScheme(uri);
  const auto content_type = UriInfo::ContentType::kVideo;
  do {
    if (scheme != UriInfo::Scheme::kUnknown) {
      break;
    }

    uri = ConcatFilePaths(video_asset.Model().dir_name, uri);
    scheme = ParseUriScheme(uri);
    if (scheme != UriInfo::Scheme::kUnknown) {
      break;
    }

    uri = ConcatFilePaths(main_uri_info.base_uri, uri);
    scheme = ParseUriScheme(uri);
  } while (false);

  if (scheme != UriInfo::Scheme::kUnknown) {
    return UriInfo{
        .scheme = scheme, .content_type = content_type, .uri = std::move(uri)};
  } else {
    return UriInfo{};
  }
}

UriInfo UriInfoFromAudioAsset(const MainResourceUriInfo& main_uri_info,
                              const AudioAsset& audio_asset) {
  auto uri = audio_asset.Model().file_name;
  auto scheme = ParseUriScheme(uri);
  const auto content_type = UriInfo::ContentType::kAudio;
  do {
    if (scheme != UriInfo::Scheme::kUnknown) {
      break;
    }

    uri = ConcatFilePaths(audio_asset.Model().dir_name, uri);
    scheme = ParseUriScheme(uri);
    if (scheme != UriInfo::Scheme::kUnknown) {
      break;
    }

    uri = ConcatFilePaths(main_uri_info.base_uri, uri);
    scheme = ParseUriScheme(uri);
  } while (false);

  if (scheme != UriInfo::Scheme::kUnknown) {
    return UriInfo{
        .scheme = scheme, .content_type = content_type, .uri = std::move(uri)};
  } else {
    return UriInfo{};
  }
}

bool IsRegularFileExists(const std::string& filePath) {
#ifdef OS_WIN
  DWORD attr = ::GetFileAttributesA(filePath.c_str());
  return (attr != INVALID_FILE_ATTRIBUTES) &&
         ((attr & FILE_ATTRIBUTE_DIRECTORY) == 0);
#else
  struct stat sb;
  return (stat(filePath.c_str(), &sb) == 0) && S_ISREG(sb.st_mode);
#endif
}

std::string FindFirstMatchingFile(const std::string& directoryPath,
                                  const std::string& fileExtension) {
#ifdef OS_WIN
  std::string searchPath = ConcatFilePaths(directoryPath, "*.*");

  WIN32_FIND_DATAA findData;
  HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);

  if (hFind == INVALID_HANDLE_VALUE) {
    return "";
  }

  try {
    do {
      if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        continue;
      }

      std::string filename(findData.cFileName);

      if (filename.size() > fileExtension.size() &&
          filename.substr(filename.size() - fileExtension.size()) ==
              fileExtension) {
        FindClose(hFind);
        return ConcatFilePaths(directoryPath, filename);
      }
    } while (FindNextFileA(hFind, &findData) != 0);

    DWORD lastError = GetLastError();
    if (lastError != ERROR_NO_MORE_FILES) {
    }
  } catch (...) {
    FindClose(hFind);
    throw;
  }
  FindClose(hFind);
#else
  DIR* dir = opendir(directoryPath.c_str());

  if (dir != NULL) {
    struct dirent* dp;
    while ((dp = readdir(dir)) != NULL) {
      std::string filename(dp->d_name);
      if (filename.size() > fileExtension.size() &&
          filename.substr(filename.size() - fileExtension.size()) ==
              fileExtension) {
        closedir(dir);
        return ConcatFilePaths(directoryPath, filename);
      }
    }
    closedir(dir);
  }
#endif
  return "";
}

std::string GetUriLocalPath(const std::string& uri) {
  if (base::BeginsWith(uri, kFile.slash)) {
    return uri.substr(strlen(kFile.slash) - 1);
  } else if (base::BeginsWith(uri, kAsset.slash)) {
    return uri.substr(strlen(kAsset.slash) - 1);
  }
  return uri;
}

bool IsAlphaVideoDirectory(const std::string& path) {
  auto actual_path = GetUriLocalPath(path);
  auto config_path = ConcatFilePaths(actual_path, kConfigJSONFileName);
  return IsRegularFileExists(config_path);
}
}  // namespace animax
}  // namespace lynx
