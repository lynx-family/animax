// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "include/resource/raw_data.h"
namespace lynx {
namespace animax {

std::unique_ptr<RawData> RawData::MakeRawData(const void* data,
                                              std::size_t length,
                                              RawDataDeleter deleter,
                                              RawDataDeleterContext context) {
  return std::unique_ptr<RawData>{new RawData(data, length, deleter, context)};
}

RawData::RawData(const void* data, std::size_t length, RawDataDeleter deleter,
                 RawDataDeleterContext context)
    : data_{data}, length_{length}, deleter_{deleter}, context_{context} {}

RawData::~RawData() {
  if (deleter_) {
    deleter_(context_);
  }
}

}  // namespace animax
}  // namespace lynx
