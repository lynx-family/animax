// Copyright 2023 The Lynx Authors. All rights reserved.

#ifndef ANIMAX_SRC_BASE_UTIL_COUNT_DOWN_EVENT_H_
#define ANIMAX_SRC_BASE_UTIL_COUNT_DOWN_EVENT_H_

#include <atomic>
#include <cstdint>

namespace lynx {
namespace animax {

class CountDownEvent final {
 public:
  CountDownEvent(int32_t max_count);
  ~CountDownEvent() = default;
  CountDownEvent(const CountDownEvent &) = delete;
  CountDownEvent &operator=(const CountDownEvent &) = delete;
  bool TryCountDown();
  void CountUp();

 private:
  std::atomic<int32_t> count_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_BASE_UTIL_COUNT_DOWN_EVENT_H_
