// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_ASSET_AUDIO_ASSET_MODEL_H_
#define ANIMAX_SRC_RESOURCE_ASSET_AUDIO_ASSET_MODEL_H_

#include <string>

namespace lynx {
namespace animax {

struct AudioAssetModel {
  /**
   *  {
          "id": "audio_0",
          "u": "images/",
          "p": "aud_0.mp3", # do not support src-polyfill.
          "e": 0
      }
   */

  std::string id;         // id
  std::string dir_name;   // u
  std::string file_name;  // p
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_ASSET_AUDIO_ASSET_MODEL_H_
