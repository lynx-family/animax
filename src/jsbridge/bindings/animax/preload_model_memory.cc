// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/jsbridge/bindings/animax/preload_model_memory.h"

#include <string>
#include <vector>

#include "src/model/composition_model.h"
#include "src/model/layer_model.h"
#include "src/model/text/font_character_model.h"
#include "src/model/text/text_gradient_model.h"

namespace lynx {
namespace animax {
namespace {

int64_t StringStorage(const std::string& value) {
  const auto begin = reinterpret_cast<uintptr_t>(&value);
  const auto data = reinterpret_cast<uintptr_t>(value.data());
  // Inline short strings are already included in their owning object. This
  // checks the actual data address rather than depending on an STL SSO limit.
  if ((data >= begin && data < begin + sizeof(value)) || !value.capacity()) {
    return 0;
  }
  return static_cast<int64_t>(value.capacity()) + 1;
}

template <typename T>
int64_t VectorStorage(const std::vector<T>& values) {
  return static_cast<int64_t>(values.capacity()) * sizeof(T);
}

template <typename Map>
int64_t MapStorage(const Map& values) {
  // Count entry values and bucket pointers; STL-specific node bookkeeping is
  // excluded. Empty default maps can use a static single-bucket sentinel.
  return static_cast<int64_t>(values.size()) *
             sizeof(typename Map::value_type) +
         (values.bucket_count() > 1
              ? static_cast<int64_t>(values.bucket_count()) * sizeof(void*)
              : 0);
}

template <typename T>
int64_t ObjectStorage(const T* value) {
  return value ? sizeof(T) : 0;
}

int64_t DocumentStorage(const Value* value) {
  if (!value || value->GetValueType() != ValueType::kDocument) return 0;
  const auto& document = static_cast<const DocumentData&>(*value);
  return sizeof(DocumentData) + StringStorage(document.GetText()) +
         StringStorage(document.GetFontName()) +
         ObjectStorage(document.GetBoxPosition()) +
         ObjectStorage(document.GetBoxSize());
}

int64_t TextStorage(AnimatableTextFrame* text) {
  if (!text) return 0;
  const auto& frames = text->GetKeyframes();
  int64_t bytes = sizeof(AnimatableTextFrame) + sizeof(KeyframeModelList) +
                  VectorStorage(frames);
  for (const auto& frame : frames) {
    if (!frame) continue;
    // Keyframes uniquely own both values, even when the text is identical.
    bytes += sizeof(KeyframeModel) +
             DocumentStorage(frame->GetStartValue<Value>()) +
             DocumentStorage(frame->GetEndValue<Value>());
  }
  return bytes;
}

int64_t TextGradientStorage(const TextGradientModel* gradient) {
  if (!gradient) return 0;
  const auto& items = gradient->GetItems();
  int64_t bytes = sizeof(TextGradientModel) + VectorStorage(items);
  for (const auto& item : items) {
    if (!item) continue;
    // The parser only creates items with a start point and gradient color.
    const auto& color = item->GetGradientColor();
    const int64_t stops = color.GetSize() > 0 ? color.GetSize() : 0;
    bytes += sizeof(TextGradientItemModel) + sizeof(PointF) +
             ObjectStorage(item->GetEndPoint()) +
             ObjectStorage(item->GetRadiusXY()) + sizeof(GradientColor) +
             (color.GetPositions() ? stops * sizeof(float) : 0) +
             (color.GetColors() ? stops * sizeof(int32_t) : 0);
  }
  return bytes;
}

int64_t LayerStorage(LayerModelList& layers) {
  int64_t bytes = VectorStorage(layers);
  for (const auto& layer : layers) {
    if (!layer) {
      continue;
    }
    bytes += sizeof(LayerModel) + StringStorage(layer->GetName()) +
             StringStorage(layer->GetRefId()) +
             VectorStorage(layer->GetShapes()) +
             VectorStorage(layer->GetMasks());
    for (const auto& mask : layer->GetMasks()) {
      bytes += ObjectStorage(mask.get());
    }
    auto frames = layer->GetSharedInOutFrames();
    if (frames) {
      bytes += sizeof(KeyframeModelList) + VectorStorage(*frames);
      for (const auto& frame : *frames) {
        bytes += ObjectStorage(frame.get());
      }
    }
    bytes += ObjectStorage(layer->GetTransform()) +
             TextStorage(layer->GetText()) +
             TextGradientStorage(layer->GetTextGradientModel()) +
             ObjectStorage(layer->GetTimeRemapping()) +
             ObjectStorage(layer->GetBlurEffect()) +
             ObjectStorage(layer->GetDropEffect()) +
             ObjectStorage(layer->GetPerspective());
    if (const auto* properties = layer->GetTextPropertiesList()) {
      bytes += sizeof(AnimatableTextPropertyList) + VectorStorage(*properties);
      for (const auto& property : *properties) {
        bytes += ObjectStorage(property.get());
      }
    }
  }
  return bytes;
}

template <typename Map>
int64_t MediaMetadataStorage(const Map& assets) {
  int64_t bytes = MapStorage(assets);
  for (const auto& [id, asset] : assets) {
    bytes += StringStorage(id);
    if (asset) {
      bytes += EstimatePreloadedAssetBytes(*asset);
    }
  }
  return bytes;
}

}  // namespace

int64_t EstimatePreloadedAssetBytes(const ImageAsset& asset) {
  const auto& metadata = asset.Model();
  return sizeof(ImageAsset) + StringStorage(metadata.id) +
         StringStorage(metadata.dir_name) + StringStorage(metadata.file_name);
}

int64_t EstimatePreloadedImageFileNameBytes(const ImageAsset& asset) {
  return StringStorage(asset.Model().file_name);
}

int64_t EstimatePreloadedAssetBytes(const FontAsset& asset) {
  const auto& metadata = asset.Model();
  return sizeof(FontAsset) + StringStorage(metadata.family) +
         StringStorage(metadata.name) + StringStorage(metadata.style) +
         StringStorage(metadata.path);
}

int64_t EstimatePreloadedAssetBytes(const AudioAsset& asset) {
  return asset.GetMemoryUsageBytes();
}

int64_t EstimatePreloadedAssetBytes(const VideoAsset& asset) {
  return asset.GetMemoryUsageBytes();
}

int64_t EstimatePreloadedModelBytes(CompositionModel& model) {
  int64_t bytes = sizeof(CompositionModel) + sizeof(RectF) +
                  sizeof(FontAssetManager) + LayerStorage(model.GetLayers()) +
                  MapStorage(model.GetLayerMap()) +
                  MapStorage(model.GetPrecomps());
  // Precomp layers are owned here. layer_map_ only holds borrowed pointers,
  // so its entries are counted above without counting their targets again.
  for (auto& [id, layers] : model.GetPrecomps()) {
    bytes += StringStorage(id) + LayerStorage(layers);
  }
  bytes += MediaMetadataStorage(model.GetImages()) +
           MediaMetadataStorage(model.GetAudios()) +
           MediaMetadataStorage(model.GetVideos());
  bytes += MapStorage(model.GetFonts());
  for (const auto& [id, asset] : model.GetFonts()) {
    bytes += StringStorage(id);
    if (asset) {
      bytes += EstimatePreloadedAssetBytes(*asset);
    }
  }
  bytes += MapStorage(model.GetCharacters());
  for (const auto& [id, character] : model.GetCharacters()) {
    if (character) {
      bytes +=
          sizeof(FontCharacterModel) + VectorStorage(character->GetShapes());
    }
  }
  bytes += VectorStorage(model.GetMarkers());
  for (const auto& marker : model.GetMarkers()) {
    if (marker) {
      bytes += sizeof(MarkerModel) + StringStorage(marker->name_);
    }
  }
  return bytes;
}

}  // namespace animax
}  // namespace lynx
