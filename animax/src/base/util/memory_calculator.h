// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_BASE_UTIL_MEMORY_CALCULATOR_H_
#define ANIMAX_SRC_BASE_UTIL_MEMORY_CALCULATOR_H_

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "src/model/rect_model.h"

namespace lynx {
namespace animax {

class CompositionModel;
class LayerModel;

using LayerModelList = std::vector<std::unique_ptr<LayerModel>>;

struct MemoryConfig {
  static constexpr int32_t kBytesPerPixel = 4;
  static constexpr int32_t kBytesToKB = 1024;
  static constexpr int32_t kBytesToMb = kBytesToKB * kBytesToKB;
  static constexpr int32_t kBaseMemoryBytes = 10LL * kBytesToMb;
  static constexpr int32_t kScreenBufferCount = 3;
  static constexpr int8_t kVideoCacheCount = 5;

#ifdef OS_IOS
  // Bytes required per GPU command on iOS (each command creates multiple
  // objects with sizes: 1024KB, 768KB, 256KB, 16KB, 640KB)
  static constexpr int32_t kGPUCommandBufferBytes =
      2 * (kBytesToKB + 768 + 256 + 16 + 640) * kBytesToKB;
#endif

  static double ToMB(int64_t bytes) {
    return static_cast<double>(bytes) / kBytesToMb;
  }
};

struct MemoryUsageInfo {
  int64_t total_memory = 0;
  int64_t image_memory = 0;
  int64_t video_texture_memory = 0;
  int64_t video_cache_memory = 0;
  int64_t mask_matte_memory = 0;
  int64_t screen_buffer_memory = 0;

  std::string GetDescription() const;
};

struct MemoryCalculatorParams {
  std::shared_ptr<CompositionModel> model;
  double view_width = 0;
  double view_height = 0;
  float scale_factor_x = 1.0f;
  float scale_factor_y = 1.0f;

  bool IsValid() const {
    return model != nullptr && view_width > 0 && view_height > 0;
  }
};

class MemoryCalculator {
 public:
  static MemoryUsageInfo CalculateMemoryUsage(
      const MemoryCalculatorParams& params);

 private:
  static int64_t CalculateImageMemory(int32_t width, int32_t height);

  static int64_t CalculateVideoTextureMemory(int32_t width, int32_t height);

  static int64_t CalculateVideoCacheMemory(int32_t width, int32_t height);

  static RectF GetLayerBounds(LayerModel* layer, CompositionModel& model);

  static int64_t CalculateMaskMatteMemory(const MemoryCalculatorParams& params);

  static int64_t CalculateScreenBufferMemory(double width, double height);

  static void VisitLayers(
      const LayerModelList& layers,
      const std::unordered_map<std::string, LayerModelList>& precomps,
      const std::function<void(LayerModel*)>& visitor);
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_BASE_UTIL_MEMORY_CALCULATOR_H_
