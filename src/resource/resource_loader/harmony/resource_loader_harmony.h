// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_HARMONY_RESOURCE_LOADER_HARMONY_H_
#define ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_HARMONY_RESOURCE_LOADER_HARMONY_H_

#include <atomic>
#include <functional>
#include <map>
#include <mutex>

#include "core/public/lynx_resource_loader.h"
#include "include/base/macros.h"
#include "include/resource/loader.h"
#include "include/resource/resource_task.h"

namespace lynx {
namespace animax {

class ANIMAX_EXPORT ResourceLoaderHarmony
    : public Loader<ResourceRequest, ResourceResponse> {
 public:
  using Loader = Loader<ResourceRequest, ResourceResponse>::Ptr;

  using Report = std::map<std::string, std::string>;
  using Reporter = std::function<void(Report)>;

  struct Reporting {
    explicit Reporting(Reporter callback, bool enable = true)
        : send(std::move(callback)), enabled(enable) {}
    void SetSource(const std::string& source) {
      std::lock_guard<std::mutex> lock(source_mutex_);
      source_ = source;
    }
    std::string Source() const {
      std::lock_guard<std::mutex> lock(source_mutex_);
      return source_;
    }
    const Reporter send;
    std::atomic<bool> enabled;

   private:
    mutable std::mutex source_mutex_;
    std::string source_ = "unknown";
  };

  explicit ResourceLoaderHarmony(
      std::shared_ptr<pub::LynxResourceLoader> loader,
      std::shared_ptr<Reporting> reporting = nullptr);

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
  std::shared_ptr<Reporting> reporting_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_HARMONY_RESOURCE_LOADER_HARMONY_H_
