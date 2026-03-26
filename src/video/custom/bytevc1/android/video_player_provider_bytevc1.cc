// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/video/custom/bytevc1/android/video_player_provider_bytevc1.h"

#include "src/base/gl/gl_include.h"
#include "src/player/android/animax_ability_android.h"
#include "src/video/android/video_asset_android.h"
#include "src/video/custom/bytevc1/android/video_decoder_bytevc1.h"
#include "src/video/custom/bytevc1/android/video_player_provider_bytevc1.h"
#include "src/video/custom/video_player_custom.h"
#include "src/video/custom/video_shader_yuv.h"

namespace lynx {
namespace animax {

std::unique_ptr<VideoPlayer> VideoPlayerProviderByteVc1::CreateVideoPlayer(
    const AnimaXAbility *ability) {
  auto decoder = std::make_unique<VideoDecoderByteVc1>();
  auto player = std::make_unique<VideoPlayerCustom>(std::move(decoder));
  player->SetTextureTarget(GL_TEXTURE_2D);
  return player;
}

std::unique_ptr<VideoShader> VideoPlayerProviderByteVc1::CreateVideoShader(
    const AnimaXAbility *ability) {
  return std::make_unique<VideoShaderYUV>();
}

bool VideoPlayerProviderByteVc1::IsSupported(
    const AnimaXAbility *ability, std::shared_ptr<VideoAsset> asset) {
  if (!ability || !asset) {
    return false;
  }

  auto *android_ability = static_cast<const AnimaXAbilityAndroid *>(ability);
  auto android_asset = std::static_pointer_cast<VideoAssetAndroid>(asset);
  if (!android_ability || !android_asset) {
    return false;
  }

  // This provider supports the video asset when:
  // - ByteVC1 decoder is enabled
  // - Video format is HEVC/H.265
  // - System lacks hardware-accelerated decoding capability for HEVC/H.265
  // - ByteVC1 library is valid
  if (!android_ability->DisableByteVC1Decoder() &&
      android_asset->IsHEVCFormat() &&
      !android_asset->IsSupportDecodeBySystem(true) &&
      ByteVC1LibLoader::Instance().IsValid()) {
    return true;
  }

  return false;
}

}  // namespace animax
}  // namespace lynx
