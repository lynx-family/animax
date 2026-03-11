// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/base/gl/gl_util.h"

#include <algorithm>

#include "base/include/no_destructor.h"
#include "src/base/gl/scoped_gl_object.h"
#include "src/base/gl/scoped_gl_reset_restore.h"
#include "src/base/log/log.h"

#if defined(OS_IOS)
#include <CoreFoundation/CFBundle.h>
#endif

namespace lynx {
namespace animax {

#if defined(OS_ANDROID)
namespace /* [Android Only] Tiled MSAA Related */ {
static const char *kGLEXTMultisampledRenderToTexture =
    "GL_EXT_multisampled_render_to_texture";
static PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC
    kGlRenderbufferStorageMultisampleEXT = nullptr;
static PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC
    kGlFramebufferTexture2DMultisampleExt = nullptr;

static constexpr const char *kTiledMSAABlitVertexShader = R"(#version 300 es
        precision highp float;
        precision mediump int;
      
      
        layout (location = 0) in vec2 aPos;
        layout (location = 1) in vec2 aUV;
      
        out vec2 vUV;
      
        void main() {
            vUV = aUV;
            gl_Position = vec4(aPos, 0.0, 1.0);
        }
      )";

static constexpr const char *kTiledMSAABlitFragmentShader = R"(#version 300 es
      precision highp float;
      precision mediump int;
    
      uniform sampler2D tex;
    
      in vec2 vUV;
    
      out vec4 FragColor;
    
      void main() {
        FragColor = texture(tex, vUV);
      }
    )";

void LoadEXTMultisampledRenderToTextureProc() {
  if (!IsTiledMSAASupported()) {
    // This device does not support GL_EXT_multisampled_render_to_texture
    return;
  }
  if (kGlRenderbufferStorageMultisampleEXT &&
      kGlFramebufferTexture2DMultisampleExt) {
    // already load functions
    return;
  }

  kGlRenderbufferStorageMultisampleEXT =
      reinterpret_cast<PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC>(
          LoadGLProc("glRenderbufferStorageMultisampleEXT"));
  kGlFramebufferTexture2DMultisampleExt =
      reinterpret_cast<PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC>(
          LoadGLProc("glFramebufferTexture2DMultisampleEXT"));

  if (!kGlFramebufferTexture2DMultisampleExt ||
      !kGlRenderbufferStorageMultisampleEXT) {
    ANIMAX_LOGE(
        "Failed to load procedures for GL_EXT_multisampled_render_to_texture");
  }
}
}  // namespace

bool IsTiledMSAASupported() {
  return IsGLExtensionSupported(kGLEXTMultisampledRenderToTexture);
}

ScopedGLObject CreateMultisampleEXTStencilBuffer(GLsizei width,
                                                 GLsizei height) {
  auto reset_renderbuffer =
      animax::ScopedGLResetRestore(GL_RENDERBUFFER_BINDING);

  GLuint stencil_buffer_object;
  glGenRenderbuffers(1, &stencil_buffer_object);
  auto stencil_buffer =
      ScopedGLObject{ScopedGLObject::kRenderbuffer, stencil_buffer_object};
  glBindRenderbuffer(GL_RENDERBUFFER, stencil_buffer);
  LoadEXTMultisampledRenderToTextureProc();
  if (kGlRenderbufferStorageMultisampleEXT) {
    kGlRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER, 4, GL_STENCIL_INDEX8,
                                         width, height);
    return stencil_buffer;
  } else {
    return ScopedGLObject{};
  }
}

ScopedGLObject CreateMultisampleEXTFramebuffer(
    GLuint texture, GLuint multisample_stencil_buffer) {
  auto reset_framebuffer = animax::ScopedGLResetRestore(GL_FRAMEBUFFER_BINDING);
  GLuint framebuffer_object;
  glGenFramebuffers(1, &framebuffer_object);
  auto framebuffer =
      ScopedGLObject(ScopedGLObject::kFramebuffer, framebuffer_object);

  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

  LoadEXTMultisampledRenderToTextureProc();
  if (texture && kGlFramebufferTexture2DMultisampleExt) {
    kGlFramebufferTexture2DMultisampleExt(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                          GL_TEXTURE_2D, texture, 0, 4);
  } else {
    return ScopedGLObject{};
  }

  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                            GL_RENDERBUFFER, multisample_stencil_buffer);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    ANIMAX_LOGE("Multisample Framebuffer is not complete.");
    return ScopedGLObject{};
  } else {
    return framebuffer;
  }
}

