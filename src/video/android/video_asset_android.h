// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_VIDEO_ANDROID_VIDEO_ASSET_ANDROID_H_
#define ANIMAX_SRC_VIDEO_ANDROID_VIDEO_ASSET_ANDROID_H_

#include "base/include/platform/android/scoped_java_ref.h"
#include "include/base/macros.h"
#include "src/resource/asset/video_asset.h"
#include "src/resource/asset/video_asset_model.h"

namespace lynx {
namespace animax {
class ANIMAX_EXPORT VideoAssetAndroid : public VideoAsset {
 public:
  explicit VideoAssetAndroid(VideoAssetModel model);
  ~VideoAssetAndroid() override = default;

  bool PrepareFrameData(const std::string& video_path) override;
  int32_t GetFrameCount() const override { return frame_count_; }

  std::vector<uint8_t> GetVideoParameterSets() const override;
  std::vector<uint8_t> GetFrameRawData(int32_t frame) const override;
  int GetPrevKeyFrame(const int32_t frame) const override;
  /**
   * Checks whether the current asset format is HEVC/H.265.
   *
   * @return true if the asset format is HEVC/H.265, false otherwise
   */
  bool IsHEVCFormat() const;
  /**
   * Checks if the asset is supported decoding by system.
   *
   * @param require_hardware_accelerated Whether hardware-accelerated decoding
   * is required:
   *   - When require_hardware_accelerated == true, API >= 29: Strictly checks
   * for hardware-accelerated decoding support; API < 29: Checks for decoding
   * support (include both hardware-accelerated decoding and software decoding,
   * since hardware-accelerated detection API is only available from API 29
   * onwards)
   *   - When require_hardware_accelerated == false: Checks for decoding support
   * (include both hardware-accelerated decoding and software decoding)
   * @return true if supported, false otherwise
   */
  bool IsSupportDecodeBySystem(bool require_hardware_accelerated) const;
  jobject JavaObject() const { return asset_.Get(); }

 private:
  lynx::base::android::ScopedGlobalJavaRef<jobject> asset_;
  int32_t frame_count_ = 0;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_VIDEO_ANDROID_VIDEO_ASSET_ANDROID_H_
