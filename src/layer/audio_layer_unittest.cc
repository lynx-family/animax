// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/layer/audio_layer.h"

#include "gtest/gtest.h"
#include "src/base/thread/task_runner.h"
#include "src/parser/composition_parser.h"
#include "src/player/animax_playback_event_handler.h"
#include "src/player/animax_renderer.h"

using namespace lynx::animax;

constexpr const char* lottie_json =
    "{\"v\":\"4.8.0\",\"fr\":30,\"ip\":0,\"op\":150,\"w\":160,\"h\":90,\"nm\":"
    "\"test\",\"ddd\":0,\"assets\":[{\"id\":\"audio_0\",\"u\":\"images/"
    "\",\"p\":\"aud_0.mp3\",\"e\":0,\"t\":2}],\"layers\":[{\"ddd\":0,\"ind\":1,"
    "\"ty\":6,\"nm\":\"27.mp3\",\"cl\":\"mp3\",\"refId\":\"audio_0\",\"sr\":1,"
    "\"ip\":0,\"op\":5006.88,\"st\":0,\"bm\":0}],\"markers\":[],\"props\":{}}";

TEST(AudioLayer, FullTest) {
  auto model =
      CompositionParser::Parse(lottie_json, std::strlen(lottie_json), 1.0f);
  EXPECT_NE(nullptr, model);
  auto layer_model = model->GetLayers().begin();
  auto layer = std::make_unique<AudioLayer>(**layer_model, *model);
  auto context = std::make_shared<AnimaXPlayerContext>();
  std::weak_ptr<AnimaXPlaybackEventHandler> weak_playback_handler;
  auto runner = GetAnimaXGPUThreadHolder(false);
  auto actor = std::make_shared<lynx::shell::LynxActor<AnimaXRenderer>>(
      std::make_unique<AnimaXRenderer>(weak_playback_handler), runner->Get());
  context->weak_renderer_actor = actor;
  layer->SetPlayerContext(context);
  layer->Init();
  Canvas canvas(nullptr, 0, 0, nullptr);
  Matrix matrix;
  layer->SetProgress(-1);
  layer->DrawLayer(canvas, matrix, 0);
  layer->SetProgress(0.5);
  layer->DrawLayer(canvas, matrix, 0);
  layer->SetProgress(2);
  layer->DrawLayer(canvas, matrix, 0);
}
