// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/resource_loader/harmony/unzip_util.h"

#include <sys/stat.h>
#include <sys/types.h>

#ifdef OS_WIN
#include <filesystem>

#include "third_party/zlib/zlib.h"
#else
#include <dirent.h>
#include <zlib.h>
#endif

#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

#include "src/base/log/log.h"
#include "src/resource/uri/uri_util.h"

namespace lynx {
namespace animax {

namespace {
uint32_t ReadUInt32(const char* data) {
  return static_cast<uint32_t>((static_cast<unsigned char>(data[0])) |
                               (static_cast<unsigned char>(data[1]) << 8) |
                               (static_cast<unsigned char>(data[2]) << 16) |
                               (static_cast<unsigned char>(data[3]) << 24));
}

uint16_t ReadUInt16(const char* data) {
  return static_cast<uint16_t>((static_cast<unsigned char>(data[0])) |
                               (static_cast<unsigned char>(data[1]) << 8));
}
}  // namespace

bool UnzipUtil::CreateDirectories(const std::string& path) {
  if (path.empty()) return false;

  struct stat info;
  if (stat(path.c_str(), &info) != 0) {
    if (errno == ENOENT) {
      // Directory does not exist, try to create parent directory recursively
      size_t pos = path.find_last_of(kPathSeparator);
      if (pos == std::string::npos) return false;
      std::string parent = path.substr(0, pos);
      if (!CreateDirectories(parent)) return false;

#ifdef OS_WIN
      std::error_code ec;
      bool created = std::filesystem::create_directories(path, ec);
      if (ec) {
        ANIMAX_LOGE("Failed to create directory: " << path << " Error: "
                                                   << ec.message());
        return false;
      }
#else
      // Now create the current directory
      if (mkdir(path.c_str(), 0755) != 0) {
        if (errno != EEXIST) {  // Ignore "file exists" error since it could
                                // happen due to race condition
          ANIMAX_LOGE("Failed to create directory: " << path << " Error: "
                                                     << strerror(errno));
          return false;
        }
      }
#endif
    } else {
      ANIMAX_LOGE("Failed to check existence of directory: "
                  << path << " Error: " << strerror(errno));
      return false;
    }
  } else if (!(info.st_mode & S_IFDIR)) {
    // Path exists but is not a directory
    ANIMAX_LOGE("Path exists but is not a directory: " << path);
    return false;
  }

  return true;
}

// Decompress the ZIP data
int UnzipUtil::Decompress(const std::vector<char>& compressed_data,
                          std::vector<char>& decompressed_data) {
  z_stream zs;
  memset(&zs, 0, sizeof(zs));

  if (inflateInit2(&zs, -MAX_WBITS) != Z_OK) {
    ANIMAX_LOGE("inflateInit2 failed");
    return -1;
  }

  zs.next_in =
      reinterpret_cast<Bytef*>(const_cast<char*>(compressed_data.data()));
  zs.avail_in = compressed_data.size();

  int ret;
  // Buffer size set to 32KB
  char buffer[32768];

  do {
    zs.next_out = reinterpret_cast<Bytef*>(buffer);
    zs.avail_out = sizeof(buffer);

    ret = inflate(&zs, 0);

    if (decompressed_data.size() < zs.total_out) {
      decompressed_data.insert(
          decompressed_data.end(), buffer,
          buffer + zs.total_out - decompressed_data.size());
    }
  } while (ret == Z_OK);

  inflateEnd(&zs);

  if (ret != Z_STREAM_END) {
    ANIMAX_LOGE("inflate failed: " << ret);
    return -1;
  }

  return 0;
}

// Unzip the specified ZIP file to the target directory
bool UnzipUtil::UnzipToPath(const std::string& src_path,
                            const std::string& target_path) {
  std::ifstream file(src_path, std::ios::binary);
  if (!file.is_open()) {
    ANIMAX_LOGE("Failed to open file: " << src_path);
    return false;
  }

  // Read ZIP file content
  std::vector<char> zip_data((std::istreambuf_iterator<char>(file)),
                             std::istreambuf_iterator<char>());
  file.close();

  size_t offset = 0;
  while (offset < zip_data.size()) {
    // PK\x01\x02: All files have been read
    if (memcmp(&zip_data[offset], "PK\x01\x02", 4) == 0) {
      return true;
    }
    // PK\x03\x04: Local file header signature - start of a new file
    if (memcmp(&zip_data[offset], "PK\x03\x04", 4) != 0) {
      ANIMAX_LOGE("Invalid local file header signature");
      return false;
    }
    offset += 4;

    // Read version and flags
    offset += 2;
    uint16_t flags = ReadUInt16(&zip_data[offset]);
    bool has_data_descriptor = (flags & 0x8) != 0;
    offset += 2;

    // Skip compression method and time/date fields
    offset += 10;

    // Read compressed file size and uncompressed file size
    uint32_t compressed_size = ReadUInt32(&zip_data[offset]);
    offset += 4;
    // Skip uncompressed size
    offset += 4;

    // Read file name length and extra field length
    uint16_t file_name_length = ReadUInt16(&zip_data[offset]);
    offset += 2;
    uint16_t extra_field_length = ReadUInt16(&zip_data[offset]);
    offset += 2;

    // Read file name
    std::string file_name(zip_data.data() + offset, file_name_length);

#ifdef OS_WIN
    std::replace(file_name.begin(), file_name.end(), '/', '\\');
#endif

    offset += file_name_length;

    ANIMAX_LOGI("Try to unzip file_name:"
                << file_name << ", compressed_size:" << compressed_size
                << ", has_data_descriptor:" << has_data_descriptor);

    // Skip extra field
    offset += extra_field_length;

    std::string full_path = target_path + kPathSeparator + file_name;
    if (file_name.back() == kPathSeparator[0]) {
      // Directory
      if (!CreateDirectories(full_path)) {
        ANIMAX_LOGE("Cannot create directory: " << full_path);
        return false;
      }
      continue;
    }

    // Create directory for file
    if (!CreateDirectories(
            full_path.substr(0, full_path.find_last_of(kPathSeparator)))) {
      ANIMAX_LOGE("Cannot create directory for file: " << full_path);
      return false;
    }

    // If compressed_size is 0 and has data descriptor, need to find the actual
    // size
    if (has_data_descriptor && compressed_size == 0) {
      // Find data descriptor signature (PK\x07\x08)
      size_t data_start = offset;
      while (offset + 4 < zip_data.size()) {
        if (memcmp(&zip_data[offset], "PK\x07\x08", 4) == 0) {
          // CRC32(4) + Compressed Size(4)
          compressed_size = ReadUInt32(&zip_data[offset + 8]);
          break;
        }
        offset++;
      }

      if (compressed_size == 0) {
        ANIMAX_LOGE("Failed to find valid data descriptor for: " << file_name);
        return false;
      }

      // Reset offset to data start
      offset = data_start;
    }

    // Decompress file
    std::vector<char> compressed_data(
        zip_data.begin() + offset, zip_data.begin() + offset + compressed_size);
    std::vector<char> decompressed_data;
    if (Decompress(compressed_data, decompressed_data) != 0) {
      return false;
    }

    std::ofstream out_file(full_path, std::ios::binary);
    if (!out_file.is_open()) {
      ANIMAX_LOGE("Failed to open output file: " << full_path);
      return false;
    }

    out_file.write(decompressed_data.data(), decompressed_data.size());
    out_file.close();

    ANIMAX_LOGI("Unzip file success: " << full_path);

    offset += compressed_size;

    // Skip data descriptor if present
    if (has_data_descriptor) {
      // Skip signature(4) + CRC32(4) + Compressed Size(4) + Uncompressed
      // Size(4)
      offset += 16;
    }
  }

  return true;
}

}  // namespace animax
}  // namespace lynx
