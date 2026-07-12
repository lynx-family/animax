// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXComposition.h>
#import "AnimaX/AnimaXComposition+Internal.h"
#include "src/model/composition_model.h"

@implementation AnimaXComposition {
  std::shared_ptr<lynx::animax::CompositionModel> _model;
}

- (instancetype)initWithSharedModel:(void *)sharedModelPtr {
  self = [super init];
  if (self) {
    auto modelPtr =
        reinterpret_cast<std::shared_ptr<lynx::animax::CompositionModel> *>(sharedModelPtr);
    if (modelPtr) {
      _model = *modelPtr;
    }
  }
  return self;
}

- (CGFloat)startFrame {
  if (!_model) {
    return 0;
  }
  return _model->GetStartFrame();
}

- (CGFloat)endFrame {
  if (!_model) {
    return 0;
  }
  return _model->GetTimelineEndFrame();
}

- (void *)internalModel {
  if (!_model) {
    return nullptr;
  }
  return static_cast<void *>(&_model);
}

@end
