// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include "src/video/custom/bytevc1/android/bytevc1_lib_loader.h"

#include "src/base/log/log.h"
#include "src/base/util/android/symbol_util.h"

namespace lynx {
namespace animax {

ByteVC1LibLoader::ByteVC1LibLoader() {
  if (!LoadLibrary()) {
    ANIMAX_LOGE("ByteVC1LibLoader: Failed to load ByteVC1 library");
  }
}

void ByteVC1LibLoader::DecoderDeleter::operator()(void *decoder) const {
  if (!ByteVC1LibLoader::Instance().IsValid()) {
    return;
  }

  ByteVC1LibLoader::Instance().dec_destroy_func_(decoder);
}

void ByteVC1LibLoader::PacketDeleter::operator()(void *packet) const {
  if (!ByteVC1LibLoader::Instance().IsValid()) {
    return;
  }

  if (ByteVC1LibLoader::Instance().is_new_api_) {
    ByteVC1LibLoader::Instance().release_ttpacket_func_(packet);
  } else {
    ByteVC1LibLoader::Instance().free_param_func_(packet);
  }
}

void ByteVC1LibLoader::FrameDeleter::operator()(void *frame) const {
  if (!ByteVC1LibLoader::Instance().IsValid() || !decoder_) {
    return;
  }

  ByteVC1LibLoader::Instance().return_frame_func_(decoder_, frame);
}

bool ByteVC1LibLoader::IsValid() const {
  return library_handle_ != nullptr && is_symbols_valid_;
}

ByteVC1LibLoader::DecoderPtr ByteVC1LibLoader::CreateDecoder() const {
  if (!IsValid()) {
    return nullptr;
  }

  auto param = alloc_default_param_func_();
  if (!param) {
    ANIMAX_LOGE(
        "ByteVC1LibLoader: create decoder failed, failed to allocate "
        "decoder parameters.");
    return nullptr;
  }

  auto decoder = DecoderPtr(dec_create_func_(param), DecoderDeleter());
  if (!decoder) {
    ANIMAX_LOGE("ByteVC1LibLoader: create decoder failed, failed to create.");
  }

  if (is_new_api_) {
    release_param_func_(param);
  } else {
    free_param_func_(param);
  }

  return decoder;
}

ByteVC1LibLoader::PacketPtr ByteVC1LibLoader::CreatePacket(
    std::vector<uint8_t> &buffer) const {
  PacketPtr packet(alloc_packet_func_(), PacketDeleter());
  if (!packet) {
    ANIMAX_LOGE("ByteVC1LibLoader: Failed to allocate packet.");
    return nullptr;
  }

  if (is_new_api_) {
    init_ttpacket_func_(packet.get(), buffer.data(), buffer.size(), 0, 0, 0, 0);
  } else {
    set_packet_bs_func_(packet.get(), buffer.data());
    set_packet_bs_len_func_(packet.get(), buffer.size());
  }

  return packet;
}

ByteVC1LibLoader::FramePtr ByteVC1LibLoader::DecodePacket(
    const DecoderPtr &decoder_ptr, const PacketPtr &packet_ptr) const {
  auto decode_ret = send_packet_func_(decoder_ptr.get(), packet_ptr.get());
  if (ByteVc1DecStat::kDecStatOK != static_cast<ByteVc1DecStat>(decode_ret)) {
    return nullptr;
  }

  std::unique_ptr<void, FrameDeleter> frame(alloc_frame_func_(),
                                            FrameDeleter(decoder_ptr.get()));
  if (!frame.get()) {
    return nullptr;
  }

  decode_ret = get_frame_func_(decoder_ptr.get(), frame.get());
  if (ByteVc1DecStat::kDecStatOK != static_cast<ByteVc1DecStat>(decode_ret)) {
    return nullptr;
  }

  bool got_frame = is_new_api_ ? frame_got_frame_func_(frame.get())
                               : get_frame_got_frame_func_(frame.get());
  if (!got_frame) {
    return nullptr;
  }

  return frame;
}

void ByteVC1LibLoader::ExtractFrameData(
    const FramePtr &frame_ptr,
    const std::shared_ptr<YUVFrameInfo> &frame_info) const {
  auto frame = frame_ptr.get();
  uint32_t width = 0, height = 0, depth = 8, fmt = 0, color_pri = 0,
           color_trc = 0, color_space = 0, slice_type = 0;
  int64_t pts = 0, opt = 0;
  uint32_t stride[YUVFrameInfo::kMaxChannels] = {};
  uint32_t linesize[YUVFrameInfo::kMaxChannels] = {};
  uint8_t *data[YUVFrameInfo::kMaxChannels] = {};

  if (is_new_api_) {
    frame_get_img_property_func_(frame, &width, &height, &pts, &opt, &depth,
                                 &color_pri, &color_trc, &color_space, &fmt,
                                 &slice_type);
    frame_get_buffer_property_func_(frame, stride, linesize, data);
  } else {
    width = get_frame_width_func_(frame);
    height = get_frame_height_func_(frame);
    depth = get_frame_bit_depth_func_(frame);
    fmt = get_frame_pix_fmt_func_(frame);
    for (int ch = 0; ch < YUVFrameInfo::kYUVChannels; ch++) {
      stride[ch] = get_frame_stride_func_(frame, ch);
      linesize[ch] = get_frame_linesize_func_(frame, ch);
      data[ch] = get_frame_data_func_(frame, ch);
    }
  }

  frame_info->SetWidth(width);
  frame_info->SetHeight(height);
  frame_info->SetBitDepth(depth);
  frame_info->SetYUVPixFmt(static_cast<YUVPixFmt>(fmt));
  for (int ch = 0; ch < YUVFrameInfo::kYUVChannels; ch++) {
    frame_info->SetLineSize(ch, linesize[ch]);
    frame_info->UpdateChannelData(ch, data[ch]);
  }
}

void ByteVC1LibLoader::SetLogCallback(const DecoderPtr &decoder,
                                      int (*log_callback)(void *, int,
                                                          const char *,
                                                          ...)) const {
  if (!IsValid() || !decoder || !log_callback) {
    return;
  }

  set_log_callback_func_(decoder.get(), reinterpret_cast<void *>(log_callback));
}

bool ByteVC1LibLoader::LoadLibrary() {
  if (library_handle_) {
    return true;
  }

  library_handle_ =
      dlopen(kByteVcLibName, RTLD_NOW | RTLD_LOCAL | RTLD_NODELETE);
  if (!library_handle_) {
    return false;
  }

  return LoadAllSymbols();
}

bool ByteVC1LibLoader::LoadAllSymbols() {
  is_symbols_valid_ = false;

  if (!library_handle_) {
    return false;
  }

  struct SymbolConfig {
    void **func_ptr;
    const char *symbol_name;
  };

  auto load_symbols = [this](const std::vector<SymbolConfig> &symbols) -> bool {
    for (const auto &symbol : symbols) {
      if (!SymbolUtil::LoadSymbol(library_handle_, *symbol.func_ptr,
                                  symbol.symbol_name)) {
        return false;
      }
    }
    return true;
  };

  std::vector<SymbolConfig> common_symbols = {
      {reinterpret_cast<void **>(&get_version_func_), "ByteVC1_get_version"},
      {reinterpret_cast<void **>(&set_log_callback_func_),
       "ByteVC1_set_log_callback"},
      {reinterpret_cast<void **>(&alloc_default_param_func_),
       "ByteVC1_alloc_default_param"},
      {reinterpret_cast<void **>(&dec_create_func_), "ByteVC1_dec_create"},
      {reinterpret_cast<void **>(&dec_destroy_func_), "ByteVC1_dec_destroy"},
      {reinterpret_cast<void **>(&alloc_packet_func_), "ByteVC1_alloc_packet"},
      {reinterpret_cast<void **>(&alloc_frame_func_), "ByteVC1_alloc_frame"},
      {reinterpret_cast<void **>(&send_packet_func_), "ByteVC1_send_packet"},
      {reinterpret_cast<void **>(&get_frame_func_), "ByteVC1_get_frame"},
      {reinterpret_cast<void **>(&return_frame_func_), "ByteVC1_return_frame"}};

  if (!load_symbols(common_symbols)) {
    return false;
  }

  if (get_version_func_) {
    get_version_func_(version_);
  }
  is_new_api_ = version_[0] >= '2';

  std::vector<SymbolConfig> version_specific_symbols;
  if (is_new_api_) {
    version_specific_symbols = {
        {reinterpret_cast<void **>(&release_param_func_),
         "ByteVC1_release_param"},
        {reinterpret_cast<void **>(&release_ttpacket_func_),
         "ByteVC1_release_ttpacket"},
        {reinterpret_cast<void **>(&init_ttpacket_func_),
         "ByteVC1_init_ttpacket"},
        {reinterpret_cast<void **>(&frame_got_frame_func_),
         "ByteVC1_frame_got_frame"},
        {reinterpret_cast<void **>(&frame_get_img_property_func_),
         "ByteVC1_frame_get_img_property"},
        {reinterpret_cast<void **>(&frame_get_buffer_property_func_),
         "ByteVC1_frame_get_buffer_property"}};
  } else {
    version_specific_symbols = {
        {reinterpret_cast<void **>(&free_param_func_), "ByteVC1_free_param"},
        {reinterpret_cast<void **>(&free_packet_func_), "ByteVC1_free_packet"},
        {reinterpret_cast<void **>(&set_packet_bs_func_),
         "ByteVC1_set_packet_bs"},
        {reinterpret_cast<void **>(&set_packet_bs_len_func_),
         "ByteVC1_set_packet_bs_len"},
        {reinterpret_cast<void **>(&dec_decode_func_), "ByteVC1_dec_decode"},
        {reinterpret_cast<void **>(&get_frame_got_frame_func_),
         "ByteVC1_get_frame_got_frame"},
        {reinterpret_cast<void **>(&get_frame_pix_fmt_func_),
         "ByteVC1_get_frame_pix_fmt"},
        {reinterpret_cast<void **>(&get_frame_width_func_),
         "ByteVC1_get_frame_width"},
        {reinterpret_cast<void **>(&get_frame_height_func_),
         "ByteVC1_get_frame_height"},
        {reinterpret_cast<void **>(&get_frame_data_func_),
         "ByteVC1_get_frame_data"},
        {reinterpret_cast<void **>(&get_frame_linesize_func_),
         "ByteVC1_get_frame_linesize"},
        {reinterpret_cast<void **>(&get_frame_stride_func_),
         "ByteVC1_get_frame_stride"},
        {reinterpret_cast<void **>(&get_frame_bit_depth_func_),
         "ByteVC1_get_frame_bit_depth"}};
  }

  if (!load_symbols(version_specific_symbols)) {
    return false;
  }

  is_symbols_valid_ = true;
  return is_symbols_valid_;
}

}  // namespace animax
}  // namespace lynx
