// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_HARMONY_UNZIP_LOADER_HARMONY_H_
#define ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_HARMONY_UNZIP_LOADER_HARMONY_H_

#include "include/resource/loader.h"
#include "include/resource/unzip_task.h"

namespace lynx {
namespace animax {

class UnzipLoaderHarmony : public Loader<UnzipRequest, UnzipResponse> {
 public:
  UnzipLoaderHarmony();
  ~UnzipLoaderHarmony() override = default;
  void Load(UnzipRequest, CallbackType callback) override;

 private:
  Loader<UnzipRequest, UnzipResponse>::Ptr loader_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_HARMONY_UNZIP_LOADER_HARMONY_H_
