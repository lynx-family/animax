// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#ifndef ANIMAX_SRC_BASE_MONITOR_TRACE_EVENT_DEF_H_
#define ANIMAX_SRC_BASE_MONITOR_TRACE_EVENT_DEF_H_

namespace lynx {
namespace animax {

inline constexpr const char* const kAnimaXTraceCategory = "animax";
namespace trace_event {

inline constexpr const char* const kRequestComposition = "RequestComposition";
inline constexpr const char* const kParseComposition = "ParseComposition";
inline constexpr const char* const kPrepareAssets = "PrepareAssets";
inline constexpr const char* const kPrepareSingleAsset = "PrepareSingleAsset";
inline constexpr const char* const kPrepareImageTexture = "PrepareImageTexture";
inline constexpr const char* const kPrepareAlphaVideoFrameData =
    "PrepareAlphaVideoFrameData";
inline constexpr const char* const kCreateSurface = "CreateSurface";
inline constexpr const char* const kRenderFrame = "RenderFrame";
inline constexpr const char* const kInterpolateFrame = "InterpolateFrame";
inline constexpr const char* const kDrawFrame = "DrawFrame";
inline constexpr const char* const kFlushFrame = "FlushFrame";
inline constexpr const char* const kSwapFrame = "SwapFrame";

inline constexpr const char* const kRequestCompositionStart =
    "RequestCompositionStart";
inline constexpr const char* const kRequestCompositionEnd =
    "RequestCompositionEnd";
inline constexpr const char* const kPrepareAssetsStart = "PrepareAssetsStart";
inline constexpr const char* const kPrepareAssetsEnd = "PrepareAssetsEnd";
inline constexpr const char* const kPrepareSingleAssetStart =
    "PrepareSingleAssetStart";
inline constexpr const char* const kPrepareSingleAssetEnd =
    "PrepareSingleAssetEnd";

}  // namespace trace_event

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_BASE_MONITOR_TRACE_EVENT_DEF_H_
