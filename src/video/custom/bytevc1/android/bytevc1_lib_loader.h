// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#ifndef ANIMAX_SRC_VIDEO_CUSTOM_BYTEVC1_ANDROID_BYTEVC1_LIB_LOADER_H_
#define ANIMAX_SRC_VIDEO_CUSTOM_BYTEVC1_ANDROID_BYTEVC1_LIB_LOADER_H_

#include <dlfcn.h>
#include <unistd.h>

#include <memory>
#include <shared_mutex>
#include <string>
#include <vector>

#include "base/include/closure.h"
#include "base/include/no_destructor.h"
#include "src/video/custom/yuv_frame_info.h"

namespace lynx {
namespace animax {

enum class ByteVc1DecStat : uint8_t {
  kDecStatOK,
  kDecStatErr,
  kDecStatAgain,
  kDecStatEnd,
};

class ByteVC1LibLoader {
 public:
  // Deleter classes for smart pointers
  struct DecoderDeleter {
    void operator()(void* decoder) const;
  };

  struct PacketDeleter {
    void operator()(void* packet) const;
  };

  struct FrameDeleter {
    void* decoder_;

    explicit FrameDeleter(void* dec = nullptr) : decoder_(dec) {}
    void operator()(void* frame) const;
  };

  using DecoderPtr = std::unique_ptr<void, DecoderDeleter>;
  using PacketPtr = std::unique_ptr<void, PacketDeleter>;
  using FramePtr = std::unique_ptr<void, FrameDeleter>;

  /**
   * Gets the singleton instance of VideoPlayerService.
   */
  static ByteVC1LibLoader& Instance() {
    static base::NoDestructor<ByteVC1LibLoader> instance;
    return *instance;
  }

  /**
   * Checks if the ByteVC1 decoder library is successfully loaded and all
   * required symbols are valid.
   * @return true if the library is loaded and all required symbols are valid,
   * false otherwise.
   */
  bool IsValid() const;

  /**
   * Creates and initializes a new ByteVC1 decoder instance.
   * Allocates decoder parameters, creates the decoder context, and returns
   * a managed pointer that automatically handles resource cleanup.
   * @return Unique pointer to the decoder instance, or nullptr if creation
   * fails.
   */
  DecoderPtr CreateDecoder() const;

  /**
   * Creates a packet for decoding from the provided bitstream data.
   * Initializes the packet with the given buffer data
   * @param buffer Reference to the vector containing encoded bitstream data.
   * @return Unique pointer to the initialized packet, or nullptr if allocation
   * fails.
   */
  PacketPtr CreatePacket(std::vector<uint8_t>& buffer) const;

  /**
   * Decodes a single packet and retrieves the resulting frame.
   * Sends the packet to the decoder, retrieves the decoded frame, and checks
   * if the frame contains valid data.
   * @param decoder_ptr Reference to the decoder instance managing the decoding
   * session.
   * @param packet_ptr Reference to the packet containing encoded data to
   * decode.
   * @return Unique pointer to the decoded frame, or nullptr if decoding fails
   *         or no frame data is available.
   */
  FramePtr DecodePacket(const DecoderPtr& decoder_ptr,
                        const PacketPtr& packet_ptr) const;

  /**
   * Extracts frame data and populates the provided YUVFrameInfo structure.
   * Retrieves frame properties (width, height, format, etc.) and plane data
   * using the APIs based on library version, and updates the
   * YUVFrameInfo object with the extracted information.
   * @param frame_ptr Reference to the frame containing decoded video data.
   * @param frame_info Shared pointer to the YUVFrameInfo object to be populated
   *                   with frame properties and data pointers.
   */
  void ExtractFrameData(const FramePtr& frame_ptr,
                        const std::shared_ptr<YUVFrameInfo>& frame_info) const;

  /**
   * Sets a custom log callback function for the decoder instance.
   * Allows the application to receive logging messages from the ByteVC1 decoder
   * for debugging and monitoring purposes.
   * @param decoder_ptr Reference to the decoder instance to configure.
   * @param log_callback Function pointer to the custom log callback that
   *                     follows the printf-style format signature.
   */
  void SetLogCallback(const DecoderPtr& decoder_ptr,
                      int (*log_callback)(void*, int, const char*, ...)) const;

 private:
  ByteVC1LibLoader();
  ~ByteVC1LibLoader() = delete;
  friend class base::NoDestructor<ByteVC1LibLoader>;

  bool LoadLibrary();
  bool LoadAllSymbols();

  static constexpr const char* kByteVcLibName = "libByteVC1_dec.so";
  void* library_handle_ = nullptr;
  bool is_new_api_ = false;
  bool is_symbols_valid_ = false;
  char version_[256] = {0};

