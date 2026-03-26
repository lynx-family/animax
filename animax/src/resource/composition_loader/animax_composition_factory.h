// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_COMPOSITION_LOADER_ANIMAX_COMPOSITION_FACTORY_H_
#define ANIMAX_SRC_RESOURCE_COMPOSITION_LOADER_ANIMAX_COMPOSITION_FACTORY_H_

#include <map>
#include <memory>
#include <string>

#include "base/include/lynx_actor.h"
#include "src/player/animax_composition_loader.h"

namespace lynx {
namespace animax {

using CompositionCallback =
    base::MoveOnlyClosure<void, std::shared_ptr<CompositionModel>,
                          const std::string&>;

class AnimaXCompositionFactory {
 public:
  static AnimaXCompositionFactory& Instance();
  virtual ~AnimaXCompositionFactory() = default;

  void CreateLoader(
      std::string scope,
      Loader<ResourceRequest, ResourceResponse>::Ptr resource_loader,
      Loader<UnzipRequest, UnzipResponse>::Ptr unzip_loader);

  void LoadJson(std::string scope, std::string json_str, float scale,
                CompositionCallback callback);

  void LoadUri(std::string scope, std::string uri, float scale,
               CompositionCallback callback);

  void Release();

 private:
  std::shared_ptr<shell::LynxActor<AnimaXCompositionLoader>> GetLoader(
      const std::string& scope);
  std::map<std::string,
           std::shared_ptr<shell::LynxActor<AnimaXCompositionLoader>>>
      composition_loaders_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_COMPOSITION_LOADER_ANIMAX_COMPOSITION_FACTORY_H_
