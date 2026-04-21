// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_INCLUDE_RESOURCE_RESOURCE_TASK_H_
#define ANIMAX_INCLUDE_RESOURCE_RESOURCE_TASK_H_

#include "include/resource/bitmap.h"
#include "include/resource/raw_data.h"
#include "include/resource/uri_info.h"
namespace lynx {
namespace animax {

enum class ResourceRequestType {
  kInvalid = 0,
  kLoadRawData,
  kLoadBitmap,
  kLoadPosterBitmap,
  kDownloadToLocal,
};

struct ResourceRequest {
  ResourceRequestType type = ResourceRequestType::kInvalid;
  UriInfo uri_info{};
  int32_t width = 0;
  int32_t height = 0;
  std::string id{};
  std::string main_uri{};
};

enum class ResourcePayloadType {
  kRawData = 0,
  kBitmap,
  kPosterBitmap,
  kFilePath,
  kEnumCount,
  kInvalid = kEnumCount,
};

struct ResourcePayload {
  ResourcePayloadType type = ResourcePayloadType::kInvalid;
  std::string path{};
  std::unique_ptr<Bitmap> bitmap{};
  std::unique_ptr<RawData> raw_data{};
};

inline ResourcePayload MakePathResourcePayload(std::string path) {
  return ResourcePayload{.type = ResourcePayloadType::kFilePath,
                         .path = std::move(path)};
}

inline ResourcePayload MakeBitmapResourcePayload(
    std::unique_ptr<Bitmap> bitmap) {
  return ResourcePayload{.type = ResourcePayloadType::kBitmap,
                         .bitmap = std::move(bitmap)};
}

inline ResourcePayload MakeRawDataResourcePayload(
    std::unique_ptr<RawData> raw_data) {
  return ResourcePayload{
      .type = ResourcePayloadType::kRawData,
      .raw_data = std::move(raw_data),
  };
}

struct ResourceResponse {
  ResourcePayload payload{};
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_INCLUDE_RESOURCE_RESOURCE_TASK_H_
