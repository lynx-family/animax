// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/base/gl/scoped_gl_reset_restore.h"

#include <GL/osmesa.h>
#include <gtest/gtest.h>

static constexpr const char *kTestVertexShader = R"(#version 300 es
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

static constexpr const char *kTestFragmentShader = R"(#version 300 es
      precision highp float;
      precision mediump int;
    
      uniform sampler2D tex;
    
      in vec2 vUV;
    
      out vec4 FragColor;
    
      void main() {
        FragColor = texture(tex, vUV);
      }
    )";

GLuint CreateShader(const char *source, GLenum type) {
  GLuint shader = glCreateShader(type);

  GLint success;
  glShaderSource(shader, 1, &source, nullptr);
  glCompileShader(shader);
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    GLchar info_log[1024];
    glGetShaderInfoLog(shader, 1024, nullptr, info_log);
    glDeleteShader(shader);
    return 0;
  }

  return shader;
}

GLuint CreateProgram(const char *vs_code, const char *fs_code) {
  GLuint vs = CreateShader(vs_code, GL_VERTEX_SHADER);
  if (0 == vs) {
    return 0;
  }
  GLuint fs = CreateShader(fs_code, GL_FRAGMENT_SHADER);
  if (0 == fs) {
    glDeleteShader(vs);
    return 0;
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
    glDeleteProgram(program);
    return 0;
  }

  return program;
}

class ScopedGLResetRestoreTest : public ::testing::Test {
 protected:
  OSMesaContext context_;
  void *buffer_;

  void SetUp() override {
    context_ = OSMesaCreateContextExt(OSMESA_RGBA, 0, 0, 0, nullptr);
    buffer_ = std::malloc(1 * 1 * 4);
    OSMesaMakeCurrent(context_, buffer_, GL_UNSIGNED_BYTE, 1, 1);
  }

  void TearDown() override {
    OSMesaDestroyContext(context_);
    std::free(buffer_);
  }
};

TEST_F(ScopedGLResetRestoreTest, ReadFramebufferBindingReset) {
  using namespace lynx::animax;
  GLuint fb1;
  GLuint fb2;

  glGenFramebuffers(1, &fb1);
  glGenFramebuffers(1, &fb2);

  glBindFramebuffer(GL_READ_FRAMEBUFFER, fb1);
  {
    ScopedGLResetRestore reset(GL_READ_FRAMEBUFFER_BINDING);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fb2);
  }
  GLint current_fb;
  glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &current_fb);
  EXPECT_EQ(current_fb, fb1);
}

TEST_F(ScopedGLResetRestoreTest, FramebufferBindingReset) {
  using namespace lynx::animax;
  GLuint fb1;
  GLuint fb2;

  glGenFramebuffers(1, &fb1);
  glGenFramebuffers(1, &fb2);

  glBindFramebuffer(GL_FRAMEBUFFER, fb1);
  {
    ScopedGLResetRestore reset(GL_FRAMEBUFFER_BINDING);
    glBindFramebuffer(GL_FRAMEBUFFER, fb2);
  }
  GLint current_fb;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &current_fb);
  EXPECT_EQ(current_fb, fb1);
}

TEST_F(ScopedGLResetRestoreTest, CombinedTextureAndProgramReset) {
  using namespace lynx::animax;
  // Generate a texture
  GLuint texture1, texture2;
  glGenTextures(1, &texture1);
  glGenTextures(1, &texture2);

  // Create a dummy shader program (details of shader creation are skipped for
  // brevity)
  GLuint program1 = CreateProgram(kTestVertexShader, kTestFragmentShader);
  GLuint program2 = CreateProgram(kTestVertexShader, kTestFragmentShader);

  // 1. Set initial values for both GL_TEXTURE_BINDING_2D and GL_CURRENT_PROGRAM
  glBindTexture(GL_TEXTURE_2D, texture1);
  glUseProgram(program1);
  // Note: Ensure the initial values are properly set, possibly by glGet*
  // functions

  {
    // Use ScopedGLResetRestore or a similar mechanism to restore the states
    ScopedGLResetRestore textureReset(GL_TEXTURE_BINDING_2D);
    ScopedGLResetRestore programReset(GL_CURRENT_PROGRAM);

    // 2. Change the values of both enums inside the scoped block
    glBindTexture(GL_TEXTURE_2D, texture2);
    glUseProgram(program2);
  }

  // 3. Verify that both values are correctly restored
  GLint restoredTexture, restoredProgram;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &restoredTexture);
  glGetIntegerv(GL_CURRENT_PROGRAM, &restoredProgram);
  EXPECT_EQ(restoredTexture, texture1);
  EXPECT_EQ(restoredProgram, program1);

  // Cleanup
  glDeleteTextures(1, &texture1);
  glDeleteTextures(1, &texture2);
  glDeleteProgram(program1);
  glDeleteProgram(program2);
}

