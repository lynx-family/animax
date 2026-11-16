// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/resource_loader/harmony/resource_loader_harmony.h"

#include <filemanagement/file_uri/oh_file_uri.h>
#include <multimedia/image_framework/image/image_common.h>
#include <multimedia/image_framework/image/image_source_native.h>

#include <fstream>

#include "base/include/string/string_utils.h"
#include "src/base/log/log.h"
#include "src/base/util/harmony/scoped_object_harmony.h"
#include "src/resource/loader/exec_loader.h"
#include "src/resource/loader/lambda_loader.h"
#include "src/resource/loader/pipe_loader.h"
#include "src/resource/uri/uri_util.h"

namespace lynx {
namespace animax {

namespace {
void ReadFileToMemory(const char* file_path,
                      std::unique_ptr<uint8_t[]>& data_ptr, size_t* data_size) {
  auto local_path = GetUriLocalPath(file_path);
  std::ifstream file(local_path, std::ios::binary | std::ios::ate);
  if (!file.is_open()) {
    ANIMAX_LOGE("Failed to open file: " << local_path);
    return;
  }

  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  data_ptr = std::unique_ptr<uint8_t[]>(new uint8_t[size]);
  if (!data_ptr) {
    ANIMAX_LOGE("Failed to allocate memory.");
    file.close();
    return;
  }

  if (!file.read(reinterpret_cast<char*>(data_ptr.get()), size)) {
    ANIMAX_LOGE("Failed to read file.");
    data_ptr.reset();
    file.close();
    return;
  }

  *data_size = static_cast<size_t>(size);
  file.close();
}

std::unique_ptr<Bitmap> LoadBitmapFromData(uint8_t* data, size_t size) {
  if (!data || size == 0) {
    ANIMAX_LOGE("LoadBitmapFromData fail, invalid input parameters");
    return nullptr;
  }

  ScopedOHObject<OH_ImageSourceNative, OH_ImageSourceNative_Release>
      source_native;
  Image_ErrorCode error_code = OH_ImageSourceNative_CreateFromData(
      data, size, source_native.WritablePtr());
  if (error_code != Image_ErrorCode::IMAGE_SUCCESS) {
    ANIMAX_LOGE("Failed to create from data, error code:"
                << static_cast<uint32_t>(error_code));
    return nullptr;
  }

  ScopedOHObject<OH_DecodingOptions, OH_DecodingOptions_Release> decode_options;
  if (OH_DecodingOptions_Create(decode_options.WritablePtr()) !=
      IMAGE_SUCCESS) {
    ANIMAX_LOGE("Failed to create decoding options");
    return nullptr;
  }

  ScopedOHObject<OH_PixelmapNative, OH_PixelmapNative_Release> pixel_map;
  error_code = OH_ImageSourceNative_CreatePixelmap(
      source_native.Ptr(), decode_options.Ptr(), pixel_map.WritablePtr());
  if (error_code != IMAGE_SUCCESS) {
    ANIMAX_LOGE("Failed to create pixelmap, error code:"
                << static_cast<uint32_t>(error_code));
    return nullptr;
  }

  ScopedOHObject<OH_Pixelmap_ImageInfo, OH_PixelmapImageInfo_Release>
      image_info;
  if (OH_PixelmapImageInfo_Create(image_info.WritablePtr()) != IMAGE_SUCCESS) {
    ANIMAX_LOGE("Failed to create image info");
    return nullptr;
  }

  if (OH_PixelmapNative_GetImageInfo(pixel_map.Ptr(), image_info.Ptr()) !=
      IMAGE_SUCCESS) {
    ANIMAX_LOGE("Failed to get image info");
    return nullptr;
  }

  uint32_t width, height;
  OH_PixelmapImageInfo_GetWidth(image_info.Ptr(), &width);
  OH_PixelmapImageInfo_GetHeight(image_info.Ptr(), &height);

  size_t buffer_size = width * height * 4;
  std::unique_ptr<uint8_t[]> pixels(new uint8_t[buffer_size]);
  if (OH_PixelmapNative_ReadPixels(pixel_map.Ptr(), pixels.get(),
                                   &buffer_size) != IMAGE_SUCCESS) {
    ANIMAX_LOGE("Failed to read pixels");
    return nullptr;
  }

  return Bitmap::MakeRGBA(
      width, height, pixels.get(),
      [](const void* p) { delete[] static_cast<const uint8_t*>(p); },
      pixels.release());
}
}  // namespace

ResourceLoaderHarmony::ResourceLoaderHarmony(
    std::shared_ptr<pub::LynxResourceLoader> loader) {
  platform_loader_ = loader;

  raw_data_http_loader_ = CreateRawDataHttpLoader();
  raw_data_file_loader_ = CreateRawDataFileLoader();
  bitmap_http_loader_ = CreateBitmapHttpLoader();
  bitmap_file_loader_ = CreateBitmapFileLoader();
  bitmap_base64_loader_ = CreateBitmapBase64Loader();
  download_to_local_http_loader_ = CreateDownloadToLocalHttpLoader();
}

Loader<ResourceRequest, ResourceResponse>::Ptr
ResourceLoaderHarmony::CreateRawDataFileLoader() {
  auto load_raw_data = MakeLambdaLoader<ResourceRequest, ResourceResponse>(
      [](ResourceRequest request, auto callback) {
        std::unique_ptr<uint8_t[]> data_ptr;
        size_t data_size = 0;
        ReadFileToMemory(request.uri_info.uri.c_str(), data_ptr, &data_size);

        if (data_ptr == nullptr || data_size == 0) {
          callback(ResourceResponse{},
                   LoaderError{.code = kFileIOFailure,
                               .message = "ReadFileToMemory failed."});
        } else {
          auto raw_data = RawData::MakeRawData(
              data_ptr.get(), data_size,
              [](const void* data_ptr) {
                delete[] static_cast<const uint8_t*>(data_ptr);
              },
              data_ptr.release());
          callback(ResourceResponse{.payload = MakeRawDataResourcePayload(
                                        std::move(raw_data))},
                   LoaderError{});
        }
      });
  return ResourceThreadExecLoader(load_raw_data);
}

Loader<ResourceRequest, ResourceResponse>::Ptr
ResourceLoaderHarmony::CreateRawDataHttpLoader() {
  auto load_raw_data = MakeLambdaLoader<ResourceRequest, ResourceResponse>(
      [platform_loader = platform_loader_](ResourceRequest request,
                                           auto callback) {
        auto shared_platform_loader = platform_loader.lock();
        if (shared_platform_loader == nullptr) {
          callback(ResourceResponse{},
                   LoaderError{.code = kReferToMessage,
                               .message = "Platform loader is null."});
          return;
        }

        shared_platform_loader->LoadResource(
            lynx::pub::LynxResourceRequest{
                request.uri_info.uri, lynx::pub::LynxResourceType::kLottie},
            [callback = std::move(callback)](
                pub::LynxResourceResponse& response) mutable {
              if (!response.Success()) {
                callback(
                    ResourceResponse{},
                    LoaderError{
                        .code = kLoaderNotHandleRequest,
                        .message =
                            "Load raw data failed, request is not success."});
                return;
              }

              auto data_size = response.data.size();
              if (data_size == 0) {
                callback(
                    ResourceResponse{},
                    LoaderError{
                        .code = kFileIOFailure,
                        .message = "Load raw data failed, data is empty."});
                return;
              }

              std::unique_ptr<uint8_t[]> data_ptr(new uint8_t[data_size]);
              if (!data_ptr) {
                callback(
                    ResourceResponse{},
                    LoaderError{
                        .code = kFileIOFailure,
                        .message =
                            "Load raw data failed, memory allocation failed."});
                return;
              }

              std::memcpy(data_ptr.get(), response.data.data(), data_size);

              auto raw_data = RawData::MakeRawData(
                  data_ptr.get(), data_size,
                  [](const void* data_ptr) {
                    delete[] static_cast<const uint8_t*>(data_ptr);
                  },
                  data_ptr.release());

              callback(ResourceResponse{.payload = MakeRawDataResourcePayload(
                                            std::move(raw_data))},
                       LoaderError{});
            });
      });
  return ResourceThreadExecLoader(load_raw_data);
}

Loader<ResourceRequest, ResourceResponse>::Ptr
ResourceLoaderHarmony::CreateBitmapHttpLoader() {
  // Step 1: Fetch bitmap data from platform.
  auto fetch_data = MakeLambdaLoader<ResourceRequest, std::vector<uint8_t>>(
      [platform_loader = platform_loader_](ResourceRequest request,
                                           auto callback) {
        auto& uri = request.uri_info.uri;
        auto shared_platform_loader = platform_loader.lock();
        if (shared_platform_loader == nullptr) {
          callback(std::vector<uint8_t>{},
                   LoaderError{.code = kReferToMessage,
                               .message = "Platform loader is null."});
          return;
        }

        shared_platform_loader->LoadResource(
            lynx::pub::LynxResourceRequest{
                uri, lynx::pub::LynxResourceType::kLottie},
            [callback = std::move(callback)](
                pub::LynxResourceResponse& response) mutable {
              if (!response.Success()) {
                callback(std::vector<uint8_t>{},
                         LoaderError{.code = kLoaderNotHandleRequest,
                                     .message = "Load bitmap failed."});
                return;
              }
              callback(std::vector<uint8_t>(response.data.begin(),
                                            response.data.end()),
                       LoaderError{});
            });
      });

  // Step 2: Decode bitmap from data. This runs on resource thread.
  auto decode_bitmap = MakeLambdaLoader<std::vector<uint8_t>, ResourceResponse>(
      [](std::vector<uint8_t> data, auto callback) {
        auto bitmap = LoadBitmapFromData(data.data(), data.size());
        if (bitmap) {
          callback(ResourceResponse{.payload = MakeBitmapResourcePayload(
                                        std::move(bitmap))},
                   LoaderError{});
        } else {
          callback(ResourceResponse{},
                   LoaderError{.code = kInvalidImageData,
                               .message = "Load bitmap from data failed."});
        }
      });

  return ResourceThreadExecLoader(fetch_data) |
         ResourceThreadExecLoader(decode_bitmap);
}

Loader<ResourceRequest, ResourceResponse>::Ptr
ResourceLoaderHarmony::CreateBitmapBase64Loader() {
  auto load_bitmap = MakeLambdaLoader<ResourceRequest, ResourceResponse>(
      [](ResourceRequest request, auto callback) {
        auto& uri = request.uri_info.uri;
        auto bitmap = LoadBitmapFromData(reinterpret_cast<uint8_t*>(uri.data()),
                                         uri.size());
        if (!bitmap) {
          callback(ResourceResponse{},
                   LoaderError{.code = kInvalidImageData,
                               .message = "Load bitmap from base64 failed."});
          return;
        }
        callback(ResourceResponse{.payload = MakeBitmapResourcePayload(
                                      std::move(bitmap))},
                 LoaderError{});
      });
  return ResourceThreadExecLoader(load_bitmap);
}

Loader<ResourceRequest, ResourceResponse>::Ptr
ResourceLoaderHarmony::CreateBitmapFileLoader() {
  auto load_bitmap = MakeLambdaLoader<ResourceRequest, ResourceResponse>(
      [](ResourceRequest request, auto callback) {
        std::unique_ptr<uint8_t[]> data_ptr;
        size_t data_size = 0;
        ReadFileToMemory(request.uri_info.uri.c_str(), data_ptr, &data_size);
        auto bitmap = LoadBitmapFromData(data_ptr.get(), data_size);
        if (!bitmap) {
          callback(
              ResourceResponse{},
              LoaderError{.code = kInvalidImageData,
                          .message = "Load bitmap from local path failed."});
          return;
        }
        callback(ResourceResponse{.payload = MakeBitmapResourcePayload(
                                      std::move(bitmap))},
                 LoaderError{});
      });
  return ResourceThreadExecLoader(load_bitmap);
}

Loader<ResourceRequest, ResourceResponse>::Ptr
ResourceLoaderHarmony::CreateDownloadToLocalHttpLoader() {
  auto download_to_local = MakeLambdaLoader<ResourceRequest, ResourceResponse>(
      [platform_loader = platform_loader_](ResourceRequest request,
                                           auto callback) {
        auto shared_platform_loader = platform_loader.lock();
        if (shared_platform_loader == nullptr) {
          callback(ResourceResponse{},
                   LoaderError{.code = kReferToMessage,
                               .message = "Platform loader is null."});
          return;
        }

        shared_platform_loader->LoadResourcePath(
            lynx::pub::LynxResourceRequest{
                request.uri_info.uri, lynx::pub::LynxResourceType::kLottie},
            [callback =
                 std::move(callback)](pub::LynxPathResponse& response) mutable {
              if (!response.Success()) {
                callback(ResourceResponse{},
                         LoaderError{.code = kLoaderNotHandleRequest,
                                     .message = "Load resource path failed."});
                return;
              }

              callback(ResourceResponse{.payload = MakePathResourcePayload(
                                            response.path)},
                       LoaderError{});
            });
      });
  return ResourceThreadExecLoader(download_to_local);
}

void ResourceLoaderHarmony::Load(ResourceRequest request,
                                 CallbackType callback) {
  switch (request.type) {
    case ResourceRequestType::kLoadRawData: {
      // TODO(aiyongbiao.rick): Construct loader matrix to support file and
      // asset scene.
      if (request.uri_info.scheme == UriInfo::Scheme::kFile) {
        raw_data_file_loader_->Load(std::move(request), std::move(callback));
      } else {
        raw_data_http_loader_->Load(std::move(request), std::move(callback));
      }
      break;
    }
    case ResourceRequestType::kLoadBitmap: {
      if (request.uri_info.scheme == UriInfo::Scheme::kFile) {
        bitmap_file_loader_->Load(std::move(request), std::move(callback));
      } else if (request.uri_info.scheme == UriInfo::Scheme::kDataURL) {
        bitmap_base64_loader_->Load(std::move(request), std::move(callback));
      } else {
        bitmap_http_loader_->Load(std::move(request), std::move(callback));
      }
      break;
    }
    case ResourceRequestType::kDownloadToLocal: {
      if (request.uri_info.scheme == UriInfo::Scheme::kFile) {
        callback(ResourceResponse{.payload = MakePathResourcePayload(
                                      GetUriLocalPath(request.uri_info.uri))},
                 LoaderError{});
      } else {
        download_to_local_http_loader_->Load(std::move(request),
                                             std::move(callback));
      }
      break;
    }
    default: {
      callback(ResourceResponse{},
               LoaderError{.code = kInvalidLoaderRequest,
                           .message = "Load fail, invalid request type."});
      break;
    }
  }
}
}  // namespace animax
}  // namespace lynx
