// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
// Copyright 2018 Airbnb, Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef ANIMAX_SRC_ANIMATION_TEXT_KEYFRAME_ANIMATION_H_
#define ANIMAX_SRC_ANIMATION_TEXT_KEYFRAME_ANIMATION_H_

#include "src/animation/keyframe_animation.h"
#include "src/model/value/document_data.h"

namespace lynx {
namespace animax {

class TextKeyframeAnimation : public KeyframeAnimation {
 public:
  TextKeyframeAnimation(std::shared_ptr<KeyframeModelList> frames)
      : KeyframeAnimation(std::move(frames)) {}

  DocumentData& GetValue(KeyframeModel& keyframe, float progress) override;
  DocumentData& GetValue() override;

  static std::unique_ptr<TextKeyframeAnimation> MakeDefault();

 private:
  std::unique_ptr<DocumentData> intermediate_ =
      ValueFactory::Make<DocumentData>();
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_ANIMATION_TEXT_KEYFRAME_ANIMATION_H_
