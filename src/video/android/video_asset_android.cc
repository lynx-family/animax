// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/video/android/video_asset_android.h"

#include "platform/android/animax_android/src/main/jni/gen/VideoAsset_jni.h"
#include "platform/android/animax_android/src/main/jni/gen/VideoAsset_register_jni.h"
#include "src/base/log/log.h"
#include "src/base/monitor/trace_event.h"
#include "src/base/util/android/jni_convert_helper.h"
#include "src/resource/asset/asset.h"
#include "src/resource/asset/video_asset_model.h"
#include "src/video/video_player.h"

namespace lynx {
namespace animax {

std::shared_ptr<VideoAsset> VideoAsset::Make(VideoAssetModel model) {
  return std::make_shared<VideoAssetAndroid>(std::move(model));
}

VideoAssetAndroid::VideoAssetAndroid(VideoAssetModel model)
    : VideoAsset(std::move(model)) {
  JNIEnv* env = lynx::base::android::AttachCurrentThread();
  asset_.Reset(env, Java_VideoAsset_create(env));
  DCHECK(JavaObject());
}

std::vector<uint8_t> VideoAssetAndroid::GetFrameRawData(int32_t frame) const {
  JNIEnv* env = lynx::base::android::AttachCurrentThread();
  auto ref = Java_VideoAsset_getFrameData(env, JavaObject(), frame);
  return lynx::animax::android::JNIConvertHelper::ConvertJavaBinary(env,
                                                                    ref.Get());
}

std::vector<uint8_t> VideoAssetAndroid::GetVideoParameterSets() const {
  JNIEnv* env = lynx::base::android::AttachCurrentThread();
  auto ref = Java_VideoAsset_getVideoParameterSets(env, JavaObject());
  return lynx::animax::android::JNIConvertHelper::ConvertJavaBinary(env,
                                                                    ref.Get());
}

int VideoAssetAndroid::GetPrevKeyFrame(const int32_t frame) const {
  JNIEnv* env = lynx::base::android::AttachCurrentThread();
  auto ret = Java_VideoAsset_getPrevKeyFrame(env, JavaObject(), frame);
  return ret;
}

bool VideoAssetAndroid::PrepareFrameData(const std::string& video_path) {
  ANIMAX_TRACE_EVENT_ASSET_ID(kPrepareAlphaVideoFrameData, Model().id);
  JNIEnv* env = lynx::base::android::AttachCurrentThread();
  is_valid_ = Java_VideoAsset_loadLocal(
      env, JavaObject(),
      lynx::animax::android::JNIConvertHelper::ConvertToJNIStringUTF(env,
                                                                     video_path)
          .Get());

  frame_count_ = Java_VideoAsset_getFrameCount(env, JavaObject());
  video_width_ = Java_VideoAsset_getWidth(env, JavaObject());
  video_height_ = Java_VideoAsset_getHeight(env, JavaObject());
  frame_rate_ = Java_VideoAsset_getFrameRate(env, JavaObject());

  ANIMAX_LOGI("PrepareFrameData finish, is_valid: "
              << std::to_string(IsValid()) << ", frame_count: " << frame_count_
              << ", video_width: " << video_width_ << ", video_height: "
              << video_height_ << ", frame_rate: " << frame_rate_);
  return IsValid();
}

bool VideoAssetAndroid::IsHEVCFormat() const {
  JNIEnv* env = lynx::base::android::AttachCurrentThread();
  auto ret = Java_VideoAsset_isHEVCFormat(env, JavaObject());
  return ret;
}

bool VideoAssetAndroid::IsSupportDecodeBySystem(
    bool require_hardware_accelerated) const {
  JNIEnv* env = lynx::base::android::AttachCurrentThread();
  auto ret = Java_VideoAsset_isSupportDecodeBySystem(
      env, JavaObject(), require_hardware_accelerated);
  return ret;
}

}  // namespace animax
}  // namespace lynx

namespace animax {
namespace jni {
bool RegisterJNIForVideoAsset(JNIEnv* env) { return RegisterNativesImpl(env); }
}  // namespace jni
}  // namespace animax
