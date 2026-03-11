// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/layer/text_helper.h"

#include "base/include/no_destructor.h"
#include "src/base/log/log.h"
#include "src/layer/font_asset_manager.h"
#include "src/layer/text_content_default.h"

#if defined(OS_IOS)
#include <dispatch/dispatch.h>

#include "src/base/util/ios/reflect_invoke.h"
#endif

namespace lynx {
namespace animax {
TextHelper &TextHelper::Impl() {
#if defined(OS_IOS)
  static dispatch_once_t once_token;
  dispatch_once(&once_token, ^{
    ReflectInvoke("AnimaXTextraRegister", "registerTextra");
  });
#endif
  TextHelper *impl = ImplRef();
  if (impl) {
    return *impl;
  }

  static base::NoDestructor<TextHelper> helper;
  return *helper;
}

void TextHelper::RegisterImpl(TextHelper *impl) {
  // Ensuring the lifecycle by the caller
  ImplRef() = impl;
}

inline TextHelper *&TextHelper::ImplRef() {
  static TextHelper *impl_ref = nullptr;
  return impl_ref;
}

TextHelper::TextHelper() { ANIMAX_LOGI("use default text impl"); }

std::unique_ptr<TextContent> TextHelper::CreateTextContent(
    const TextContentDataSource &data_source) {
  return std::make_unique<TextContentDefault>(data_source);
}

std::unique_ptr<FontAssetManager> TextHelper::CreateFontAssetManager() {
  return std::make_unique<FontAssetManager>();
}

}  // namespace animax
}  // namespace lynx