ScopedGLObject CreateTiledMSAAProgram() {
  return ScopedGLObject(
      ScopedGLObject::Type::kProgram,
      CreateProgram(kTiledMSAABlitVertexShader, kTiledMSAABlitFragmentShader));
}
#endif  // defined(OS_ANDROID)

namespace /* FXAA Shader Program */ {
constexpr static const char *kFXAAVertexShader = R"(#version 300 es
        precision highp float;
        precision mediump int;


        layout (location = 0) in vec2 aPos;
        layout (location = 1) in vec2 aUV;

        out vec2 vUV;

        void main() {
            vUV = aUV;
            gl_Position = vec4(aPos, 0.0, 1.0);
        }
      )";
constexpr static const char *kFXAAFragmentShader = R"(#version 300 es
      precision highp float;
      precision mediump int;

      uniform sampler2D tex;

      in vec2 vUV;

      out vec4 FragColor;

      #define FXAA_SPAN_MAX 4.0
      #define FXAA_REDUCE_MIN (1.0 / 64.0)
      #define FXAA_REDUCE_MUL (1.0 / 2.0)

      void main() {
        ivec2 resolution = textureSize(tex, 0);
        vec2 texelSize = 1.0 / vec2(resolution.x, resolution.y);

        vec3 luma = vec3(0.299, 0.587, 0.114);
        float lumaTL = dot(luma, texture(tex, vUV + (vec2(-1.0, -1.0) * texelSize)).xyz);
        float lumaTR = dot(luma, texture(tex, vUV + (vec2(1.0, -1.0) * texelSize)).xyz);
        float lumaBL = dot(luma, texture(tex, vUV + (vec2(-1.0, 1.0) * texelSize)).xyz);
        float lumaBR = dot(luma, texture(tex, vUV + (vec2(1.0, 1.0) * texelSize)).xyz);
        float lumaM  = dot(luma, texture(tex, vUV).xyz);

        vec2 dir;
        dir.x = -((lumaTL + lumaTR) - (lumaBL + lumaBR));
        dir.y = ((lumaTL + lumaBL) - (lumaTR + lumaBR));

        float dirReduce = max((lumaTL + lumaTR + lumaBL + lumaBR) * (FXAA_REDUCE_MUL * 0.25), FXAA_REDUCE_MIN);
        float inverseDirAdjustment = 1.0/(min(abs(dir.x), abs(dir.y)) + dirReduce);

        dir = min(vec2(FXAA_SPAN_MAX, FXAA_SPAN_MAX),
            max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX), dir * inverseDirAdjustment)) * texelSize;


        vec4 result1 = (1.0/2.0) * (
          texture(tex, vUV + (dir * vec2(1.0/3.0 - 0.5))) +
          texture(tex, vUV + (dir * vec2(2.0/3.0 - 0.5))));

        vec4 result2 = result1 * (1.0/2.0) + (1.0/4.0) * (
          texture(tex, vUV + (dir * vec2(0.0/3.0 - 0.5)))+
          texture(tex, vUV + (dir * vec2(3.0/3.0 - 0.5))));


        float lumaMin = min(lumaM, min(min(lumaTL, lumaTR), min(lumaBL, lumaBR)));
        float lumaMax = max(lumaM, max(max(lumaTL, lumaTR), max(lumaBL, lumaBR)));
        float lumaResult2 = dot(luma, result2.xyz);

        if(lumaResult2 < lumaMin || lumaResult2 > lumaMax)
          FragColor = result1;
        else
          FragColor = result2;
      }
    )";
}  // namespace

#if defined(OS_IOS)
using SkityGLFuncPtr = void (*)();
SkityGLFuncPtr GetProcLoader(const char *procname) {
  static CFBundleRef esBundle =
      CFBundleGetBundleWithIdentifier(CFSTR("com.apple.opengles"));
  CFStringRef symbolName = CFStringCreateWithCString(
      kCFAllocatorDefault, procname, kCFStringEncodingASCII);
  SkityGLFuncPtr symbol =
      (SkityGLFuncPtr)CFBundleGetFunctionPointerForName(esBundle, symbolName);
  CFRelease(symbolName);
  return symbol;
}

