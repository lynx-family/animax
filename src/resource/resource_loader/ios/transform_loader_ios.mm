// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/resource_loader/ios/transform_loader_ios.h"
#include <AnimaX/AnimaXContext.h>
#include "src/base/thread/task_runner.h"
#include "src/resource/loader/exec_loader.h"
#include "src/resource/loader/pipe_loader.h"
#include "src/resource/resource_loader/ios/base_transform_loaders_ios.h"
#include "src/resource/resource_loader/ios/resource_util.h"

namespace lynx {
namespace animax {

namespace {

TransformLoaderIOS::Ptr MatrixGetLoader(const TransformLoaderIOS::TransformMatrix& matrix,
                                        TransformRequestInputType input,
                                        ResourcePayloadType output) {
  return matrix[static_cast<size_t>(input)][static_cast<size_t>(output)];
}

void MatrixSetLoader(TransformLoaderIOS::TransformMatrix& matrix, TransformRequestInputType input,
                     ResourcePayloadType output, TransformLoaderIOS::Ptr loader) {
  matrix[static_cast<size_t>(input)][static_cast<size_t>(output)] = std::move(loader);
}

TransformLoaderIOS::TransformMatrix MakeMatrix(AnimaXContext* context) {
  using LoaderIOS = TransformLoaderBaseIOS;
  auto ns_data_raw_data = LoaderIOS::Make<NSDataRawDataTransformLoader>();
  auto ns_data_bitmap =
      ResourceThreadExecLoader(LoaderIOS::Make<NSDataBitmapTransformLoader>(context));
  auto ns_string_ns_data = ResourceThreadExecLoader(
      Loader<TransformRequestIOS, TransformRequestIOS>::Make<FilePathNSStringToNSDataLoader>());
  auto ns_string_raw_data = ns_string_ns_data | ns_data_raw_data;
  auto ns_string_bitmap = ns_string_ns_data | ns_data_bitmap;
  auto ns_string_path = LoaderIOS::Make<FilePathNSStringToFilePathStringLoader>();

  auto ns_data_file = ResourceThreadExecLoader(LoaderIOS::Make<NSDataToFilePathStringLoader>());

  auto matrix = TransformLoaderIOS::TransformMatrix{};
  MatrixSetLoader(matrix, TransformRequestInputType::kNSData, ResourcePayloadType::kRawData,
                  ns_data_raw_data);
  MatrixSetLoader(matrix, TransformRequestInputType::kNSData, ResourcePayloadType::kBitmap,
                  ns_data_bitmap);
  MatrixSetLoader(matrix, TransformRequestInputType::kNSData, ResourcePayloadType::kFilePath,
                  ns_data_file);
  MatrixSetLoader(matrix, TransformRequestInputType::kFilePathNSString,
                  ResourcePayloadType::kRawData, ns_string_raw_data);
  MatrixSetLoader(matrix, TransformRequestInputType::kFilePathNSString,
                  ResourcePayloadType::kBitmap, ns_string_bitmap);
  MatrixSetLoader(matrix, TransformRequestInputType::kFilePathNSString,
                  ResourcePayloadType::kFilePath, ns_string_path);
  return matrix;
};
}  // namespace

void TransformLoaderIOS::Load(TransformRequestIOS request, CallbackType callback) {
  auto loader = MatrixGetLoader(matrix_, request.input_type, request.output_type);
  if (!loader) {
    callback(TransformResponseIOS{},
             LoaderError{.code = LoaderErrorCode::kInvalidLoader,
                         .message = "Transform Loader is unable to transform: Failed to find "
                                    "matching loader. for input_type: " +
                                    std::to_string(static_cast<int32_t>(request.input_type)) +
                                    " output_type: " +
                                    std::to_string(static_cast<int32_t>(request.output_type))});
    return;
  }
  loader->Load(std::move(request), [callback = std::move(callback)](TransformResponseIOS response,
                                                                    LoaderError error) mutable {
    callback(std::move(response), std::move(error));
  });
}

TransformLoaderIOS::TransformLoaderIOS(AnimaXContext* context) : matrix_(MakeMatrix(context)) {}

}  // namespace animax
}  // namespace lynx
