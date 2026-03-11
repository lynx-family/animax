#include "src/base/gl/gl_util.h"

#include <GL/osmesa.h>

#include "gtest/gtest.h"

class GLUtilTestWithContext : public ::testing::Test {
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

class GLUtilTestWithContextLargeBuffer : public ::testing::Test {
 protected:
  OSMesaContext context_;
  void* buffer_;

  void SetUp() override {
    context_ = OSMesaCreateContextExt(OSMESA_RGBA, 0, 0, 0, nullptr);
    buffer_ = std::malloc(16 * 16 * 4);
    OSMesaMakeCurrent(context_, buffer_, GL_UNSIGNED_BYTE, 16, 16);
  }

  void TearDown() override {
    OSMesaDestroyContext(context_);
    std::free(buffer_);
  }
};

TEST(SplitAndSortExtensionsTest, EmptyString) {
  std::string extensions = "";
  auto result = lynx::animax::SplitAndSortExtensions(extensions);
  EXPECT_TRUE(result.empty());
}

TEST(SplitAndSortExtensionsTest, SingleExtension) {
  std::string extensions = "GL_OES_vertex_array_object";
  auto result = lynx::animax::SplitAndSortExtensions(extensions);
  EXPECT_EQ(result.size(), 1);
  EXPECT_EQ(result[0], "GL_OES_vertex_array_object");
}

TEST(SplitAndSortExtensionsTest, MultipleExtensions) {
  std::string extensions =
      "GL_OES_vertex_array_object GL_EXT_texture_filter_anisotropic";
  auto result = lynx::animax::SplitAndSortExtensions(extensions);
  EXPECT_EQ(result.size(), 2);
  EXPECT_EQ(result[0], "GL_EXT_texture_filter_anisotropic");
  EXPECT_EQ(result[1], "GL_OES_vertex_array_object");
}

TEST(IsExtensionSupported, ExtensionExist) {
  using namespace lynx::animax;
  auto extensions = SplitAndSortExtensions(
      "GL_OES_vertex_array_object GL_EXT_texture_filter_anisotropic");
  EXPECT_TRUE(IsExtensionSupported(extensions, "GL_OES_vertex_array_object"));
}

TEST(IsExtensionSupported, ExtensionNotExist) {
  using namespace lynx::animax;
  auto extensions = SplitAndSortExtensions(
      "GL_OES_vertex_array_object GL_EXT_texture_filter_anisotropic");
  EXPECT_FALSE(IsExtensionSupported(extensions,
                                    "NONONONONO_GL_OES_vertex_array_object"));
}

TEST(IsExtensionSupported, EmptyExtension) {
  using namespace lynx::animax;
  auto extensions = SplitAndSortExtensions(
      "GL_OES_vertex_array_object GL_EXT_texture_filter_anisotropic");
  EXPECT_FALSE(IsExtensionSupported(extensions, ""));
}

TEST_F(GLUtilTestWithContext, IsGLExtensionSupported) {
  auto extensions = lynx::animax::GetGLExtensions();
  EXPECT_TRUE(!extensions.empty());
  auto support = lynx::animax::IsGLExtensionSupported("GL_MESA_window_pos");
  EXPECT_TRUE(support);
  support = lynx::animax::IsGLExtensionSupported(
      "SOME_EXTENSION_THAT_WILL_NEVER_EXIST_TRUST_ME");
  EXPECT_FALSE(support);
}

TEST_F(GLUtilTestWithContext, GetProcLoader) {
  EXPECT_TRUE(lynx::animax::GetGLProcLoader() != nullptr);
}

TEST_F(GLUtilTestWithContext, CreateProgramFailedShouldReturn0) {
  auto program = lynx::animax::CreateProgram("junk", "garbage");
  EXPECT_EQ(program, 0);
}

TEST_F(GLUtilTestWithContext, FXAAProgram) {
  auto program = lynx::animax::CreateFXAAProgram();
  EXPECT_NE(program, 0);
  EXPECT_EQ(glGetError(), 0);
  glDeleteProgram(program);
  EXPECT_EQ(glGetError(), 0);
}

TEST_F(GLUtilTestWithContext,
       CreateTexImage2D_ValidDimensions_ReturnsValidTexture) {
  using namespace lynx::animax;

  const GLsizei width = 1;
  const GLsizei height = 1;

  auto tex = CreateTexImage2D(width, height);

  // Test if texture is valid.
  EXPECT_TRUE(glIsTexture(tex));

  // Test the parameters and dimensions.
  glBindTexture(GL_TEXTURE_2D, tex);
  GLint texWidth, texHeight, wrapS, wrapT, minFilter, magFilter;
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &texWidth);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &texHeight);
  glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, &wrapS);
  glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, &wrapT);
  glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &minFilter);
  glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, &magFilter);

  EXPECT_EQ(width, texWidth);
  EXPECT_EQ(height, texHeight);
  EXPECT_EQ(GL_CLAMP_TO_EDGE, wrapS);
  EXPECT_EQ(GL_CLAMP_TO_EDGE, wrapT);
  EXPECT_EQ(GL_LINEAR, minFilter);
  EXPECT_EQ(GL_LINEAR, magFilter);
}

