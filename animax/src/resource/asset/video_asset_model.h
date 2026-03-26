// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_ASSET_VIDEO_ASSET_MODEL_H_
#define ANIMAX_SRC_RESOURCE_ASSET_VIDEO_ASSET_MODEL_H_

#include <array>
#include <string>

namespace lynx {
namespace animax {

struct VideoAssetModel {
  /**
   *  {
          "id": "video_0",
          "x": 0,
          "y": 574,
          "w": 750,
          "h": 574,
          "ax": 0,
          "ay": 0,
          "aw": 750,
          "ah": 574,
          "u": "https://path/",
          "p": "video.mp4",
      }
   */

  std::array<int32_t, 4> rgb_frame = {0, 0, 0, 0};
  std::array<int32_t, 4> a_frame = {0, 0, 0, 0};
  std::string id{};         // id
  std::string dir_name{};   // u
  std::string file_name{};  // p
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_ASSET_VIDEO_ASSET_MODEL_H_
