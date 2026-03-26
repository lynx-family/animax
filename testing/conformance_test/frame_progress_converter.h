// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_TESTING_CONFORMANCE_TEST_FRAME_PROGRESS_CONVERTER_H_
#define ANIMAX_TESTING_CONFORMANCE_TEST_FRAME_PROGRESS_CONVERTER_H_

#include "src/model/composition_model.h"

namespace lynx {
namespace animax {

class FrameProgressConverter {
 public:
  explicit FrameProgressConverter(CompositionModel& model)
      : start_frame_(model.GetStartFrame()), end_frame_(model.GetEndFrame()) {}

  bool IsFrameValid(int32_t frame) {
    return frame >= start_frame_ && frame <= end_frame_;
  }

  float ProgressForFrame(int32_t frame);

 private:
  int32_t start_frame_{};
  int32_t end_frame_{};
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_TESTING_CONFORMANCE_TEST_FRAME_PROGRESS_CONVERTER_H_