TEST_F(ScopedGLResetRestoreTest, CombinedActiveTextureAndVertexArrayBinding) {
  using namespace lynx::animax;
  // Set initial values for both GL_ACTIVE_TEXTURE and GL_VERTEX_ARRAY_BINDING
  glActiveTexture(GL_TEXTURE0);
  GLuint vao1, vao2;
  glGenVertexArrays(1, &vao1);
  glBindVertexArray(vao1);

  {
    ScopedGLResetRestore activeTextureReset(GL_ACTIVE_TEXTURE);
    ScopedGLResetRestore vertexArrayReset(GL_VERTEX_ARRAY_BINDING);

    // Change the values of both enums inside the scoped block
    glActiveTexture(GL_TEXTURE1);
    glGenVertexArrays(1, &vao2);
    glBindVertexArray(vao2);
  }

  // Verify that both values are correctly restored
  GLint restoredActiveTexture, restoredVAO;
  glGetIntegerv(GL_ACTIVE_TEXTURE, &restoredActiveTexture);
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &restoredVAO);
  EXPECT_EQ(restoredActiveTexture, GL_TEXTURE0);
  EXPECT_EQ(restoredVAO, vao1);

  // Cleanup
  glDeleteVertexArrays(1, &vao1);
  glDeleteVertexArrays(1, &vao2);
}

TEST_F(ScopedGLResetRestoreTest,
       CombinedArrayBufferAndElementArrayBufferBinding) {
  using namespace lynx::animax;
  // Set initial values for both GL_ARRAY_BUFFER_BINDING and
  // GL_ELEMENT_ARRAY_BUFFER_BINDING
  GLuint buffer1, buffer2, ebo1, ebo2;
  glGenBuffers(1, &buffer1);
  glBindBuffer(GL_ARRAY_BUFFER, buffer1);
  glGenBuffers(1, &ebo1);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo1);

  {
    ScopedGLResetRestore arrayBufferReset(GL_ARRAY_BUFFER_BINDING);
    ScopedGLResetRestore elementArrayBufferReset(
        GL_ELEMENT_ARRAY_BUFFER_BINDING);

    // Change the values of both enums inside the scoped block
    glGenBuffers(1, &buffer2);
    glBindBuffer(GL_ARRAY_BUFFER, buffer2);
    glGenBuffers(1, &ebo2);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo2);
  }

  // Verify that both values are correctly restored
  GLint restoredBuffer, restoredEBO;
  glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &restoredBuffer);
  glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &restoredEBO);
  EXPECT_EQ(restoredBuffer, buffer1);
  EXPECT_EQ(restoredEBO, ebo1);

  // Cleanup
  glDeleteBuffers(1, &buffer1);
  glDeleteBuffers(1, &buffer2);
  glDeleteBuffers(1, &ebo1);
  glDeleteBuffers(1, &ebo2);
}

TEST_F(ScopedGLResetRestoreTest, RenderbufferBindingReset) {
  using namespace lynx::animax;

  GLuint rb1, rb2;

  glGenRenderbuffers(1, &rb1);
  glGenRenderbuffers(1, &rb2);

  glBindRenderbuffer(GL_RENDERBUFFER, rb1);
  {
    ScopedGLResetRestore reset(GL_RENDERBUFFER_BINDING);
    glBindRenderbuffer(GL_RENDERBUFFER, rb2);
  }

  GLint current_rb;
  glGetIntegerv(GL_RENDERBUFFER_BINDING, &current_rb);
  EXPECT_EQ(current_rb, rb1);
}