void *kGLProcLoader = reinterpret_cast<void *>(&GetProcLoader);
#elif defined(OS_ANDROID)
void *kGLProcLoader = reinterpret_cast<void *>(eglGetProcAddress);
#elif defined(OS_HARMONY)
void *kGLProcLoader = reinterpret_cast<void *>(eglGetProcAddress);
#elif defined(ANIMAX_GL_USE_OSMESA)
void *kGLProcLoader = reinterpret_cast<void *>(OSMesaGetProcAddress);
#elif defined(OS_WIN)
void *kGLProcLoader = reinterpret_cast<void *>(eglGetProcAddress);
#elif defined(OS_OSX)
void *kGLProcLoader = nullptr;
#elif defined(OS_WASM)
void *kGLProcLoader = reinterpret_cast<void *>(eglGetProcAddress);
#endif

void *GetGLProcLoader() { return kGLProcLoader; }

void *LoadGLProc(const char *proc_name) {
  using GlProcLoaderType = void *(*)(const char *);
  auto *loader = reinterpret_cast<GlProcLoaderType>(GetGLProcLoader());
  return loader(proc_name);
}

std::vector<std::string> SplitAndSortExtensions(const std::string &extensions) {
  auto extensionsVector = std::vector<std::string>{};

  size_t startPos = 0;
  size_t spacePos = extensions.find(' ');

  while (spacePos != std::string::npos) {
    extensionsVector.push_back(
        extensions.substr(startPos, spacePos - startPos));
    startPos = spacePos + 1;
    spacePos = extensions.find(' ', startPos);
  }

  // For the last extension (since there is no space after it)
  if (startPos < extensions.length()) {
    extensionsVector.push_back(extensions.substr(startPos));
  }

  std::sort(extensionsVector.begin(), extensionsVector.end());
  return extensionsVector;
}

std::vector<std::string> SplitAndSortGLExtensions() {
  const auto *extensions =
      reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS));
  if (!extensions) {
    return std::vector<std::string>{};
  }

  return SplitAndSortExtensions(extensions);
}

bool IsExtensionSupported(const std::vector<std::string> &extensions,
                          const std::string &extension) {
  return std::binary_search(extensions.begin(), extensions.end(), extension);
}

const std::vector<std::string> &GetGLExtensions() {
  static base::NoDestructor<std::vector<std::string>> extensions =
      base::NoDestructor<std::vector<std::string>>{
          std::vector<std::string>{SplitAndSortGLExtensions()}};
  return *extensions;
}

bool IsGLExtensionSupported(const std::string &extension) {
  return IsExtensionSupported(GetGLExtensions(), extension);
}

GLuint CreateShader(const char *source, GLenum type) {
  GLuint shader = glCreateShader(type);

  GLint success;
  glShaderSource(shader, 1, &source, nullptr);
  glCompileShader(shader);
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    GLchar info_log[1024];
    glGetShaderInfoLog(shader, 1024, nullptr, info_log);
    ANIMAX_LOGE("shader compile error " << info_log);
    glDeleteShader(shader);
    return 0;
  }

  return shader;
}

GLuint CreateProgram(const char *vs_code, const char *fs_code) {
  GLuint vs = CreateShader(vs_code, GL_VERTEX_SHADER);
  if (0 == vs) {
    return ScopedGLObject{};
  }
  GLuint fs = CreateShader(fs_code, GL_FRAGMENT_SHADER);
  if (0 == fs) {
    glDeleteShader(vs);
    return ScopedGLObject{};
  }

  GLuint program = glCreateProgram();
  GLint success;

  glAttachShader(program, vs);
  glAttachShader(program, fs);
  glLinkProgram(program);
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  glDeleteShader(vs);
  glDeleteShader(fs);
  if (!success) {
    GLchar info_log[1024];
    glGetProgramInfoLog(program, 1024, nullptr, info_log);
    ANIMAX_LOGE("program link error " << info_log);
    glDeleteProgram(program);
    return ScopedGLObject{};
  }

  return program;
}

ScopedGLObject CreateFXAAProgram() {
  return ScopedGLObject(ScopedGLObject::Type::kProgram,
                        CreateProgram(kFXAAVertexShader, kFXAAFragmentShader));
}