  // ============================================================================
  // Library Information & Logging
  // ============================================================================

  /**
   * Gets the version information of the ByteVC1 decoder library.
   * @param version Buffer to store version string (minimum 256 bytes).
   */
  void (*get_version_func_)(char* version) = nullptr;

  /**
   * Sets a custom log callback function for the decoder.
   * @param ctx Decoder context pointer.
   * @param callback Log callback function pointer.
   */
  void (*set_log_callback_func_)(void* ctx, void* callback) = nullptr;

  // ============================================================================
  // Decoder Parameter Management
  // ============================================================================

  /**
   * Allocates and initializes decoder parameters with default values.
   * @return Pointer to allocated ByteVC1DecParam structure.
   */
  void* (*alloc_default_param_func_)() = nullptr;

  /**
   * Releases decoder parameters allocated by alloc_default_param_func_ (New
   * API).
   * @param param Decoder parameters pointer to release.
   */
  void (*release_param_func_)(void* param) = nullptr;

  /**
   * Releases decoder parameters allocated by alloc_default_param_func_ (Old
   * API).
   * @param param Decoder parameters pointer to release.
   */
  void (*free_param_func_)(void* param) = nullptr;

  // ============================================================================
  // Decoder Lifecycle Management
  // ============================================================================

  /**
   * Creates a new decoder instance with the specified parameters.
   * @param param Decoder configuration parameters.
   * @return Pointer to created decoder context.
   */
  void* (*dec_create_func_)(void* param) = nullptr;

  /**
   * Destroys a decoder instance and releases all associated resources.
   * @param ctx Decoder context pointer to destroy.
   */
  void (*dec_destroy_func_)(void* ctx) = nullptr;

  // ============================================================================
  // Packet Management (Input Data)
  // ============================================================================

  /**
   * Allocates a new packet for holding compressed bitstream data.
   * @return Pointer to allocated TTPacket structure.
   */
  void* (*alloc_packet_func_)() = nullptr;

  /**
   * Releases a packet allocated by alloc_packet_func_ (New API).
   * @param packet Packet pointer to release.
   */
  void (*release_ttpacket_func_)(void* packet) = nullptr;

  /**
   * Releases a packet allocated by alloc_packet_func_ (Old API).
   * @param packet Packet pointer to release.
   */
  void (*free_packet_func_)(void* packet) = nullptr;

  /**
   * Sets the bitstream data buffer for a packet (Old API).
   * @param packet Packet pointer.
   * @param bs Bitstream data buffer.
   */
  void (*set_packet_bs_func_)(void* packet, uint8_t* bs) = nullptr;

  /**
   * Sets the length of the bitstream data in a packet (Old API).
   * @param packet Packet pointer.
   * @param bs_len Length of bitstream data in bytes.
   */
  void (*set_packet_bs_len_func_)(void* packet, uint32_t bs_len) = nullptr;

  /**
   * Initializes packet with all parameters in a single call (New API).
   * @param packet Packet pointer to initialize.
   * @param bs Bitstream data buffer.
   * @param bs_len Length of bitstream data in bytes.
   * @param drop_frame Whether to drop this frame during decoding (1: drop, 0:
   * decode).
   * @param drop_rate Decoder drop rate setting.
   * @param pts Presentation timestamp for the frame.
   * @param opt Optional parameter for the frame.
   */
  void (*init_ttpacket_func_)(void* packet, uint8_t* bs, unsigned int bs_len,
                              int drop_frame, float drop_rate, long long pts,
                              long long opt) = nullptr;

  // ============================================================================
  // Frame Management (Output Data)
  // ============================================================================

  /**
   * Allocates a frame for holding decoded output data.
   * @return Pointer to allocated TTFrame structure.
   */
  void* (*alloc_frame_func_)() = nullptr;

  // ============================================================================
  // Decoding Operations
  // ============================================================================

  /**
   * Synchronous frame decoding (Old API).
   * @param ctx Decoder context pointer.
   * @param packet Packet containing bitstream data to decode.
   * @param frame Frame to store decoded output.
   * @return Decoding status (0: success, <0: error).
   */
  int (*dec_decode_func_)(void* ctx, void* packet, void* frame) = nullptr;

  /**
   * Sends a packet to the decoder for asynchronous decoding.
   * @param ctx Decoder context pointer.
   * @param packet Packet containing bitstream data to decode.
   * @return Decoding status (0: success, 1: error).
   */
  int (*send_packet_func_)(void* ctx, void* packet) = nullptr;

  /**
   * Retrieves a decoded frame from the decoder.
   * @param ctx Decoder context pointer.
   * @param frame Frame to store decoded output.
   * @return Status (0: success, 1: error).
   */
  int (*get_frame_func_)(void* ctx, void* frame) = nullptr;

