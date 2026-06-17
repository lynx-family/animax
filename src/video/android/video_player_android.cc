// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/video/android/video_player_android.h"

#include <cstring>
#include <utility>

#include "base/include/platform/android/scoped_java_ref.h"
#include "platform/android/animax_android/src/main/jni/gen/IVideoPlayer_jni.h"
#include "platform/android/animax_android/src/main/jni/gen/IVideoPlayer_register_jni.h"
#include "src/base/gl/scoped_gl_reset_restore.h"
#include "src/base/log/log.h"
#include "src/base/thread/thread_assert.h"
#include "src/player/android/animax_ability_android.h"
#include "src/player/android/gl_context_android.h"
#include "src/render/texture_info_gl.h"
#include "src/video/android/video_asset_android.h"

namespace lynx {
namespace animax {

VideoPlayerAndroid::VideoPlayerAndroid(std::shared_ptr<AnimaXAbility> ability) {
  JNIEnv *env = lynx::base::android::AttachCurrentThread();
  if (ability) {
    backend_ = ability->GetBackend();
    auto android_ability =
        std::static_pointer_cast<AnimaXAbilityAndroid>(std::move(ability));
    if (android_ability) {
      player_.Reset(env, android_ability->CreateVideoPlayer(
                             reinterpret_cast<jlong>(this)));
    }
  }
  DCHECK(player_.Get());
  transform_.fill(0.f);
}

VideoPlayerAndroid::~VideoPlayerAndroid() {
  if (video_texture_) {
    ThreadAssert::Assert(ThreadAssert::Type::kGPU);
    // Vulkan path: ensure the offscreen EGL context is current before deleting
    // the GL texture (the GPU thread has no GL context otherwise). On the GL
    // backend ScopedEGLContext is a no-op (context already current).
    ScopedEGLContext egl(backend_ == ContextBackend::kVulkan);
    if (egl.ready()) {
      glDeleteTextures(1, &video_texture_);
    } else if (backend_ == ContextBackend::kVulkan) {
      ANIMAX_LOGE(
          "VideoPlayerAndroid dtor: MakeCurrent failed, skip "
          "glDeleteTextures to avoid UB (texture may leak)");
    }
  }
  JNIEnv *env = lynx::base::android::AttachCurrentThread();
  Java_IVideoPlayer_destroy(env, player_.Get());
}

std::unique_ptr<TextureInfo> VideoPlayerAndroid::UpdateTexture(
    const int32_t frame) {
  ThreadAssert::Assert(ThreadAssert::Type::kGPU);
  const int32_t to_frame = frame;
  DCHECK(0 <= to_frame && to_frame < asset_->GetFrameCount());
  if (current_frame_ == to_frame) {
    // don't need draw
    return nullptr;
  }
  JNIEnv *env = lynx::base::android::AttachCurrentThread();
  bool success = Java_IVideoPlayer_updateSurface(env, player_.Get(), to_frame);
  if (!success) {
    // don't need draw
    return nullptr;
  }
  lynx::base::android::ScopedLocalJavaRef<jfloatArray> transform =
      Java_IVideoPlayer_getTransformMatrix(env, player_.Get());
  float *transform_matrix = env->GetFloatArrayElements(transform.Get(), 0);
  memcpy(transform_.data(), transform_matrix,
         transform_.size() * sizeof(float));
  current_frame_ = to_frame;
  DCHECK(video_texture_);
  return std::make_unique<TextureInfoGL>(
      video_texture_, asset_->GetVideoWidth(), asset_->GetVideoHeight(),
      GL_TEXTURE_EXTERNAL_OES);
}

const std::array<float, 16> &VideoPlayerAndroid::GetTransform() {
  return transform_;
}

void VideoPlayerAndroid::CreateVideoTexture() {
  ThreadAssert::Assert(ThreadAssert::Type::kGPU);
  lynx::animax::ScopedGLResetRestore s(GL_TEXTURE_BINDING_EXTERNAL_OES);
  glGenTextures(1, &video_texture_);
  glBindTexture(GL_TEXTURE_EXTERNAL_OES, video_texture_);
  glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  JNIEnv *env = lynx::base::android::AttachCurrentThread();
  Java_IVideoPlayer_setSurface(env, player_.Get(), video_texture_);
}

void VideoPlayerAndroid::AttachAsset(std::shared_ptr<VideoAsset> asset) {
  DCHECK(asset);
  asset_ = std::static_pointer_cast<VideoAssetAndroid>(asset);
  JNIEnv *env = lynx::base::android::AttachCurrentThread();
  Java_IVideoPlayer_attachAsset(env, player_.Get(), asset_->JavaObject());
  // CreateVideoTexture issues GL calls (glGenTextures + setSurface), so under
  // Vulkan scope the offscreen EGL context around it. No-op on the GL backend.
  ScopedEGLContext egl(backend_ == ContextBackend::kVulkan);
  if (egl.ready()) {
    CreateVideoTexture();
  } else if (backend_ == ContextBackend::kVulkan) {
    ANIMAX_LOGE(
        "VideoPlayerAndroid::AttachAsset: EGL MakeCurrent failed, "
        "skipping CreateVideoTexture (video texture may leak)");
  }
}

void VideoPlayerAndroid::NotifyErrorEvent(const std::string &err_msg) {
  if (auto listener = weak_listener_.lock()) {
    listener->OnLayerError(EventError::kVideoPlayerError, err_msg);
  }
}

std::unique_ptr<VideoPlayer> VideoPlayer::MakeVideoPlayer(
    std::shared_ptr<AnimaXAbility> ability) {
  return std::unique_ptr<VideoPlayerAndroid>(
      new VideoPlayerAndroid(std::move(ability)));
}

}  // namespace animax
}  // namespace lynx

namespace animax {
namespace jni {
bool RegisterJNIForIVideoPlayer(JNIEnv *env) {
  return RegisterNativesImpl(env);
}
}  // namespace jni
}  // namespace animax
