// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(NSUInteger, AnimationType) { AnimationTypeLottie = 0, AnimationTypeAnimaX = 1 };

typedef NS_ENUM(NSInteger, LottieAdapterAnimationSourceType) {
  LottieAdapterAnimationNone,
  LottieAdapterAnimationNamed,
  LottieAdapterAnimationJSON,
  LottieAdapterAnimationFilePath,
  LottieAdapterAnimationURL,
};

/// Configuration toggles for the adapter:
/// - Selects the animation type (`animationType`)
/// - Enables image mode optimization (`useImageMode`)
/// - Enables multi-thread acceleration (`multiThreadAccelerate`)
///
/// You can set process-wide defaults via:
///   [LottieAdapterFactory setGlobalOption:option]
/// or override per instance via:
///   [[LottieAdapterView alloc] initWithOptions:option]
@interface LottieAdapterOption : NSObject <NSCopying>
@property(nonatomic, assign) AnimationType type;
@property(nonatomic, assign) BOOL useImageMode;
@property(nonatomic, assign) BOOL multiThreadAccelerate;
- (instancetype)init;
@end

/// A unified construction parameter object used to create adapters via:
///   [LottieAdapterFactory createWithRequest:request]
///
/// This is an internal wiring model that consolidates source and options.
/// Direct usage is not recommended; prefer constructing animations through
/// LottieAdapterView factory methods or initializers instead.
@interface LottieAdapterRequest : NSObject
@property(nonatomic, assign, readonly) LottieAdapterAnimationSourceType sourceType;
@property(nonatomic, copy, readonly, nullable) NSString *name;
@property(nonatomic, strong, readonly, nullable) NSBundle *bundle;
@property(nonatomic, strong, readonly, nullable) NSDictionary *json;
@property(nonatomic, copy, readonly, nullable) NSString *filePath;
@property(nonatomic, strong, readonly, nullable) NSURL *url;
@property(nonatomic, copy, readonly, nullable) LottieAdapterOption *options;
+ (instancetype)animationWithOptions:(nullable LottieAdapterOption *)options;
+ (instancetype)animationNamed:(nonnull NSString *)name
                      inBundle:(nullable NSBundle *)bundle
                       options:(nullable LottieAdapterOption *)options;
+ (instancetype)animationFromJSON:(nonnull NSDictionary *)json
                         inBundle:(nullable NSBundle *)bundle
                          options:(nullable LottieAdapterOption *)options;
+ (instancetype)animationWithFilePath:(nonnull NSString *)filePath
                              options:(nullable LottieAdapterOption *)options;
+ (instancetype)animationWithURL:(nonnull NSURL *)url
                         options:(nullable LottieAdapterOption *)options;
@end

NS_ASSUME_NONNULL_END
