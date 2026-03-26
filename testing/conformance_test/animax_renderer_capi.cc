// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "testing/conformance_test/animax_renderer_capi.h"

#include <chrono>
#include <cstring>
#include <future>
#include <iostream>
#include <string>

#include "base/include/no_destructor.h"
#include "include/resource/bitmap.h"
#include "src/base/log/log.h"
#include "src/layer/textra/text_helper_textra.h"
#include "src/model/composition_model.h"
#include "src/render/surface_gl.h"
#include "src/resource/composition_loader/composition_asset_task.h"
#include "src/resource/log_util.h"
#include "testing/conformance_test/animax_renderer_osmesa.h"
#include "testing/conformance_test/osmesa_context.h"
#include "testing/conformance_test/resource_pipeline.h"

namespace {

void EnsureGPUContext() {
  lynx::animax::AnimaXOSMesaContext::ThreadLocalInstance().MakeCurrent();
  static lynx::base::NoDestructor<std::shared_ptr<skity::GPUContext>> context{
      lynx::animax::GetGPUContext()};
}

void EnsureTextra() {
  static lynx::base::NoDestructor<lynx::animax::TextHelperTextra> impl;
  lynx::animax::TextHelper::RegisterImpl(&(*impl));
}

}  // namespace

struct _AnimaXRenderTask {
  lynx::animax::AnimaXRendererOSMesa renderer;
};

struct _AnimaXRenderResult {
  std::unique_ptr<lynx::animax::Bitmap> bitmap;
  std::string error;
};

AnimaXRenderTask AnimaXRenderTaskCreate(const char* lottie_json_file_path,
                                        int32_t width, int32_t height) {
  std::promise<std::shared_ptr<lynx::animax::CompositionModel>> model_promise{};
  auto model_future = model_promise.get_future();
  lynx::animax::GetSharedCompoistionPipeline()->LoadCompositionModelFromURI(
      lottie_json_file_path, 1.f,
      [&model_promise](lynx::animax::CompositionAssetResponse result,
                       auto error) mutable {
        if (error) {
          std::cerr << "Failed to load composition model, error: " << error
                    << std::endl;
          model_promise.set_value(nullptr);
          return;
        }

        for (auto& asset_response : result.asset_responses) {
          if (asset_response.error) {
            std::cerr << "Failed to asset: " << asset_response.id
                      << "error: " << asset_response.error << std::endl;
            model_promise.set_value(nullptr);
            return;
          }
        }

        model_promise.set_value(result.model);
      });

  auto status = model_future.wait_for(std::chrono::seconds{10});
  if (status != std::future_status::ready) {
    std::cerr << "Failed to load composition model, timeout!" << std::endl;
    return nullptr;
  }

  EnsureTextra();
  EnsureGPUContext();
  auto model = model_future.get();
  DCHECK(model != nullptr);
  return new _AnimaXRenderTask{
      .renderer = lynx::animax::AnimaXRendererOSMesa{model, width, height},
  };
}

void AnimaXRenderTaskDestroy(AnimaXRenderTask task) { delete task; }

AnimaXRenderResult AnimaXRenderTaskRender(AnimaXRenderTask task,
                                          int32_t frame) {
  auto& renderer = task->renderer;
  auto bitmap = renderer.DrawFrame(frame);
  if (!bitmap || bitmap->Height() == 0 || bitmap->Width() == 0 ||
      bitmap->Pixels() == nullptr) {
    return new _AnimaXRenderResult{
        .error = "Failed to render frame",
    };
  }
  return new _AnimaXRenderResult{
      .bitmap = std::move(bitmap),
  };
}
void AnimaXRenderResultDestroy(AnimaXRenderResult result) { delete result; }

int32_t AnimaXRenderResultGetRGBABitmapSize(AnimaXRenderResult result) {
  auto size = result->bitmap->Height() * result->bitmap->Width() *
              result->bitmap->BytesPerPixel();
  return size;
}

void AnimaXRenderResultCopyRGBABitmap(AnimaXRenderResult result,
                                      uint8_t* bitmap) {
  auto* bitmap_data = result->bitmap->Pixels();
  auto size = AnimaXRenderResultGetRGBABitmapSize(result);
  memcpy(bitmap, bitmap_data, size);
}
