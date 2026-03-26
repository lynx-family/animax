// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "include/base/player_event_tracker.h"

#include "gtest/gtest.h"

using namespace lynx::animax;

class PlayerEventTrackerTest : public ::testing::Test {
 protected:
  PlayerEventTracker tracker;
};

// Test that events are initialized to false
TEST_F(PlayerEventTrackerTest, EventsInitiallyFalse) {
  auto events = tracker.GetEvents();
  for (bool event : events) {
    EXPECT_FALSE(event) << "Event should initially be false";
  }
}

// Test resetting all events to false
TEST_F(PlayerEventTrackerTest, ResetEvents) {
  tracker.MarkEvent(PlayerEventTracker::AnimationEventType::kReady);
  tracker.MarkEvent(PlayerEventTracker::AnimationEventType::kStart);
  EXPECT_EQ(
      tracker.GetEventValue(PlayerEventTracker::AnimationEventType::kReady),
      true);
  tracker.Reset();
  EXPECT_EQ(
      tracker.GetEventValue(PlayerEventTracker::AnimationEventType::kReady),
      false);
  auto events = tracker.GetEvents();
  for (bool event : events) {
    EXPECT_FALSE(event) << "All events should be false after Reset";
  }
}

// Test retrieving event names
TEST_F(PlayerEventTrackerTest, GetEventNames) {
  const auto& event_names = tracker.GetEventNames();
  ASSERT_EQ(event_names.size(), PlayerEventTracker::kNumEventTypes);
  EXPECT_EQ(event_names[static_cast<std::size_t>(
                PlayerEventTracker::AnimationEventType::kReady)],
            "is_ready");
  EXPECT_EQ(event_names[static_cast<std::size_t>(
                PlayerEventTracker::AnimationEventType::kStart)],
            "has_started");
  EXPECT_EQ(event_names[static_cast<std::size_t>(
                PlayerEventTracker::AnimationEventType::kError)],
            "had_errors");
  EXPECT_EQ(event_names[static_cast<std::size_t>(
                PlayerEventTracker::AnimationEventType::kCancel)],
            "has_cancelled");
  EXPECT_EQ(event_names[static_cast<std::size_t>(
                PlayerEventTracker::AnimationEventType::kRepeat)],
            "has_repeated");
  EXPECT_EQ(event_names[static_cast<std::size_t>(
                PlayerEventTracker::AnimationEventType::kUpdate)],
            "has_updated");
  EXPECT_EQ(event_names[static_cast<std::size_t>(
                PlayerEventTracker::AnimationEventType::kComplete)],
            "has_completed");
  EXPECT_EQ(event_names[static_cast<std::size_t>(
                PlayerEventTracker::AnimationEventType::kWarning)],
            "has_warning");
}
