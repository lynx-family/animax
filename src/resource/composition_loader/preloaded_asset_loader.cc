// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/composition_loader/preloaded_asset_loader.h"

#include <limits>

#include "skity/io/data.hpp"
#include "skity/text/typeface.hpp"
#include "src/base/thread/task_runner.h"
#include "src/resource/loader/all_settled_loader.h"
#include "src/resource/loader/lambda_loader.h"

namespace lynx {
namespace animax {
namespace {

PreloadedAssetResponse PrepareAsset(ResourceType type,
                                    const ResourceRequest& request,
                                    ResourceResponse response,
                                    LoaderError error) {
  PreloadedAssetResponse result;
  result.result = {.type = type, .id = request.id, .error = std::move(error)};
  if (result.result.error) {
    // Bitmap decoding happens inside the resource pipeline.
    if (result.result.error.code == kInvalidImage ||
        result.result.error.code == kInvalidImageData ||
        result.result.error.code == kBitmapError) {
      result.stage = PreloadedAssetStage::kPrepare;
    }
    return result;
  }
  result.stage = PreloadedAssetStage::kPrepare;
  if (type == ResourceType::kImage) {
    auto& bitmap = response.payload.bitmap;
    if (response.payload.type != ResourcePayloadType::kBitmap || !bitmap ||
        !bitmap->Pixels() || bitmap->Width() == 0 || bitmap->Height() == 0) {
      result.result.error = {kInvalidImageData, "Image has no decoded pixels"};
    } else if (request.image_size_mode == ImageSizeMode::kExact &&
               (bitmap->Width() != static_cast<uint32_t>(request.width) ||
                bitmap->Height() != static_cast<uint32_t>(request.height))) {
      result.result.error = {
          kInvalidImageData,
          "Image service did not provide the requested dimensions"};
    } else {
      const uint64_t pixels =
          static_cast<uint64_t>(bitmap->Width()) * bitmap->Height();
      const auto bytes_per_pixel = bitmap->BytesPerPixel();
      if (!bytes_per_pixel ||
          pixels > (std::numeric_limits<int64_t>::max() - sizeof(Bitmap)) /
                       bytes_per_pixel) {
        result.result.error = {kInvalidImageData,
                               "Image memory size is out of range"};
        return result;
      }
      result.payload_bytes = sizeof(Bitmap) + pixels * bytes_per_pixel;
      result.bitmap = std::move(bitmap);
    }
  } else if (type == ResourceType::kFont) {
    auto& font_data = response.payload.raw_data;
    if (response.payload.type != ResourcePayloadType::kRawData || !font_data ||
        !font_data->Data() || font_data->Length() == 0) {
      result.result.error = {kInvalidRawData, "Font has no raw data"};
    } else if (static_cast<uint64_t>(font_data->Length()) >
               std::numeric_limits<int64_t>::max() - sizeof(RawData)) {
      result.result.error = {kInvalidRawData,
                             "Font memory size is out of range"};
    } else {
      // IsLoaded only checks byte ownership; Font::MakeFont can silently use
      // a default typeface. Validate with the same parser without fallback so
      // invalid replacement data is reported before binding. Keep the existing
      // lazy Font creation and RawData ownership used by rendering.
      auto bytes = skity::Data::MakeWithProc(
          font_data->Data(), font_data->Length(), nullptr, nullptr);
      if (!skity::Typeface::MakeFromData(bytes)) {
        result.result.error = {kInvalidRawData,
                               "Font data could not be parsed"};
        return result;
      }
      result.payload_bytes =
          sizeof(RawData) + static_cast<uint64_t>(font_data->Length());
      // Preserve FontAsset's lazy parsing when its raw data is published.
      result.font_data = std::move(font_data);
    }
  } else if (type == ResourceType::kAudio) {
    auto asset =
        AudioAsset::Make({.id = request.id, .file_name = request.uri_info.uri});
    if (!asset) {
      result.result.error = {kNoSuchLoader, "Audio preparation is unavailable"};
    } else if (response.payload.type != ResourcePayloadType::kFilePath ||
               response.payload.path.empty()) {
      result.result.error = {kInvalidFilePath, "Audio requires a local file"};
    } else {
      result.result.error = asset->PrepareForPreload(response.payload.path);
      if (!result.result.error) {
        result.payload_bytes = asset->GetMemoryUsageBytes();
        result.audio = std::move(asset);
      }
    }
  } else if (type == ResourceType::kVideo) {
    auto asset =
        VideoAsset::Make({.id = request.id, .file_name = request.uri_info.uri});
    if (!asset) {
      result.result.error = {kNoSuchLoader, "Video preparation is unavailable"};
    } else {
      if (response.payload.type == ResourcePayloadType::kFilePath) {
        result.result.error =
            asset->PrepareFrameDataForPreload(response.payload.path);
      } else if (response.payload.type == ResourcePayloadType::kRawData) {
        result.result.error = {
            kNoSuchLoader, "Independent video data preparation is unavailable"};
      } else {
        result.result.error = {kInvalidRawData, "Invalid video payload"};
      }
      if (!result.result.error) {
        result.payload_bytes = asset->GetMemoryUsageBytes();
        result.video = std::move(asset);
      }
    }
  } else {
    result.result.error = {kInvalidArgument, "Unsupported resource type"};
  }
  return result;
}

}  // namespace

void LoadPreloadedAsset(ResourceType type, ResourceRequest request,
                        Loader<ResourceRequest, ResourceResponse>::Ptr loader,
                        std::function<void(PreloadedAssetResponse)> callback) {
  request.cache_by_content = true;
  auto completion = std::make_shared<decltype(callback)>(std::move(callback));
  auto consume = [type, request, completion](ResourceResponse response,
                                             LoaderError error) {
    fml::TaskRunner::RunNowOrPostTask(
        GetAnimaXResourceThread(),
        [type, request, completion, response = std::move(response),
         error = std::move(error)]() mutable {
          // Service implementations may deliver callbacks from different
          // threads. Serialize and consume the completion once before prepare.
          auto done = std::move(*completion);
          if (done) {
            done(PrepareAsset(type, request, std::move(response),
                              std::move(error)));
          }
        });
  };
  if (!loader) {
    consume({}, {kInvalidLoader, "Resource loader is unavailable"});
    return;
  }
  loader->Load(std::move(request), std::move(consume));
}

void LoadPreloadedAssets(
    std::vector<PreloadedAssetRequest> requests,
    Loader<ResourceRequest, ResourceResponse>::Ptr loader,
    std::function<void(std::vector<PreloadedAssetResponse>)> callback) {
  auto asset_loader =
      MakeLambdaLoader<PreloadedAssetRequest, PreloadedAssetResponse>(
          [loader = std::move(loader)](PreloadedAssetRequest request,
                                       auto callback) {
            auto completion =
                std::make_shared<decltype(callback)>(std::move(callback));
            LoadPreloadedAsset(request.type, std::move(request.request), loader,
                               [completion](PreloadedAssetResponse response) {
                                 (*completion)(std::move(response), {});
                               });
          });
  auto batch_loader =
      AllSettledLoader<PreloadedAssetRequest, PreloadedAssetResponse>::Make<
          AllSettledLoader<PreloadedAssetRequest, PreloadedAssetResponse>>(
          std::move(asset_loader));
  batch_loader->Load(std::move(requests),
                     [callback = std::move(callback)](
                         AllSettledResponse<PreloadedAssetResponse> response,
                         LoaderError) mutable {
                       // Empty batches complete synchronously inside
                       // AllSettledLoader.
                       fml::TaskRunner::RunNowOrPostTask(
                           GetAnimaXResourceThread(),
                           [callback = std::move(callback),
                            responses = std::move(response.outs)]() mutable {
                             callback(std::move(responses));
                           });
                     });
}

}  // namespace animax
}  // namespace lynx
