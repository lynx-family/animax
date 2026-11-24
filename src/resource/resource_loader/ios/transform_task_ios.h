// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_IOS_TRANSFORM_TASK_IOS_H_
#define ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_IOS_TRANSFORM_TASK_IOS_H_

#import <Foundation/Foundation.h>

#include "include/resource/loader.h"
#include "include/resource/resource_task.h"

namespace lynx {
namespace animax {

enum class TransformRequestInputType {
  kNSData = 0,
  kFilePathNSString,
  kEnumCount,
  kInvalid = kEnumCount,
};

struct TransformRequestIOS {
  id input = nil;
  TransformRequestInputType input_type = TransformRequestInputType::kInvalid;
  ResourcePayloadType output_type = ResourcePayloadType::kInvalid;
  /* If the (input_type, output_type) is (kNSData, kFilePath),
   * this default_file_path will be used if not empty.
   * If default_file_path is empty, a temp file based on NSData's hash will be
   * used.
   */
  std::string default_file_path_for_ns_data_file_path_conversion;
  // Only independent image requests opt into exact resizing. Zero dimensions
  // retain the original decode size; legacy requests leave these fields zero.
  uint32_t image_width = 0;
  uint32_t image_height = 0;
  bool cache_by_content = false;
};

using TransformResponseIOS = ResourceResponse;
using TransformLoaderBaseIOS =
    Loader<TransformRequestIOS, TransformResponseIOS>;

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_IOS_TRANSFORM_TASK_IOS_H_
