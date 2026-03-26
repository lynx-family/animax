// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_ASSET_IMAGE_ASSET_H_
#define ANIMAX_SRC_RESOURCE_ASSET_IMAGE_ASSET_H_

#include <string>

#include "include/resource/bitmap.h"
#include "src/render/image.h"
#include "src/resource/asset/asset.h"
#include "src/resource/asset/image_asset_model.h"

namespace lynx {
namespace animax {

class RealContext;

class ImageAsset : public BaseAsset<ImageAsset> {
 public:
  /**
   * Constructs an ImageAsset with a specified ImageAssetModel.
   *
   * @param model ImageAssetModel used to initialize the ImageAsset.
   */
  explicit ImageAsset(ImageAssetModel model);

  ~ImageAsset() override = default;

  /**
   * Checks if the image asset has been successfully loaded.
   *
   * @return true either if the bitmap has been set or the Image has been
   * loaded.
   */
  bool IsLoaded() const;

  /**
   * Determines whether the image asset is stored in Base64 format.
   *
   * @return true if the image is in Base64 format; false otherwise.
   */
  bool IsBase64() const;

  /**
   * Checks if the image asset uses polyfill (URI for the image is left as %s).
   *
   * @return true if polyfill is used for this image; false otherwise.
   */
  bool IsPolyfill() const;

  /**
   * Sets the bitmap data for the image asset.
   *
   * @param bitmap Unique pointer to a Bitmap object representing the image
   * data.
   */
  void SetBitmap(std::unique_ptr<Bitmap> bitmap);

  /**
   * Retrieves the Image object using bitmap for rendering in a specified
   * context.
   *
   * @param real_context The context in which the Image will be used.
   * @return Pointer to the Image object, or nullptr if the image cannot be
   * rendered in the given context.
   */
  Image* GetImage(RealContext* real_context);

  /**
   * Retrieves the current ImageAssetModel.
   *
   * @return Constant reference to the ImageAssetModel.
   */
  const ImageAssetModel& Model() const { return model_; }

  /**
   * Resets the ImageAssetModel with new data and unloads any currently loaded
   * image data.
   *
   * @param info New ImageAssetModel to replace the current one.
   */
  void ResetModel(ImageAssetModel info);

 private:
  // Temporary friend class allowed to fetch resources; to be removed after
  // refactoring.
  friend class ImageFetcher;

  ImageAssetModel model_{};
  std::unique_ptr<Bitmap> bitmap_;
  std::unique_ptr<Image> image_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_ASSET_IMAGE_ASSET_H_
