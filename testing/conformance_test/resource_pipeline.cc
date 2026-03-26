// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "testing/conformance_test/resource_pipeline.h"

#include <condition_variable>
#include <fstream>
#include <iostream>
#include <mutex>

#include "base/include/fml/thread.h"
#include "base/include/no_destructor.h"
#include "src/base/thread/task_runner.h"
#include "src/resource/loader/lambda_loader.h"
#include "src/resource/resource_loader_listener.h"
#include "third_party/libpng/png.h"

namespace lynx {
namespace animax {

namespace {

struct ReadPNGData {
  std::vector<unsigned char> image_data;
  int width = 0;
  int height = 0;
  std::string error_message;
};

struct ReadPNGResult {
  std::unique_ptr<Bitmap> bitmap;
  std::string error_message;
};

struct PNGReadContext {
  std::string error_message;
};

// Error handling for libpng
void PNGError(png_structp png_ptr, png_const_charp msg) {
  PNGReadContext* context =
      reinterpret_cast<PNGReadContext*>(png_get_error_ptr(png_ptr));
  if (context) {
    context->error_message = "libpng error: ";
    context->error_message += msg;
  }
  longjmp(png_jmpbuf(png_ptr), 1);
}

void PNGWarning(png_structp png_ptr, png_const_charp msg) {}

ReadPNGData PNGRead(const char* filename) {
  ReadPNGData result;

  FILE* file = fopen(filename, "rb");
  if (!file) {
    result.error_message = "File could not be opened for reading";
    return result;
  }

  PNGReadContext read_context;
  png_structp png_ptr = png_create_read_struct(
      PNG_LIBPNG_VER_STRING, &read_context, PNGError, PNGWarning);
  if (!png_ptr) {
    fclose(file);
    result.error_message = "Failed to create PNG read struct";
    return result;
  }

  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    png_destroy_read_struct(&png_ptr, nullptr, nullptr);
    fclose(file);
    result.error_message = "Failed to create PNG info struct";
    return result;
  }

  if (setjmp(png_jmpbuf(png_ptr))) {
    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
    fclose(file);
    result.error_message = read_context.error_message;
    return result;
  }

  png_init_io(png_ptr, file);
  png_read_info(png_ptr, info_ptr);

  result.width = png_get_image_width(png_ptr, info_ptr);
  result.height = png_get_image_height(png_ptr, info_ptr);
  png_byte color_type = png_get_color_type(png_ptr, info_ptr);
  png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);

  // Apply transformations
  if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png_ptr);
  if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
    png_set_expand_gray_1_2_4_to_8(png_ptr);
  if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
    png_set_tRNS_to_alpha(png_ptr);

  png_read_update_info(png_ptr, info_ptr);

  // Read image data
  result.image_data.resize(result.width * result.height * 4);
  std::vector<png_bytep> row_pointers(result.height);

  for (int y = 0; y < result.height; y++) {
    row_pointers[y] = &result.image_data[y * result.width * 4];
  }

  png_read_image(png_ptr, row_pointers.data());

  png_read_end(png_ptr, nullptr);
  png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
  fclose(file);

  return result;
}

ReadPNGResult ReadPNGToBitmap(const std::string& file_path) {
  ReadPNGResult result;

  ReadPNGData png_data = PNGRead(file_path.c_str());
  if (!png_data.error_message.empty()) {
    result.error_message = png_data.error_message;
    return result;
  }

  auto* image_data =
      new std::vector<unsigned char>(std::move(png_data.image_data));

  // Premultiply alpha
  unsigned char* pixel = image_data->data();
  for (size_t i = 0; i < png_data.width * png_data.height; ++i, pixel += 4) {
    float alpha = pixel[3] / 255.0f;
    pixel[0] = static_cast<unsigned char>(pixel[0] * alpha);
    pixel[1] = static_cast<unsigned char>(pixel[1] * alpha);
    pixel[2] = static_cast<unsigned char>(pixel[2] * alpha);
    // pixel[3] remains the same (alpha channel)
  }

  result.bitmap = Bitmap::MakeRGBA(
      png_data.width, png_data.height, image_data->data(),
      [](const void* data) {
        delete reinterpret_cast<const std::vector<unsigned char>*>(data);
      },
      image_data);

  return result;
}

}  // namespace

Loader<ResourceRequest, ResourceResponse>::Ptr GetSharedResourcePipeline() {
  static Loader<ResourceRequest, ResourceResponse>::Ptr shared_pipeline =
      MakeLambdaLoader<ResourceRequest, ResourceResponse>([](ResourceRequest
                                                                 request,
                                                             auto callback) {
        if (request.uri_info.scheme != UriInfo::Scheme::kFile) {
          callback({}, LoaderError{.code = kInvalidLoader,
                                   "Only support loading file."});
          return;
        }

        const auto file_path = request.uri_info.uri;
        if (request.type == ResourceRequestType::kLoadBitmap) {
          auto [bitmap, error_msg] = ReadPNGToBitmap(file_path);
          if (!error_msg.empty()) {
            callback({}, LoaderError{kUserDefined, error_msg});
          } else {
            callback(ResourceResponse{.payload = MakeBitmapResourcePayload(
                                          std::move(bitmap))},
                     {});
          }
          return;
        }

        if (request.type == ResourceRequestType::kLoadRawData) {
          std::ifstream file(file_path, std::ios::binary);
          if (!file.is_open()) {
            callback({}, LoaderError{kUserDefined,
                                     "Failed to open file: " + file_path});
            return;
          }
          file.seekg(0, std::ios::end);
          auto file_size = file.tellg();
          if (file_size <= 0) {
            callback({}, LoaderError{
                             kUserDefined,
                             "Failed to get file size of file: " + file_path});
            return;
          }
          file.seekg(0, std::ios::beg);
          auto buffer = std::make_unique<char[]>(file_size);
          file.read(buffer.get(), file_size);

          auto raw_data = RawData::MakeRawData(
              buffer.get(), file_size,
              [](const void* buffer) {
                delete[] reinterpret_cast<const char*>(buffer);
              },
              buffer.release());

          callback(ResourceResponse{.payload = MakeRawDataResourcePayload(
                                        std::move(raw_data))},
                   {});
          return;
        }

        callback({}, LoaderError{kUserDefined, "Unsupported resource type."});
      });
  return shared_pipeline;
}

std::shared_ptr<CompositionModelPipeline> GetSharedCompoistionPipeline() {
  static std::shared_ptr<CompositionModelPipeline> composition_model_pipeline =
      []() {
        auto composition_model_pipeline =
            std::make_shared<CompositionModelPipeline>();
        composition_model_pipeline->Init(GetSharedResourcePipeline(), nullptr);
        return composition_model_pipeline;
      }();
  return composition_model_pipeline;
}

}  // namespace animax
}  // namespace lynx