TEST_F(GLUtilTestWithContext,
       CreateStencilBuffer_ValidDimensions_ReturnsValidStencilBuffer) {
  using namespace lynx::animax;

  const GLsizei width = 1;
  const GLsizei height = 1;

  auto stencil_buffer = CreateStencilBuffer(width, height);

  // Test if renderbuffer is valid.
  EXPECT_TRUE(glIsRenderbuffer(stencil_buffer));

  // Test the parameters and dimensions.
  glBindRenderbuffer(GL_RENDERBUFFER, stencil_buffer);
  GLint rbWidth, rbHeight, rbFormat;
  glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH,
                               &rbWidth);
  glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT,
                               &rbHeight);
  glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_INTERNAL_FORMAT,
                               &rbFormat);

  EXPECT_EQ(width, rbWidth);
  EXPECT_EQ(height, rbHeight);
  EXPECT_EQ(GL_STENCIL_INDEX8, rbFormat);
}

TEST_F(GLUtilTestWithContext,
       CreateTexImage2D_InvalidDimensions_ReturnsInvalidTexture) {
  using namespace lynx::animax;

  std::vector<GLsizei> invalidDimensions = {-1, 0};
  for (auto dim : invalidDimensions) {
    auto tex = CreateTexImage2D(dim, dim);
    EXPECT_EQ(tex.Type(), ScopedGLObject::Type::kInvalid);
  }
}

TEST_F(GLUtilTestWithContext,
       CreateStencilBuffer_InvalidDimensions_ReturnsInvalidStencilBuffer) {
  using namespace lynx::animax;

  std::vector<GLsizei> invalidDimensions = {-1, 0};
  for (auto dim : invalidDimensions) {
    auto stencil_buffer = CreateStencilBuffer(dim, dim);
    EXPECT_EQ(stencil_buffer.Type(), ScopedGLObject::Type::kInvalid);
  }
}

TEST_F(GLUtilTestWithContext,
       CreateFramebuffer_ValidAttachments_ReturnsValidFramebuffer) {
  using namespace lynx::animax;

  GLsizei width = 1;
  GLsizei height = 1;

  // Create valid attachments
  auto color_texture = CreateTexImage2D(width, height);
  auto stencil_buffer = CreateStencilBuffer(width, height);

  auto framebuffer = CreateFramebuffer(color_texture, stencil_buffer);

  // Test if framebuffer is valid.
  EXPECT_TRUE(glIsFramebuffer(framebuffer));

  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  EXPECT_EQ(glCheckFramebufferStatus(GL_FRAMEBUFFER), GL_FRAMEBUFFER_COMPLETE);
}

TEST_F(GLUtilTestWithContext,
       CreateFramebuffer_InvalidColorAttachment_ReturnsInvalidFramebuffer) {
  using namespace lynx::animax;

  // Create an invalid color attachment
  ScopedGLObject invalidColorAttachment(ScopedGLObject::Type::kInvalid, 0);
  auto stencil_buffer = CreateStencilBuffer(1, 1);
  auto framebuffer = CreateFramebuffer(invalidColorAttachment, stencil_buffer);

  EXPECT_EQ(framebuffer.Type(), ScopedGLObject::Type::kInvalid);
}

TEST_F(GLUtilTestWithContext,
       CreateFramebuffer_NoStencilAttachment_DoesNotAttachStencil) {
  using namespace lynx::animax;

  auto color_texture = CreateTexImage2D(1, 1);

  auto framebuffer = CreateFramebuffer(color_texture, 0);

  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  GLint stencilAttachment;
  glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                                        GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,
                                        &stencilAttachment);
  EXPECT_EQ(stencilAttachment, 0);
}

TEST_F(GLUtilTestWithContext,
       AttachStencilBufferToFramebuffer_ValidInputs_AttachesStencilBuffer) {
  using namespace lynx::animax;

  GLsizei width = 1;
  GLsizei height = 1;

  // Create a framebuffer and stencil buffer
  auto color_texture = CreateTexImage2D(width, height);
  GLuint framebuffer_object;
  glGenFramebuffers(1, &framebuffer_object);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_object);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         color_texture, 0);

  auto stencil_buffer = CreateStencilBuffer(width, height);

  // Attach stencil buffer to the framebuffer
  AttachStencilBufferToFramebuffer(framebuffer_object, stencil_buffer);

  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_object);
  GLint stencilAttachment;
  glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                                        GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,
                                        &stencilAttachment);

  EXPECT_EQ(stencilAttachment, stencil_buffer);
  EXPECT_EQ(glCheckFramebufferStatus(GL_FRAMEBUFFER), GL_FRAMEBUFFER_COMPLETE);
}

