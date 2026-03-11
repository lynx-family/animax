// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#ifndef ANIMAX_SRC_BASE_MONITOR_TRACE_EVENT_H_
#define ANIMAX_SRC_BASE_MONITOR_TRACE_EVENT_H_

#include <stdint.h>

#include "base/trace/native/trace_event.h"
#include "src/base/monitor/trace_event_def.h"

namespace lynx {
namespace animax {

enum class TraceEventType : uint8_t {
  kRequestCompositionStart,
  kRequestCompositionEnd,
  kParseCompositionStart,
  kParseCompositionEnd,
  kPrepareAssetsStart,
  kPrepareAssetsEnd,
  kPrepareSingleAssetStart,
  kPrepareSingleAssetEnd,
  kPrepareImageTextureStart,
  kPrepareImageTextureEnd,
  kPrepareAlphaVideoFrameDataStart,
  kPrepareAlphaVideoFrameDataEnd,
  kCreateSurfaceStart,
  kCreateSurfaceEnd,
  kRenderFrameStart,
  kRenderFrameEnd,
  kInterpolateFrameStart,
  kInterpolateFrameEnd,
  kDrawFrameStart,
  kDrawFrameEnd,
  kFlushFrameStart,
  kFlushFrameEnd,
  kSwapFrameStart,
  kSwapFrameEnd,
  kGPUFrameStart,
  kGPUFrameEnd,
  kEnumCount,
};

/**
 * If the operation between begin and end is in different FlushTask operation,
 * we need to use instant event to record the event.
 */
#define ANIMAX_TRACE_INSTANT_ASSET_ID(event, asset_id, ...)           \
  TRACE_EVENT_INSTANT(::lynx::animax::kAnimaXTraceCategory,           \
                      ::lynx::animax::trace_event::event, "asset_id", \
                      asset_id, ##__VA_ARGS__)

#define ANIMAX_TRACE_INSTANT_MAIN_URI(event, main_uri, ...)           \
  TRACE_EVENT_INSTANT(::lynx::animax::kAnimaXTraceCategory,           \
                      ::lynx::animax::trace_event::event, "main_uri", \
                      main_uri, ##__VA_ARGS__)

#define ANIMAX_TRACE_INSTANT(event, ...)                    \
  TRACE_EVENT_INSTANT(::lynx::animax::kAnimaXTraceCategory, \
                      ::lynx::animax::trace_event::event, ##__VA_ARGS__)

/**
 * If the operation between begin and end is in the same FlushTask operation,
 * we need to use begin and end event to record the event.
 */
#define ANIMAX_TRACE_EVENT_BEGIN(event, ...)              \
  TRACE_EVENT_BEGIN(::lynx::animax::kAnimaXTraceCategory, \
                    ::lynx::animax::trace_event::event, ##__VA_ARGS__)
#define ANIMAX_TRACE_EVENT_BEGIN_MAIN_URI(event, main_uri, ...) \
  TRACE_EVENT_BEGIN(::lynx::animax::kAnimaXTraceCategory,       \
                    ::lynx::animax::trace_event::event, "main_uri", main_uri)
#define ANIMAX_TRACE_EVENT_BEGIN_ASSET_ID(event, asset_id, ...) \
  TRACE_EVENT_BEGIN(::lynx::animax::kAnimaXTraceCategory,       \
                    ::lynx::animax::trace_event::event, "asset_id", asset_id)
#define ANIMAX_TRACE_EVENT_BEGIN_MAIN_URI_ASSET_ID(event, main_uri, asset_id, \
                                                   ...)                       \
  TRACE_EVENT_BEGIN(::lynx::animax::kAnimaXTraceCategory,                     \
                    ::lynx::animax::trace_event::event, "main_uri", main_uri, \
                    "asset_id", asset_id)
#define ANIMAX_TRACE_EVENT_END(...) \
  TRACE_EVENT_END(::lynx::animax::kAnimaXTraceCategory, ##__VA_ARGS__)

#define ANIMAX_TRACE_EVENT(event, ...)              \
  TRACE_EVENT(::lynx::animax::kAnimaXTraceCategory, \
              ::lynx::animax::trace_event::event, ##__VA_ARGS__)

#define ANIMAX_TRACE_EVENT_MAIN_URI(event, main_uri, ...)               \
  TRACE_EVENT(::lynx::animax::kAnimaXTraceCategory,                     \
              ::lynx::animax::trace_event::event, "main_uri", main_uri, \
              ##__VA_ARGS__)

#define ANIMAX_TRACE_EVENT_ASSET_ID(event, asset_id, ...)               \
  TRACE_EVENT(::lynx::animax::kAnimaXTraceCategory,                     \
              ::lynx::animax::trace_event::event, "asset_id", asset_id, \
              ##__VA_ARGS__);

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_BASE_MONITOR_TRACE_EVENT_H_
