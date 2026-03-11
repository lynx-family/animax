// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXFrameInfo.h>
#import <AnimaX/AnimaXValueCallback.h>
#include <memory>
#import "AnimaXValueCallback+Internal.h"
#import "AnimaXValueParam+Internal.h"

#include "include/property/animax_value_param.h"
#include "src/base/log/log.h"
#include "src/model/value/base_value.h"
#include "src/property/animax_frame_info.h"
#include "src/property/animax_value_callback.h"

namespace lynx {
namespace animax {

class AnimaXValueCallbackWrapper : public AnimaXValueCallback {
 public:
  using GetValueFunction = std::function<std::unique_ptr<Value>(
      const Value*, const AnimaXFrameInfo&, LayerPropertyType)>;

  explicit AnimaXValueCallbackWrapper(GetValueFunction get_value_function,
                                      LayerPropertyType property_type)
      : get_value_function_(get_value_function), property_type_(property_type) {}

  ~AnimaXValueCallbackWrapper() override = default;

  std::unique_ptr<Value> GetValue(const Value* original_value,
                                  const AnimaXFrameInfo& frame_info) override {
    if (!get_value_function_) {
      return nullptr;
    }
    return get_value_function_(original_value, frame_info, property_type_);
  }

 private:
  GetValueFunction get_value_function_;
  LayerPropertyType property_type_;
};

}  // namespace animax
}  // namespace lynx

@implementation AnimaXValueCallback

#pragma mark - Initialization

- (instancetype)init {
  self = [super init];
  return self;
}

#pragma mark - Abstract Methods

- (nullable AnimaXValueParam*)getValue:(AnimaXFrameInfo*)frameInfo {
  return nullptr;
}

@end

@implementation AnimaXValueCallback (Internal)

- (nullable id)getValueInternalWithStartFrame:(float)startFrame
                                     endFrame:(float)endFrame
                                   startValue:(AnimaXValueParam*)startValue
                                     endValue:(AnimaXValueParam*)endValue
                               linearProgress:(float)linearProgress
                         interpolatedProgress:(float)interpolatedProgress
                              overallProgress:(float)overallProgress {
  AnimaXFrameInfo* frameInfo = [[AnimaXFrameInfo alloc] initWithStartFrame:startFrame
                                                                  endFrame:endFrame
                                                                startValue:startValue
                                                                  endValue:endValue
                                                            linearProgress:linearProgress
                                                      interpolatedProgress:interpolatedProgress
                                                           overallProgress:overallProgress];

  return [self getValue:frameInfo];
}

+ (std::shared_ptr<lynx::animax::AnimaXValueCallback>)
    createNativeCallback:(AnimaXValueCallback*)iosCallback
            propertyType:(LayerPropertyType)propertyType {
  if (!iosCallback) {
    return nullptr;
  }

  auto layerType = static_cast<lynx::animax::LayerPropertyType>(propertyType);

  // Create a holder to maintain strong references to OC objects
  struct CallbackHolder {
    AnimaXValueCallback* __strong callback;

    explicit CallbackHolder(AnimaXValueCallback* cb) : callback(cb) {}
    ~CallbackHolder() = default;
  };

  // Define the getValue function that bridges OC to native
  auto getValueFunction =
      [holder = std::make_shared<CallbackHolder>(iosCallback)](
          const lynx::animax::Value* originalValue, const lynx::animax::AnimaXFrameInfo& frameInfo,
          lynx::animax::LayerPropertyType propertyType) -> std::unique_ptr<lynx::animax::Value> {
    // Convert native frame info to OC types
    AnimaXValueParam* startValue =
        [AnimaXValueParam valueParamWithNativeValue:frameInfo.GetStartValue()];
    AnimaXValueParam* endValue =
        [AnimaXValueParam valueParamWithNativeValue:frameInfo.GetEndValue()];

    // Create OC frame info object
    AnimaXFrameInfo* frameInfoOC =
        [[AnimaXFrameInfo alloc] initWithStartFrame:frameInfo.GetStartFrame()
                                           endFrame:frameInfo.GetEndFrame()
                                         startValue:startValue
                                           endValue:endValue
                                     linearProgress:frameInfo.GetLinearProgress()
                               interpolatedProgress:frameInfo.GetInterpolatedProgress()
                                    overallProgress:frameInfo.GetOverallProgress()];

    // Call OC callback directly in current thread
    AnimaXValueParam* result = [holder->callback getValue:frameInfoOC];

    if (!result) {
      return nullptr;
    }

    // Convert result back to native Value
    auto nativeValueParam = [result toNative];
    if (!nativeValueParam) {
      return nullptr;
    }

    return nativeValueParam->ToValue(propertyType);
  };

  return std::make_shared<lynx::animax::AnimaXValueCallbackWrapper>(getValueFunction, layerType);
}

@end