TEST_F(ScopedGLResetRestoreTest, TextureBinding2DReset) {
  using namespace lynx::animax;

  GLuint tex1, tex2;

  glGenTextures(1, &tex1);
  glGenTextures(1, &tex2);

  glBindTexture(GL_TEXTURE_2D, tex1);
  {
    ScopedGLResetRestore reset(GL_TEXTURE_BINDING_2D);
    glBindTexture(GL_TEXTURE_2D, tex2);
  }

  GLint current_tex;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &current_tex);
  EXPECT_EQ(current_tex, tex1);
}

TEST_F(ScopedGLResetRestoreTest, CurrentProgramReset) {
  using namespace lynx::animax;

  GLuint program1 = CreateProgram(kTestVertexShader, kTestFragmentShader);
  GLuint program2 = CreateProgram(kTestVertexShader, kTestFragmentShader);

  glUseProgram(program1);
  EXPECT_EQ(glGetError(), 0);
  {
    ScopedGLResetRestore reset(GL_CURRENT_PROGRAM);
    glUseProgram(program2);
  }

  GLint current_program;
  glGetIntegerv(GL_CURRENT_PROGRAM, &current_program);
  EXPECT_EQ(current_program, program1);

  // Clean up the programs after the test
  glDeleteProgram(program1);
  glDeleteProgram(program2);
}

TEST_F(ScopedGLResetRestoreTest, VertexArrayBindingReset) {
  using namespace lynx::animax;

  GLuint vao1, vao2;

  glGenVertexArrays(1, &vao1);
  glGenVertexArrays(1, &vao2);

  glBindVertexArray(vao1);
  {
    ScopedGLResetRestore reset(GL_VERTEX_ARRAY_BINDING);
    glBindVertexArray(vao2);
  }

  GLint current_vao;
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &current_vao);
  EXPECT_EQ(current_vao, vao1);

  // Clean up after the test
  glDeleteVertexArrays(1, &vao1);
  glDeleteVertexArrays(1, &vao2);
}

TEST_F(ScopedGLResetRestoreTest, ArrayBufferBindingReset) {
  using namespace lynx::animax;

  GLuint vbo1, vbo2;

  glGenBuffers(1, &vbo1);
  glGenBuffers(1, &vbo2);

  glBindBuffer(GL_ARRAY_BUFFER, vbo1);
  {
    ScopedGLResetRestore reset(GL_ARRAY_BUFFER_BINDING);
    glBindBuffer(GL_ARRAY_BUFFER, vbo2);
  }

  GLint current_vbo;
  glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &current_vbo);
  EXPECT_EQ(current_vbo, vbo1);

  // Clean up after the test
  glDeleteBuffers(1, &vbo1);
  glDeleteBuffers(1, &vbo2);
}

TEST_F(ScopedGLResetRestoreTest, ElementArrayBufferBindingReset) {
  using namespace lynx::animax;

  GLuint ebo1, ebo2;

  glGenBuffers(1, &ebo1);
  glGenBuffers(1, &ebo2);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo1);
  {
    ScopedGLResetRestore reset(GL_ELEMENT_ARRAY_BUFFER_BINDING);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo2);
  }

  GLint current_ebo;
  glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &current_ebo);
  EXPECT_EQ(current_ebo, ebo1);

  // Clean up after the test
  glDeleteBuffers(1, &ebo1);
  glDeleteBuffers(1, &ebo2);
}

TEST_F(ScopedGLResetRestoreTest, ActiveTextureReset) {
  using namespace lynx::animax;

  glActiveTexture(GL_TEXTURE1);  // Set the active texture unit to GL_TEXTURE1
  {
    ScopedGLResetRestore reset(GL_ACTIVE_TEXTURE);
    glActiveTexture(GL_TEXTURE2);  // Change it to GL_TEXTURE2 within the scope
  }

  GLint current_active_texture;
  glGetIntegerv(GL_ACTIVE_TEXTURE, &current_active_texture);
  EXPECT_EQ(current_active_texture,
            GL_TEXTURE1);  // After the block, it should return to GL_TEXTURE1
}

