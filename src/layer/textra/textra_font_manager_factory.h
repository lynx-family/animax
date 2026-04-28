// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_LAYER_TEXTRA_TEXTRA_FONT_MANAGER_FACTORY_H_
#define ANIMAX_SRC_LAYER_TEXTRA_TEXTRA_FONT_MANAGER_FACTORY_H_

#include <memory>

#include "src/layer/textra/textra_include.h"

namespace lynx {
namespace animax {

std::shared_ptr<ttoffice::tttext::IFontManager> CreateTextraFontManager();

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_LAYER_TEXTRA_TEXTRA_FONT_MANAGER_FACTORY_H_
