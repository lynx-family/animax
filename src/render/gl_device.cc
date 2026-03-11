// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/render/gl_device.h"

#include "src/base/gl/gl_include.h"

namespace lynx {
namespace animax {

const SkityGLDevice* SkityGLDevice::GetGlobalDevice() {
  static SkityGLDevice* g_instance = nullptr;

  if (g_instance == nullptr) {
    g_instance = new SkityGLDevice;

    // load all extension
    GLint n_exts = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &n_exts);

    for (int i = 0; i < n_exts; i++) {
      g_instance->gl_extensions_.insert(
          reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i)));
    }
  }

  return g_instance;
}

bool SkityGLDevice::ExtensionEnabled(const char* name) const {
  return gl_extensions_.find(name) != gl_extensions_.end();
}

}  // namespace animax
}  // namespace lynx
