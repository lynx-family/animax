// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_INCLUDE_PLAYER_COMPOSITION_BINDING_H_
#define ANIMAX_INCLUDE_PLAYER_COMPOSITION_BINDING_H_

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>

#include "include/base/macros.h"

namespace lynx {
namespace animax {
class CompositionModel;
class AnimaXPlayer;
class AnimaXRenderer;
struct CompositionBindingState;

struct CompositionBindingError {
  std::string code;
  std::string message;
  bool IsSuccess() const { return code.empty(); }
};

// The host serializes this short callback with its own invalidation. It must
// neither dispatch the callback nor perform JS work while running it.
using CompositionHostGuard = std::function<bool(std::function<void()>)>;

struct CompositionBindingCommitResult {
  std::shared_ptr<CompositionModel> model;
  std::string code;
  std::string message;
};
using CompositionBindingCommit =
    std::function<CompositionBindingCommitResult()>;

// One terminal result, independent of whether a Surface is currently present.
// Completion runs outside the player/host locks; callers dispatch to their Env.
class ANIMAX_EXPORT CompositionBindingRequest {
 public:
  using Completion = std::function<void(bool, std::string, std::string)>;
  uint64_t Id() const { return id_; }
  bool IsPending() const;
  void Fail(std::string code, std::string message);

 private:
  friend class AnimaXPlayer;
  friend class AnimaXRenderer;
  enum class Phase { kPending, kCommitted, kFailed };
  CompositionBindingRequest(std::shared_ptr<CompositionBindingState> state,
                            uint64_t id, Completion completion);
  std::shared_ptr<CompositionBindingState> state_;
  const uint64_t id_;
  // Writers hold Player state; readers may hold Composition data instead.
  std::atomic<Phase> phase_{Phase::kPending};
  Completion completion_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_INCLUDE_PLAYER_COMPOSITION_BINDING_H_
