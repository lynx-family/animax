// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_ASSET_FONT_ASSET_H_
#define ANIMAX_SRC_RESOURCE_ASSET_FONT_ASSET_H_

#include <cstdint>
#include <memory>
#include <mutex>
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
   * Publishes resource data without replacing the GPU-owned Font. GetFont()
   * rebuilds it lazily when the data revision changes. Empty data clears the
   * resource data.
   * @param raw_data Unique pointer to the raw data of the font.
   */
  void SetRawData(std::unique_ptr<RawData> raw_data);

  /**
   * Determines whether the font is loaded.
   * External fonts require nonempty resource data. A system-font fallback
   * created before that data arrives must not suppress the resource request.
   *
   * @return true if the font is loaded; false otherwise.
   */
  bool IsLoaded() const;

  // Safe to observe from the resource and GPU threads.
  ANIMAX_EXPORT uint64_t GetDataRevision() const;

  /**
   * Retrieves the Font object associated with this asset.
   * Loads or refreshes the Font on the GPU thread. SetRawData() does not
   * invalidate the returned pointer, but a later GetFont() or ResetModel() may.
   * Use GetFontSnapshot() to retain it across refreshes.
   *
   * @return Pointer to the Font object.
   */
  ANIMAX_EXPORT Font* GetFont();

  // Keeps one GPU-owned font alive throughout drawing or measurement, even if
  // a later GetFont() refreshes the asset after a resource update.
  ANIMAX_EXPORT std::shared_ptr<Font> GetFontSnapshot();

 private:
  FontAssetModel model_{};
  mutable std::mutex data_mutex_;
  std::unique_ptr<RawData> raw_data_ = nullptr;
  uint64_t data_revision_ = 0;
  uint64_t font_data_revision_ = 0;
  std::shared_ptr<Font> font_ = nullptr;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_ASSET_FONT_ASSET_H_
