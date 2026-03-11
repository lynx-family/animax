// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_LAYER_TEXTRA_TEXTRA_INCLUDE_H_
#define ANIMAX_SRC_LAYER_TEXTRA_TEXTRA_INCLUDE_H_

#include <textra/fontmgr_collection.h>
#include <textra/layout_drawer.h>
#include <textra/platform/skity/skity_canvas_helper.h>
#include <textra/text_layout.h>

#if defined(OS_IOS)
#include <textra/platform/skity/skity_font_manager_coretext.h>
#else
#include <textra/platform/skity/skity_font_manager.h>
#endif

namespace lynx {
namespace animax {

using TTFontMgrCollection = ttoffice::tttext::FontmgrCollection;
#if defined(OS_IOS)
using TTSkityFontManager = ttoffice::tttext::SkityFontManagerCoreText;
#else
using TTSkityFontManager = ttoffice::tttext::SkityFontManager;
#endif  // OS_IOS

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_LAYER_TEXTRA_TEXTRA_INCLUDE_H_
