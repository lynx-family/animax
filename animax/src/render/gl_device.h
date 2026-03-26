// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#ifndef ANIMAX_SRC_RENDER_GL_DEVICE_H_
#define ANIMAX_SRC_RENDER_GL_DEVICE_H_

#include <set>
#include <string>

namespace lynx {
namespace animax {

class SkityGLDevice final {
 public:
  ~SkityGLDevice() = default;

  static const SkityGLDevice* GetGlobalDevice();

  bool ExtensionEnabled(const char* name) const;

 private:
  SkityGLDevice() = default;

 private:
  std::set<std::string> gl_extensions_ = {};
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RENDER_GL_DEVICE_H_
