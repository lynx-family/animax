// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import "AnimaXPropertyUpdater.h"

#import <Foundation/Foundation.h>
#import <dispatch/dispatch.h>

#import <AnimaX/AnimaXFrameInfo.h>
#import <AnimaX/AnimaXKeyPath.h>
#import <AnimaX/AnimaXKeyPathListCallback.h>
#import <AnimaX/AnimaXPropertyCallback.h>
#import <AnimaX/AnimaXValueCallback.h>
#import <AnimaX/AnimaXValueParam.h>
#import "AnimaXKeyPath+Internal.h"
#import "AnimaXValueCallback+Internal.h"
#import "AnimaXValueParam+Internal.h"

#include "include/player/animax_player.h"
#include "include/property/property_update_request.h"
#include "include/property/property_update_response.h"
#include "src/base/log/log.h"
#include "src/property/animax_property_updater.h"
#include "src/property/property_updater.h"

#include <memory>

@interface AnimaXPropertyUpdater () {
  std::weak_ptr<lynx::animax::AnimaXPlayer> _playerWeakPtr;
}

@end

@implementation AnimaXPropertyUpdater

#pragma mark - Initialization

- (instancetype)initWithPlayer:(void *)playerPtr {
  self = [super init];
  if (self) {
    if (playerPtr) {
      auto *sharedPtr = static_cast<std::shared_ptr<lynx::animax::AnimaXPlayer> *>(playerPtr);
      _playerWeakPtr = *sharedPtr;
    }
  }
  return self;
}

#pragma mark - Property Updates

- (void)updateLayerProperty:(AnimaXKeyPath *)keyPath
               propertyType:(LayerPropertyType)propertyType
                      value:(AnimaXValueParam *)value
                   callback:(nullable id<AnimaXPropertyCallback>)callback {
  if (![self validateLayerPropertyInputs:keyPath value:value callback:callback]) {
    return;
  }

  auto player = _playerWeakPtr.lock();
  if (!player) {
    [self callError:callback withMessage:@"Property updater not available"];
    return;
  }

  auto layerType = static_cast<lynx::animax::LayerPropertyType>(propertyType);
  auto nativeKeyPath = [keyPath toNative];
  auto nativeValue = [value toNative];
  auto nativeCallback = [self createNativeCallback:callback];

  if (!nativeKeyPath || !nativeValue) {
    [self callError:callback withMessage:@"Failed to convert parameters"];
    return;
  }

  auto request = lynx::animax::LayerStaticRequest::Make(
      layerType, std::move(nativeKeyPath), std::move(nativeValue), std::move(nativeCallback));

  if (request) {
    player->UpdateLayerProperty(std::move(request));
  } else {
    [self callError:callback withMessage:@"Failed to create layer request"];
  }
}

- (void)updateResourceProperty:(AnimaXKeyPath *)keyPath
                  propertyType:(ResourcePropertyType)propertyType
                         value:(AnimaXValueParam *)value
                      callback:(nullable id<AnimaXPropertyCallback>)callback {
  if (![self validateResourcePropertyInputs:keyPath value:value callback:callback]) {
    return;
  }

  auto player = _playerWeakPtr.lock();
  if (!player) {
    [self callError:callback withMessage:@"Property updater not available"];
    return;
  }

  auto resourceType = static_cast<lynx::animax::ResourcePropertyType>(propertyType);

  std::vector<std::string> keys = [keyPath toStringVector];
  if (keys.empty()) {
    [self callError:callback withMessage:@"Empty key path"];
    return;
  }

  std::string resourceId = keys[0];
  auto nativeValue = [value toNative];
  auto nativeCallback = [self createNativeCallback:callback];

  if (!nativeValue) {
    [self callError:callback withMessage:@"Failed to convert value parameter"];
    return;
  }

  // Create and execute request
  auto request = lynx::animax::ResourceUpdateRequest::Make(
      resourceType, std::move(resourceId), std::move(nativeValue), std::move(nativeCallback));

  if (request) {
    player->SetResourceProperty(std::move(request));
  } else {
    [self callError:callback withMessage:@"Failed to create resource request"];
  }
}

- (void)addLayerPropertyCallback:(LayerPropertyType)propertyType
                         keyPath:(AnimaXKeyPath *)keyPath
                   valueCallback:(AnimaXValueCallback *)valueCallback
                        callback:(nullable id<AnimaXPropertyCallback>)callback {
  if (![self validateCallbackInputs:keyPath valueCallback:valueCallback callback:callback]) {
    return;
  }

  auto player = _playerWeakPtr.lock();
  if (!player) {
    [self callError:callback withMessage:@"Property updater not available"];
    return;
  }

  // Convert OC types to native C++ types
  auto layerType = static_cast<lynx::animax::LayerPropertyType>(propertyType);
  auto nativeKeyPath = [keyPath toNative];

  if (!nativeKeyPath) {
    [self callError:callback withMessage:@"Failed to convert key path"];
    return;
  }

  // Create native value callback wrapper
  auto nativeValueCallback = [AnimaXValueCallback createNativeCallback:valueCallback
                                                          propertyType:propertyType];
  if (!nativeValueCallback) {
    [self callError:callback withMessage:@"Failed to create value callback wrapper"];
    return;
  }

  auto nativeCallback = [self createNativeCallback:callback];

  // Create and execute request
  auto request = lynx::animax::LayerCallbackRequest::Make(layerType, std::move(nativeKeyPath),
                                                          std::move(nativeValueCallback),
                                                          std::move(nativeCallback));

  if (request) {
    player->AddLayerPropertyCallback(std::move(request));
  } else {
    [self callError:callback withMessage:@"Failed to create callback request"];
  }
}

