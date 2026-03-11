// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_INCLUDE_RESOURCE_URI_INFO_H_
#define ANIMAX_INCLUDE_RESOURCE_URI_INFO_H_

#include <string>
#include <unordered_map>

namespace lynx {
namespace animax {

/**
 * The UriInfo struct represents the information parsed from a given URI,
 * including its scheme and content type. It provides methods to parse and
 * extract this information from a URI string.
 */
struct UriInfo {
  /**
   * Enum representing the content type of the resource.
   */
  enum class ContentType {
    kJson = 0,  // The content type is a JSON file.
    kZip,       // The content type is a ZIP file.
    kFont,      // The content type is a font file.
    kImage,     // The content type is an image file.
    kVideo,     // The content type is a video file.
    kAudio,     // The content type is a audio file.
    kEnumCount,
    kUnknown = kEnumCount,  // The content type is unknown.
  };

  /**
   * Enum representing the scheme of the URI.
   */
  enum class Scheme {
    kAsset = 0,  // The resource is an asset.
    kFile,       // The resource is a local file.
    kHttp,       // The resource is accessed via HTTP.
    kDataURL,    // The resource is a Data URL.
    kEnumCount,
    kUnknown = kEnumCount,  // The scheme of the URI is unknown.
  };

  /**
   * Check if the UriInfo is valid.
   * A valid UriInfo has non-empty uri, both content_type and scheme are not
   * unknown.
   */
  bool Valid() const {
    return scheme != UriInfo::Scheme::kUnknown &&
           content_type != UriInfo::ContentType::kUnknown && !uri.empty();
  }

  /**
   * The parsed scheme of the URI.
   */
  Scheme scheme = Scheme::kUnknown;

  /**
   * The content type of the resource.
   */
  ContentType content_type = ContentType::kUnknown;

  /**
   * The original URI string.
   */
  std::string uri{};
};

struct MainResourceUriInfo {
  /**
   * A base URI extract from the uri. This is used when the asset path is a
   * relative path. The abosulte path to the asset will be derived by the base
   * URI and the asset path.
   */
  std::string base_uri{};

  /**
   * The directory of images associated with the resources.
   * This folder contains image files that are referenced by the resources.
   */
  std::string image_folder{};

  /**
   * A mapping from asset names to their full resource URLs.
   * This is used to replace placeholder strings with actual URLs in the loading
   * process.
   */
  std::unordered_map<std::string, std::string> polyfill{};

  /**
   * The URI for the main resource (Lottie JSON or AlhpaVideo zip)
   */
  std::string main_uri{};
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_INCLUDE_RESOURCE_URI_INFO_H_
