// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_LAYER_TEXTRA_TEXT_CONTENT_LAYOUT_CONTEXT_H_
#define ANIMAX_SRC_LAYER_TEXTRA_TEXT_CONTENT_LAYOUT_CONTEXT_H_

#include "src/layer/textra/textra_include.h"

namespace lynx {
namespace animax {
class TextContentDataSource;
class TextContentLayoutContext {
 public:
  void Layout(const TextContentDataSource& data_source,
              const skity::Paint& paint);
  ttoffice::tttext::LayoutRegion* GetLayoutRegion() const {
    return layout_region_.get();
  }
  bool IsBoxMode() const { return is_box_mode_; }

 private:
  std::unique_ptr<ttoffice::tttext::LayoutRegion> layout_region_;
  std::unique_ptr<TTParagraph> paragraph_;
  ttoffice::tttext::SkityPainter tt_painter_;
  bool is_box_mode_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_LAYER_TEXTRA_TEXT_CONTENT_LAYOUT_CONTEXT_H_
