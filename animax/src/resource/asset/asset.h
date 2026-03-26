// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_ASSET_ASSET_H_
#define ANIMAX_SRC_RESOURCE_ASSET_ASSET_H_

namespace lynx {
namespace animax {

enum class ResourceType {
  kUnknown = 0,
  kImage,
  kFont,
  kVideo,
  kAudio,
};

class FontAsset;
class ImageAsset;
class VideoAsset;
class AudioAsset;
class AssetVisitor;

/**
 * Abstract base class for an AssetVisitor that defines a visitor interface
 * for different types of assets. This is used here to perform operations across
 * a set of Asset classes without modifying the classes themselves.
 */
class AssetVisitor {
 public:
  virtual ~AssetVisitor() = default;
  virtual void Visit(FontAsset& asset) = 0;
  virtual void Visit(ImageAsset& asset) = 0;
  virtual void Visit(VideoAsset& asset) = 0;
  virtual void Visit(AudioAsset& asset) = 0;
};

class Asset {
 public:
  virtual ~Asset() = default;

  /**
   * Accepts a visitor that will perform operations on this Asset. The specific
   * visitor method to be called is determined by the type of the Asset.
   *
   * @param visitor Reference to the AssetVisitor that will operate on this
   * Asset.
   */
  virtual void AcceptVisitor(AssetVisitor& visitor) = 0;
};

template <typename T>
class BaseAsset : public Asset {
 public:
  ~BaseAsset() override = default;
  void AcceptVisitor(AssetVisitor& visitor) override {
    visitor.Visit(*static_cast<T*>(this));
  }
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_ASSET_ASSET_H_
