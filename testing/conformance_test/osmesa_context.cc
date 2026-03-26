// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "testing/conformance_test/osmesa_context.h"

namespace lynx {
namespace animax {

namespace {

OSMesaContext CreateOSMesaContext() {
  // Create a context with attributes for OpenGL ES 3.0
  int attribs[] = {OSMESA_FORMAT,
                   OSMESA_RGBA,
                   OSMESA_PROFILE,
                   OSMESA_CORE_PROFILE,
                   OSMESA_CONTEXT_MAJOR_VERSION,
                   3,
                   OSMESA_CONTEXT_MINOR_VERSION,
                   0,
                   0};

  OSMesaContext ctx = OSMesaCreateContextAttribs(attribs, nullptr);
  return ctx;
}
}  // namespace

bool AnimaXOSMesaContext::MakeCurrent(int width, int height, void* buffer) {
  return GL_TRUE ==
         OSMesaMakeCurrent(context_, buffer, GL_UNSIGNED_BYTE, width, height);
}

bool AnimaXOSMesaContext::MakeCurrent() {
  return GL_TRUE ==
         OSMesaMakeCurrent(context_, buffer_.data(), GL_UNSIGNED_BYTE, 1, 1);
}

AnimaXOSMesaContext& AnimaXOSMesaContext::ThreadLocalInstance() {
  thread_local AnimaXOSMesaContext* context = new AnimaXOSMesaContext{};
  return *context;
}

AnimaXOSMesaContext::~AnimaXOSMesaContext() { OSMesaDestroyContext(context_); }

AnimaXOSMesaContext::AnimaXOSMesaContext() : context_(CreateOSMesaContext()) {}

}  // namespace animax
}  // namespace lynx
