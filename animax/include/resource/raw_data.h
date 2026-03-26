// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_INCLUDE_RESOURCE_RAW_DATA_H_
#define ANIMAX_INCLUDE_RESOURCE_RAW_DATA_H_

#include <memory>

#include "include/base/macros.h"

namespace lynx {
namespace animax {

class ANIMAX_EXPORT RawData {
 public:
  using RawDataDeleter = void (*)(const void*);
  using RawDataDeleterContext = const void*;

  static std::unique_ptr<RawData> MakeRawData(const void* data,
                                              std::size_t length,
                                              RawDataDeleter deleter,
                                              RawDataDeleterContext context);
  ~RawData();

  const void* Data() const { return data_; }
  std::size_t Length() const { return length_; }

 private:
  RawData(const void* data, std::size_t length, RawDataDeleter deleter,
          RawDataDeleterContext context);
  const void* data_;
  std::size_t length_;
  RawDataDeleter deleter_;
  RawDataDeleterContext context_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_INCLUDE_RESOURCE_RAW_DATA_H_
