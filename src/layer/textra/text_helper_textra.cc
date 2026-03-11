// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/layer/textra/text_helper_textra.h"

#include "base/include/no_destructor.h"
#include "src/base/log/log.h"
#include "src/layer/textra/font_asset_manager_textra.h"
#include "src/layer/textra/text_content_textra.h"

namespace lynx {
namespace animax {

TextHelperTextra::TextHelperTextra() { ANIMAX_LOGI("use textra text impl"); }

std::unique_ptr<TextContent> TextHelperTextra::CreateTextContent(
    const TextContentDataSource& data_source) {
  return std::make_unique<TextContentTextra>(data_source);
}

std::unique_ptr<FontAssetManager> TextHelperTextra::CreateFontAssetManager() {
  return std::make_unique<FontAssetManagerTextra>();
}

}  // namespace animax
}  // namespace lynx
