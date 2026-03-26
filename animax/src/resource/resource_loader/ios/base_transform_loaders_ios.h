// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_IOS_BASE_TRANSFORM_LOADERS_IOS_H_
#define ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_IOS_BASE_TRANSFORM_LOADERS_IOS_H_

#include "include/resource/loader.h"
#include "src/resource/resource_loader/ios/transform_task_ios.h"

@class AnimaXContext;
@protocol AnimaXImageDecoderService;

namespace lynx {
namespace animax {

class NSDataBitmapTransformLoader : public TransformLoaderBaseIOS {
 public:
  NSDataBitmapTransformLoader(AnimaXContext* context = nullptr);
  ~NSDataBitmapTransformLoader() override = default;
  void Load(TransformRequestIOS task, CallbackType callback) override;

 private:
  id<AnimaXImageDecoderService> decoder_impl_ = nil;
};

class NSDataRawDataTransformLoader : public TransformLoaderBaseIOS {
 public:
  ~NSDataRawDataTransformLoader() override = default;
  void Load(TransformRequestIOS task, CallbackType callback) override;
};

class FilePathNSStringToFilePathStringLoader : public TransformLoaderBaseIOS {
 public:
  ~FilePathNSStringToFilePathStringLoader() override = default;
  void Load(TransformRequestIOS task, CallbackType callback) override;
};

class NSDataToFilePathStringLoader : public TransformLoaderBaseIOS {
 public:
  ~NSDataToFilePathStringLoader() override = default;
  void Load(TransformRequestIOS task, CallbackType callback) override;
};

class FilePathNSStringToNSDataLoader : public Loader<TransformRequestIOS, TransformRequestIOS> {
 public:
  ~FilePathNSStringToNSDataLoader() override = default;
  void Load(TransformRequestIOS task, CallbackType callback) override;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_IOS_BASE_TRANSFORM_LOADERS_IOS_H_
