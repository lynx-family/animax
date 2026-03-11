// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "include/base/player_event_tracker.h"

#include "base/include/no_destructor.h"

namespace lynx {
namespace animax {

PlayerEventTracker::PlayerEventTracker() { Reset(); }

void PlayerEventTracker::MarkEvent(AnimationEventType type) {
  events_[static_cast<std::size_t>(type)] = true;
}

bool PlayerEventTracker::GetEventValue(const AnimationEventType type) {
  return events_[static_cast<std::size_t>(type)];
}

void PlayerEventTracker::Reset() { events_.fill(false); }

const PlayerEventTracker::EventArray& PlayerEventTracker::GetEvents() {
  return events_;
}

// The event names must exactly match EventType(s) defined in the header file.
const PlayerEventTracker::EventNameArray& PlayerEventTracker::GetEventNames() {
  static base::NoDestructor<EventNameArray> event_names{EventNameArray{
      "is_ready", "has_started", "had_errors", "has_cancelled", "has_repeated",
      "has_updated", "has_completed", "has_warning"}};
  return *event_names;
}
}  // namespace animax
}  // namespace lynx
