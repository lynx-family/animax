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

#include "src/model/value/base_value.h"

#include "include/base/util/color_util.h"

namespace lynx {
namespace animax {

// Integer implementation
Integer::Integer(int32_t value) : value_(value) {}
Integer::Integer() : value_(std::nullopt) {}

bool Integer::IsEmpty() const { return !value_.has_value(); }

std::unique_ptr<Value> Integer::Copy() const {
  if (IsEmpty()) {
    return ValueFactory::Make<Integer>();
  } else {
    return ValueFactory::Make<Integer>(Get());
  }
}

// Float implementation
Float::Float(float value) : value_(value) {}
Float::Float() : value_(std::nullopt) {}

bool Float::IsEmpty() const { return !value_.has_value(); }

std::unique_ptr<Value> Float::Copy() const {
  if (IsEmpty()) {
    return ValueFactory::Make<Float>();
  } else {
    return ValueFactory::Make<Float>(Get());
  }
}

float Float::Get() const { return value_.value_or(0.0f); }

void Float::Reset() { value_.reset(); }

void Float::Set(float value) { value_ = value; }

// Color implementation
Color::Color()
    : a_(std::nullopt), r_(std::nullopt), g_(std::nullopt), b_(std::nullopt) {}

Color::Color(uint8_t a, uint8_t r, uint8_t g, uint8_t b)
    : a_(a), r_(r), g_(g), b_(b) {}

Color::Color(int32_t value) {
  a_ = (value >> 24) & 0xff;
  r_ = (value >> 16) & 0xff;
  g_ = (value >> 8) & 0xff;
  b_ = value & 0xff;
}

Color::Color(const std::string& color) { Set(color); }

bool Color::IsEmpty() const {
  return !a_.has_value() || !r_.has_value() || !g_.has_value() ||
         !b_.has_value();
}

std::unique_ptr<Value> Color::Copy() const {
  if (IsEmpty()) {
    return ValueFactory::Make<Color>();
  } else {
    return ValueFactory::Make<Color>(GetA(), GetR(), GetG(), GetB());
  }
}

int32_t Color::ToInt(uint8_t a, uint8_t r, uint8_t g, uint8_t b) {
  return (a << 24) | (r << 16) | (g << 8) | b;
}

void Color::Reset() {
  a_.reset();
  r_.reset();
  g_.reset();
  b_.reset();
}

void Color::Set(int32_t value) {
  auto a = static_cast<uint8_t>((value >> 24) & 0xff);
  auto r = static_cast<uint8_t>((value >> 16) & 0xff);
  auto g = static_cast<uint8_t>((value >> 8) & 0xff);
  auto b = static_cast<uint8_t>(value & 0xff);
  Set(a, r, g, b);
}

void Color::Set(uint8_t a, uint8_t r, uint8_t g, uint8_t b) {
  a_ = a;
  r_ = r;
  g_ = g;
  b_ = b;
}

void Color::Set(const std::string& color) {
  int32_t color_value = ColorUtil::ParseHexColor(color);
  Set(color_value);
}

}  // namespace animax
}  // namespace lynx
