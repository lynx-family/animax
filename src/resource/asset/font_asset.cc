// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/asset/font_asset.h"

#include "src/render/font.h"

namespace lynx {
namespace animax {

void FontAsset::SetRawData(std::unique_ptr<RawData> raw_data) {
  std::lock_guard<std::mutex> lock(data_mutex_);
  if (raw_data && (!raw_data->Data() || raw_data->Length() == 0)) {
    raw_data.reset();
  }
  raw_data_ = std::move(raw_data);
  ++data_revision_;
}

bool FontAsset::IsLoaded() const {
  std::lock_guard<std::mutex> lock(data_mutex_);
  return raw_data_ || (model_.path_origin == FontPathOrigin::kLocal && font_);
}

uint64_t FontAsset::GetDataRevision() const {
  std::lock_guard<std::mutex> lock(data_mutex_);
  return data_revision_;
}

Font* FontAsset::GetFont() { return GetFontSnapshot().get(); }

std::shared_ptr<Font> FontAsset::GetFontSnapshot() {
  std::lock_guard<std::mutex> lock(data_mutex_);
  if (font_ && font_data_revision_ == data_revision_) {
    return font_;
  }

  if (raw_data_) {
    font_ =
        Font::MakeFont(raw_data_->Data(), raw_data_->Length(), Model().style);

  } else {
    font_ = Font::MakeFromName(Model().family, Model().style);
  }
  font_data_revision_ = data_revision_;

  return font_;
}

void FontAsset::ResetModel(FontAssetModel info) {
  std::lock_guard<std::mutex> lock(data_mutex_);
  model_ = std::move(info);
  raw_data_ = nullptr;
  font_ = nullptr;
  ++data_revision_;
}

}  // namespace animax
}  // namespace lynx
