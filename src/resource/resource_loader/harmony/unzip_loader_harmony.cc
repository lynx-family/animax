// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/resource_loader/harmony/unzip_loader_harmony.h"

#include <filesystem>

#include "src/resource/loader/exec_loader.h"
#include "src/resource/loader/lambda_loader.h"
#include "src/resource/resource_loader/harmony/unzip_util.h"

namespace lynx {
namespace animax {

namespace {
static constexpr std::string_view kUnzipSuffix = "animax_unzip_";

std::filesystem::path GetParentPath(const std::string& path) {
  return std::filesystem::path(path).parent_path();
}

std::string GetHashString(const std::string& path) {
  uintptr_t pointer_value = reinterpret_cast<uintptr_t>(path.c_str());
  size_t hash_value = std::hash<uintptr_t>{}(pointer_value);

  std::stringstream ss;
  ss << std::hex << std::setw(16) << std::setfill('0') << hash_value;
  return ss.str();
}
}  // namespace

UnzipLoaderHarmony::UnzipLoaderHarmony() {
  auto unzip = MakeLambdaLoader<UnzipRequest, UnzipResponse>(
      [](UnzipRequest request, auto callback) {
        auto& zip_path = request.zip_file_path;
        auto parent_path = GetParentPath(zip_path);

        if (zip_path.empty() || parent_path.empty()) {
          callback(UnzipResponse{},
                   LoaderError{.code = kInvalidArgument,
                               .message = "Zip or unzip path is invalid."});
          return;
        }

        auto hash_str = GetHashString(zip_path);
        auto unzip_path =
            (parent_path / std::string(kUnzipSuffix).append(hash_str)).string();

        // todo(aiyongbiao.rick): In some cases, the unzip utility may succeed
        // but return false.
        UnzipUtil::UnzipToPath(zip_path, unzip_path);
        callback(UnzipResponse{.path = std::move(unzip_path)}, LoaderError{});
      });
  loader_ = ResourceThreadExecLoader(unzip);
}

void UnzipLoaderHarmony::Load(UnzipRequest request, CallbackType callback) {
  loader_->Load(std::move(request), std::move(callback));
}

}  // namespace animax
}  // namespace lynx
