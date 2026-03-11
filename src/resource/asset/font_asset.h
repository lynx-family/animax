// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_ASSET_FONT_ASSET_H_
#define ANIMAX_SRC_RESOURCE_ASSET_FONT_ASSET_H_

#include <memory>
#include <string>

#include "include/base/macros.h"
#include "include/resource/raw_data.h"
#include "src/resource/asset/asset.h"
#include "src/resource/asset/font_asset_model.h"

namespace lynx {
namespace animax {

enum class FontPathOrigin : uint8_t;
class Font;

class FontAsset : public BaseAsset<FontAsset> {
 public:
  /**
   * Constructs a default FontAsset.
   */
  FontAsset() = default;

  /**
   * Constructs a FontAsset with a specified FontAssetModel.
   *
   * @param model FontAssetModel used to initialize the FontAsset.
   */
  explicit FontAsset(FontAssetModel model) : model_{std::move(model)} {}

  ~FontAsset() override = default;

  /**
   * Retrieves the current FontAssetModel.
   *
   * @return Constant reference to the FontAssetModel.
   */
  const FontAssetModel& Model() const { return model_; }

  /**
   * Replaces the current FontAssetModel with a new model and unloads the
   * associated font.
   *
   * @param info New FontAssetModel to replace the current one.
   */
  void ResetModel(FontAssetModel info);

  /**
   * Sets the raw data for the font.
   * If the font is never loaded, call GetFont() will load the font with the raw
   * data.
   * @param raw_data Unique pointer to the raw data of the font.
   */
  void SetRawData(std::unique_ptr<RawData> raw_data);

  /**
   * Determines whether the font is loaded.
   * A font is considered loaded, either if its raw data is set, or if the font
   * has been loaded.
   *
   * @return true if the font is loaded; false otherwise.
   */
  bool IsLoaded() const;

  /**
   * Retrieves the Font object associated with this asset.
   * Loads the Font if it is not already loaded.
   *
   * @return Pointer to the Font object.
   */
  ANIMAX_EXPORT Font* GetFont();

 private:
  FontAssetModel model_{};
  std::unique_ptr<RawData> raw_data_ = nullptr;
  std::shared_ptr<Font> font_ = nullptr;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_ASSET_FONT_ASSET_H_
