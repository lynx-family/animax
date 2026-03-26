// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/loader/all_settled_loader.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "include/resource/loader_error.h"

using namespace ::testing;
using namespace lynx::animax;

namespace {

class MockIntIntLoader : public Loader<int, int> {
 public:
  ~MockIntIntLoader() override = default;
  MOCK_METHOD(void, Load, (int, CallbackType), (override));
};

}  // namespace

TEST(AllSettledLoaderTest, EmptyInVector) {
  auto mock_loader = std::make_shared<MockIntIntLoader>();
  EXPECT_CALL(*mock_loader, Load(_, _)).Times(0);
  auto all_settled_loader =
      AllSettledLoader<int, int>::Make<AllSettledLoader<int, int>>(mock_loader);
  all_settled_loader->Load(std::vector<int>{},
                           [](auto res, auto error) mutable {
                             EXPECT_FALSE(!!error);
                             EXPECT_TRUE(res.outs.empty());
                             EXPECT_TRUE(res.errors.empty());
                           });
}

TEST(AllSettledLoaderTest, Success) {
  auto mock_loader = std::make_shared<MockIntIntLoader>();
  auto task_number = 5;
  auto counter = 0;
  EXPECT_CALL(*mock_loader, Load(_, _))
      .Times(5)
      .WillRepeatedly(
          Invoke([&counter](auto a, auto cb) { cb(counter++, {}); }));
  auto all_settled_loader =
      AllSettledLoader<int, int>::Make<AllSettledLoader<int, int>>(mock_loader);
  all_settled_loader->Load(std::vector<int>(task_number, 0),
                           [task_number](auto res, auto error) mutable {
                             EXPECT_FALSE(!!error);
                             EXPECT_TRUE(res.outs.size() == task_number);
                             EXPECT_TRUE(res.errors.size() == task_number);
                             for (int i = 0; i < task_number; ++i) {
                               EXPECT_TRUE(res.outs[i] == i);
                               EXPECT_FALSE(!!res.errors[i]);
                             }
                           });
}

TEST(AllSettledLoaderTest, CallbackCalledMoreThanOnce) {
  auto mock_loader = std::make_shared<MockIntIntLoader>();
  auto task_number = 5;
  auto counter = 0;
  EXPECT_CALL(*mock_loader, Load(_, _))
      .Times(5)
      .WillRepeatedly(Invoke([&counter](auto a, auto cb) {
        cb(counter++, {});
        // call callback more than once
        cb(2, {});
        cb(2, LoaderError{.code = kInvalidCallback});
      }));
  auto all_settled_loader =
      AllSettledLoader<int, int>::Make<AllSettledLoader<int, int>>(mock_loader);
  all_settled_loader->Load(std::vector<int>(task_number, 0),
                           [task_number](auto res, auto error) mutable {
                             EXPECT_FALSE(!!error);
                             EXPECT_TRUE(res.outs.size() == task_number);
                             EXPECT_TRUE(res.errors.size() == task_number);
                             for (int i = 0; i < task_number; ++i) {
                               EXPECT_TRUE(res.outs[i] == i);
                               EXPECT_FALSE(!!res.errors[i]);
                             }
                           });
}

TEST(AllSettledLoaderTest, Failure) {
  auto mock_loader = std::make_shared<MockIntIntLoader>();
  auto task_number = 5;
  auto counter = 0;
  Sequence s;
  {
    InSequence in_seq;
    EXPECT_CALL(*mock_loader, Load(_, _))
        .Times(2)
        .InSequence(s)
        .WillRepeatedly(
            Invoke([&counter](auto a, auto cb) { cb(counter++, {}); }));
    EXPECT_CALL(*mock_loader, Load(_, _))
        .InSequence(s)
        .WillOnce(Invoke([&counter](auto a, auto cb) {
          cb(counter++, LoaderError{.code = kInvalidLoader});
        }));
    EXPECT_CALL(*mock_loader, Load(_, _))
        .Times(2)
        .InSequence(s)
        .WillRepeatedly(
            Invoke([&counter](auto a, auto cb) { cb(counter++, {}); }));
  }

  auto all_settled_loader =
      AllSettledLoader<int, int>::Make<AllSettledLoader<int, int>>(mock_loader);
  all_settled_loader->Load(std::vector<int>(task_number, 0),
                           [task_number](auto res, auto error) mutable {
                             EXPECT_FALSE(!!error);
                             EXPECT_TRUE(res.outs.size() == task_number);
                             EXPECT_TRUE(res.errors.size() == task_number);
                             for (int i = 0; i < task_number; ++i) {
                               EXPECT_TRUE(res.outs[i] == i);
                               if (i != 2) {
                                 EXPECT_FALSE(!!res.errors[i]);
                               } else {
                                 EXPECT_TRUE(!!res.errors[i]);
                               }
                             }
                           });
}
