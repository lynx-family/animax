// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PLAYER_ANIMAX_EVENT_H_
#define ANIMAX_SRC_PLAYER_ANIMAX_EVENT_H_

#include <sstream>
#include <string>
#include <unordered_set>

#include "include/player/animax_event.h"

namespace lynx {
namespace animax {

const uint32_t kRecommendedVideoLimit = 3;
const uint32_t kRecommendedImageLimit = 20;
const uint32_t kRecommendedFontLimit = 5;

const char* StringifyEventWarning(EventWarning warning);

class IEventParams {
 public:
  virtual ~IEventParams() = default;
};

class TapParams : public IEventParams {
 public:
  TapParams(std::unordered_set<std::string> hit_layers)
      : hit_layers_(hit_layers) {}
  ~TapParams() override = default;

  std::unordered_set<std::string> hit_layers_;
};

class FrameParams : public IEventParams {
 public:
  FrameParams(double current_frame) : current_frame_(current_frame) {}
  ~FrameParams() override = default;

  double current_frame_;
};

class FpsParams : public FrameParams {
 public:
  FpsParams(double current_frame, int32_t max_drop_rate, double fps)
      : FrameParams(current_frame), max_drop_rate_(max_drop_rate), fps_(fps) {}
  ~FpsParams() override = default;

  int32_t max_drop_rate_;
  double fps_;
};

template <typename T>
class CodeMessageParams : public IEventParams {
 public:
  CodeMessageParams(T code, std::string message)
      : code_(static_cast<int32_t>(code)), message_(std::move(message)) {}
  ~CodeMessageParams() override = default;

  int32_t code_;
  std::string message_;
};

using ErrorParams = CodeMessageParams<EventError>;
using WarningParams = CodeMessageParams<EventWarning>;
}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PLAYER_ANIMAX_EVENT_H_