ScopedGLObject CreateTexImage2D(GLsizei width, GLsizei height) {
  if (width <= 0 || height <= 0) {
    return ScopedGLObject{};
  }
  auto reset_texture = animax::ScopedGLResetRestore(GL_TEXTURE_BINDING_2D);

  GLuint tex_object;
  glGenTextures(1, &tex_object);
  auto tex = ScopedGLObject(ScopedGLObject::kTexture, tex_object);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, static_cast<GLsizei>(width),
               static_cast<GLsizei>(height), 0, GL_RGBA, GL_UNSIGNED_BYTE,
               reinterpret_cast<void *>(0));
  return tex;
}

ScopedGLObject CreateStencilBuffer(GLsizei width, GLsizei height) {
  if (width <= 0 || height <= 0) {
    return ScopedGLObject{};
  }
  auto reset_renderbuffer =
      animax::ScopedGLResetRestore(GL_RENDERBUFFER_BINDING);

  GLuint stencil_buffer_object;
  glGenRenderbuffers(1, &stencil_buffer_object);
  auto stencil_buffer =
      ScopedGLObject{ScopedGLObject::kRenderbuffer, stencil_buffer_object};
  glBindRenderbuffer(GL_RENDERBUFFER, stencil_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, width, height);
  return stencil_buffer;
}

ScopedGLObject CreateFramebuffer(const ScopedGLObject &color_attachment,
                                 GLuint stencil_buffer) {
  auto reset_framebuffer = animax::ScopedGLResetRestore(GL_FRAMEBUFFER_BINDING);

  GLuint framebuffer_object;
  glGenFramebuffers(1, &framebuffer_object);
  auto framebuffer =
      ScopedGLObject(ScopedGLObject::kFramebuffer, framebuffer_object);

  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

  if (color_attachment.Type() == ScopedGLObject::kRenderbuffer) {
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                              GL_RENDERBUFFER, color_attachment);
  } else if (color_attachment.Type() == ScopedGLObject::kTexture) {
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           color_attachment, 0);
  } else {
    ANIMAX_LOGE(
        "Color Attachment Provided is neither a TEXTURE_2D or a RENDERBUFFER.");
    return ScopedGLObject{};
  }

  if (stencil_buffer) {
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                              GL_RENDERBUFFER, stencil_buffer);
  }

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    ANIMAX_LOGE("Framebuffer is not complete.");
    return ScopedGLObject{};
  }

  return framebuffer;
}

void AttachStencilBufferToFramebuffer(GLuint framebuffer,
                                      GLuint stencil_buffer) {
  auto reset_framebuffer = animax::ScopedGLResetRestore(GL_FRAMEBUFFER_BINDING);

  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                            GL_RENDERBUFFER, stencil_buffer);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    ANIMAX_LOGE("Framebuffer is not complete.");
  }
}

VAOVBOPair CreateFullScreenQuadVAOVBOPair() {
  auto resets = BundleScopedGLResetRestore(
      {GL_VERTEX_ARRAY_BINDING, GL_ARRAY_BUFFER_BINDING});
  GLuint vao_object;
  glGenVertexArrays(1, &vao_object);
  auto vao = ScopedGLObject(ScopedGLObject::kVertexArray, vao_object);
  glBindVertexArray(vao);

  GLuint vbo_object;
  glGenBuffers(1, &vbo_object);
  auto vbo = ScopedGLObject(ScopedGLObject::kBuffer, vbo_object);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  std::array<float, 16> vertex{
      -1.f, -1.f, 0.f, 0.f,  // v1
      1.f,  -1.f, 1.f, 0.f,  // v2
      -1.f, 1.f,  0.f, 1.f,  // v3
      1.f,  1.f,  1.f, 1.f,  // v4
  };
  glBufferData(GL_ARRAY_BUFFER, vertex.size() * sizeof(float), vertex.data(),
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                        reinterpret_cast<void *>(0));
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                        reinterpret_cast<void *>(2 * sizeof(float)));
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  return VAOVBOPair{std::move(vao), std::move(vbo)};
}

void DiscardStencilBuffer() {
  auto discards = std::array<GLenum, 1>{GL_STENCIL_ATTACHMENT};
  glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, discards.data());
}

void DiscardColorAttachment() {
  auto discards = std::array<GLenum, 1>{GL_COLOR_ATTACHMENT0};
  glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, discards.data());
}

}  // namespace animax
}  // namespace lynx
