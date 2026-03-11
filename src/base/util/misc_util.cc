// Copyright 2024 The Lynx Authors. All rights reserved.

#include "src/base/util/misc_util.h"

#include "base/include/string/string_utils.h"

namespace lynx {
namespace animax {

float Lerp(const float& a, const float& b, const float& percentage) {
  return a + percentage * (b - a);
}

int32_t Lerp(const int32_t& a, const int32_t& b, const float& percentage) {
  return a + static_cast<int32_t>(percentage * (b - a));
}

void Lerp(const Integer& a, const Integer& b, const float& percentage,
          Integer* out) {
  if (out) {
    out->Set(Lerp(a.Get(), b.Get(), percentage));
  }
}

void Lerp(const Float& a, const Float& b, const float& percentage, Float* out) {
  if (out) {
    out->Set(Lerp(a.Get(), b.Get(), percentage));
  }
}

int32_t FloorMod(const int32_t& x, const int32_t& y) {
  int32_t r = x / y;
  // If the signs are different and modulo not zero, round down
  if ((x ^ y) < 0 && (r * y != x)) {
    r--;
  }
  return x - r * y;
}

void GammaEvaluate(const Color& start_color, const Color& end_color,
                   const float& progress, Color* const out) {
  if (!out) {
    return;
  }
  out->Set(Lerp(static_cast<float>(start_color.GetA()),
                static_cast<float>(end_color.GetA()), progress),
           Lerp(static_cast<float>(start_color.GetR()),
                static_cast<float>(end_color.GetR()), progress),
           Lerp(static_cast<float>(start_color.GetG()),
                static_cast<float>(end_color.GetG()), progress),
           Lerp(static_cast<float>(start_color.GetB()),
                static_cast<float>(end_color.GetB()), progress));
}

int32_t GammaEvaluate(const int32_t& start_color, const int32_t& end_color,
                      const float& progress) {
  return Color::ToInt(
      Lerp((start_color >> 24) & 0xff, (end_color >> 24) & 0xff, progress),
      Lerp((start_color >> 16) & 0xff, (end_color >> 16) & 0xff, progress),
      Lerp((start_color >> 8) & 0xff, (end_color >> 8) & 0xff, progress),
      Lerp(start_color & 0xff, end_color & 0xff, progress));
}

bool IsUrlValid(const std::string& url) {
  return lynx::base::BeginsWith(url, "http") ||
         lynx::base::BeginsWith(url, "asset") ||
         lynx::base::BeginsWith(url, "file");
}

}  // namespace animax
}  // namespace lynx
