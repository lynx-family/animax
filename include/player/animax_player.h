// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_INCLUDE_PLAYER_ANIMAX_PLAYER_H_
#define ANIMAX_INCLUDE_PLAYER_ANIMAX_PLAYER_H_

#include <atomic>
#include <functional>
#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "base/include/closure.h"
#include "base/include/lynx_actor.h"
#include "include/base/animax_metrics.h"
#include "include/base/macros.h"
#include "include/base/player_event_tracker.h"
#include "include/base/visibility_state.h"
#include "include/player/animax_event.h"
#include "include/player/animax_fit_position.h"
#include "include/player/animax_surface.h"
#include "include/property/property_update_request.h"

namespace lynx {
namespace animax {
class LayerModel;
class AnimaXValueAnimator;
class CompositionModelPipeline;
class CompositionLayer;
class CompositionModel;
class Canvas;
class Matrix;
class Surface;
class VSyncMonitor;
class AnimaXAbility;
class AnimaXMetricsManager;
class GPUThreadHolder;
class AnimaXEventDispatcher;
class AnimaXPlaybackEventHandler;
class AnimaXMainController;
class AnimaXRenderer;
class AnimaXCompositionLoader;
class AnimaXPlayerBuilder;
struct AnimaXPlayerContext;
struct CompositionAssetResponse;
struct LoaderError;

class ANIMAX_EXPORT AnimaXPlayer
    : public std::enable_shared_from_this<AnimaXPlayer> {
 public:
  /**
   * Destructor.
   * IMPORTANT: Always call Destroy() before destroying the player instance.
   */
  ~AnimaXPlayer();
  /**
   * Destroy all resources associated with this AnimaXPlayer.
   * Must be called when the player is no longer needed to release resources
   * properly.
   */
  void Destroy();
  /**
   * Reset the animation status to initial state.
   */
  void Reload();
  /**
   * Create a Surface object using the provided factory function.
   * The factory function will be executed on the GPU thread.
   * @param creation_factory Factory function that creates and returns an
   * AnimaXSurface.
   */
  void CreateSurface(SurfaceCreationFactory creation_factory);
  /**
   * Update the current surface using the provided factory function.
   * The factory function will be executed on the GPU thread.
   * @param update_factory Factory function that creates and returns an
   * AnimaXSurface.
   */
  void UpdateSurface(SurfaceUpdateFactory update_factory);
  /**
   * Set the path to the image folder.
   * Must be called before SetJson or SetSrc.
   * @param image_folder Path string to the image folder.
   */
  void SetImageFolder(std::string image_folder);
  /**
   * Set polyfill mappings for source paths.
   * @param polyfill Unordered map of source key-value pairs for polyfill.
   */
  void SetSrcPolyfill(std::unordered_map<std::string, std::string> polyfill);
  /**
   * Set the composition model for this player.
   * @param model Shared pointer to CompositionModel instance.
   */
  void SetComposition(std::shared_ptr<CompositionModel> model);
  /**
   * Set the animation source from a file path or URL.
   * @param src Source string path or URL.
   */
  void SetSrc(const std::string &src);
  /**
   * Set the animation source from a JSON string.
   * @param json Pointer to a null-terminated JSON string.
   */
  void SetJson(const char *json);
  /**
   * Set the animation source from a JSON string.
   * @param json JSON string.
   */
  void SetJson(std::string json);
  /**
   * Enable or disable looping of the animation.
   * @param loop True to enable looping, false to disable.
   */
  void SetLoop(const bool loop);
  /**
   * Set the number of times the animation should loop.
   * @param loop_count Number of loops, -1 for infinite looping.
   */
  void SetLoopCount(const int32_t loop_count);
  /**
   * Enable or disable auto-reverse playback.
   * @param auto_reverse True to enable auto-reverse, false otherwise.
   */
  void SetAutoReverse(const bool auto_reverse);
  /**
   * Set the playback speed multiplier.
   * @param speed Playback speed, where 1.0 is normal speed.
   */
  void SetSpeed(const double speed);
  /**
   * Set the current progress of the animation.
   * @param progress Value between 0.0 and 1.0 representing animation progress.
   */
  void SetProgress(const double progress);
  /**
   * Enable or disable autoplay on animation load.
   * @param autoplay True to autoplay, false otherwise.
   */
  void SetAutoplay(const bool autoplay);
  /**
   * Enable or disable dynamic resource loading.
   * @param dynamic True to enable dynamic resource loading.
   */
  void SetDynamicResource(bool dynamic);
  /**
   * Set the start frame for playback.
   * @param start_frame Frame index to start playback from.
   */
  void SetStartFrame(const double start_frame);
  /**
   * Set the end frame for playback.
   * @param end_frame Frame index to end playback at.
   */
  void SetEndFrame(const double end_frame);
  /**
   * Enable or disable keeping the last frame visible after playback ends.
   * @param keep_last_frame True to keep last frame, false otherwise.
   */
  void SetKeepLastFrame(const bool keep_last_frame);
  /**
   * Set the interval for FPS event callbacks.
   * @param interval Interval in milliseconds.
   */
  void SetFpsEventInterval(const long interval);
  /**
   * Set the maximum frame rate for playback.
   * @param max_frame_rate Maximum frames per second.
   */
  void SetMaxFrameRate(const double max_frame_rate);
  /**
   * Set the object-fit property for rendering.
   * @param object_fit ObjectFit enum value.
   */
  void SetObjectFit(const ObjectFit object_fit);
  /**
   * Set the object-position property for rendering.
   * @param object_position ObjectPosition enum value.
   */
  void SetObjectPosition(const ObjectPosition object_position);
  /**
   * Mutes the audio layer.
   * The audio layer could be muted and unmuted when the animation is playing.
   * @param muted if true, mutes audio layer.
   */
  void SetMuted(bool mute);

  /**
   * Enables or disables audio layer.
   * The audio layer will not be parsed when disabled.
   * Call this function before `SetSrc`.
   * @param enable if true, enables audio layer.
   */
  void SetEnableAudio(bool enable);

  /**
   * Start or resume playback of the animation.
   */
  void Play();
  /**
   * Pause the animation playback.
   */
  void Pause();
  /**
   * Resume the animation playback if paused.
   */
  void Resume();
  /**
   * Stop the animation playback.
   */
  void Stop();
  /**
   * Seek to a specific frame in the animation.
   * @param frame Frame index to seek to.
   */
  void Seek(double frame);
  /**
   * Subscribe to update events at a specific frame.
   * @param frame Frame index to subscribe to.
   */
  void SubscribeUpdateEvent(int32_t frame);
  /**
   * Unsubscribe from update events at a specific frame.
   * @param frame Frame index to unsubscribe from.
   */
  void UnsubscribeUpdateEvent(int32_t frame);
  /**
   * Subscribe or unsubscribe to multiple update events.
   * @param frames Set of frame indices to subscribe/unsubscribe.
   * @param subscribe True to subscribe, false to unsubscribe.
   */
  void SubscribeUpdateEvents(std::unordered_set<int32_t> frames,
                             bool subscribe);
  /**
   * Get the current playback frame of the animation within the start and end
   * frame range. Note: Only valid on the JS thread.
   * @return Current playback frame.
   */
  double GetCurrentFrame();
  /**
   * Get the current animation progress processed by the player.
   * Note: Only valid on the JS thread.
   * @return Current progress in the range [0, 1].
   */
  double GetProgress();
  /**
   * Play a segment of the animation from start_frame to end_frame.
   * @param start_frame Frame index to start playback.
   * @param end_frame Frame index to end playback.
   */
  void PlaySegment(double start_frame, double end_frame);
  /**
   * Get the current width of the animation surface.
   * @return Width in pixels.
   */
  double GetWidth();
  /**
   * Get the current height of the animation surface.
   * @return Height in pixels.
   */
  double GetHeight();
  /**
   * Get the total duration of the current animation in milliseconds.
   * Note: Only valid on the JS thread.
   * @return Duration in milliseconds.
   */
  double GetDurationMs();
  /**
   * Check whether the animation is currently playing.
   * Thread-safe but may be slightly inaccurate.
   * @return True if animating, false otherwise.
   */
  bool IsAnimating();
  /**
   * Notify that the player is visible.
   * Resumes animation if necessary.
   * @param state Current visibility state.
   */
  void OnShow(VisibilityState state);
  /**
   * Notify that the player is not visible.
   * Pauses animation playback.
   * @param state Current visibility state.
   */
  void OnHide(VisibilityState state);
  /**
   * Get performance metrics collected by the player.
   * @return Unordered map of metric names to their double values.
   */
  std::unordered_map<std::string, double> GetPerfMetrics();
  /**
   * Get the array of tracked player events.
   * @return EventArray containing tracked events.
   */
  PlayerEventTracker::EventArray GetEventTrackingArray();
  /**
   * Get the names of tracked events.
   * @return Reference to an array of event names.
   */
  const PlayerEventTracker::EventNameArray &GetEventNames();
  /**
   * Notify the player of a progress update.
   *
   * @param progress Normalized progress value (0.0 to 1.0).
   * @param current_frame Current frame index.
   */
  void OnProgress(double progress, double current_frame);
  /**
   * Handle a tap event at the specified coordinates.
   * @param x X coordinate of the tap.
   * @param y Y coordinate of the tap.
   */
  void OnTap(float x, float y);
  /**
   * Callback for GetLayerBounds result.
   * params are: success, x, y, width, height
   */
  using LayerBoundsCallback =
      base::MoveOnlyClosure<void, bool, float, float, float, float>;
  /**
   * Get the bounds of a specific layer by name.
   * This is an async operation, result will be returned via callback.
   *
   * @param layer_name The name of the layer to search for (UTF-8 encoded),
   * must not be empty
   * @param bounds_space The coordinate space of the layer bounds.
   * Default: kParent
   * @param callback The callback to receive the result, will be called on the
   * main thread
   */
  void GetLayerBounds(const std::string &layer_name,
                      LayerBoundsSpace bounds_space,
                      LayerBoundsCallback callback);
  /**
   * Mark the platform surface as invalid or valid.
   * @param isInvalid True to mark as invalid, false otherwise.
   */
  void MarkPlatformSurfaceAsInvalid(bool isInvalid);
  /**
   * Export performance data from the metrics manager.
   * @param callback Callback to be called when external metrics are ready.
   */
  void ExportDataFromMetricsManager(ExternalMetricsReadyCallback callback);
  /**
   * Get the keys for a given key path.
   * @param keyPath The key path to query.
   * @param callback Callback to receive the key path results.
   */
  void GetKeysForKeyPath(std::unique_ptr<AnimaXKeyPath> keyPath,
                         AnimaXKeyPathCallback callback);
  /**
   * Updates a static layer property on the GPU thread.
   * @param request The layer static property update request
   */
  void UpdateLayerProperty(std::unique_ptr<LayerStaticRequest> request);
  /**
   * Updates a resource property on the GPU thread.
   * @param request The resource property update request
   */
  void SetResourceProperty(std::unique_ptr<ResourceUpdateRequest> request);
  /**
   * Adds a value callback for dynamic layer property updates on the GPU thread.
   * @param request The layer callback request
   */
  void AddLayerPropertyCallback(std::unique_ptr<LayerCallbackRequest> request);
  /**
   * Loads the assets for the currently loaded composition model.
   * This request is usually called after the dynamic property update is
   * complete.
   * @param completion - Callback function to be invoked when the loading is
   * finished.
   */
  void LoadAssetsWithCallback(base::MoveOnlyClosure<void> completion);
  /**
   * Get estimated memory usage (in byte)
   */
  int64_t GetEstimatedMemoryUsage();

  /**
   * Set estimated memory usage (in byte)
   */
  void SetEstimatedMemoryUsage(int64_t usage);

#ifdef OS_IOS
  /**
   * Notify that the app has entered foreground.
   * Must be called on the main thread.
   * Resumes GPU thread rendering.
   */
  void OnAppEnterForeground();
  /**
   * Notify that the app has entered background.
   * Must be called on the main thread.
   * Pauses GPU thread rendering to save resources.
   */
  void OnAppEnterBackground();
#endif

 private:
  friend class AnimaXPlayerBuilder;

  explicit AnimaXPlayer(AnimaXPlayerBuilder &builder);

  void Init(AnimaXPlayerBuilder &builder);
  void LoadCompositionAssets(std::shared_ptr<CompositionModel> composition,
                             base::MoveOnlyClosure<void> completion);
  void OnCompositionLoaded(int32_t src_index, CompositionAssetResponse &res,
                           LoaderError &error);
  void UpdateComposition(int32_t src_index,
                         std::shared_ptr<CompositionModel> model);
  bool EnableDynamicResourceFeature();

  float scale_ = 1.f;
  struct SourceState {
    std::string src;
    std::size_t json_hash = 0;
    std::size_t json_size = 0;
    std::atomic<int32_t> index{0};

    int32_t GetIndex() { return index.load(); }

    void Reset() {
      src.clear();
      json_hash = 0;
      json_size = 0;
      index++;
    }

    // Returns true if src changed
    bool SetSrc(const std::string &new_src) {
      if (!src.empty() && src == new_src) {
        return false;
      }
      src = new_src;
      json_hash = 0;
      json_size = 0;
      index++;
      return true;
    }

    // Returns true if the json content is different from the current state.
    bool SetJson(const std::string &new_json) {
      std::size_t new_size = new_json.size();
      std::size_t new_hash = std::hash<std::string>{}(new_json);
      if (json_size == new_size && json_hash == new_hash) {
        return false;
      }
      src.clear();
      json_size = new_size;
      json_hash = new_hash;
      index++;
      return true;
    }
  };
  SourceState source_state_;

  // Loader for resource thread operations
  std::shared_ptr<shell::LynxActor<AnimaXCompositionLoader>> loader_actor_;

  // Multi thread
  std::shared_ptr<AnimaXMetricsManager> metrics_manager_;
  std::shared_ptr<AnimaXAbility> ability_;

  bool disable_playback_on_asset_load_failure_ = false;

  // Main controller for all main thread operations
  std::shared_ptr<GPUThreadHolder> gpu_thread_holder_;
  std::shared_ptr<shell::LynxActor<AnimaXMainController>> controller_actor_;

  // Renderer for all GPU thread operations
  std::shared_ptr<shell::LynxActor<AnimaXRenderer>> renderer_actor_;
  std::shared_ptr<AnimaXPlaybackEventHandler> playback_handler_;

  std::shared_ptr<AnimaXPlayerContext> player_context_;

  std::atomic<int64_t> estimated_memory_usage_{0};

#ifdef OS_IOS
  bool is_in_background_ = false;
  std::mutex background_mutex_;
  std::condition_variable background_cv_;
#endif
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_INCLUDE_PLAYER_ANIMAX_PLAYER_H_
