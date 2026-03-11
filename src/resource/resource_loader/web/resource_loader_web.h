// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_WEB_RESOURCE_LOADER_WEB_H_
#define ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_WEB_RESOURCE_LOADER_WEB_H_

#include <emscripten/val.h>

#include <list>
#include <map>

#include "base/include/no_destructor.h"
#include "include/resource/loader.h"
#include "include/resource/resource_task.h"

namespace lynx {
namespace animax {

class ResourceLoaderWeb : public Loader<ResourceRequest, ResourceResponse> {
 public:
  ResourceLoaderWeb();

  ~ResourceLoaderWeb() override;

  void Load(ResourceRequest request, CallbackType callback) override;

  using ResourceLoaderImpl = std::function<void(
      const std::string& url, int32_t result_type, int32_t callback_id)>;
  void SetLoaderImpl(ResourceLoaderImpl impl);

  struct ResourceLoadResult {
    int32_t callback_id = 0;
    bool success;
    uint32_t size = 0, width = 0, height = 0;
    const uint8_t* data = nullptr;
    std::string error_message;
  };
  static void OnResourceLoaded(const ResourceLoadResult& result);

  struct CallbackInfo {
    ResourceRequestType type;
    CallbackType callback;
  };
  struct CallbackManager {
    int32_t next_id = 0, next_loader_id = 0;
    std::map<int32_t, std::weak_ptr<CallbackInfo>> callbacks;
  };
  static base::NoDestructor<CallbackManager> callback_manager;

 private:
  ResourceLoaderImpl loader_impl_ = nullptr;
  std::list<std::shared_ptr<CallbackInfo>> callback_list_;
  const int32_t loader_id_ = 0;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_WEB_RESOURCE_LOADER_WEB_H_
