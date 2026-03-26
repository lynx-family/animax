// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include "src/resource/log_util.h"

#include "include/resource/loader_error.h"
#include "src/resource/asset/asset.h"
#include "src/resource/asset/audio_asset.h"
#include "src/resource/asset/font_asset.h"
#include "src/resource/asset/image_asset.h"
#include "src/resource/asset/video_asset.h"
#include "src/resource/composition_loader/composition_asset_task.h"

namespace lynx {
namespace animax {

namespace {

constexpr std::size_t kMaxFileNameLength = 64;

std::string TruncateString(const std::string& str, std::size_t max_length) {
  return str.length() > max_length ? str.substr(0, max_length) + "..." : str;
}

class AssetToStreamConverter : public AssetVisitor {
 public:
  AssetToStreamConverter(std::ostream& os, bool verbose)
      : os_{os}, verbose_{verbose} {}

  void Visit(FontAsset& asset) override {
    os_ << "{ ";
    os_ << "type: FontAsset, ";
    os_ << "family: " << asset.Model().family << ", ";
    os_ << "name: " << asset.Model().name;

    if (verbose_) {
      os_ << ", style: " << asset.Model().style;
      os_ << ", ascent: " << asset.Model().ascent;
      os_ << ", path: "
          << TruncateString(asset.Model().path, kMaxFileNameLength);
      os_ << ", path_origin: " << static_cast<int>(asset.Model().path_origin);
    }

    os_ << ", is_loaded: " << (asset.IsLoaded() ? "true" : "false");
    os_ << " }";
  }

  void Visit(ImageAsset& asset) override {
    os_ << "{ ";
    os_ << "type: ImageAsset, ";
    os_ << "id: " << asset.Model().id;

    if (verbose_) {
      os_ << ", width: " << asset.Model().width;
      os_ << ", height: " << asset.Model().height;
      os_ << ", dir_name: "
          << TruncateString(asset.Model().dir_name, kMaxFileNameLength);
      os_ << ", file_name: "
          << TruncateString(asset.Model().file_name, kMaxFileNameLength);
    }

    os_ << ", is_loaded: " << (asset.IsLoaded() ? "true" : "false");
    os_ << " }";
  }

  void Visit(VideoAsset& asset) override {
    os_ << "{ ";
    os_ << "type: VideoAsset, ";
    os_ << "id: " << asset.Model().id;

    if (verbose_) {
      os_ << ", dir_name: "
          << TruncateString(asset.Model().dir_name, kMaxFileNameLength);
      os_ << ", file_name: "
          << TruncateString(asset.Model().file_name, kMaxFileNameLength);
    }

    os_ << ", is_valid: " << (asset.IsValid() ? "true" : "false");
    os_ << " }";
  }

  void Visit(AudioAsset& asset) override {
    os_ << "{ ";
    os_ << "type: AudioAsset, ";
    os_ << "id: " << asset.Model().id;

    if (verbose_) {
      os_ << ", dir_name: "
          << TruncateString(asset.Model().dir_name, kMaxFileNameLength);
      os_ << ", file_name: "
          << TruncateString(asset.Model().file_name, kMaxFileNameLength);
    }

    os_ << " }";
  }

 private:
  std::ostream& os_;
  bool verbose_;
};

}  // namespace

std::ostream& operator<<(std::ostream& os, Asset& asset) {
  AssetToStreamConverter converter{os, /*verbose=*/false};
  asset.AcceptVisitor(converter);
  return os;
}

std::ostream& operator<<(std::ostream& os, const AssetVerboseWrapper& wrapper) {
  AssetToStreamConverter converter{os, /*verbose=*/true};
  if (wrapper.asset_) {
    wrapper.asset_->AcceptVisitor(converter);
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const LoaderError& error) {
  os << "{ code: " << error.code << ", message: " << error.message << " }";
  return os;
}

std::string ResourceTypeToString(ResourceType type) {
  switch (type) {
    case ResourceType::kUnknown:
      return "Unknown";
    case ResourceType::kImage:
      return "Image";
    case ResourceType::kFont:
      return "Font";
    case ResourceType::kVideo:
      return "Video";
    default:
      return "Invalid";
  }
}

std::ostream& operator<<(std::ostream& os, const ResourceType& type) {
  os << ResourceTypeToString(type);
  return os;
}

std::ostream& operator<<(std::ostream& os, const AssetResponse& response) {
  os << "{ type: " << response.type << ", id: " << response.id
     << ", error: " << response.error << " }";
  return os;
}

std::ostream& operator<<(std::ostream& os,
                         const CompositionAssetResponse& res) {
  os << "CompositionAssetLoader finished loading " << res.asset_responses.size()
     << " assets.\n";

  bool has_failed = false;
  for (const auto& response : res.asset_responses) {
    if (response.error.code == kSuccess) {
      continue;
    }
    if (!has_failed) {
      os << " failed_assets = [";
      os << response;
      has_failed = true;
    } else {
      os << ", " << response;
    }
  }
  if (has_failed) {
    os << "]";
  }

  return os;
}

}  // namespace animax
}  // namespace lynx
