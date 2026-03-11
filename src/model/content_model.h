// Copyright 2023 The Lynx Authors. All rights reserved.
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

#ifndef ANIMAX_SRC_MODEL_CONTENT_MODEL_H_
#define ANIMAX_SRC_MODEL_CONTENT_MODEL_H_

#include <memory>

#include "src/content/content.h"

namespace lynx {
namespace animax {

class BaseLayer;
class CompositionModel;

enum class ModelType : uint8_t { kUnknown = 0, kShapeGroup, kTransform };

class ContentModel {
 public:
  ContentModel() = default;

  virtual ~ContentModel() = default;

  virtual std::unique_ptr<Content> CreateContent(CompositionModel& composition,
                                                 BaseLayer& layer) = 0;

  virtual ModelType Type() { return ModelType::kUnknown; }

  std::string name_;
  bool hidden_ = false;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_MODEL_CONTENT_MODEL_H_
