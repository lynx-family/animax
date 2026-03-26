// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RENDER_TEXTURE_INFO_FRAME_DATA_H_
#define ANIMAX_SRC_RENDER_TEXTURE_INFO_FRAME_DATA_H_

#include <memory>
#include <vector>

#include "src/base/gl/gl_include.h"
#include "src/render/texture_info_gl.h"
#include "src/video/custom/yuv_frame_info.h"

namespace lynx {
namespace animax {

class TextureInfoFrameData : public TextureInfoGL {
 public:
  TextureInfoFrameData(std::shared_ptr<YUVFrameInfo> frame_info,
                       uint32_t target)
      : TextureInfoGL(0, frame_info->GetWidth(), frame_info->GetHeight(),
                      target),
        frame_info_(std::move(frame_info)) {}

  const std::shared_ptr<YUVFrameInfo>& GetFrameInfo() const {
    return frame_info_;
  }

 private:
  std::shared_ptr<YUVFrameInfo> frame_info_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RENDER_TEXTURE_INFO_FRAME_DATA_H_
