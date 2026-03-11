// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_ASSET_VIDEO_ASSET_H_
#define ANIMAX_SRC_RESOURCE_ASSET_VIDEO_ASSET_H_

#include <array>
#include <memory>
#include <string>
#include <vector>

#include "src/resource/asset/asset.h"
#include "src/resource/asset/video_asset_model.h"

namespace lynx {
namespace animax {

class VideoAsset : public BaseAsset<VideoAsset>,
                   public std::enable_shared_from_this<VideoAsset> {
 public:
  /**
   * Factory method to create and initialize a shared VideoAsset instance.
   *
   * @param model Video Asset Model
   * @return Shared pointer to the initialized VideoAsset.
   */
  static std::shared_ptr<VideoAsset> Make(VideoAssetModel model);
  ~VideoAsset() override = default;

  /**
   * Loads the video from a local path.
   *
   * @param path Local file system path to the video file.
   */
  void LoadLocal(const std::string& path);

  /**
   * Prepares video frame data for playback.
   * This function is platform-specific and left for specific implementation on
   * different platforms.
   *
   * @param video_path Path to the video file.
   * @return true if frame data is successfully prepared; false otherwise.
   */
  virtual bool PrepareFrameData(const std::string& video_path) = 0;

  /**
   * Retrieves the total number of frames in the video.
   * This function is platform-specific and left for specific implementation on
   * different platforms.
   * @return The total number of frames.
   */
  virtual int32_t GetFrameCount() const = 0;
  /**
   * Retrieves the previous keyframe index before or at the specified frame.
   * @param frame The current frame index from which to find the previous
   * keyframe.
   * @return The index of the previous keyframe that is <= the specified frame.
   *         Returns -1 if no keyframe is found or if the input is invalid.
   */
  virtual int GetPrevKeyFrame(const int32_t frame) const { return -1; }
  /**
   * Retrieves the video parameter sets (e.g., VPS, SPS, PPS for HEVC).
   * This function extracts the parameter sets needed to initialize the video
   * decoder. The implementation is platform-specific.
   * @return A vector containing the concatenated parameter set NAL units.
   */
  virtual std::vector<uint8_t> GetVideoParameterSets() const { return {}; }
  /**
   * Retrieves raw frame data for the specified frame index.
   * This function copies the compressed video data for a single frame from the
   * video asset. The implementation handles platform-specific frame data access
   * and memory management.
   * @param frame The zero-based index of the frame to retrieve.
   * @return A vector containing the raw frame data, or empty vector if frame is
   * invalid.
   */
  virtual std::vector<uint8_t> GetFrameRawData(int32_t frame) const {
    return {};
  }

  int32_t GetVideoWidth() const { return video_width_; }
  int32_t GetVideoHeight() const { return video_height_; }
  float GetFrameRate() const { return frame_rate_; }
  int32_t GetRgbX() const { return model_.rgb_frame[0]; }
  int32_t GetRgbY() const { return model_.rgb_frame[1]; }
  int32_t GetRgbWidth() const { return model_.rgb_frame[2]; }
  int32_t GetRgbHeight() const { return model_.rgb_frame[3]; }
  int32_t GetAlphaX() const { return model_.a_frame[0]; }
  int32_t GetAlphaY() const { return model_.a_frame[1]; }
  int32_t GetAlphaWidth() const { return model_.a_frame[2]; }
  int32_t GetAlphaHeight() const { return model_.a_frame[3]; }

  /**
   * Checks if the video asset is considered valid.
   *
   * @return true if valid; false otherwise.
   */
  bool IsValid() const { return is_valid_.load(); }
  /**
   * Returns the current VideoAssetModel.
   *
   * @return Constant reference to the VideoAssetModel.
   */
  const VideoAssetModel& Model() const { return model_; }
  /**
   * Resets the video asset model with new information.
   * After resetting, the asset is considered invalid.
   * @param info New VideoAssetModel to replace the current one.
   */
  void ResetModel(VideoAssetModel info);

  /**
   * Compatibility with legacy issue
   * Some configs don't contain param rgbFrame and aFrame, so it's necessary to
   * recover these params from video file.
   */
  void RecoverFrameIfNecessary();

  /**
   * Constructs a full URL (dir_name and file_name) to access the video based on
   * a given prefix.
   *
   * @param prefix URL prefix.
   * @return Full URL as a string.
   */
  std::string ComposeUrl(const std::string& prefix) const;

 protected:
  explicit VideoAsset(VideoAssetModel info);

  VideoAssetModel model_{};
  int32_t video_width_ = 0;
  int32_t video_height_ = 0;
  double frame_rate_ = 0.f;

  std::atomic<bool> is_valid_{false};
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_ASSET_VIDEO_ASSET_H_
