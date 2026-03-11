// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_INCLUDE_BASE_PLAYER_EVENT_TRACKER_H_
#define ANIMAX_INCLUDE_BASE_PLAYER_EVENT_TRACKER_H_

#include <array>
#include <string>

#include "include/base/macros.h"

namespace lynx {
namespace animax {

class ANIMAX_EXPORT PlayerEventTracker {
 public:
  // Remember to change the GetEventNames() method when you are changing
  // EventType(s).
  enum class AnimationEventType {
    kReady = 0,
    kStart,
    kError,
    kCancel,
    kRepeat,
    kUpdate,
    kComplete,
    kWarning,
    kSize  // Size may need renaming (see below)
  };

  static constexpr size_t kNumEventTypes =
      static_cast<size_t>(AnimationEventType::kSize);

  using EventArray = std::array<bool, kNumEventTypes>;
  using EventNameArray =
      std::array<std::string, PlayerEventTracker::kNumEventTypes>;

 public:
  PlayerEventTracker();

  void Reset();
  void MarkEvent(AnimationEventType type);
  bool GetEventValue(const AnimationEventType type);
  const EventArray& GetEvents();
  const EventNameArray& GetEventNames();

 private:
  EventArray events_{};
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_INCLUDE_BASE_PLAYER_EVENT_TRACKER_H_
