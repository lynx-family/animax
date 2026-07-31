// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

// Copyright 2018 Airbnb, Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "src/parser/composition_parser.h"

#include <string.h>

#include <algorithm>

#include "src/base/log/log.h"
#include "src/base/util/composition_frame_util.h"
#include "src/model/composition_model.h"
#include "src/parser/layer_parser.h"
#include "src/parser/text/font_character_parser.h"
#include "src/parser/text/font_parser.h"
#include "src/resource/asset/image_asset.h"
#include "src/resource/asset/video_asset.h"
#include "src/resource/asset/video_asset_model.h"

namespace lynx {
namespace animax {

namespace {
static constexpr const auto kAudioSuffix = {".mp3", ".aac", ".wav",
                                            ".m4a", ".ogg", ".mp4"};
bool IsAudio(const std::string& filename) {
  return !filename.empty() &&
         std::any_of(kAudioSuffix.begin(), kAudioSuffix.end(),
                     [filename](std::string_view suffix) {
                       return filename.length() >= suffix.length() &&
                              filename.substr(filename.length() -
                                              suffix.length()) == suffix;
                     });
}
}  // namespace

// TODO(lixianruo.cyrus): Compose these params into a struct.
std::shared_ptr<CompositionModel> CompositionParser::Parse(const char* str_data,
                                                           size_t length,
                                                           float scale,
                                                           bool enable_audio) {
  rapidjson::Document document;
  document.Parse(str_data, length);
  if (document.HasParseError()) {
    ANIMAX_LOGE("CompositionParser.Parse error_code:"
                << std::to_string(document.GetParseError())
                << ", msg:" << document.GetParseErrorMsg());
    return nullptr;
  }

  if (!document.IsObject()) {
    ANIMAX_LOGE("document is not a object");
    return nullptr;
  }

  auto composition_model =
      std::shared_ptr<CompositionModel>(new CompositionModel(scale));
  composition_model->parse_context_.enable_audio_ = enable_audio;
  int32_t width = 0, height = 0;
  float start_frame = 0, end_frame = 0, frame_rate = 0;
  bool enable_3d = false;

  for (auto it = document.MemberBegin(); it != document.MemberEnd(); it++) {
    const auto& key = it->name.GetString();
    if (strcmp(key, "w") == 0) {
      width = it->value.GetInt();
    } else if (strcmp(key, "h") == 0) {
      height = it->value.GetInt();
    } else if (strcmp(key, "ip") == 0) {
      start_frame = it->value.GetFloat();
    } else if (strcmp(key, "op") == 0) {
      end_frame =
          CompositionFrameUtil::ToPlaybackEndFrame(it->value.GetFloat());
    } else if (strcmp(key, "fr") == 0) {
      frame_rate = it->value.GetFloat();
    } else if (strcmp(key, "v") == 0) {
      const auto& version = it->value.GetString();
      ANIMAX_LOGI("current json file version:" << version);
    } else if (strcmp(key, "layers") == 0) {
      ParseLayers(it->value, *composition_model);
    } else if (strcmp(key, "assets") == 0) {
      ParseAssets(it->value, *composition_model);
    } else if (strcmp(key, "fonts") == 0) {
      ParseFonts(it->value, *composition_model);
    } else if (strcmp(key, "chars") == 0) {
      ParseChars(it->value, *composition_model);
    } else if (strcmp(key, "markers") == 0) {
      ParseMarkers(it->value, *composition_model);
    } else if (strcmp(key, "videos") == 0) {
      ParseVideos(it->value, *composition_model);
    } else if (strcmp(key, "ddd") == 0) {
      enable_3d = it->value.GetInt() == 1;
    }
  }

  auto rect =
      std::unique_ptr<RectF>(new RectF(0, 0, scale * width, scale * height));

  ANIMAX_LOGI("total layers size:"
              << std::to_string(composition_model->GetLayers().size()));
  composition_model->Init(std::move(rect), start_frame, end_frame, frame_rate,
                          enable_3d);

  return composition_model;
}

void CompositionParser::ParseLayers(rapidjson::Value& value,
                                    CompositionModel& composition) {
  const auto& array = value.GetArray();
  int32_t image_count = 0;
  bool enable_audio = composition.parse_context_.enable_audio_;

  auto& layers = composition.GetLayers();
  auto& layer_map = composition.GetLayerMap();
  for (auto it = array.Begin(); it != array.End(); it++) {
    auto layer = LayerParser::Parse(it->Move(), composition);
    if (layer->GetLayerType() == LayerType::kAudio && !enable_audio) {
      continue;
    }
    if (layer->GetLayerType() == LayerType::kImage) {
      image_count++;
    }
    layer_map[layer->GetId()] = layer.get();
    layers.push_back(std::move(layer));
  }

  if (image_count > 4) {
    ANIMAX_LOGI("image layer count is larger than 4, image_count:"
                << std::to_string(image_count));
  }
}

void CompositionParser::ParseAssets(rapidjson::Value& value,
                                    CompositionModel& composition) {
  const auto& array = value.GetArray();
  auto& precomps = composition.GetPrecomps();
  auto& images = composition.GetImages();
  auto& audios = composition.GetAudios();

  for (auto it = array.Begin(); it != array.End(); it++) {
    auto info = ImageAssetModel{};
    LayerModelList layers{};
    bool has_file_name = false;

    const auto& object = it->GetObject();
    for (auto object_it = object.MemberBegin(); object_it != object.MemberEnd();
         object_it++) {
      const auto& object_key = object_it->name.GetString();
      if (strcmp(object_key, "id") == 0) {
        if (object_it->value.IsString()) {
          info.id.assign(object_it->value.GetString(),
                         object_it->value.GetStringLength());
        }
      } else if (strcmp(object_key, "layers") == 0) {
        const auto& layer_array = object_it->value.GetArray();
        for (auto layer_it = layer_array.Begin(); layer_it != layer_array.End();
             layer_it++) {
          auto layer_model = LayerParser::Parse(layer_it->Move(), composition);
          layers.push_back(std::move(layer_model));
        }
      } else if (strcmp(object_key, "w") == 0) {
        info.width = object_it->value.GetInt();
      } else if (strcmp(object_key, "h") == 0) {
        info.height = object_it->value.GetInt();
      } else if (strcmp(object_key, "p") == 0) {
        if (object_it->value.IsString()) {
          info.file_name.assign(object_it->value.GetString(),
                                object_it->value.GetStringLength());
          has_file_name = !info.file_name.empty();
        }
      } else if (strcmp(object_key, "u") == 0) {
        if (object_it->value.IsString()) {
          info.dir_name.assign(object_it->value.GetString(),
                               object_it->value.GetStringLength());
        }
      }
    }

    if (info.id.empty()) {
      continue;
    }

    const auto info_id = info.id;
    const auto file_name = info.file_name;
    if (has_file_name && IsAudio(file_name)) {
      if (!composition.parse_context_.enable_audio_) {
        continue;
      }
      audios[info_id] =
          AudioAsset::Make({info.id, info.dir_name, info.file_name});
    } else if (has_file_name) {
      images[info_id] =
          std::shared_ptr<ImageAsset>(new ImageAsset{std::move(info)});
    } else if (object.HasMember("layers")) {
      precomps[info_id] = std::move(layers);
    }
  }
}

void CompositionParser::ParseFonts(rapidjson::Value& value,
                                   CompositionModel& composition) {
  auto& fonts = composition.GetFonts();
  const auto& object = value.GetObject();
  for (auto it = object.MemberBegin(); it != object.MemberEnd(); it++) {
    const auto& key = it->name.GetString();
    if (strcmp(key, "list") == 0) {
      const auto& array = it->value.GetArray();
      for (auto array_it = array.Begin(); array_it != array.End(); array_it++) {
        auto font_asset = FontParser::Parse(array_it->Move(), composition);
        fonts[font_asset->Model().name] = font_asset;
      }
    }
  }
}

void CompositionParser::ParseChars(rapidjson::Value& value,
                                   CompositionModel& composition) {
  auto& characters = composition.GetCharacters();
  const auto& array = value.GetArray();
  for (auto array_it = array.Begin(); array_it != array.End(); array_it++) {
    auto character = FontCharacterParser::Parse(array_it->Move(), composition);
    characters[character->HashCode()] = std::move(character);
  }
}

void CompositionParser::ParseMarkers(rapidjson::Value& value,
                                     CompositionModel& composition) {
  auto& markers = composition.GetMarkers();
  const auto& array = value.GetArray();
  for (auto it = array.Begin(); it != array.End(); it++) {
    const auto& object = it->GetObject();
    for (auto object_it = object.MemberBegin(); object_it != object.MemberEnd();
         object_it++) {
      const auto& name = object_it->name.GetString();

      auto model = std::unique_ptr<MarkerModel>(new MarkerModel());
      if (strcmp(name, "cm") == 0) {
        model->name_ = object_it->value.GetString();
      } else if (strcmp(name, "tm") == 0) {
        model->start_frame_ = object_it->value.GetFloat();
      } else if (strcmp(name, "dr") == 0) {
        model->duration_frames_ = object_it->value.GetFloat();
      }

      markers.push_back(std::move(model));
    }
  }
}

void CompositionParser::ParseVideos(rapidjson::Value& value,
                                    CompositionModel& composition) {
  if (!value.IsArray()) {
    return;
  }
  const auto& array = value.GetArray();
  auto& videos = composition.GetVideos();

  for (auto it = array.Begin(); it != array.End(); it++) {
    if (!it->IsObject()) {
      continue;
    }
    std::string id;
    std::array<int32_t, 4> rgb_frame;
    rgb_frame.fill(0);
    std::array<int32_t, 4> a_frame;
    a_frame.fill(0);
    std::string image_file_name;
    std::string relative_folder;

    const auto& object = it->GetObject();
    for (auto object_it = object.MemberBegin(); object_it != object.MemberEnd();
         object_it++) {
      const auto& object_key = object_it->name.GetString();
      if (strcmp(object_key, "id") == 0) {
        id = object_it->value.GetString();
      } else if (strcmp(object_key, "x") == 0) {
        rgb_frame[0] = object_it->value.GetInt();
      } else if (strcmp(object_key, "y") == 0) {
        rgb_frame[1] = object_it->value.GetInt();
      } else if (strcmp(object_key, "w") == 0) {
        rgb_frame[2] = object_it->value.GetInt();
      } else if (strcmp(object_key, "h") == 0) {
        rgb_frame[3] = object_it->value.GetInt();
      } else if (strcmp(object_key, "ax") == 0) {
        a_frame[0] = object_it->value.GetInt();
      } else if (strcmp(object_key, "ay") == 0) {
        a_frame[1] = object_it->value.GetInt();
      } else if (strcmp(object_key, "aw") == 0) {
        a_frame[2] = object_it->value.GetInt();
      } else if (strcmp(object_key, "ah") == 0) {
        a_frame[3] = object_it->value.GetInt();
      } else if (strcmp(object_key, "p") == 0) {
        image_file_name = object_it->value.GetString();
      } else if (strcmp(object_key, "u") == 0) {
        relative_folder = object_it->value.GetString();
      }
    }
    auto model = VideoAssetModel{
        .rgb_frame = rgb_frame,
        .a_frame = a_frame,
        .id = id,
        .dir_name = relative_folder,
        .file_name = image_file_name,
    };
    videos[id] = VideoAsset::Make(std::move(model));
  }
}

}  // namespace animax
}  // namespace lynx
