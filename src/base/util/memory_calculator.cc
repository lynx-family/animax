// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/base/util/memory_calculator.h"

#include <sstream>
#include <unordered_set>

#include "src/model/composition_model.h"
#include "src/model/layer_model.h"
#include "src/resource/composition_loader/composition_asset_task.h"

namespace lynx {
namespace animax {

std::string MemoryUsageInfo::GetDescription() const {
  if (total_memory <= 0) {
    return "";
  }

  std::ostringstream oss;
  oss << "Composition memory usage: total=" << MemoryConfig::ToMB(total_memory)
      << " MB";

  if (image_memory > 0) {
    oss << ", images=" << MemoryConfig::ToMB(image_memory) << " MB";
  }
  if (video_texture_memory > 0) {
    oss << ", video_texture=" << MemoryConfig::ToMB(video_texture_memory)
        << " MB";
  }
  if (video_cache_memory > 0) {
    oss << ", video_cache=" << MemoryConfig::ToMB(video_cache_memory) << " MB";
  }
  if (mask_matte_memory > 0) {
    oss << ", mask_matte=" << MemoryConfig::ToMB(mask_matte_memory) << " MB";
  }
  if (screen_buffer_memory > 0) {
    oss << ", screen_buffer=" << MemoryConfig::ToMB(screen_buffer_memory)
        << " MB";
  }
  return oss.str();
}

int64_t MemoryCalculator::CalculateImageMemory(int32_t width, int32_t height) {
  if (width <= 0 || height <= 0) {
    return 0;
  }

  return static_cast<int64_t>(width) * height * MemoryConfig::kBytesPerPixel;
}

int64_t MemoryCalculator::CalculateVideoTextureMemory(int32_t width,
                                                      int32_t height) {
  if (width <= 0 || height <= 0) {
    return 0;
  }

  // the width of video texture is half of the width of alpha video
  int64_t texture_width = width * 0.5;
  int64_t texture_height = height;
  return texture_width * texture_height * MemoryConfig::kBytesPerPixel;
}

int64_t MemoryCalculator::CalculateVideoCacheMemory(int32_t width,
                                                    int32_t height) {
  if (width <= 0 || height <= 0) {
    return 0;
  }
  return static_cast<int64_t>(width) * height * MemoryConfig::kBytesPerPixel *
         MemoryConfig::kVideoCacheCount;
}

RectF MemoryCalculator::GetLayerBounds(LayerModel* layer,
                                       CompositionModel& model) {
  if (!layer) {
    return RectF();
  }

  int32_t width = 0;
  int32_t height = 0;

  switch (layer->GetLayerType()) {
    case LayerType::kSolid:
      width = layer->GetSolidWidth();
      height = layer->GetSolidHeight();
      break;
    case LayerType::kPreComp:
      width = static_cast<int32_t>(layer->GetPreCompWidth());
      height = static_cast<int32_t>(layer->GetPreCompHeight());
      break;
    case LayerType::kImage: {
      auto& images = model.GetImages();
      auto it = images.find(layer->GetRefId());
      if (it != images.end() && it->second) {
        width = it->second->Model().width;
        height = it->second->Model().height;
      }
      break;
    }
    case LayerType::kAlphaVideo: {
      auto& videos = model.GetVideos();
      auto it = videos.find(layer->GetRefId());
      if (it != videos.end() && it->second) {
        width = it->second->GetVideoWidth();
        height = it->second->GetVideoHeight();
      }
      break;
    }
    default:
      return RectF();
  }

  width = fmin(width, model.GetBounds().GetWidth());
  height = fmin(height, model.GetBounds().GetHeight());
  return RectF(0, 0, width, height);
}

void MemoryCalculator::VisitLayers(
    const LayerModelList& layers,
    const std::unordered_map<std::string, LayerModelList>& precomps,
    const std::function<void(LayerModel*)>& visitor) {
  for (const auto& layer : layers) {
    if (!layer) {
      continue;
    }
    visitor(layer.get());
    if (layer->GetLayerType() == LayerType::kPreComp) {
      auto it = precomps.find(layer->GetRefId());
      if (it != precomps.end()) {
        VisitLayers(it->second, precomps, visitor);
      }
    }
  }
}

int64_t MemoryCalculator::CalculateMaskMatteMemory(
    const MemoryCalculatorParams& params) {
  int64_t total_memory = 0;
  if (!params.IsValid()) {
    return total_memory;
  }
  auto& layers = params.model->GetLayers();
  auto& precomps = params.model->GetPrecomps();
  std::unordered_set<int64_t> cached_layer_sizes;

  VisitLayers(layers, precomps, [&](LayerModel* layer) {
    bool has_mask = !layer->GetMasks().empty();
    bool has_matte = layer->IsMatteTarget();
    if (has_mask || has_matte) {
      auto bounds = GetLayerBounds(layer, *params.model);
      int32_t width = static_cast<int32_t>(bounds.GetWidth());
      int32_t height = static_cast<int32_t>(bounds.GetHeight());

      if (width > 0 && height > 0) {
        if (params.scale_factor_x > 0 && params.scale_factor_y > 0) {
          width = width * params.scale_factor_x;
          height = height * params.scale_factor_y;
        }
        // skity will reuse the texture with the same size
        int64_t key = (static_cast<int64_t>(width) << 32) | height;
        if (cached_layer_sizes.find(key) == cached_layer_sizes.end()) {
          int64_t layer_memory = static_cast<int64_t>(width) * height *
                                 MemoryConfig::kBytesPerPixel;
          total_memory += layer_memory;
          cached_layer_sizes.insert(key);
        }
      }
    }
  });

#ifdef OS_IOS
  total_memory += MemoryConfig::kGPUCommandBufferBytes;
#endif

  return total_memory;
}

int64_t MemoryCalculator::CalculateScreenBufferMemory(double width,
                                                      double height) {
  if (width <= 0 || height <= 0) {
    return 0;
  }
  return static_cast<int64_t>(width) * height * MemoryConfig::kBytesPerPixel *
         MemoryConfig::kScreenBufferCount;
}

MemoryUsageInfo MemoryCalculator::CalculateMemoryUsage(
    const MemoryCalculatorParams& params) {
  MemoryUsageInfo info;
  if (!params.IsValid()) {
    return info;
  }

  auto& images = params.model->GetImages();
  for (const auto& pair : images) {
    const auto& image_asset = pair.second;
    if (image_asset) {
      const auto& image_model = image_asset->Model();
      int64_t image_memory =
          CalculateImageMemory(image_model.width, image_model.height);
      info.image_memory += image_memory;
    }
  }

  auto& videos = params.model->GetVideos();
  for (const auto& pair : videos) {
    const auto& video_asset = pair.second;
    if (video_asset) {
      int32_t width = video_asset->GetVideoWidth();
      int32_t height = video_asset->GetVideoHeight();
      info.video_cache_memory += CalculateVideoCacheMemory(width, height);
    }
  }

  auto& layers = params.model->GetLayers();
  auto& precomps = params.model->GetPrecomps();
  VisitLayers(layers, precomps, [&](LayerModel* layer) {
    if (layer->GetLayerType() == LayerType::kAlphaVideo) {
      const std::string& ref_id = layer->GetRefId();
      auto it = videos.find(ref_id);
      if (it != videos.end() && it->second) {
        int32_t width = it->second->GetVideoWidth();
        int32_t height = it->second->GetVideoHeight();
        info.video_texture_memory += CalculateVideoTextureMemory(width, height);
      }
    }
  });

  double final_view_width = params.view_width;
  double final_view_height = params.view_height;

  if (final_view_width <= 0 || final_view_height <= 0) {
    final_view_width = params.model->GetBounds().GetWidth();
    final_view_height = params.model->GetBounds().GetHeight();
  }

  info.mask_matte_memory = CalculateMaskMatteMemory(params);
  info.screen_buffer_memory =
      CalculateScreenBufferMemory(final_view_width, final_view_height);

  info.total_memory = info.image_memory + info.video_texture_memory +
                      info.video_cache_memory + info.mask_matte_memory +
                      info.screen_buffer_memory +
                      MemoryConfig::kBaseMemoryBytes;

  return info;
}

}  // namespace animax
}  // namespace lynx