TEST_F(ScopedGLResetRestoreTest, ViewportReset) {
  using namespace lynx::animax;

  GLint initialViewport[4] = {0, 0, 100, 100};
  GLint modifiedViewport[4] = {50, 50, 200, 200};

  glViewport(initialViewport[0], initialViewport[1], initialViewport[2],
             initialViewport[3]);
  {
    ScopedGLResetRestore reset(GL_VIEWPORT);
    glViewport(modifiedViewport[0], modifiedViewport[1], modifiedViewport[2],
               modifiedViewport[3]);
  }

  GLint currentViewport[4];
  glGetIntegerv(GL_VIEWPORT, currentViewport);

  EXPECT_EQ(currentViewport[0], initialViewport[0]);
  EXPECT_EQ(currentViewport[1], initialViewport[1]);
  EXPECT_EQ(currentViewport[2], initialViewport[2]);
  EXPECT_EQ(currentViewport[3], initialViewport[3]);
}

TEST_F(ScopedGLResetRestoreTest, ColorClearValueReset) {
  using namespace lynx::animax;

  GLfloat initialColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
  GLfloat modifiedColor[4] = {1.0f, 0.0f, 0.0f, 1.0f};

  glClearColor(initialColor[0], initialColor[1], initialColor[2],
               initialColor[3]);
  {
    ScopedGLResetRestore reset(GL_COLOR_CLEAR_VALUE);
    glClearColor(modifiedColor[0], modifiedColor[1], modifiedColor[2],
                 modifiedColor[3]);
  }

  GLfloat currentColor[4];
  glGetFloatv(GL_COLOR_CLEAR_VALUE, currentColor);

  EXPECT_EQ(currentColor[0], initialColor[0]);
  EXPECT_EQ(currentColor[1], initialColor[1]);
  EXPECT_EQ(currentColor[2], initialColor[2]);
  EXPECT_EQ(currentColor[3], initialColor[3]);
}

TEST_F(ScopedGLResetRestoreTest, ColorWriteMaskReset) {
  using namespace lynx::animax;

  GLint initialColor[4] = {0, 1, 1, 0};
  GLint modifiedColor[4] = {1, 1, 0, 0};

  glColorMask(initialColor[0], initialColor[1], initialColor[2],
              initialColor[3]);
  {
    ScopedGLResetRestore reset(GL_COLOR_WRITEMASK);
    glColorMask(modifiedColor[0], modifiedColor[1], modifiedColor[2],
                modifiedColor[3]);
  }

  GLint currentColor[4];
  glGetIntegerv(GL_COLOR_WRITEMASK, currentColor);

  EXPECT_EQ(currentColor[0], initialColor[0]);
  EXPECT_EQ(currentColor[1], initialColor[1]);
  EXPECT_EQ(currentColor[2], initialColor[2]);
  EXPECT_EQ(currentColor[3], initialColor[3]);
}

TEST_F(ScopedGLResetRestoreTest, StencilClearValueReset) {
  using namespace lynx::animax;

  GLint initialStencil = 0;
  GLint modifiedStencil = 1;

  glClearStencil(initialStencil);
  {
    ScopedGLResetRestore reset(GL_STENCIL_CLEAR_VALUE);
    glClearStencil(modifiedStencil);
  }

  GLint currentStencil;
  glGetIntegerv(GL_STENCIL_CLEAR_VALUE, &currentStencil);
  EXPECT_EQ(currentStencil, initialStencil);
}

TEST_F(ScopedGLResetRestoreTest, DepthClearValueReset) {
  using namespace lynx::animax;

  GLfloat initialDepth = 1.0f;
  GLfloat modifiedDepth = 0.5f;

  glClearDepthf(initialDepth);
  {
    ScopedGLResetRestore reset(GL_DEPTH_CLEAR_VALUE);
    glClearDepthf(modifiedDepth);
  }

  GLfloat currentDepth;
  glGetFloatv(GL_DEPTH_CLEAR_VALUE, &currentDepth);
  EXPECT_EQ(currentDepth, initialDepth);
}

