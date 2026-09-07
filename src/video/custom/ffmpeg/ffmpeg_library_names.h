// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_VIDEO_CUSTOM_FFMPEG_FFMPEG_LIBRARY_NAMES_H_
#define ANIMAX_SRC_VIDEO_CUSTOM_FFMPEG_FFMPEG_LIBRARY_NAMES_H_

#include <string>
#include <vector>

namespace lynx {
namespace animax {

// Returns the FFmpeg dynamic libraries supplied by the active build provider.
// The public provider uses upstream FFmpeg's versioned DLL layout. Internal
// consumers can replace the provider through build_overrides/animax.gni.
std::vector<std::string> GetFFmpegLibraryNames();

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_VIDEO_CUSTOM_FFMPEG_FFMPEG_LIBRARY_NAMES_H_
