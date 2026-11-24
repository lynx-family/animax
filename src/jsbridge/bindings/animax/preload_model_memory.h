// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_JSBRIDGE_BINDINGS_ANIMAX_PRELOAD_MODEL_MEMORY_H_
#define ANIMAX_SRC_JSBRIDGE_BINDINGS_ANIMAX_PRELOAD_MODEL_MEMORY_H_

#include <cstdint>

namespace lynx {
namespace animax {

class CompositionModel;
class ImageAsset;
class FontAsset;
class AudioAsset;
class VideoAsset;

// Estimate observable model storage, including text keyframes and their
// document strings, and prepared audio/video CPU data. Image bitmaps are
// accounted separately by the unbound Composition wrapper. This is not a
// complete heap size: private/polymorphic shape, other animated-value and
// glyph-string allocations, platform decoder allocations,
// allocator/control-block overhead and GPU data are not included.
int64_t EstimatePreloadedModelBytes(CompositionModel& model);

// The asset contribution included above, excluding separately tracked image
// pixels and font RawData. Used to move each charge to its actual owner.
int64_t EstimatePreloadedAssetBytes(const ImageAsset& asset);
int64_t EstimatePreloadedAssetBytes(const FontAsset& asset);
int64_t EstimatePreloadedAssetBytes(const AudioAsset& asset);
int64_t EstimatePreloadedAssetBytes(const VideoAsset& asset);

// Included in the ImageAsset estimate, but released by GetImage before the
// asset itself. Uses the same heap-string calculation as the model estimate.
int64_t EstimatePreloadedImageFileNameBytes(const ImageAsset& asset);

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_JSBRIDGE_BINDINGS_ANIMAX_PRELOAD_MODEL_MEMORY_H_
