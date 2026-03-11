// Copyright 2023 The Lynx Authors. All rights reserved.

#include "src/base/util/count_down_event.h"

namespace lynx {
namespace animax {
CountDownEvent::CountDownEvent(int32_t max_count) { count_.store(max_count); }

bool CountDownEvent::TryCountDown() {
  auto origin = count_.fetch_sub(1);
  if (origin <= 0) {
    count_.fetch_add(1);
    return false;
  }
  return true;
}
void CountDownEvent::CountUp() { count_.fetch_add(1); }

}  // namespace animax
}  // namespace lynx