  /**
   * Returns a frame back to the decoder after processing (asynchronous mode).
   * @param ctx Decoder context pointer.
   * @param frame Frame to return to decoder.
   * @return Status (0: success, 1: error).
   */
  int (*return_frame_func_)(void* ctx, void* frame) = nullptr;

  // ============================================================================
  // Frame Status Checking
  // ============================================================================

  /**
   * Checks if a frame contains decoded data (New API).
   * @param frame Frame to check.
   * @return 1 if frame contains data, 0 otherwise.
   */
  int (*frame_got_frame_func_)(void* frame) = nullptr;

  /**
   * Checks if a frame contains decoded data (Old API).
   * @param frame Frame to check.
   * @return 1 if frame contains data, 0 otherwise.
   */
  int (*get_frame_got_frame_func_)(void* frame) = nullptr;

  // ============================================================================
  // Frame Property Access (New API)
  // ============================================================================

  /**
   * Gets comprehensive image properties from a decoded frame (New API).
   * @param frame Frame to query.
   * @param width[out] Frame width in pixels.
   * @param height[out] Frame height in pixels.
   * @param pts[out] Presentation timestamp.
   * @param opt[out] Optional parameter.
   * @param bit_depth[out] Bits per pixel component (8, 10, etc.).
   * @param color_primaries[out] Color primaries information.
   * @param color_trc[out] Transfer characteristics.
   * @param colorspace[out] Colorspace matrix coefficients.
   * @param pix_fmt[out] Pixel format enumeration.
   * @param slice_type[out] Frame type (I/P/B).
   */
  void (*frame_get_img_property_func_)(
      void* frame, unsigned int* width, unsigned int* height, int64_t* pts,
      int64_t* opt, unsigned int* bit_depth, unsigned int* color_primaries,
      unsigned int* color_trc, unsigned int* colorspace, unsigned int* pix_fmt,
      unsigned int* slice_type) = nullptr;

  /**
   * Gets buffer properties from a decoded frame (New API).
   * @param frame Frame to query.
   * @param linewidth[out] Array of valid pixels per line for each plane.
   * @param linesize[out] Array of byte strides for each plane.
   * @param data[out] Array of data pointers for each plane.
   */
  void (*frame_get_buffer_property_func_)(void* frame, uint32_t* linewidth,
                                          uint32_t* linesize,
                                          uint8_t** data) = nullptr;

  // ============================================================================
  // Frame Property Access (Old API)
  // ============================================================================

  /**
   * Gets the pixel format of a decoded frame (Old API).
   * @param frame Frame to query.
   * @return Pixel format enumeration (0: I400, 1: I420, 2: I422, 3: I444).
   */
  int (*get_frame_pix_fmt_func_)(void* frame) = nullptr;

  /**
   * Gets the width of a decoded frame (Old API).
   * @param frame Frame to query.
   * @return Frame width in pixels.
   */
  uint32_t (*get_frame_width_func_)(void* frame) = nullptr;

  /**
   * Gets the height of a decoded frame (Old API).
   * @param frame Frame to query.
   * @return Frame height in pixels.
   */
  uint32_t (*get_frame_height_func_)(void* frame) = nullptr;

  /**
   * Gets the data pointer for a specific plane of a decoded frame (Old API).
   * @param frame Frame to query.
   * @param channel Plane channel (0: Y, 1: U, 2: V).
   * @return Pointer to plane data.
   */
  uint8_t* (*get_frame_data_func_)(void* frame, int channel) = nullptr;

  /**
   * Gets the line size (stride) for a specific plane (Old API).
   * @param frame Frame to query.
   * @param channel Plane channel (0: Y, 1: U, 2: V).
   * @return Line size in bytes for the specified plane.
   */
  uint32_t (*get_frame_linesize_func_)(void* frame, int channel) = nullptr;

  /**
   * Gets the valid pixel stride for a specific plane (Old API).
   * @param frame Frame to query.
   * @param channel Plane channel (0: Y, 1: U, 2: V).
   * @return Valid pixel width in bytes for the specified plane.
   */
  uint32_t (*get_frame_stride_func_)(void* frame, int channel) = nullptr;

  /**
   * Gets the bit depth of a decoded frame (Old API).
   * @param frame Frame to query.
   * @return Bit depth (1: 8-bit, 2: 10-bit, etc.).
   */
  uint8_t (*get_frame_bit_depth_func_)(void* frame) = nullptr;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_VIDEO_CUSTOM_BYTEVC1_ANDROID_BYTEVC1_LIB_LOADER_H_
