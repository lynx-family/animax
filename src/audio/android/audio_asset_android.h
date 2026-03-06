// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_AUDIO_AUDIO_ASSET_ANDROID_H_
#define ANIMAX_SRC_AUDIO_AUDIO_ASSET_ANDROID_H_

#include "base/include/platform/android/scoped_java_ref.h"
#include "src/resource/asset/audio_asset.h"

namespace lynx {
namespace animax {

class AudioAssetAndroid : public AudioAsset {
 public:
  ~AudioAssetAndroid() override = default;
  explicit AudioAssetAndroid(AudioAssetModel model);
  void LoadLocal(const std::string& file_path) override;
  jobject JavaObject() const { return asset_.Get(); }

 private:
  lynx::base::android::ScopedGlobalJavaRef<jobject> asset_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_AUDIO_AUDIO_ASSET_ANDROID_H_
