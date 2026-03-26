// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_HARMONY_UNZIP_UTIL_H_
#define ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_HARMONY_UNZIP_UTIL_H_

#include <string>
#include <vector>

namespace lynx {
namespace animax {

class UnzipUtil {
 public:
  /**
   * Unzips the specified ZIP file to the target directory.
   *
   * @param src_path The path to the source ZIP file.
   * @param target_path The path to the target directory.
   * @return true if the operation was successful, false otherwise.
   */
  static bool UnzipToPath(const std::string& src_path,
                          const std::string& target_path);

 private:
  // Recursively create directories
  static bool CreateDirectories(const std::string& path);

  // Decompress the ZIP data
  static int Decompress(const std::vector<char>& compressed_data,
                        std::vector<char>& decompressed_data);
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_HARMONY_UNZIP_UTIL_H_
