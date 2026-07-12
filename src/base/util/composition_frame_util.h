// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_BASE_UTIL_COMPOSITION_FRAME_UTIL_H_
#define ANIMAX_SRC_BASE_UTIL_COMPOSITION_FRAME_UTIL_H_

namespace lynx {
namespace animax {

class CompositionFrameUtil {
 public:
  // CompositionModel stores an inclusive playback boundary. Convert the raw
  // half-open timeline values only at parser and reporting boundaries.
  static float ToPlaybackEndFrame(float timeline_end_frame) {
    return timeline_end_frame -
           static_cast<float>(kPlaybackEndFrameOffsetFrames);
  }

  static float ToTimelineEndFrame(float playback_end_frame) {
    return static_cast<float>(playback_end_frame +
                              kPlaybackEndFrameOffsetFrames);
  }

  static double ToTimelineDurationFrames(float timeline_start_frame,
                                         float playback_end_frame) {
    const auto timeline_end_frame = ToTimelineEndFrame(playback_end_frame);
    return timeline_end_frame > timeline_start_frame
               ? timeline_end_frame - timeline_start_frame
               : 0.0;
  }

  static long ToTimelineDurationMs(float timeline_start_frame,
                                   float playback_end_frame, float frame_rate) {
    if (frame_rate <= 0.0f) {
      return 0;
    }
    return static_cast<long>(
        ToTimelineDurationFrames(timeline_start_frame, playback_end_frame) *
        1000.0 / frame_rate);
  }

 private:
  static constexpr double kPlaybackEndFrameOffsetFrames = 0.01;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_BASE_UTIL_COMPOSITION_FRAME_UTIL_H_
