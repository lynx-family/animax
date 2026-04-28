// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PLAYER_ANIMAX_POSTER_RENDERER_H_
#define ANIMAX_SRC_PLAYER_ANIMAX_POSTER_RENDERER_H_

#include <string>

#include "include/player/animax_fit_position.h"
#include "include/player/animax_surface.h"
#include "include/resource/loader.h"
#include "include/resource/resource_task.h"
#include "src/model/rect_model.h"

namespace lynx {
namespace animax {

class Image;
class AnimaXPlaybackEventHandler;

class AnimaXPosterRenderer
    : public std::enable_shared_from_this<AnimaXPosterRenderer> {
 public:
  AnimaXPosterRenderer(Loader<ResourceRequest, ResourceResponse>::Ptr loader,
                       std::weak_ptr<AnimaXPlaybackEventHandler> handler);
  inline void Invalidate() {
    state_ = static_cast<uint8_t>(PosterRendererState::kInvalidPoster);
  }

  // UI Method
  void SetPoster(const std::string& poster);
  SurfaceCreationFactory WrapSurfaceCreationForPoster(
      SurfaceCreationFactory factory);
  SurfaceUpdateFactory WrapSurfaceUpdateForPoster(SurfaceUpdateFactory factory);
  inline void SetObjectFit(ObjectFit object_fit) {
    object_fit_ = object_fit;
    TryRerenderPoster();
  }
  inline void SetObjectPosition(ObjectPosition object_position) {
    object_position_ = object_position;
    TryRerenderPoster();
  }
  void TryRerenderPoster();

  // GPU Method
  SurfaceCreationFactory RequestGPURendering();

 private:
  enum class PosterRendererState : uint8_t {
    kWaitForRendering,
    kRenderOnUIThread,
    kRenderOnGPUThread,
    kInvalidPoster
  };
  void TryRenderPoster();
  void RenderPoster();
  std::atomic<uint8_t> state_ =
      static_cast<uint8_t>(PosterRendererState::kWaitForRendering);
  std::atomic<bool> should_rerender_ = false;
  SurfaceCreationFactory factory_;
  Loader<ResourceRequest, ResourceResponse>::Ptr loader_;
  std::unique_ptr<Bitmap> bitmap_;
  std::unique_ptr<Image> image_;
  std::weak_ptr<AnimaXPlaybackEventHandler> weak_playback_handler_;
  ObjectFit object_fit_ = ObjectFit::kContain;
  ObjectPosition object_position_ = ObjectPosition::kCenter;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PLAYER_ANIMAX_POSTER_RENDERER_H_
