// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/player/ios/vsync_monitor_ios.h"
#include "src/base/log/log.h"
#include "src/base/thread/thread_assert.h"

#import <Foundation/Foundation.h>

#pragma mark - AnimaXVSyncMonitor

@interface AnimaXVSyncMonitor : NSObject

typedef void (^AnimaXVSyncMonitorCallback)(NSInteger timestamp);

@property(atomic) CADisplayLink* displayLink;
@property(atomic) BOOL isInBackground;

@end

@implementation AnimaXVSyncMonitor {
  AnimaXVSyncMonitorCallback _callback;
}

- (instancetype)init {
  self = [super init];
  if (self) {
    // The newly constructed display link retains the self.
    // You have to call "invalidate" explicitly to release a AnimaXVSyncMonitor.
    _displayLink = [CADisplayLink displayLinkWithTarget:self
                                               selector:@selector(displayLinkCallback:)];
    [_displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];
    _displayLink.paused = YES;

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(appWillEnterForeground:)
                                                 name:UIApplicationWillEnterForegroundNotification
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(appDidEnterBackground:)
                                                 name:UIApplicationDidEnterBackgroundNotification
                                               object:nil];
  }
  return self;
}

- (void)appWillEnterForeground:(UIApplication*)application {
  _isInBackground = NO;
}

- (void)appDidEnterBackground:(UIApplication*)application {
  _isInBackground = YES;
}

- (void)displayLinkCallback:(CADisplayLink*)displayLink {
  if (_isInBackground || _displayLink == nil) {
    return;
  }

  displayLink.paused = YES;
  if (_callback) {
    _callback(_displayLink.timestamp * 1e+9);
  }
  _callback = nil;
}

- (void)requestVSync:(AnimaXVSyncMonitorCallback)callback {
  if (!callback || !_displayLink) {
    return;
  }
  _displayLink.paused = NO;
  _callback = callback;
}

- (void)invalidate {
  [[NSNotificationCenter defaultCenter] removeObserver:self];
  if (_displayLink) {
    ANIMAX_LOGI("AnimaXVSyncMonitor invalidate");
    [_displayLink invalidate];
    _displayLink = nil;
  }
  _callback = nil;
}

- (void)dealloc {
  ANIMAX_LOGI("AnimaXVSyncMonitor dealloc");
  [self invalidate];
}

@end

#pragma mark - VSyncMonitorIOS

namespace lynx {
namespace animax {

template <typename Block>
void VSyncMonitorIOS::ExecuteOnMainThread(Block block) {
  if ([NSThread isMainThread]) {
    block();
  } else {
    dispatch_async(dispatch_get_main_queue(), block);
  }
}

VSyncMonitorIOS::VSyncMonitorIOS() {
  ExecuteOnMainThread(^{
    monitor_ = [[AnimaXVSyncMonitor alloc] init];
  });
}

VSyncMonitorIOS::~VSyncMonitorIOS() {
  AnimaXVSyncMonitor* monitor = monitor_;
  ExecuteOnMainThread(^{
    [monitor invalidate];
  });
}

void VSyncMonitorIOS::RequestVSync(VSyncMonitor::Callback callback) {
  auto callback_ptr = std::shared_ptr<Callback>{new Callback{std::move(callback)}};
  // Call from AnimaX_Main thread to UI thread to request VSync.
  __weak AnimaXVSyncMonitor* weak_monitor = monitor_;
  ExecuteOnMainThread(^{
    if (!weak_monitor) {
      return;
    }

    [weak_monitor requestVSync:^(NSInteger timestamp) {
      (*callback_ptr)(timestamp);
    }];
  });
}

}  // namespace animax
}  // namespace lynx
