// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RENDER_MASK_FILTER_H_
#define ANIMAX_SRC_RENDER_MASK_FILTER_H_

#include <memory>

namespace skity {
class MaskFilter;
}
namespace lynx {
namespace animax {

class MaskFilter {
 public:
  explicit MaskFilter(std::shared_ptr<skity::MaskFilter> filter);

  ~MaskFilter();

  std::shared_ptr<skity::MaskFilter> const& GetMaskFilter() const {
    return mask_filter_;
  }

  static std::unique_ptr<MaskFilter> MakeBlur(float radius);

 private:
  std::shared_ptr<skity::MaskFilter> mask_filter_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RENDER_MASK_FILTER_H_
