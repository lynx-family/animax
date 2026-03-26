// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_ASSET_IMAGE_ASSET_MODEL_H_
#define ANIMAX_SRC_RESOURCE_ASSET_IMAGE_ASSET_MODEL_H_

#include <string>

namespace lynx {
namespace animax {

struct ImageAssetModel {
  /**
   *  {
          "id": "image_0",
          "w": 132,
          "h": 745,
          "u": "images/",
          "p": "img_0.png", # if use src-polyfill, set this to %s
          "e": 0
      }
   */

  int32_t width = 0;        // w
  int32_t height = 0;       // h
  std::string id{};         // id
  std::string dir_name{};   // u
  std::string file_name{};  // p
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_ASSET_IMAGE_ASSET_MODEL_H_
