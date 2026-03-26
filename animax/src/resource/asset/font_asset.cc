// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/asset/font_asset.h"

#include "src/render/font.h"

namespace lynx {
namespace animax {

void FontAsset::SetRawData(std::unique_ptr<RawData> raw_data) {
  raw_data_ = std::move(raw_data);
}

bool FontAsset::IsLoaded() const { return font_ || raw_data_; }

Font* FontAsset::GetFont() {
  if (font_) {
    return font_.get();
  }

  if (raw_data_) {
    font_ =
        Font::MakeFont(raw_data_->Data(), raw_data_->Length(), Model().style);

  } else {
    font_ = Font::MakeFromName(Model().family, Model().style);
  }

  return font_.get();
}

void FontAsset::ResetModel(FontAssetModel info) {
  model_ = std::move(info);
  raw_data_ = nullptr;
  font_ = nullptr;
}

}  // namespace animax
}  // namespace lynx
