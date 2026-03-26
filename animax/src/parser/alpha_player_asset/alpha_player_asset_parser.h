// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PARSER_ALPHA_PLAYER_ASSET_ALPHA_PLAYER_ASSET_PARSER_H_
#define ANIMAX_SRC_PARSER_ALPHA_PLAYER_ASSET_ALPHA_PLAYER_ASSET_PARSER_H_

#include <memory>

namespace lynx {
namespace animax {
class CompositionModel;

class AlphaPlayerAssetParser final {
 public:
  static std::shared_ptr<CompositionModel> Parse(const std::string &unzip_path,
                                                 const char *config,
                                                 size_t length, float scale,
                                                 bool enable_audio = false);
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PARSER_ALPHA_PLAYER_ASSET_ALPHA_PLAYER_ASSET_PARSER_H_
