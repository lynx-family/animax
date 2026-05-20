// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/video/ios/video_shader_ios.h"
#include <memory>
#include "base/include/no_destructor.h"
#include "src/base/log/log.h"
#include "src/render/image_mtl.h"
#include "src/render/texture_info_mtl.h"
#include "src/video/ios/metal_shaders.h"

#import <CoreVideo/CoreVideo.h>
#import <Metal/Metal.h>

namespace lynx {
namespace animax {

struct MetalResourceManager {
  id<MTLDevice> device;
  id<MTLLibrary> library;
  id<MTLRenderPipelineState> pipeline_state;
  id<MTLBuffer> position_factor_buffer;

  MetalResourceManager() {
    device = MTLCreateSystemDefaultDevice();
    if (device) {
      InitializeLibrary();
      InitializePipelineState();
      InitializeSharedBuffer();
    }
  }

  void InitializeLibrary() {
    if (!device) {
      return;
    }

    NSString *shader_source =
        [NSString stringWithUTF8String:lynx::animax::kMetalShaderSource.c_str()];
    NSError *shader_error = nil;
    library = [device newLibraryWithSource:shader_source options:nil error:&shader_error];
    if (!library) {
      ANIMAX_LOGE("Failed to create Metal library: " <<
                  [shader_error.localizedDescription UTF8String]);
    }
  }

  void InitializePipelineState() {
    if (!device || !library) {
      return;
    }

    // Load vertex and fragment functions
    id<MTLFunction> vertex_function = [library newFunctionWithName:@"vertexShader"];
    id<MTLFunction> fragment_function = [library newFunctionWithName:@"fragmentShader"];

    if (!vertex_function || !fragment_function) {
      return;
    }

    // Create render pipeline descriptor
    MTLRenderPipelineDescriptor *pipeline_descriptor = [[MTLRenderPipelineDescriptor alloc] init];
    [pipeline_descriptor setVertexFunction:vertex_function];
    [pipeline_descriptor setFragmentFunction:fragment_function];
    [pipeline_descriptor.colorAttachments[0] setPixelFormat:MTLPixelFormatBGRA8Unorm];

    // Apply blend configuration
    [pipeline_descriptor.colorAttachments[0] setSourceRGBBlendFactor:MTLBlendFactorSourceAlpha];
    [pipeline_descriptor.colorAttachments[0] setSourceAlphaBlendFactor:MTLBlendFactorSourceAlpha];
    [pipeline_descriptor.colorAttachments[0]
        setDestinationRGBBlendFactor:MTLBlendFactorOneMinusSourceAlpha];
    [pipeline_descriptor.colorAttachments[0]
        setDestinationAlphaBlendFactor:MTLBlendFactorOneMinusSourceAlpha];
    [pipeline_descriptor.colorAttachments[0] setBlendingEnabled:YES];
    [pipeline_descriptor.colorAttachments[0] setRgbBlendOperation:MTLBlendOperationAdd];
    [pipeline_descriptor.colorAttachments[0] setAlphaBlendOperation:MTLBlendOperationAdd];

    // Create render pipeline state
    NSError *error = nil;
    pipeline_state = [device newRenderPipelineStateWithDescriptor:pipeline_descriptor error:&error];

    if (error) {
      ANIMAX_LOGE("Error creating pipeline state: " << [error.localizedDescription UTF8String]);
    }
  }