- (void)getKeysForKeyPath:(AnimaXKeyPath *)keyPath
                 callback:(nullable id<AnimaXKeyPathListCallback>)callback {
  auto player = _playerWeakPtr.lock();
  if (!player) {
    if (callback) {
      [callback onCallback:@[]];
    }
    return;
  }

  if (!keyPath) {
    if (callback) {
      [callback onCallback:@[]];
    }
    return;
  }

  auto nativeKeyPath = [keyPath toNative];
  if (!nativeKeyPath) {
    if (callback) {
      [callback onCallback:@[]];
    }
    return;
  }

  player->GetKeysForKeyPath(std::move(nativeKeyPath),
                            [callback](const std::vector<lynx::animax::AnimaXKeyPath> &key_paths) {
                              if (callback) {
                                NSMutableArray<AnimaXKeyPath *> *result = [NSMutableArray array];
                                for (const auto &kp : key_paths) {
                                  AnimaXKeyPath *objCKeyPath =
                                      [[AnimaXKeyPath alloc] initWithNativeKeyPath:kp];
                                  if (objCKeyPath) {
                                    [result addObject:objCKeyPath];
                                  }
                                }
                                [callback onCallback:result];
                              }
                            });
}

#pragma mark - Private Helper Methods

- (BOOL)validateLayerPropertyInputs:(AnimaXKeyPath *)keyPath
                              value:(AnimaXValueParam *)value
                           callback:(nullable id<AnimaXPropertyCallback>)callback {
  auto player = _playerWeakPtr.lock();
  if (!player) {
    [self callError:callback withMessage:@"AnimaX player is null"];
    return NO;
  }

  if (!keyPath) {
    [self callError:callback withMessage:@"Key path is null"];
    return NO;
  }

  if (!value) {
    [self callError:callback withMessage:@"Value parameter is null"];
    return NO;
  }

  return YES;
}

- (BOOL)validateResourcePropertyInputs:(AnimaXKeyPath *)keyPath
                                 value:(AnimaXValueParam *)value
                              callback:(nullable id<AnimaXPropertyCallback>)callback {
  auto player = _playerWeakPtr.lock();
  if (!player) {
    [self callError:callback withMessage:@"AnimaX player is null"];
    return NO;
  }

  if (!keyPath) {
    [self callError:callback withMessage:@"Key path is null"];
    return NO;
  }

  if (!value) {
    [self callError:callback withMessage:@"Value parameter is null"];
    return NO;
  }

  return YES;
}

- (BOOL)validateCallbackInputs:(AnimaXKeyPath *)keyPath
                 valueCallback:(AnimaXValueCallback *)valueCallback
                      callback:(nullable id<AnimaXPropertyCallback>)callback {
  auto player = _playerWeakPtr.lock();
  if (!player) {
    [self callError:callback withMessage:@"AnimaX player is null"];
    return NO;
  }

  if (!keyPath) {
    [self callError:callback withMessage:@"Key path is null"];
    return NO;
  }

  if (!valueCallback) {
    [self callError:callback withMessage:@"Value callback is null"];
    return NO;
  }

  return YES;
}

#pragma mark - Callback Helper Methods

- (std::function<void(const lynx::animax::PropertyUpdateResponse &response)>)createNativeCallback:
    (nullable id<AnimaXPropertyCallback>)callback {
  if (!callback) {
    return nullptr;
  }

  __weak typeof(self) weakSelf = self;
  return [callback, weakSelf](const lynx::animax::PropertyUpdateResponse &response) {
    __strong typeof(weakSelf) strongSelf = weakSelf;
    if (strongSelf) {
      [strongSelf handlePropertyCallback:callback response:response];
    }
  };
}

- (void)callError:(nullable id<AnimaXPropertyCallback>)callback
      withMessage:(NSString *)errorMessage {
  if (!callback) {
    return;
  }
  NSArray *errorMessages = errorMessage ? @[ errorMessage ] : @[];
  dispatch_async(dispatch_get_main_queue(), ^{
    [callback onError:errorMessages];
  });
}

- (void)handlePropertyCallback:(nullable id<AnimaXPropertyCallback>)callback
                      response:(const lynx::animax::PropertyUpdateResponse &)response {
  if (!callback) {
    return;
  }

  dispatch_async(dispatch_get_main_queue(), ^{
    if (response.IsSuccess()) {
      [callback onSuccess];
    } else {
      std::vector<std::string> messages = response.GetUpdateMessages();
      NSMutableArray *errorMessages = [NSMutableArray arrayWithCapacity:messages.size()];
      for (const auto &msg : messages) {
        [errorMessages addObject:[NSString stringWithUTF8String:msg.c_str()]];
      }
      [callback onError:errorMessages];
    }
  });
}

@end
