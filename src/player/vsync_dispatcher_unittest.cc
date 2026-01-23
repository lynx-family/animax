// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include "src/player/vsync_dispatcher.h"

#include <memory>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "include/player/vsync_monitor.h"

using namespace lynx::animax;

class MockPlatformVSyncDispatcher : public VSyncDispatcher {
 public:
  MOCK_METHOD(void, RequestVSync, (), (override));
};

class VSyncDispatcherTest : public ::testing::Test {
 protected:
  void SetUp() override {
    mock_dispatcher_ = std::make_unique<MockPlatformVSyncDispatcher>();
  }
  void TearDown() override { mock_dispatcher_.reset(); }
  std::unique_ptr<MockPlatformVSyncDispatcher> mock_dispatcher_;
};

TEST_F(VSyncDispatcherTest, SubscribeOnceTriggersRequestVSync) {
  EXPECT_CALL(*mock_dispatcher_, RequestVSync()).Times(1);
  mock_dispatcher_->SubscribeOnce([](int64_t) {});
}

TEST_F(VSyncDispatcherTest, CancelRemovesPendingCallback) {
  auto token = mock_dispatcher_->SubscribeOnce([](int64_t) {});
  mock_dispatcher_->Cancel(token);
  bool callback_called = false;
  mock_dispatcher_->SubscribeOnce([&](int64_t) { callback_called = true; });
  mock_dispatcher_->OnVSync(1000);
  EXPECT_TRUE(callback_called);
}

TEST_F(VSyncDispatcherTest, OnVSyncExecutesBatchCallbacks) {
  int callback_count = 0;
  mock_dispatcher_->SubscribeOnce([&](int64_t) { callback_count++; });
  mock_dispatcher_->SubscribeOnce([&](int64_t) { callback_count++; });
  mock_dispatcher_->OnVSync(1000);
  EXPECT_EQ(callback_count, 2);
}
