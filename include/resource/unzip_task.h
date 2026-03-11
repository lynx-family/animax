// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_INCLUDE_RESOURCE_UNZIP_TASK_H_
#define ANIMAX_INCLUDE_RESOURCE_UNZIP_TASK_H_

#include <string>

namespace lynx {
namespace animax {

struct UnzipRequest {
  std::string zip_file_path;
  std::string unzip_folder_path;
};

struct UnzipResponse {
  std::string path;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_INCLUDE_RESOURCE_UNZIP_TASK_H_
