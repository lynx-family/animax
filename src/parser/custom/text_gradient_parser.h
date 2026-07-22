// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PARSER_CUSTOM_TEXT_GRADIENT_PARSER_H_
#define ANIMAX_SRC_PARSER_CUSTOM_TEXT_GRADIENT_PARSER_H_

#include "third_party/rapidjson/document.h"

namespace lynx {
namespace animax {

class CompositionModel;

class TextGradientParser final {
 public:
  static void Parse(rapidjson::Value& value, CompositionModel& composition);
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PARSER_CUSTOM_TEXT_GRADIENT_PARSER_H_
