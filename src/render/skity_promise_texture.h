// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RENDER_SKITY_PROMISE_TEXTURE_H_
#define ANIMAX_SRC_RENDER_SKITY_PROMISE_TEXTURE_H_

#include <atomic>
#include <functional>
#include <memory>
#include <utility>

#include "skity/graphic/image.hpp"

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

  static std::shared_ptr<skity::Image> MakeRetainedImage(
      Payload payload, GenerateCallback callback, skity::TextureFormat format,
      size_t width, size_t height, skity::AlphaType alpha_type) {
    auto* promise =
        new SkityPromiseTexture(std::move(payload), std::move(callback));
    promise->references_ = std::make_shared<std::atomic<uint32_t>>(1);
    auto owner = std::shared_ptr<void>(promise, &ReleaseRetainedContext);
    auto image = skity::Image::MakePromiseTextureImage2(
        format, width, height, alpha_type, &GenerateRetainedTexture,
        &ReleaseRetainedContext, promise);
    auto* raw_image = image.get();
    // The display list can retain the image before the promise is fulfilled.
    return std::shared_ptr<skity::Image>(
        raw_image, [image = std::move(image),
                    owner = std::move(owner)](skity::Image*) mutable {
          image.reset();
          owner.reset();
        });
  }

 private:
  static std::shared_ptr<skity::Texture> GenerateRetainedTexture(
      void* data, skity::GPUContext* context) {
    auto* promise = static_cast<SkityPromiseTexture*>(data);
    auto texture = GeneratePromiseTexture(data, context);
    if (texture && texture->GetGPUTexture()) {
      promise->references_->fetch_add(1);
      return texture;
    }
    return {};
  }

  static void ReleaseRetainedContext(void* data) {
    auto* promise = static_cast<SkityPromiseTexture*>(data);
    if (promise->references_->fetch_sub(1) == 1) {
      delete promise;
    }
  }

  std::shared_ptr<skity::Texture> Generate(skity::GPUContext* context) {
    return callback_(context, payload_);
  }

  Payload payload_;
  GenerateCallback callback_;
  std::shared_ptr<std::atomic<uint32_t>> references_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RENDER_SKITY_PROMISE_TEXTURE_H_
