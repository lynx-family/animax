// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RENDER_SKITY_PROMISE_TEXTURE_H_
#define ANIMAX_SRC_RENDER_SKITY_PROMISE_TEXTURE_H_

#include <functional>
#include <memory>
#include <utility>

namespace skity {
class GPUContext;
class Texture;
}  // namespace skity

namespace lynx {
namespace animax {

template <typename Payload>
class SkityPromiseTexture final {
 public:
  using GenerateCallback = std::function<std::shared_ptr<skity::Texture>(
      skity::GPUContext*, Payload)>;

  SkityPromiseTexture(Payload payload, GenerateCallback callback)
      : payload_(std::move(payload)), callback_(std::move(callback)) {}

  static std::shared_ptr<skity::Texture> GeneratePromiseTexture(
      void* data, skity::GPUContext* context) {
    if (!data || !context) {
      return {};
    }
    return static_cast<SkityPromiseTexture*>(data)->Generate(context);
  }

  static void ReleasePromiseContext(void* data) {
    delete static_cast<SkityPromiseTexture*>(data);
  }

 private:
  std::shared_ptr<skity::Texture> Generate(skity::GPUContext* context) {
    return callback_(context, payload_);
  }

  Payload payload_;
  GenerateCallback callback_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RENDER_SKITY_PROMISE_TEXTURE_H_
