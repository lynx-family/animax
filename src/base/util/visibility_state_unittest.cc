// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/base/util/visibility_state.h"

#include "gtest/gtest.h"

using namespace lynx::animax;

class VisibilityStateTest : public ::testing::Test {
 protected:
  VisibilityStateTest() = default;
  ~VisibilityStateTest() override = default;
};

// Test string conversion for each visibility state
TEST_F(VisibilityStateTest, StringifyVisibilityState) {
  EXPECT_STREQ(StringifyVisibilityState(VisibilityState::kBackground),
               "kBackground");
  EXPECT_STREQ(StringifyVisibilityState(VisibilityState::kVisible), "kVisible");
  EXPECT_STREQ(StringifyVisibilityState(VisibilityState::kOpacity), "kOpacity");
  EXPECT_STREQ(StringifyVisibilityState(VisibilityState::kSize), "kSize");
  EXPECT_STREQ(StringifyVisibilityState(VisibilityState::kAttach), "kAttach");
  EXPECT_STREQ(StringifyVisibilityState(VisibilityState::kAggregated),
               "kAggregated");

  // Test unknown state handling
  EXPECT_STREQ(StringifyVisibilityState(static_cast<VisibilityState>(0xFF)),
               "Unknown");
}

// Test the array of all visibility states returned by GetAllVisibilityStates
TEST_F(VisibilityStateTest, GetAllVisibilityStates) {
  auto states = GetAllVisibilityStates();
  EXPECT_EQ(states.size(), kVisibilityStateCount);

  // Verify each state is in the correct order
  EXPECT_EQ(states[0], VisibilityState::kBackground);
  EXPECT_EQ(states[1], VisibilityState::kVisible);
  EXPECT_EQ(states[2], VisibilityState::kOpacity);
  EXPECT_EQ(states[3], VisibilityState::kSize);
  EXPECT_EQ(states[4], VisibilityState::kAttach);
  EXPECT_EQ(states[5], VisibilityState::kAggregated);
}

// Test string conversion for different visibility state flag combinations
TEST_F(VisibilityStateTest, StringifyVisibilityStateFlag) {
  // Test empty flag returns "None"
  EXPECT_EQ(StringifyVisibilityStateFlag(0), "None");

  // Test single flag conversion
  EXPECT_EQ(StringifyVisibilityStateFlag(
                static_cast<uint8_t>(VisibilityState::kBackground)),
            "kBackground");

  // Test combination of two flags
  uint8_t two_flags = static_cast<uint8_t>(VisibilityState::kBackground) |
                      static_cast<uint8_t>(VisibilityState::kVisible);
  EXPECT_EQ(StringifyVisibilityStateFlag(two_flags), "kBackground|kVisible");

  // Test combination of three flags
  uint8_t three_flags = static_cast<uint8_t>(VisibilityState::kBackground) |
                        static_cast<uint8_t>(VisibilityState::kVisible) |
                        static_cast<uint8_t>(VisibilityState::kOpacity);
  EXPECT_EQ(StringifyVisibilityStateFlag(three_flags),
            "kBackground|kVisible|kOpacity");

  // Test combination of all flags
  uint8_t all_flags = static_cast<uint8_t>(VisibilityState::kBackground) |
                      static_cast<uint8_t>(VisibilityState::kVisible) |
                      static_cast<uint8_t>(VisibilityState::kOpacity) |
                      static_cast<uint8_t>(VisibilityState::kSize) |
                      static_cast<uint8_t>(VisibilityState::kAttach) |
                      static_cast<uint8_t>(VisibilityState::kAggregated);
  EXPECT_EQ(StringifyVisibilityStateFlag(all_flags),
            "kBackground|kVisible|kOpacity|kSize|kAttach|kAggregated");
}

// Test bit operations and values of VisibilityState enum
TEST_F(VisibilityStateTest, VisibilityStateBitOperations) {
  // Verify the bit value of each state
  EXPECT_EQ(static_cast<uint8_t>(VisibilityState::kBackground), 1);
  EXPECT_EQ(static_cast<uint8_t>(VisibilityState::kVisible), 1 << 1);
  EXPECT_EQ(static_cast<uint8_t>(VisibilityState::kOpacity), 1 << 2);
  EXPECT_EQ(static_cast<uint8_t>(VisibilityState::kSize), 1 << 3);
  EXPECT_EQ(static_cast<uint8_t>(VisibilityState::kAttach), 1 << 4);
  EXPECT_EQ(static_cast<uint8_t>(VisibilityState::kAggregated), 1 << 5);

  // Verify no bit overlap between different states
  for (const auto& state1 : GetAllVisibilityStates()) {
    for (const auto& state2 : GetAllVisibilityStates()) {
      if (state1 != state2) {
        EXPECT_EQ(static_cast<uint8_t>(state1) & static_cast<uint8_t>(state2),
                  0);
      }
    }
  }
}
