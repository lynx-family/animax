// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PLAYER_ANIMAX_RENDERER_H_
#define ANIMAX_SRC_PLAYER_ANIMAX_RENDERER_H_

#include <memory>
#include <string>

#include "base/include/lynx_actor.h"
#include "include/player/animax_fit_position.h"
#include "include/player/animax_surface.h"
#include "src/base/monitor/metrics_data_source.h"
#include "src/base/monitor/timestamp_recorder.h"
#include "src/property/property_updater.h"

namespace lynx {
namespace animax {

struct AnimaXPlayerContext;
class AnimaXPlayer;
class AnimaXMainController;
class AnimaXPlaybackEventHandler;
class CompositionModel;
class CompositionLayer;
class LayerModel;
class Matrix;
class AnimaXMetricsManager;
class Canvas;
class AnimaXPropertyUpdater;
class AudioController;

class AnimaXRenderer : public MetricsDataSource {
 public:
  explicit AnimaXRenderer(
      std::weak_ptr<AnimaXPlaybackEventHandler> weak_playback_handler);
  ~AnimaXRenderer();

  void Init(std::shared_ptr<AnimaXPlayerContext>);

  // Surface management
  void CreateSurface(SurfaceCreationFactory creation_factory);
  void UpdateSurface(SurfaceUpdateFactory update_factory);
  double GetWidth() const { return width_; }
  double GetHeight() const { return height_; }

  // Composition management
  void UpdateComposition(int32_t src_index,
                         std::shared_ptr<CompositionModel> model);
  CompositionLayer* GetCompositionLayer() const { return layer_.get(); }
  std::shared_ptr<CompositionModel> GetComposition() const { return model_; }
  bool IsCompositionAssetsLoaded();
  void EnsureSubscribeValidOrWarn();

  // Rendering operation
  void Render(double progress);

  // Tap event handler
  void OnTap(float x, float y);

  // Object fit and position
  void SetObjectFit(const ObjectFit object_fit);
  void SetObjectPosition(const ObjectPosition object_position);

  // Utility methods
  void Reload();
  void Destroy();
  void MarkPlatformSurfaceAsInvalid(bool is_invalid);

  // Property update
  void GetKeysForKeyPath(std::unique_ptr<AnimaXKeyPath> key_path,
                         AnimaXKeyPathCallback callback);
  void UpdateLayerProperty(std::unique_ptr<LayerStaticRequest> request);
  void SetResourceProperty(std::unique_ptr<ResourceUpdateRequest> request);
  void AddLayerPropertyCallback(std::unique_ptr<LayerCallbackRequest> request);

  // FPS event handling
  void SetFpsEventInterval(long interval);

  // MetricsDataSource implementation
  void Trace(TraceEventType type) override;
  TimestampArray ExportTimestamps() const override;
  MetricsMap ExportMetricsMap() const override;

  // audio
  void AddAudioController(std::weak_ptr<AudioController> controller);
  void SetVolume(double volume);
  void SetMute(bool mute);
  void OnResume();
  void OnPause();
  void OnEnd();
  void OnCancel();

 private:
  std::unique_ptr<AnimaXSurface> ReleaseSurface();
  void UpdateSurfaceInternal(std::unique_ptr<AnimaXSurface> surface);
  void ResizeCanvas(Canvas& canvas);
  void NotifyFirstFrameIfNeeded();

  void StartAnimationIfNeeded();
  void StartAnimation();

  std::unique_ptr<AnimaXSurface> surface_;
  bool has_surface_before_ = false;

  /**
   * Indicates if the platform surface was invalid at creation (e.g., size 0x0
   * on Android). This flag prevents flush operations for fake surfaces,
   * avoiding rendered artifacts.
   */
  bool is_invalid_platform_surface_ = false;

  std::shared_ptr<CompositionModel> model_;
  std::unique_ptr<CompositionLayer> layer_;
  std::unique_ptr<LayerModel> layer_model_;
  std::unique_ptr<Matrix> canvas_matrix_;

  ObjectFit object_fit_ = ObjectFit::kContain;
  ObjectPosition object_position_ = ObjectPosition::kCenter;

  std::weak_ptr<AnimaXPlayerContext> weak_context_;
  std::weak_ptr<shell::LynxActor<AnimaXMainController>> weak_controller_actor_;
  std::weak_ptr<AnimaXPlaybackEventHandler> weak_playback_handler_;

  // State variables
  float width_ = 0.f;
  float height_ = 0.f;
  int32_t model_width_ = 0;
  int32_t model_height_ = 0;

  bool has_rendered_first_frame_ = false;
  bool is_destroyed_ = false;

  std::unique_ptr<AnimaXPropertyUpdater> property_updater_;
  PeriodicalTimestampRecorder gpu_thread_recorder_;

  // audio
  std::vector<std::weak_ptr<AudioController>> audio_controllers_;
  bool mute_ = false;
  double volume_ = 0;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PLAYER_ANIMAX_RENDERER_H_
