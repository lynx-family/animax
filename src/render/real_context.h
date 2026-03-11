// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RENDER_REAL_CONTEXT_H_
#define ANIMAX_SRC_RENDER_REAL_CONTEXT_H_

namespace skity {
class GPUContext;
}
namespace lynx {
namespace animax {
enum class ContextBackend {
  kOpenGL,
  kVulkan,
  kMetal,
  kSoftware,
};

class RealContext {
 public:
  RealContext(skity::GPUContext* context) : context_(context) {}
  ~RealContext() = default;

  skity::GPUContext* Get() const { return context_; }

  ContextBackend GetBackendType() const;

 private:
  skity::GPUContext* context_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RENDER_REAL_CONTEXT_H_
