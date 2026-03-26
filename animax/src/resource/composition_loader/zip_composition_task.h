// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_COMPOSITION_LOADER_ZIP_COMPOSITION_TASK_H_
#define ANIMAX_SRC_RESOURCE_COMPOSITION_LOADER_ZIP_COMPOSITION_TASK_H_

#include "include/resource/raw_data.h"

namespace lynx {
namespace animax {

struct ZipCompositionModelTask {
  CompositionModelRequest request{};
  UriInfo zip_file_path{};
  UriInfo json_file_path{};
  std::unique_ptr<RawData> json_data{};
  CompositionModelResponse response{};
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_COMPOSITION_LOADER_ZIP_COMPOSITION_TASK_H_
