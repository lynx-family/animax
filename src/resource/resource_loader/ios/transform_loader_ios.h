// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_IOS_TRANSFORM_LOADER_IOS_H_
#define ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_IOS_TRANSFORM_LOADER_IOS_H_

#include <array>

#include "include/resource/loader.h"
#include "src/resource/resource_loader/ios/transform_task_ios.h"

@class AnimaXContext;

namespace lynx {
namespace animax {

// TransformLoaderIOS acts as an intermediary, facilitating the conversion of OC
// data types to AnimaX data types.
//
// The chart below summarizes the capabilities of the Transform Loader:
//
// clang-format off
//
//     X = implemented | R = post on AnimaXResource thread
//                                                                          
//               output_type                                                
//               ──────────────────┐                                        
//            ┌─────────┬───────┬──▼────┬────────┐                          
//            │         │RawData│Bitmap │FilePath│                          
//            ├─────────┼───────┼───────┼────────┤                          
//            │NSData   │X      │X R    │X R     │                          
// input_type │         │       │       │*1      │                          
// ──────────►├─────────┼───────┼───────┼────────┤                          
//            │FilePath │X R    │X R    │X       │                          
//            │NSString │       │       │*2      │                          
//            └─────────┴───────┴───────┴────────┘
//
// clang-format on
//
//  *1: Save NSData to default_file_path_for_ns_data_file_path_conversion
//      if given, otherwise, save to a temporary file
//
//  *2: Just convert the NSString to std::string
//
class TransformLoaderIOS : public TransformLoaderBaseIOS {
 public:
  using TransformMatrix =
      std::array<std::array<Ptr, static_cast<std::size_t>(ResourcePayloadType::kEnumCount)>,
                 static_cast<std::size_t>(TransformRequestInputType::kEnumCount)>;
  TransformLoaderIOS(AnimaXContext* context = nullptr);
  ~TransformLoaderIOS() override = default;
  void Load(TransformRequestIOS task, CallbackType callback) override;

 private:
  TransformMatrix matrix_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_IOS_TRANSFORM_LOADER_IOS_H_
