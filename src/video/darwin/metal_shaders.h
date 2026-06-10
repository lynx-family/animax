// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#ifndef ANIMAX_SRC_VIDEO_DARWIN_METAL_SHADERS_H_
#define ANIMAX_SRC_VIDEO_DARWIN_METAL_SHADERS_H_

#include <simd/simd.h>

#include <string>

namespace lynx {
namespace animax {

typedef enum VertexInputIndex {
  VertexInputIndexVertices = 0,
  VertexInputIndexRealRenderPositionFactor = 1,
} VertexInputIndex;

typedef enum FragmentTextureIndex {
  FragmentTextureIndexTexture = 0,
} FragmentTextureIndex;

typedef struct {
  vector_float4 position;
  vector_float2 rgb_texture_coordinate;
  vector_float2 alpha_texture_coordinate;
} Vertex;

static const std::string kMetalShaderSource = R"(
#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

typedef enum VertexInputIndex {
  VertexInputIndexVertices = 0,
  VertexInputIndexRealRenderPositionFactor = 1,
} VertexInputIndex;

typedef enum FragmentTextureIndex {
  FragmentTextureIndexTexture = 0,
} FragmentTextureIndex;

typedef struct {
  vector_float4 position;
  vector_float2 rgb_texture_coordinate;
  vector_float2 alpha_texture_coordinate;
} Vertex;

typedef struct {
    float4 clipSpacePosition [[ position ]];
    float2 rgb_texture_coordinate;
    float2 alpha_texture_coordinate;
} RasterizerData;

vertex RasterizerData vertexShader(uint vertexID [[ vertex_id ]],
                                   constant Vertex *vertexArray [[ buffer(VertexInputIndexVertices) ]],
                                   constant vector_float4 *realRenderPositionFactorPointer [[buffer(VertexInputIndexRealRenderPositionFactor)]])
{
    RasterizerData out;
    vector_float4 realRenderPosFactor = vector_float4(*realRenderPositionFactorPointer);
    out.clipSpacePosition = realRenderPosFactor * vertexArray[vertexID].position;
    out.rgb_texture_coordinate = vertexArray[vertexID].rgb_texture_coordinate;
    out.alpha_texture_coordinate = vertexArray[vertexID].alpha_texture_coordinate;
    return out;
}

fragment float4 fragmentShader(RasterizerData input [[ stage_in ]],
                               texture2d<float> texture [[ texture(FragmentTextureIndexTexture) ]])
{
    constexpr sampler textureSampler (mag_filter::linear, min_filter::linear);
    float3 rgb = texture.sample(textureSampler, input.rgb_texture_coordinate).rgb;
    float alpha = texture.sample(textureSampler, input.alpha_texture_coordinate).r;
    return float4(rgb, alpha);
}
)";

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_VIDEO_DARWIN_METAL_SHADERS_H_
