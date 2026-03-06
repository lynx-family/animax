// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/parser/alpha_player_asset/alpha_player_asset_parser.h"

#include <array>

#include "src/base/log/log.h"
#include "src/model/animatable/animatable_transform_model.h"
#include "src/model/composition_model.h"
#include "src/model/layer_model.h"
#include "src/model/rect_model.h"
#include "src/model/text/font_character_model.h"
#include "src/model/value/base_value.h"
#include "src/resource/asset/audio_asset.h"
#include "src/resource/asset/audio_asset_model.h"
#include "src/resource/asset/video_asset.h"
#include "src/resource/asset/video_asset_model.h"
#include "src/resource/uri/uri_util.h"
#include "third_party/rapidjson/document.h"

namespace {
static constexpr const char *kVideoId = "0";
static constexpr const char *kAudioId = "1";

bool ParseRect(std::array<int32_t, 4> &rect, const rapidjson::Value &value) {
  if (!value.IsArray()) {
    return false;
  }
  const auto &array = value.GetArray();
  int32_t index = 0;
  for (auto it = array.Begin(); it != array.End() && index < 4; ++it, ++index) {
    rect[index] = it->GetInt();
  }
  return (4 == index);
}

void AddVideoAsset(const std::shared_ptr<lynx::animax::CompositionModel> &model,
                   const std::string &video_path,
                   const std::array<int32_t, 4> &rgb_frame,
                   const std::array<int32_t, 4> &alpha_frame) {
  if (video_path.empty()) {
    return;
  }
  auto video_asset_model = lynx::animax::VideoAssetModel{
      .rgb_frame = rgb_frame,
      .a_frame = alpha_frame,
      .id = kVideoId,
      .dir_name = std::string{},
      .file_name = std::string{},
  };
  std::shared_ptr<lynx::animax::VideoAsset> video =
      lynx::animax::VideoAsset::Make(std::move(video_asset_model));
  video->LoadLocal(video_path);
  video->RecoverFrameIfNecessary();
  model->GetVideos()[kVideoId] = video;
}

void AddAlphaVideoLayer(
    const std::shared_ptr<lynx::animax::CompositionModel> &model,
    const float frame_count) {
  using lynx::animax::Float;
  using lynx::animax::KeyframeModel;
  using lynx::animax::ValueFactory;

  auto layer_model = lynx::animax::LayerModel::Make(*model);

  const float in_frame = 0.f;
  const float out_frame = frame_count;

  auto &in_out_frames = layer_model->GetInOutFrames();
  in_out_frames.emplace_back(std::unique_ptr<KeyframeModel>(new KeyframeModel(
      *model, ValueFactory::Make<Float>(1), ValueFactory::Make<Float>(1),
      nullptr, in_frame, out_frame)));
  in_out_frames.emplace_back(std::unique_ptr<KeyframeModel>(new KeyframeModel(
      *model, ValueFactory::Make<Float>(0), ValueFactory::Make<Float>(0),
      nullptr, out_frame, Float::kMax)));
  layer_model->Init("AlphaPlayerTransformed",
                    lynx::animax::LayerType::kAlphaVideo,
                    1,         // layer id
                    -1,        // parent id
                    kVideoId,  // ref id
                    0,         // solid width
                    0,         // solid height
                    ValueFactory::Make<lynx::animax::Color>(),
                    1.f,      // time stretch
                    0.f,      // start frame
                    0.f,      // pre comp width
                    0.f,      // pre comp height
                    nullptr,  // text
                    nullptr,  // text properties
                    false,    // hidden
                    std::unique_ptr<lynx::animax::AnimatableTransformModel>(
                        new lynx::animax::AnimatableTransformModel()),
                    static_cast<int>(lynx::animax::MatteType::kUnknown),
                    -1,       // matte parent
                    false,    // is matte target
                    nullptr,  // time remapping
                    nullptr,  // blur effect
                    nullptr,  // drop shadow effect
                    false,    // 3d
                    nullptr   // perspective

  );

  model->GetLayerMap()[layer_model->GetId()] = layer_model.get();
  model->GetLayers().push_back(std::move(layer_model));
}

void AddAudioAsset(const std::shared_ptr<lynx::animax::CompositionModel> &model,
                   const std::string &video_path) {
  if (video_path.empty()) {
    return;
  }
  auto audio_asset_model = lynx::animax::AudioAssetModel{
      .id = kAudioId,
      .dir_name = std::string{},
      .file_name = std::string{},
  };
  std::shared_ptr<lynx::animax::AudioAsset> audio =
      lynx::animax::AudioAsset::Make(std::move(audio_asset_model));
  if (audio) {
    audio->LoadLocal(video_path);
  }
  model->GetAudios()[kAudioId] = audio;
}

void AddAudioLayer(const std::shared_ptr<lynx::animax::CompositionModel> &model,
                   const float frame_count) {
  using lynx::animax::Float;
  using lynx::animax::KeyframeModel;
  using lynx::animax::ValueFactory;

  auto layer_model = lynx::animax::LayerModel::Make(*model);

  const float in_frame = 0.f;
  const float out_frame = frame_count;

  auto &in_out_frames = layer_model->GetInOutFrames();
  in_out_frames.emplace_back(std::unique_ptr<KeyframeModel>(new KeyframeModel(
      *model, ValueFactory::Make<Float>(1), ValueFactory::Make<Float>(1),
      nullptr, in_frame, out_frame)));
  in_out_frames.emplace_back(std::unique_ptr<KeyframeModel>(new KeyframeModel(
      *model, ValueFactory::Make<Float>(0), ValueFactory::Make<Float>(0),
      nullptr, out_frame, Float::kMax)));
  layer_model->Init("AlphaPlayerAudio", lynx::animax::LayerType::kAudio,
                    2,         // layer id
                    -1,        // parent id
                    kAudioId,  // ref id
                    0,         // solid width
                    0,         // solid height
                    ValueFactory::Make<lynx::animax::Color>(),
                    1.f,      // time stretch
                    0.f,      // start frame
                    0.f,      // pre comp width
                    0.f,      // pre comp height
                    nullptr,  // text
                    nullptr,  // text properties
                    false,    // hidden
                    std::unique_ptr<lynx::animax::AnimatableTransformModel>(
                        new lynx::animax::AnimatableTransformModel()),
                    static_cast<int>(lynx::animax::MatteType::kUnknown),
                    -1,       // matte parent
                    false,    // is matte target
                    nullptr,  // time remapping
                    nullptr,  // blur effect
                    nullptr,  // drop shadow effect
                    false,    // 3d
                    nullptr   // perspective
  );
  model->GetLayerMap()[layer_model->GetId()] = layer_model.get();
  model->GetLayers().push_back(std::move(layer_model));
}

std::shared_ptr<lynx::animax::CompositionModel> ParseAsset(
    const std::string &unzip_path, const float scale,
    const rapidjson::Value &value, bool enable_audio) {
  if (!value.IsObject()) {
    return nullptr;
  }
  // video asset property
  std::array<int32_t, 4> rgb_frame;
  rgb_frame.fill(0);
  std::array<int32_t, 4> alpha_frame;
  alpha_frame.fill(0);
  std::string video_path;

  const auto &object = value.GetObject();
  for (auto it = object.MemberBegin(); it != object.MemberEnd(); ++it) {
    const auto &key = it->name.GetString();
    if (strcmp(key, "path") == 0) {
      video_path =
          unzip_path + lynx::animax::kPathSeparator + it->value.GetString();
    } else if (strcmp(key, "aFrame") == 0) {
      if (!ParseRect(alpha_frame, it->value)) {
        return nullptr;
      }
    } else if (strcmp(key, "rgbFrame") == 0) {
      if (!ParseRect(rgb_frame, it->value)) {
        return nullptr;
      }
    }
  }

  auto model = std::shared_ptr<lynx::animax::CompositionModel>(
      new lynx::animax::CompositionModel(scale));
  // add videos
  AddVideoAsset(model, video_path, rgb_frame, alpha_frame);
  auto video = model->GetVideos()[kVideoId];
  if (!video || !video->IsValid()) {
    return nullptr;
  }
  AddAlphaVideoLayer(model, video->GetFrameCount());
  if (enable_audio) {
    // add audios
    AddAudioAsset(model, video_path);
    //    TODO(lixianruo.cyrus): Audio is not implemented at all platforms yet,
    //    empty audio is expected. auto audio = model->GetAudios()[kAudioId]; if
    //    (!audio || !audio->IsValid()) {
    //      return nullptr;
    //    }
    AddAudioLayer(model, video->GetFrameCount());
  }

  model->Init(
      std::unique_ptr<lynx::animax::RectF>(new lynx::animax::RectF(
          0, 0, scale * video->GetRgbWidth(), scale * video->GetRgbHeight())),
      0.f, video->GetFrameCount() - 0.01f, video->GetFrameRate(), false);
  return model;
}

}  // namespace

namespace lynx {
namespace animax {

std::shared_ptr<CompositionModel> AlphaPlayerAssetParser::Parse(
    const std::string &unzip_path, const char *config, size_t length,
    float scale, bool enable_audio) {
  rapidjson::Document document;
  document.Parse(config, length);
  if (document.HasParseError()) {
    ANIMAX_LOGE("parse config.json error");
    return nullptr;
  }

  if (!document.IsObject()) {
    ANIMAX_LOGE("content of config.json isn't a object");
    return nullptr;
  }
  for (auto it = document.MemberBegin(); it != document.MemberEnd(); ++it) {
    const auto &key = it->name.GetString();
    if (strcmp(key, "portrait") == 0) {
      return ParseAsset(unzip_path, scale, it->value, enable_audio);
    }
  }
  ANIMAX_LOGE("there isn't 'portrait' in config.json");
  return nullptr;
}

}  // namespace animax
}  // namespace lynx
