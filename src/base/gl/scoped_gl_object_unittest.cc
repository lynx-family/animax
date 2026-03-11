// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/base/gl/scoped_gl_object.h"

#include <GL/osmesa.h>
#include <gtest/gtest.h>

class ScopedGLObjectTest : public ::testing::Test {
 protected:
  OSMesaContext context_;
  void* buffer_;

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

TEST_F(ScopedGLObjectTest, Constructor) {
  using namespace lynx::animax;
  GLuint framebuffer;
  glGenFramebuffers(1, &framebuffer);
  auto original =
      ScopedGLObject(ScopedGLObject::Type::kFramebuffer, framebuffer);
  EXPECT_EQ(static_cast<GLuint>(original), framebuffer);
  EXPECT_EQ(original.Type(), ScopedGLObject::Type::kFramebuffer);
}

TEST_F(ScopedGLObjectTest, InvalidConstructor) {
  using namespace lynx::animax;
#if !defined(NDEBUG)
  auto test_lambda = []() {
    GLuint framebuffer = 1234;
    auto original = ScopedGLObject(
        static_cast<enum ScopedGLObject::Type>(INT_MAX), framebuffer);
  };
  EXPECT_DEATH(test_lambda(), "");
#else
  ScopedGLObject(static_cast<enum ScopedGLObject::Type>(INT_MAX), 123);
#endif
}

TEST_F(ScopedGLObjectTest, MoveAssignmentOperator) {
  using namespace lynx::animax;
  GLuint framebuffer;
  glGenFramebuffers(1, &framebuffer);
  {
    auto original =
        ScopedGLObject(ScopedGLObject::Type::kFramebuffer, framebuffer);
    auto moved = std::move(original);
    EXPECT_EQ(original.Type(), ScopedGLObject::Type::kInvalid);
    EXPECT_EQ(static_cast<GLuint>(original), 0);
    EXPECT_EQ(moved.Type(), ScopedGLObject::Type::kFramebuffer);
    EXPECT_EQ(static_cast<GLuint>(moved), framebuffer);
  }
}

TEST_F(ScopedGLObjectTest, MoveConstrctor) {
  using namespace lynx::animax;
  GLuint framebuffer;
  glGenFramebuffers(1, &framebuffer);
  {
    auto original =
        ScopedGLObject(ScopedGLObject::Type::kFramebuffer, framebuffer);
    auto moved = ScopedGLObject{std::move(original)};
    EXPECT_EQ(original.Type(), ScopedGLObject::Type::kInvalid);
    EXPECT_EQ(static_cast<GLuint>(original), 0);
    EXPECT_EQ(moved.Type(), ScopedGLObject::Type::kFramebuffer);
    EXPECT_EQ(static_cast<GLuint>(moved), framebuffer);
  }
}

TEST_F(ScopedGLObjectTest, SelfMoveConstrctor) {
  using namespace lynx::animax;
  GLuint framebuffer;
  glGenFramebuffers(1, &framebuffer);
  {
    auto original =
        ScopedGLObject(ScopedGLObject::Type::kFramebuffer, framebuffer);
    original = std::move(original);
    EXPECT_EQ(static_cast<GLuint>(original), framebuffer);
    EXPECT_EQ(original.Type(), ScopedGLObject::Type::kFramebuffer);
  }
}

TEST_F(ScopedGLObjectTest, DestructorDeletesGLObject) {
  using namespace lynx::animax;
  GLuint fb;
  glGenFramebuffers(1, &fb);
  glBindFramebuffer(GL_FRAMEBUFFER, fb);
  EXPECT_EQ(glGetError(), GL_NO_ERROR);
  {
    GLuint rb;
    glGenRenderbuffers(1, &rb);
    glBindRenderbuffer(GL_RENDERBUFFER, rb);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 1, 1);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                              GL_RENDERBUFFER, rb);
    EXPECT_EQ(glGetError(), GL_NO_ERROR);
    { ScopedGLObject renderbuffer(ScopedGLObject::kRenderbuffer, rb); }
    GLint renderbufferName = INT_MAX;
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                          GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,
                                          &renderbufferName);
    EXPECT_EQ(renderbufferName, 0);
  }
  {
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 buffer_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           texture, 0);
    EXPECT_EQ(glGetError(), 0);
    { ScopedGLObject renderbuffer(ScopedGLObject::kTexture, texture); }
    GLint textureName = INT_MAX;
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                          GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,
                                          &textureName);
    EXPECT_EQ(textureName, 0);
  }
  {
    GLuint vao_object;
    glGenVertexArrays(1, &vao_object);
    { auto vao = ScopedGLObject(ScopedGLObject::kVertexArray, vao_object); }
    glBindVertexArray(vao_object);
    EXPECT_EQ(glGetError(), GL_INVALID_OPERATION);
  }
  {
    GLuint program = glCreateProgram();
    { auto p = ScopedGLObject(ScopedGLObject::kProgram, program); }
    glUseProgram(program);
    EXPECT_EQ(glGetError(), GL_INVALID_VALUE);
  }
  {
    GLuint vbo_object;
    glGenBuffers(1, &vbo_object);
    std::array<float, 16> vertex{
        -1.f, -1.f, 0.f, 0.f,  // v1
        1.f,  -1.f, 1.f, 0.f,  // v2
        -1.f, 1.f,  0.f, 1.f,  // v3
        1.f,  1.f,  1.f, 1.f,  // v4
    };
    glBindBuffer(GL_ARRAY_BUFFER, vbo_object);
    glBufferData(GL_ARRAY_BUFFER, vertex.size() * sizeof(float), vertex.data(),
                 GL_STATIC_DRAW);
    { auto b = ScopedGLObject(ScopedGLObject::kBuffer, vbo_object); }
    GLint binding_buffer = INT_MAX;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &binding_buffer);
    EXPECT_EQ(binding_buffer, 0);
  }
  glDeleteFramebuffers(1, &fb);
}
