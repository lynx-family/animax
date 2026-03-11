// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_LAYER_TEXT_HELPER_H_
#define ANIMAX_SRC_LAYER_TEXT_HELPER_H_

#include <memory>

#include "include/base/macros.h"
#include "src/layer/font_asset_manager.h"
#include "src/layer/text_content.h"
#include "src/layer/text_content_data_source.h"

namespace lynx {
namespace animax {

class ANIMAX_EXPORT TextHelper {
 public:
  static TextHelper& Impl();
  static void RegisterImpl(TextHelper* impl);

  TextHelper();
  virtual ~TextHelper() = default;

  virtual std::unique_ptr<TextContent> CreateTextContent(
      const TextContentDataSource& data_source);

  virtual std::unique_ptr<FontAssetManager> CreateFontAssetManager();

 private:
  static TextHelper*& ImplRef();
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_LAYER_TEXT_HELPER_H_
