// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/player/animax_poster_renderer.h"

#include <memory>

#include "src/base/log/log.h"
#include "src/base/util/object_fit_util.h"
#include "src/player/animax_empty_surface.h"
#include "src/player/animax_playback_event_handler.h"
#include "src/render/canvas.h"
#include "src/render/image.h"
#include "src/render/image_gl.h"
#include "src/resource/uri/uri_util.h"

namespace lynx {
namespace animax {
static constexpr int sFakeSize = 100;

AnimaXPosterRenderer::AnimaXPosterRenderer(
    Loader<ResourceRequest, ResourceResponse>::Ptr loader,
    std::weak_ptr<AnimaXPlaybackEventHandler> handler)
    : loader_(loader), weak_playback_handler_(handler) {
  if (!loader_) {
    Invalidate();
  }
}

void AnimaXPosterRenderer::SetPoster(const std::string &poster) {
  if (state_ != static_cast<uint8_t>(PosterRendererState::kWaitForRendering)) {
    return;
  }
  UriInfo info{.scheme = ParseUriScheme(poster),
               .content_type = UriInfo::ContentType::kImage,
               .uri = std::move(poster)};
  ResourceRequest request{.type = ResourceRequestType::kLoadPosterBitmap,
                          .uri_info = info,
                          .width = sFakeSize,
                          .height = sFakeSize};
  loader_->Load(request, [weak_self = weak_from_this()](
                             ResourceResponse response, LoaderError error) {
    auto self = weak_self.lock();
    auto expected =
        static_cast<uint8_t>(PosterRendererState::kWaitForRendering);
    if (!error && self &&
        self->state_.compare_exchange_strong(
            expected,
            static_cast<uint8_t>(PosterRendererState::kRenderOnUIThread))) {
      self->bitmap_ = std::move(response.payload.bitmap);
      self->TryRenderPoster();
    }
  });
}

SurfaceCreationFactory AnimaXPosterRenderer::WrapSurfaceCreationForPoster(
    SurfaceCreationFactory factory) {
  auto expected = static_cast<uint8_t>(PosterRendererState::kWaitForRendering);
  if (!state_.compare_exchange_strong(
          expected,
          static_cast<uint8_t>(PosterRendererState::kRenderOnUIThread))) {
    return factory;
  }
  factory_ = std::move(factory);
  TryRenderPoster();
  return []() { return MakeEmptySurface(sFakeSize, sFakeSize); };
}

SurfaceUpdateFactory AnimaXPosterRenderer::WrapSurfaceUpdateForPoster(
    SurfaceUpdateFactory factory) {
  auto expected = static_cast<uint8_t>(PosterRendererState::kWaitForRendering);
  if (!state_.compare_exchange_strong(
          expected,
          static_cast<uint8_t>(PosterRendererState::kRenderOnUIThread))) {
    return factory;
  }
  auto previous = std::move(factory_);
  factory_ = [factory = std::move(factory),
              previous =
                  std::move(previous)]() -> std::unique_ptr<AnimaXSurface> {
    return factory(previous ? previous() : nullptr);
  };
  TryRenderPoster();
  return [](std::unique_ptr<AnimaXSurface>) {
    return MakeEmptySurface(sFakeSize, sFakeSize);
  };
}

SurfaceCreationFactory AnimaXPosterRenderer::RequestGPURendering() {
  auto expected = static_cast<uint8_t>(PosterRendererState::kWaitForRendering);
  if (state_.compare_exchange_strong(
          expected,
          static_cast<uint8_t>(PosterRendererState::kRenderOnGPUThread))) {
    return std::move(factory_);
  } else if (expected ==
             static_cast<uint8_t>(PosterRendererState::kRenderOnUIThread)) {
    should_rerender_ = true;
  }
  return nullptr;
}

void AnimaXPosterRenderer::TryRenderPoster() {
  DCHECK(state_ ==
         static_cast<uint8_t>(PosterRendererState::kRenderOnUIThread));
  auto expected = static_cast<uint8_t>(PosterRendererState::kRenderOnUIThread);
  if (factory_ && (bitmap_ || image_)) {
    RenderPoster();
  }
  if (!state_.compare_exchange_strong(
          expected,
          static_cast<uint8_t>(PosterRendererState::kWaitForRendering)) ||
      !should_rerender_) {
    return;
  }
  auto handler = weak_playback_handler_.lock();
  if (handler) {
    handler->OnRerender();
  }
}

void AnimaXPosterRenderer::TryRerenderPoster() {
  auto expected = static_cast<uint8_t>(PosterRendererState::kWaitForRendering);
  if (state_.compare_exchange_strong(
          expected,
          static_cast<uint8_t>(PosterRendererState::kRenderOnUIThread))) {
    TryRenderPoster();
  }
}

void AnimaXPosterRenderer::RenderPoster() {
  DCHECK(factory_);
  auto surface = factory_();
  if (!surface || !surface->Valid() || surface->Width() <= 0 ||
      surface->Height() <= 0) {
    return;
  }
  Canvas *canvas = surface->Canvas();
  if (!canvas) {
    return;
  }
  canvas->ResetMatrix();
  if (!image_ && bitmap_) {
    image_.reset(new ImageGL(std::move(bitmap_), canvas->GetRealContext()));
  }
  if (!image_ || image_->GetWidth() <= 0 || image_->GetHeight() <= 0) {
    return;
  }
  RectF src{0, 0, image_->GetWidth(), image_->GetHeight()};
  RectF dst = CalculateObjectFitLayout(surface->Width(), surface->Height(),
                                       image_->GetWidth(), image_->GetHeight(),
                                       object_fit_, object_position_)
                  .rect;
  Paint paint;
  canvas->DrawImageRect(*image_, src, dst, paint);
  surface->Flush();
}

}  // namespace animax
}  // namespace lynx
