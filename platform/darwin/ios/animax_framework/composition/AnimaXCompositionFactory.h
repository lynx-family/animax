// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXComposition.h>
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@class AnimaXContext;

typedef void (^AnimaXCompositionCompletionBlock)(AnimaXComposition *_Nullable composition,
                                                 NSError *_Nullable error);

@interface AnimaXCompositionFactory : NSObject

+ (instancetype)new NS_UNAVAILABLE;
- (instancetype)init NS_UNAVAILABLE;

+ (AnimaXCompositionFactory *)shared;

- (void)loadJson:(NSString *)json
     withContext:(AnimaXContext *)context
      completion:(AnimaXCompositionCompletionBlock)completion;
- (void)loadUri:(NSString *)uri
    withContext:(AnimaXContext *)context
     completion:(AnimaXCompositionCompletionBlock)completion;

- (void)loadJson:(NSString *)json completion:(AnimaXCompositionCompletionBlock)completion;
- (void)loadUri:(NSString *)uri completion:(AnimaXCompositionCompletionBlock)completion;

- (void)releaseAll;

@end

NS_ASSUME_NONNULL_END
