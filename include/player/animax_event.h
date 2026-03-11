// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_INCLUDE_PLAYER_ANIMAX_EVENT_H_
#define ANIMAX_INCLUDE_PLAYER_ANIMAX_EVENT_H_

#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "base/include/closure.h"
#include "include/base/macros.h"

namespace lynx {
namespace animax {

enum class Event : uint8_t {
  kCompletion = 0,  // Play completion
  kStart,           // Animation starts
  kRepeat,          // A new loop starts
  kCancel,          // A playing animation is stop
  kReady,   // Resource and real surface are ready, you can call Play() if
            // autoplay is false
  kUpdate,  // A new frame
  kError,   // Error occurs, more information can be found in error code and
            // error message
  kFps,     // Fps and max drop value event
  kCompositionReady,  // Composition resource is ready
  kTapLayer,          // The tap layer event
  kFirstFrame,        // First frame is flushed
  kWarning,           // Warning occurs
};

enum class EventError : int32_t {
  kNoError = 0,
  kResourceNotFound = 1,
  // kScaleImageFailed = 2, // Reserved
  // kRecreateBitmapFailed = 3, // Reserved
  // kLocalResourceNotFound = 4, // Reserved
  kSurfaceInitError = 10,
  // kThreadInitError = 11, // Deprecated
  kLibraryInitError = 12,
  kVideoPlayerError = 100,
  kVideoPlayerErrorHasOccurred = 101,
};

enum class EventWarning : int32_t {
  kNoWarning = 0,
  kInvalidLottieFormat = 101,
  kIllegalAlphaVideoSize = 102,
  kAssetCountOverLimit = 103,
  kDynamicResourceCannotBeLoaded = 104,
  kStartFrameGreaterThanEndFrame = 201,
  kIllegalSubscribedStartFrame = 202,
  kExecuteBeforeReady = 301,
};

struct ANIMAX_EXPORT EventParamValue {
  enum class Type {
    kNone = 0,
    kInt32,
    kDouble,
    kString,
    kStringVector,
  };
  Type type = Type::kNone;
  std::optional<int32_t> int_val;
  std::optional<double> double_val;
  std::unique_ptr<std::string> string_val;
  std::unique_ptr<std::vector<std::string>> string_vector_val;

  EventParamValue() = default;
  EventParamValue(int32_t v) : type(Type::kInt32), int_val(v) {}
  EventParamValue(double v) : type(Type::kDouble), double_val(v) {}
  EventParamValue(const std::string& v)
      : type(Type::kString), string_val(std::make_unique<std::string>(v)) {}
  EventParamValue(std::unique_ptr<std::vector<std::string>> v)
      : type(Type::kStringVector), string_vector_val(std::move(v)) {}
};

struct EventKeys {
  static constexpr const char* kCode = "code";
  static constexpr const char* kMessage = "msg";
  static constexpr const char* kFps = "fps";
  static constexpr const char* kMaxDropRate = "max_drop_rate";
  static constexpr const char* kLayerList = "layerList";
  static constexpr const char* kAnimationId = "animationID";
  static constexpr const char* kCurrent = "current";
  static constexpr const char* kTotal = "total";
  static constexpr const char* kLoopIndex = "loopIndex";
  static constexpr const char* kPlayerId = "elementID";
};

class AnimaXPlayer;

using EventParamMap = std::map<std::string, EventParamValue>;
using EventListener = base::MoveOnlyClosure<void, AnimaXPlayer*, const Event,
                                            const EventParamMap&>;

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_INCLUDE_PLAYER_ANIMAX_EVENT_H_
