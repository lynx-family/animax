// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_INCLUDE_RESOURCE_LOADER_ERROR_H_
#define ANIMAX_INCLUDE_RESOURCE_LOADER_ERROR_H_

#include <string>

namespace lynx {
namespace animax {

enum LoaderErrorCode : uint32_t {
  kSuccess = 0,
  kInvalidLoaderRequest = 100,
  kInvalidLoader = 101,
  kInvalidCallback = 102,
  kInvalidArgument = 103,
  kNoSuchLoader = 104,

  kFileDoesNotExist = 400,
  kFileNoAccess = 401,
  kFileIOFailure = 402,

  kReferToMessage = 500,
  kInvalidImage = 600,
  kInvalidImageData = 601,
  kBitmapError = 602,

  kUserDefined = 700,
  kUnzipFailed = 800,
  kLoaderNotHandleRequest = 900,
  kInvalidRawData = 1000,
  kInvalidFilePath = 1100,
  kCompositionParserError = 1200,
};

struct LoaderError {
  uint32_t code;
  std::string message;

  explicit operator bool() { return code != 0; }
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_INCLUDE_RESOURCE_LOADER_ERROR_H_
