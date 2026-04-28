// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/layer/textra/textra_font_manager_factory.h"

namespace lynx {
namespace animax {

std::shared_ptr<ttoffice::tttext::IFontManager> CreateTextraFontManager() {
  return std::make_shared<TTSkityFontManager>();
}

}  // namespace animax
}  // namespace lynx