  void InitializeSharedBuffer() {
    if (!device) {
      return;
    }

    // Initialize position factor buffer
    vector_float4 position_factor = {1.0f, 1.0f, 1.0f, 1.0f};
    position_factor_buffer = [device newBufferWithBytes:&position_factor
                                                 length:sizeof(vector_float4)
                                                options:MTLResourceStorageModeShared];
  }
};

MetalResourceManager &GetGlobalMetalResources() {
  static base::NoDestructor<MetalResourceManager> instance;
  return *instance;
}

VideoShaderIOS::VideoShaderIOS()
    : mtl_device_(nil),
      pipeline_state_(nil),
      position_factor_(nil),
      command_queue_(nil),
      vertex_buffer_(nil),
      output_texture_(nil),
      w_(0),
      h_(0),
      is_valid_(false) {}

std::unique_ptr<VideoShader> VideoShader::Make(std::shared_ptr<AnimaXAbility> ability) {
  return std::make_unique<VideoShaderIOS>();
}

bool VideoShaderIOS::Valid() { return is_valid_; }

void VideoShaderIOS::Init(int32_t w, int32_t h, const std::array<float, 4> &rgb_frame,
                          const std::array<float, 4> &a_frame) {
  if (is_valid_) {
    return;
  }

  w_ = w;
  h_ = h;
  rgb_frame_ = rgb_frame;
  a_frame_ = a_frame;

  if (!InitMetalResources()) {
    ANIMAX_LOGE("Failed to initialize Metal resources");
    return;
  }

  if (!SetupVertexData()) {
    ANIMAX_LOGE("Failed to setup vertex data");
    return;
  }

  if (!CreateOutputTexture()) {
    ANIMAX_LOGE("Failed to create output texture");
    return;
  }

  is_valid_ = true;
}

void VideoShaderIOS::Draw(std::unique_ptr<TextureInfo> texture_info,
                          const std::array<float, 16> &transform) {
  if (!is_valid_) {
    return;
  }

  TextureInfoMTL *texture_mtl = static_cast<TextureInfoMTL *>(texture_info.get());
  id<MTLTexture> input_texture = texture_mtl->GetMTLTexture();

  if (!input_texture) {
    return;
  }

  // Create command buffer
  id<MTLCommandBuffer> command_buffer = [command_queue_ commandBuffer];

  // Create render pass descriptor
  MTLRenderPassDescriptor *render_pass_descriptor = [MTLRenderPassDescriptor renderPassDescriptor];
  [render_pass_descriptor.colorAttachments[0] setTexture:output_texture_];
  [render_pass_descriptor.colorAttachments[0] setLoadAction:MTLLoadActionClear];
  [render_pass_descriptor.colorAttachments[0] setClearColor:MTLClearColorMake(0.0, 0.0, 0.0, 0.0)];
  [render_pass_descriptor.colorAttachments[0] setStoreAction:MTLStoreActionStore];

  // Create render command encoder
  id<MTLRenderCommandEncoder> render_encoder =
      [command_buffer renderCommandEncoderWithDescriptor:render_pass_descriptor];

  [render_encoder setRenderPipelineState:pipeline_state_];
  [render_encoder setVertexBuffer:vertex_buffer_ offset:0 atIndex:VertexInputIndexVertices];

  // Set position factor buffer (required by vertex shader)
  [render_encoder setVertexBuffer:position_factor_
                           offset:0
                          atIndex:VertexInputIndexRealRenderPositionFactor];

  // Set fragment texture
  [render_encoder setFragmentTexture:input_texture atIndex:FragmentTextureIndexTexture];

  // Draw
  [render_encoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];

  [render_encoder endEncoding];
  [command_buffer commit];
}

std::unique_ptr<Image> VideoShaderIOS::GetOutputImage(RealContext *context) {
  if (!is_valid_ || output_texture_ == nil) {
    return std::unique_ptr<Image>();
  }

  if (context->GetBackendType() == ContextBackend::kMetal) {
    TextureInfoMTL info(output_texture_, w_, h_);
    return std::make_unique<ImageMTL>(&info, context);
  } else {
    // unsupported backend type
    return std::unique_ptr<Image>();
  }
}

bool VideoShaderIOS::InitMetalResources() {
  MetalResourceManager &global_resources = GetGlobalMetalResources();
  mtl_device_ = global_resources.device;
  pipeline_state_ = global_resources.pipeline_state;
  position_factor_ = global_resources.position_factor_buffer;

  if (!mtl_device_ || !pipeline_state_ || !position_factor_) {
    return false;
  }

  command_queue_ = [mtl_device_ newCommandQueue];
  return command_queue_ != nil;
}

bool VideoShaderIOS::SetupVertexData() {
  if (!mtl_device_) {
    return false;
  }

  // Calculate texture coordinates for RGB and Alpha regions
  float rgb_x = rgb_frame_[0];
  float rgb_y = rgb_frame_[1];
  float rgb_w = rgb_frame_[2];
  float rgb_h = rgb_frame_[3];

  float alpha_x = a_frame_[0];
  float alpha_y = a_frame_[1];
  float alpha_w = a_frame_[2];
  float alpha_h = a_frame_[3];

  // Create vertex data with separate RGB and Alpha texture coordinates
  Vertex vertices[] = {
      // Bottom-left
      {{-1.0f, -1.0f, 0.0f, 1.0f}, {rgb_x, rgb_y + rgb_h}, {alpha_x, alpha_y + alpha_h}},
      // Bottom-right
      {{1.0f, -1.0f, 0.0f, 1.0f},
       {rgb_x + rgb_w, rgb_y + rgb_h},
       {alpha_x + alpha_w, alpha_y + alpha_h}},
      // Top-left
      {{-1.0f, 1.0f, 0.0f, 1.0f}, {rgb_x, rgb_y}, {alpha_x, alpha_y}},
      // Top-right
      {{1.0f, 1.0f, 0.0f, 1.0f}, {rgb_x + rgb_w, rgb_y}, {alpha_x + alpha_w, alpha_y}}};

  vertex_buffer_ = [mtl_device_ newBufferWithBytes:vertices
                                            length:sizeof(vertices)
                                           options:MTLResourceStorageModeShared];
  return vertex_buffer_ != nil;
}

bool VideoShaderIOS::CreateOutputTexture() {
  if (!mtl_device_) {
    return false;
  }

  MTLTextureDescriptor *texture_descriptor =
      [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm
                                                         width:w_
                                                        height:h_
                                                     mipmapped:NO];
  texture_descriptor.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
  output_texture_ = [mtl_device_ newTextureWithDescriptor:texture_descriptor];
  return output_texture_ != nil;
}

}  // namespace animax
}  // namespace lynx