TEST_F(GLUtilTestWithContextLargeBuffer,
       RenderFullScreenQuad_UsingVBOandVAO_FillsScreen) {
  using namespace lynx::animax;

  // 1. Create VBO and VAO
  auto vao_vbo = CreateFullScreenQuadVAOVBOPair();
  auto& vao = vao_vbo.vao_;
  auto& vbo = vao_vbo.vbo_;

  // 2. Create shader program
  const char* vertexShaderSource = R"(
        #version 330 core
        layout(location = 0) in vec2 position;
        layout(location = 1) in vec2 texcoord;
        out vec2 Texcoord;
        void main() {
            gl_Position = vec4(position, 0.0, 1.0);
            Texcoord = texcoord;
        }
    )";

  const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        in vec2 Texcoord;
        void main() {
            FragColor = vec4(1.0); // white color for full-screen quad
        }
    )";

  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
  glCompileShader(vertexShader);

  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
  glCompileShader(fragmentShader);

  GLuint shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  glUseProgram(shaderProgram);

  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT);

  unsigned char pixel[4];
  glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
  EXPECT_EQ(pixel[0], 0);    // Red
  EXPECT_EQ(pixel[1], 0);    // Green
  EXPECT_EQ(pixel[2], 0);    // Blue
  EXPECT_EQ(pixel[3], 255);  // Alpha

  // 3. Render the quad
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  // 4. Check the result
  glReadPixels(8, 8, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);

  EXPECT_EQ(pixel[0], 255);  // Red
  EXPECT_EQ(pixel[1], 255);  // Green
  EXPECT_EQ(pixel[2], 255);  // Blue
  EXPECT_EQ(pixel[3], 255);  // Alpha

  // Cleanup
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
  glDeleteProgram(shaderProgram);
}

TEST_F(GLUtilTestWithContext, CreateTexImage2D_RestoresPreviousBinding) {
  using namespace lynx::animax;

  GLuint initialTexture;
  glGenTextures(1, &initialTexture);
  glBindTexture(GL_TEXTURE_2D, initialTexture);

  CreateTexImage2D(1, 1);

  GLint currentBinding;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &currentBinding);
  EXPECT_EQ(currentBinding, initialTexture);

  glDeleteTextures(1, &initialTexture);
}

TEST_F(GLUtilTestWithContext, CreateStencilBuffer_RestoresPreviousBinding) {
  using namespace lynx::animax;

  GLuint initialRenderBuffer;
  glGenRenderbuffers(1, &initialRenderBuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, initialRenderBuffer);

  CreateStencilBuffer(1, 1);

  GLint currentBinding;
  glGetIntegerv(GL_RENDERBUFFER_BINDING, &currentBinding);
  EXPECT_EQ(currentBinding, initialRenderBuffer);

  glDeleteRenderbuffers(1, &initialRenderBuffer);
}

TEST_F(GLUtilTestWithContext, CreateFramebuffer_RestoresPreviousBinding) {
  using namespace lynx::animax;

  GLuint initialFramebuffer;
  glGenFramebuffers(1, &initialFramebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, initialFramebuffer);

  auto color_texture = CreateTexImage2D(1, 1);
  CreateFramebuffer(color_texture, 0);

  GLint currentBinding;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentBinding);
  EXPECT_EQ(currentBinding, initialFramebuffer);

  glDeleteFramebuffers(1, &initialFramebuffer);
}

TEST_F(GLUtilTestWithContext,
       AttachStencilBufferToFramebuffer_RestoresPreviousBinding) {
  using namespace lynx::animax;

  GLuint initialFramebuffer;
  glGenFramebuffers(1, &initialFramebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, initialFramebuffer);

  GLuint framebuffer_object;
  glGenFramebuffers(1, &framebuffer_object);
  AttachStencilBufferToFramebuffer(framebuffer_object, 0);

  GLint currentBinding;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentBinding);
  EXPECT_EQ(currentBinding, initialFramebuffer);

  glDeleteFramebuffers(1, &initialFramebuffer);
  glDeleteFramebuffers(1, &framebuffer_object);
}

TEST_F(GLUtilTestWithContext,
       CreateFullScreenQuadVAOVBO_RestoresPreviousBinding) {
  using namespace lynx::animax;

  GLuint initialBuffer;
  glGenBuffers(1, &initialBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, initialBuffer);

  GLuint initialVAO;
  glGenVertexArrays(1, &initialVAO);
  glBindVertexArray(initialVAO);

  auto vao_vbo = CreateFullScreenQuadVAOVBOPair();

  GLint currentBinding;
  glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &currentBinding);
  EXPECT_EQ(currentBinding, initialBuffer);

  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &currentBinding);
  EXPECT_EQ(currentBinding, initialVAO);

  glDeleteBuffers(1, &initialBuffer);
  glDeleteVertexArrays(1, &initialVAO);
}
