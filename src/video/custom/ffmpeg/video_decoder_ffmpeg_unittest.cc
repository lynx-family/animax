// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/video/custom/ffmpeg/video_decoder_ffmpeg.h"

#include "gtest/gtest.h"
#include "src/base/thread/thread_assert.h"
#include "src/video/custom/video_decoder_custom.h"
#include "src/video/custom/yuv_frame_info.h"

using namespace lynx::animax;

namespace {

class VideoDecoderFFmpegTest : public ::testing::Test {
 protected:
  void SetUp() override { ThreadAssert::Init(ThreadAssert::Type::kResource); }
  void TearDown() override { ThreadAssert::Init(previous_thread_); }

 private:
  const ThreadAssert::Type previous_thread_ = ThreadAssert::GetCurrent();
};

TEST_F(VideoDecoderFFmpegTest, InitialState) {
  auto decoder = std::make_unique<VideoDecoderFFmpeg>();
  EXPECT_FALSE(decoder->IsValid());
}

TEST_F(VideoDecoderFFmpegTest, CreateDecoderWithNull) {
  auto decoder = std::make_unique<VideoDecoderFFmpeg>();

  EXPECT_FALSE(decoder->CreateDecoder(nullptr));
  EXPECT_FALSE(decoder->IsValid());
}

TEST_F(VideoDecoderFFmpegTest, DecodeFrameWithoutDecoder) {
  auto decoder = std::make_unique<VideoDecoderFFmpeg>();
  std::vector<uint8_t> data;
  auto reusable_frame = std::make_shared<YUVFrameInfo>();

  EXPECT_EQ(nullptr, decoder->DecodeFrameData(data, reusable_frame));
}

TEST_F(VideoDecoderFFmpegTest, PacketApiRejectsUninitializedDecoder) {
  VideoDecoderFFmpeg decoder;
  std::vector<uint8_t> packet{0, 0, 0, 0};
  auto frame = std::make_shared<YUVFrameInfo>();
  frame->SetPts(17);
  const auto reusable_frame = frame;

  EXPECT_EQ(VideoSendStatus::kError, decoder.SendPacket(packet, 17));
  EXPECT_EQ(VideoDecodeStatus::kError, decoder.ReceiveFrame(frame));
  EXPECT_EQ(reusable_frame, frame);
  EXPECT_EQ(17, frame->GetPts());

  ThreadAssert::Init(ThreadAssert::Type::kGPU);
  EXPECT_FALSE(decoder.Flush());
}

}  // namespace
