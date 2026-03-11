//  Copyright 2024 The Lynx Authors. All rights reserved.

#include <stdio.h>
#include "src/base/log/log.h"
#include "src/base/util/animax_native.h"

namespace lynx {
namespace animax {

void Log(LogMessage* msg) {
  auto* fn = lynx::animax::AnimaXNative::Instance().GetALogFunction();
  auto stream_str = msg->stream().str();

  if (fn == nullptr) {
    NSLog(@"[AnimaX] %s", stream_str.c_str());
  } else {
    fn(msg->level(), "AnimaX", stream_str.c_str());
  }
}

}  // namespace animax
}  // namespace lynx