TEST_F(ScopedGLResetRestoreTest, DepthWriteMaskReset) {
  using namespace lynx::animax;

  glDepthMask(GL_TRUE);
  {
    ScopedGLResetRestore reset(GL_DEPTH_WRITEMASK);
    glDepthMask(GL_FALSE);
  }

  GLboolean currentDepthMask;
  glGetBooleanv(GL_DEPTH_WRITEMASK, &currentDepthMask);
  EXPECT_EQ(currentDepthMask, GL_TRUE);
}

TEST_F(ScopedGLResetRestoreTest, StencilWriteMaskReset) {
  using namespace lynx::animax;

  GLuint initialMask = 0xFFFF;
  GLuint modifiedMask = 0x0;

  glStencilMask(initialMask);
  {
    ScopedGLResetRestore reset(GL_STENCIL_WRITEMASK);
    glStencilMask(modifiedMask);
  }

  GLint currentMask;
  glGetIntegerv(GL_STENCIL_WRITEMASK, &currentMask);
  EXPECT_EQ(currentMask, initialMask);
}

TEST_F(ScopedGLResetRestoreTest, StateToggleReset) {
  using namespace lynx::animax;

  // List of states to test
  GLenum states[] = {GL_BLEND, GL_CULL_FACE, GL_SCISSOR_TEST, GL_STENCIL_TEST,
                     GL_DEPTH_TEST};

  for (GLenum state : states) {
    // Testing: Initially Enabled -> Disabled
    glEnable(state);
    {
      ScopedGLResetRestore reset(state);
      glDisable(state);
    }
    EXPECT_TRUE(glIsEnabled(state));

    // Testing: Initially Disabled -> Enabled
    glDisable(state);
    {
      ScopedGLResetRestore reset(state);
      glEnable(state);
    }
    EXPECT_FALSE(glIsEnabled(state));
  }
}

TEST_F(ScopedGLResetRestoreTest, BlendParametersReset) {
  using namespace lynx::animax;

  GLint initialSrcRGB, initialSrcAlpha, initialDstRGB, initialDstAlpha;

  // Store initial blend parameters
  glGetIntegerv(GL_BLEND_SRC_RGB, &initialSrcRGB);
  glGetIntegerv(GL_BLEND_SRC_ALPHA, &initialSrcAlpha);
  glGetIntegerv(GL_BLEND_DST_RGB, &initialDstRGB);
  glGetIntegerv(GL_BLEND_DST_ALPHA, &initialDstAlpha);

  {
    ScopedGLResetRestore reset(
        GL_BLEND_SRC_RGB);  // Assumes this special class captures and restores
                            // all four states
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
  }

  GLint currentSrcRGB, currentSrcAlpha, currentDstRGB, currentDstAlpha;
  glGetIntegerv(GL_BLEND_SRC_RGB, &currentSrcRGB);
  glGetIntegerv(GL_BLEND_SRC_ALPHA, &currentSrcAlpha);
  glGetIntegerv(GL_BLEND_DST_RGB, &currentDstRGB);
  glGetIntegerv(GL_BLEND_DST_ALPHA, &currentDstAlpha);

  EXPECT_EQ(currentSrcRGB, initialSrcRGB);
  EXPECT_EQ(currentSrcAlpha, initialSrcAlpha);
  EXPECT_EQ(currentDstRGB, initialDstRGB);
  EXPECT_EQ(currentDstAlpha, initialDstAlpha);
}

TEST_F(ScopedGLResetRestoreTest, BlendAndDepthStatesReset) {
  using namespace lynx::animax;

  // Enable the GL_BLEND and GL_DEPTH_TEST states
  glEnable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);

  {
    BundleScopedGLResetRestore bundle({GL_BLEND, GL_DEPTH_TEST});
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
  }

  // After the bundle scope, the states should be restored to their enabled
  // status
  EXPECT_TRUE(glIsEnabled(GL_BLEND));
  EXPECT_TRUE(glIsEnabled(GL_DEPTH_TEST));
}

TEST_F(ScopedGLResetRestoreTest, UnsupportedResetRestore) {
  using namespace lynx::animax;
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#if !defined(NDEBUG)
  auto test_lambda = []() { ScopedGLResetRestore reset(GL_NONE); };
  EXPECT_DEATH(test_lambda(), "");
#else
  ScopedGLResetRestore reset(GL_NONE);
#endif
}
