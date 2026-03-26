// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_LAYER_TEXTRA_TEXT_HELPER_TEXTRA_H_
#define ANIMAX_SRC_LAYER_TEXTRA_TEXT_HELPER_TEXTRA_H_

#include "src/layer/text_helper.h"

namespace lynx {
namespace animax {

class TextHelperTextra final : public TextHelper {
 public:
  TextHelperTextra();

  std::unique_ptr<TextContent> CreateTextContent(
      const TextContentDataSource& data_source) override;

  std::unique_ptr<FontAssetManager> CreateFontAssetManager() override;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_LAYER_TEXTRA_TEXT_HELPER_TEXTRA_H_
