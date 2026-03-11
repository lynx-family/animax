#include "gtest/gtest.h"
#include "src/base/gl/gl_util.h"

class GLUtilTestWithEGLContext : public ::testing::Test {
 protected:
  EGLDisplay display_;
  EGLConfig config_;
  EGLContext context_;
  EGLSurface surface_;

  void SetUp() override {
    display_ = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    ASSERT_NE(display_, EGL_NO_DISPLAY);
    ASSERT_TRUE(eglInitialize(display_, nullptr, nullptr));
    GLint attrib_list[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};

    const auto ChooseEGLConfig = [](EGLDisplay display,
                                    bool stencil = true) -> EGLConfig {
      EGLint attributes[] = {
          // clang-format off
          EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
          EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
          EGL_RED_SIZE,        8,
          EGL_GREEN_SIZE,      8,
          EGL_BLUE_SIZE,       8,
          EGL_ALPHA_SIZE,      8,
          EGL_DEPTH_SIZE,      0,
          EGL_STENCIL_SIZE,    stencil ? 8 : 0,
          EGL_NONE,            // termination sentinel
          // clang-format on
      };

      EGLint config_count = 0;
      EGLConfig config = nullptr;

      if (!eglChooseConfig(display, attributes, &config, 1, &config_count)) {
        return nullptr;
      }

      if (config == nullptr || config_count == 0) {
        return nullptr;
      }

      // eglChooseConfig return ture && config_count > 0 && config != nullptr
      return config;
    };

    config_ = ChooseEGLConfig(display_);
    context_ = eglCreateContext(display_, config_, nullptr, attrib_list);
    ASSERT_NE(context_, EGL_NO_CONTEXT);
    const EGLint attrib_list_surface[] = {EGL_WIDTH, 1, EGL_HEIGHT, 1,
                                          EGL_NONE};
    surface_ = eglCreatePbufferSurface(display_, config_, attrib_list_surface);
    ASSERT_NE(surface_, EGL_NO_SURFACE);
    eglMakeCurrent(display_, surface_, surface_, context_);
  }

  void TearDown() override {
    eglDestroySurface(display_, surface_);
    eglDestroyContext(display_, context_);
  }
};

TEST_F(GLUtilTestWithEGLContext, GLExtensions) {
  using namespace lynx::animax;
  const auto gl_extension = GetGLExtensions();
  ASSERT_FALSE(gl_extension.empty());
  for (const auto& extension : gl_extension) {
    ASSERT_TRUE(IsGLExtensionSupported(extension));
  }
  ASSERT_FALSE(IsGLExtensionSupported(
      "GL_KHR_just_some_extension_that_will_never_be_supported"));
}

TEST_F(GLUtilTestWithEGLContext, TiledMSAANotSupportedShouldCreateInvalid) {
  using namespace lynx::animax;
  if (!IsTiledMSAASupported()) {
    const auto stencil_buffer = CreateMultisampleEXTStencilBuffer(1, 1);
    ASSERT_EQ(stencil_buffer.Type(), ScopedGLObject::kInvalid);
    const auto texture = CreateTexImage2D(1, 1);
    ASSERT_EQ(texture.Type(), ScopedGLObject::kTexture);
    ASSERT_NE(static_cast<GLuint>(texture), 0);
    const auto framebuffer =
        CreateMultisampleEXTFramebuffer(texture, stencil_buffer);
    ASSERT_EQ(framebuffer.Type(), ScopedGLObject::kInvalid);
  }
}

TEST_F(GLUtilTestWithEGLContext, TiledMSAASupportedShouldCreateValid) {
  using namespace lynx::animax;
  if (IsTiledMSAASupported()) {
    const auto stencil_buffer = CreateMultisampleEXTStencilBuffer(1, 1);
    ASSERT_EQ(stencil_buffer.Type(), ScopedGLObject::kRenderbuffer);
    ASSERT_NE(static_cast<GLuint>(stencil_buffer), 0);
    const auto texture = CreateTexImage2D(1, 1);
    ASSERT_EQ(texture.Type(), ScopedGLObject::kTexture);
    ASSERT_NE(static_cast<GLuint>(texture), 0);
    const auto framebuffer =
        CreateMultisampleEXTFramebuffer(texture, stencil_buffer);
    ASSERT_EQ(framebuffer.Type(), ScopedGLObject::kFramebuffer);
    ASSERT_NE(static_cast<GLuint>(framebuffer), 0);
  }
}
