// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/video/harmony/video_player_harmony.h"

#include "src/base/gl/scoped_gl_reset_restore.h"
#include "src/base/log/log.h"
#include "src/base/thread/thread_assert.h"
#include "src/player/animax_ability.h"

namespace lynx {
namespace animax {

VideoPlayerHarmony::VideoPlayerHarmony() {
  // Flip Y
  transform_ = {1.f, 0.f, 0.f, 0.f, 0.f, -1.f, 0.f, 0.f,
                0.f, 0.f, 1.f, 0.f, 0.f, 1.f,  0.f, 1.f};
}

VideoPlayerHarmony::~VideoPlayerHarmony() {
  ThreadAssert::Assert(ThreadAssert::Type::kGPU);

  // Stop codec
  if (av_codec_) {
    OH_VideoDecoder_Flush(av_codec_);
    OH_VideoDecoder_SetSurface(av_codec_, nullptr);
    OH_VideoDecoder_Stop(av_codec_);
    OH_VideoDecoder_Destroy(av_codec_);
    av_codec_ = nullptr;
  }

  // Destroy surface window
  if (native_window_) {
    OH_NativeWindow_DestroyNativeWindow(native_window_);
    native_window_ = nullptr;
  }

  // Detach and destroy native image
  if (native_image_) {
    OH_NativeImage_DetachContext(native_image_);
  }

  // Delete texture after detach from context
  if (video_texture_ > 0) {
    glDeleteTextures(1, &video_texture_);
    video_texture_ = 0;
  }

  // Destroy native image in the end
  if (native_image_) {
    OH_NativeImage_Destroy(&native_image_);
    native_image_ = nullptr;
  }
}

void VideoPlayerHarmony::OnError(OH_AVCodec *codec, int32_t errorCode,
                                 void *userData) {
  ANIMAX_LOGI("OnError");
}

void VideoPlayerHarmony::OnStreamChanged(OH_AVCodec *codec, OH_AVFormat *format,
                                         void *userData) {
  // ignore
}

void VideoPlayerHarmony::OnNeedInputBuffer(OH_AVCodec *codec, uint32_t index,
                                           OH_AVBuffer *buffer,
                                           void *userData) {
  CodecData *codec_data = static_cast<CodecData *>(userData);
  if (codec_data == nullptr) {
    return;
  }
  std::unique_lock<std::mutex> lock(codec_data->in_mutex);
  codec_data->in_queue.emplace(index, buffer);
  codec_data->in_cond.notify_all();
}

void VideoPlayerHarmony::OnNeedOutputBuffer(OH_AVCodec *codec, uint32_t index,
                                            OH_AVBuffer *buffer,
                                            void *userData) {
  CodecData *codec_data = static_cast<CodecData *>(userData);
  if (codec_data == nullptr) {
    return;
  }
  std::unique_lock<std::mutex> lock(codec_data->out_mutex);
  codec_data->out_queue.emplace(index, buffer);
  codec_data->out_cond.notify_all();
}

std::unique_ptr<TextureInfo> VideoPlayerHarmony::UpdateTexture(
    const int32_t frame) {
  if (asset_ == nullptr || data_ == nullptr || av_codec_ == nullptr) {
    ANIMAX_LOGI("asset, data or av_codec is nullptr");
    return nullptr;
  }

  auto texture_info = std::make_unique<TextureInfoGL>(
      video_texture_, asset_->GetVideoWidth(), asset_->GetVideoHeight(),
      GL_TEXTURE_EXTERNAL_OES);

  if (next_output_frame_ == AdvanceFrameCounter(frame)) {
    // Frame already rendered, no update needed.
    return texture_info;
  }

  UpdateFrameCounters(frame);

  if (!RenderFrame(frame)) {
    ANIMAX_LOGE("Failed to render frame: " << frame);
    return texture_info;
  }

  UpdateSurfaceImage();

  return texture_info;
}

void VideoPlayerHarmony::UpdateSurfaceImage() {
  // Retry for the case that the surface image is not updated.
  for (int32_t i = 0; i < kSurfaceUpdateRetryCount; i++) {
    auto ret = OH_NativeImage_UpdateSurfaceImage(native_image_);
    if (ret == 0) {
      break;
    }
    ANIMAX_LOGW("OH_NativeImage_UpdateSurfaceImage fail, ret: " << ret);
  }
}

void VideoPlayerHarmony::UpdateFrameCounters(int32_t frame) {
  auto prev_keyframe = GetPrevKeyframe(frame);
  auto steps_from_keyframe = frame - prev_keyframe;
  auto steps_from_current = frame - next_output_frame_;

  /**
   * There are 2 cases that need to flush the cache and reset the frame counter:
   * 1. The target frame is larger than the current one, and the target frame
   * has a closer keyframe.
   * 2. The target frame is less than the current one. Reverse seeking has poor
   * performance for now.
   */
  if ((steps_from_current > 0 && steps_from_current > steps_from_keyframe) ||
      steps_from_current < 0) {
    next_input_frame_ = prev_keyframe;
    next_output_frame_ = prev_keyframe;
  }
}

int32_t VideoPlayerHarmony::AdvanceFrameCounter(int32_t frame) {
  return (frame + 1) % data_->frame_count;
}

int32_t VideoPlayerHarmony::GetPrevKeyframe(int32_t frame) const {
  auto &keyframe_index = data_->keyframe_index;
  if (keyframe_index.empty() || frame < keyframe_index.front()) {
    return 0;
  }

  auto it =
      std::lower_bound(keyframe_index.begin(), keyframe_index.end(), frame);

  if (it != keyframe_index.end() && *it == frame) {
    return *it;
  }

  if (it == keyframe_index.begin()) {
    return 0;
  }

  --it;
  return *it;
}

bool VideoPlayerHarmony::RenderFrame(int32_t frame) {
  while (next_output_frame_ <= frame) {
    if (!DecodeFrame(next_input_frame_)) {
      ANIMAX_LOGE("Failed to decode frame: " << next_input_frame_);
      return false;
    }
    next_input_frame_ = AdvanceFrameCounter(next_input_frame_);

    if (!ProcessOutputFrame(next_output_frame_ == frame)) {
      ANIMAX_LOGE("Failed to process output frame: " << next_output_frame_);
      return false;
    }
    next_output_frame_ = AdvanceFrameCounter(next_output_frame_);

    // Play from the start, exit the process loop
    if (next_output_frame_ == 0) {
      break;
    }
  }

  return true;
}

std::chrono::milliseconds VideoPlayerHarmony::GetTimeout(int32_t frame) const {
  auto is_keyframe =
      std::find(data_->keyframe_index.begin(), data_->keyframe_index.end(),
                frame) != data_->keyframe_index.end();
  if (is_keyframe) {
    return kBaseTimeout * kMaxTimeoutCount;
  } else if (timeout_count_ > 0) {
    return kBaseTimeout * timeout_count_;
  } else {
    return kBaseTimeout;
  }
}

void VideoPlayerHarmony::IncreaseTimeoutCount() {
  if (timeout_count_ < kMaxTimeoutCount) {
    timeout_count_ += 1;
  }
}

bool VideoPlayerHarmony::DecodeFrame(int32_t frame) {
  auto &frame_info = data_->frame_list[frame];
  std::unique_lock<std::mutex> lock(codec_data_.in_mutex);
  if (!codec_data_.in_cond.wait_for(lock, GetTimeout(frame), [this]() {
        return !codec_data_.in_queue.empty();
      })) {
    IncreaseTimeoutCount();
    ANIMAX_LOGE("Timeout waiting for input buffer");
    return false;
  }

  CodecBufferInfo in_info = codec_data_.in_queue.front();
  codec_data_.in_queue.pop();
  lock.unlock();

  // Copy frame data to input buffer
  std::memcpy(in_info.GetAddr(), data_->buffer_data.data() + frame_info.begin,
              frame_info.size);

  // Set buffer attributes
  OH_AVCodecBufferAttr attr{.pts = frame_info.timestamp,
                            .size = frame_info.size,
                            .offset = 0,
                            .flags = frame_info.flags};
  OH_AVBuffer_SetBufferAttr(in_info.buffer, &attr);

  // Push input buffer to decoder
  auto err_code = OH_VideoDecoder_PushInputBuffer(av_codec_, in_info.index);
  if (err_code != OH_AVErrCode::AV_ERR_OK) {
    ANIMAX_LOGE("OH_VideoDecoder_PushInputBuffer failed");
    return false;
  }

  return true;
}

bool VideoPlayerHarmony::ProcessOutputFrame(bool render) {
  std::unique_lock<std::mutex> out_lock(codec_data_.out_mutex);
  if (!codec_data_.out_cond.wait_for(
          out_lock, GetTimeout(next_output_frame_),
          [this]() { return !codec_data_.out_queue.empty(); })) {
    IncreaseTimeoutCount();
    ANIMAX_LOGE("Timeout waiting for output buffer");
    return false;
  }

  CodecBufferInfo out_info = codec_data_.out_queue.front();
  codec_data_.out_queue.pop();
  out_lock.unlock();

  OH_AVErrCode err_code;
  if (render) {
    err_code = OH_VideoDecoder_RenderOutputBuffer(av_codec_, out_info.index);
  } else {
    err_code = OH_VideoDecoder_FreeOutputBuffer(av_codec_, out_info.index);
  }

  if (err_code != OH_AVErrCode::AV_ERR_OK) {
    ANIMAX_LOGE("Failed to process output buffer");
    return false;
  }

  return true;
}

const std::array<float, 16> &VideoPlayerHarmony::GetTransform() {
  return transform_;
}

void VideoPlayerHarmony::InitNativeWindow() {
  if (asset_ == nullptr || native_window_) {
    return;
  }

  ThreadAssert::Assert(ThreadAssert::Type::kGPU);
  lynx::animax::ScopedGLResetRestore s(GL_TEXTURE_BINDING_EXTERNAL_OES);
  glGenTextures(1, &video_texture_);
  glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Create the native image, the video texture will not bind yet.
  native_image_ =
      OH_NativeImage_Create(video_texture_, GL_TEXTURE_EXTERNAL_OES);
  // Attach the texture to native image and call glBindTexture inside.
  auto ret = OH_NativeImage_AttachContext(native_image_, video_texture_);
  if (ret != 0) {
    ANIMAX_LOGI("OH_NativeImage_AttachContext fail, ret: " << ret);
  }
  // Hold the native window, used to update the image texture.
  native_window_ = OH_NativeImage_AcquireNativeWindow(native_image_);
  // Initialize the window's size by video's config.
  OH_NativeWindow_NativeWindowHandleOpt(native_window_, SET_BUFFER_GEOMETRY,
                                        asset_->GetVideoWidth(),
                                        asset_->GetVideoHeight());
}

void VideoPlayerHarmony::AttachAsset(VideoAsset *asset) {
  DCHECK(asset);
  asset_ = reinterpret_cast<VideoAssetHarmony *>(asset);
  if (asset_) {
    data_ = &asset_->GetVideoData();
  }

  if (!data_) {
    ANIMAX_LOGE("AttachAsset fail, raw data is null.")
    return;
  }

  InitNativeWindow();
  InitCodec();
}

void VideoPlayerHarmony::NotifyErrorEvent(const std::string &err_msg) {
  if (listener_) {
    listener_->OnVideoPlayerError(err_msg);
  }
}

void VideoPlayerHarmony::InitCodec() {
  if (data_ == nullptr || !asset_->IsValid() || av_codec_ != nullptr ||
      native_window_ == nullptr) {
    ANIMAX_LOGE("InitCodec fail.");
    return;
  }

  auto *video_format = data_->video_format.get();
  const char *mime_name;
  OH_AVFormat_GetStringValue(video_format, OH_MD_KEY_CODEC_MIME, &mime_name);
  ANIMAX_LOGI("Create codec by mime:" << mime_name);

  av_codec_ = OH_VideoDecoder_CreateByMime(mime_name);
  OH_AVErrCode err_code = OH_VideoDecoder_Configure(av_codec_, video_format);
  if (err_code != OH_AVErrCode::AV_ERR_OK) {
    ANIMAX_LOGE("OH_VideoDecoder_Configure fail");
    return;
  }

  OH_AVCodecCallback cb = {&OnError, &OnStreamChanged, &OnNeedInputBuffer,
                           &OnNeedOutputBuffer};
  OH_VideoDecoder_RegisterCallback(av_codec_, cb, &codec_data_);

  err_code = OH_VideoDecoder_SetSurface(av_codec_, native_window_);
  if (err_code != OH_AVErrCode::AV_ERR_OK) {
    ANIMAX_LOGE("OH_VideoDecoder_SetSurface fail");
    return;
  }

  err_code = OH_VideoDecoder_Prepare(av_codec_);
  if (err_code != OH_AVErrCode::AV_ERR_OK) {
    ANIMAX_LOGE("OH_VideoDecoder_Prepare fail");
    return;
  }

  err_code = OH_VideoDecoder_Start(av_codec_);
  if (err_code != OH_AVErrCode::AV_ERR_OK) {
    ANIMAX_LOGE("OH_VideoDecoder_Start fail");
    return;
  }

  ANIMAX_LOGI("InitCodec success.");
}

std::unique_ptr<VideoPlayer> VideoPlayer::MakeVideoPlayer(
    const AnimaXAbility *ability_ptr) {
  return std::unique_ptr<VideoPlayer>(new VideoPlayerHarmony());
}
}  // namespace animax
}  // namespace lynx
