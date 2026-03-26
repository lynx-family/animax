// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_HARMONY_RESOURCE_LOADER_HARMONY_H_
#define ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_HARMONY_RESOURCE_LOADER_HARMONY_H_

#include "core/public/lynx_resource_loader.h"
#include "include/resource/loader.h"
#include "include/resource/resource_task.h"

namespace lynx {
namespace animax {

class ResourceLoaderHarmony : public Loader<ResourceRequest, ResourceResponse> {
 public:
  using Loader = Loader<ResourceRequest, ResourceResponse>::Ptr;

  ResourceLoaderHarmony(std::shared_ptr<pub::LynxResourceLoader> loader);

  ~ResourceLoaderHarmony() override = default;

  void Load(ResourceRequest request, CallbackType callback) override;

 private:
  Loader CreateRawDataHttpLoader();

  Loader CreateRawDataFileLoader();

  Loader CreateBitmapHttpLoader();

  Loader CreateBitmapFileLoader();

  Loader CreateBitmapBase64Loader();

  Loader CreateDownloadToLocalHttpLoader();

  Loader raw_data_file_loader_;

  Loader raw_data_http_loader_;

  Loader bitmap_http_loader_;

  Loader bitmap_file_loader_;

  Loader bitmap_base64_loader_;

  Loader download_to_local_http_loader_;

  std::weak_ptr<pub::LynxResourceLoader> platform_loader_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_HARMONY_RESOURCE_LOADER_HARMONY_H_
