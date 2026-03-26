// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "testing/gl_test_util_android.h"

#include "src/base/gl/gl_include.h"
#include "src/base/gl/gl_util.h"

namespace lynx {
namespace animax {
namespace testing {

std::vector<RGBAPixel> ReadPixels(int width, int height) {
  std::vector<RGBAPixel> pixels(width * height);
  auto* buffer = std::malloc(width * height * 4);
  auto* uint8_buffer = static_cast<uint8_t*>(buffer);
  glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      int offset = (y * width + x) * 4;
      pixels[y * width + x] =
          RGBAPixel{uint8_buffer[offset], uint8_buffer[offset + 1],
                    uint8_buffer[offset + 2], uint8_buffer[offset + 3]};
    }
  }
  std::free(buffer);
  return pixels;
}

void RenderTriangle() {
  // Vertex Shader
  const char* vertexShaderSource = R"glsl(#version 300 es
    layout(location = 0) in vec3 aPos;
    void main() {
      gl_Position = vec4(aPos.x,aPos.y,aPos.z, 1.0);
    }
  )glsl";

  // Fragment Shader
  const char* fragmentShaderSource = R"glsl(#version 300 es
    precision mediump float;
    out vec4 FragColor;
    void main() {
      FragColor = vec4(0.0, 0.0, 1.0, 1.0);
    }
  )glsl";

  auto shaderProgram =
      lynx::animax::CreateProgram(vertexShaderSource, fragmentShaderSource);

  // Triangle vertices to cover the left half of the screen
  float vertices[] = {-0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f, 0.0f, 0.5f, 0.0f};

  GLuint VBO;
  glGenBuffers(1, &VBO);
  GLuint VAO;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // Set vertex attribute pointers
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
  glEnableVertexAttribArray(0);

  glUseProgram(shaderProgram);
  glBindVertexArray(VAO);
  glDrawArrays(GL_TRIANGLES, 0, 3);

  // Clean up
  glDeleteBuffers(1, &VBO);
  glDeleteVertexArrays(1, &VAO);
}

}  // namespace testing
}  // namespace animax
}  // namespace lynx
