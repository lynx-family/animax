// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXService.h>
#import <Foundation/Foundation.h>

// URL Related Keys
FOUNDATION_EXPORT NSString *const AnimaXMonitorSourceUrl;
FOUNDATION_EXPORT NSString *const AnimaXMonitorPageUrl;
FOUNDATION_EXPORT NSString *const AnimaXMonitorIntegrationType;

// Associated Object Key
FOUNDATION_EXPORT void const *AnimaXMonitorUrlHolderKey;

@interface AnimaXMonitorUrlHolder : NSObject
@property(atomic, copy) NSString *currentUrl;
- (instancetype)init;
- (NSString *)getCurrentUrl;
- (void)updateUrl:(NSString *)url;
@end

@protocol AnimaXMonitorService <AnimaXService>
@property(nonatomic, readonly) AnimaXMonitorUrlHolder *urlHolder;
- (void)reportError:(NSDictionary<NSString *, NSObject *> *)params;
- (void)reportPerformance:(NSDictionary<NSString *, NSObject *> *)params;
@end

@interface NSObject (AnimaXMonitorServiceDefaultImpl)
@property(nonatomic, readonly) AnimaXMonitorUrlHolder *urlHolder;
@end
