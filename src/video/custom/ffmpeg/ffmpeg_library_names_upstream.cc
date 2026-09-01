// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/video/custom/ffmpeg/ffmpeg_library_names.h"

namespace lynx {
namespace animax {

std::vector<std::string> GetFFmpegLibraryNames() {
  // FFmpeg 7.x library majors. LoadLibrary resolves dependencies of these
  // libraries from the application directory.
  return {
      "avformat-61.dll",
      "avcodec-61.dll",
      "avutil-59.dll",
  };
}

}  // namespace animax
}  // namespace lynx
