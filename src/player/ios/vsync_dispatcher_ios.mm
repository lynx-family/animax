// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/player/vsync_dispatcher.h"

#include "base/include/no_destructor.h"
#include "src/base/log/log.h"

#import <Foundation/Foundation.h>

#pragma mark - AnimaXDisplayLinkVSync

@interface AnimaXDisplayLinkVSync : NSObject
@property(atomic) CADisplayLink* displayLink;
@end

typedef void (^AnimaXDisplayLinkVSyncCallback)(double timestampSeconds);

@implementation AnimaXDisplayLinkVSync {
  AnimaXDisplayLinkVSyncCallback _callback;
}

- (instancetype)init {
  NSAssert([NSThread isMainThread], @"Must be created on main thread");
  self = [super init];
  if (self) {
    // The newly constructed display link retains the self.
    // You have to call "invalidate" explicitly to release a AnimaXDisplayLinkVSync.
    _displayLink = [CADisplayLink displayLinkWithTarget:self
                                               selector:@selector(displayLinkCallback:)];
    [_displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];
    _displayLink.paused = YES;
  }
  return self;
}

- (void)displayLinkCallback:(CADisplayLink*)displayLink {
  NSAssert([NSThread isMainThread], @"Must be called on main thread");
  if (_displayLink == nil) {
    return;
  }

  displayLink.paused = YES;
  AnimaXDisplayLinkVSyncCallback cb = _callback;
  _callback = nil;
  if (cb) {
    cb(displayLink.timestamp);  // seconds as double
  }
}

- (void)requestVSync:(AnimaXDisplayLinkVSyncCallback)callback {
  NSAssert([NSThread isMainThread], @"Must be called on main thread");
  if (!callback || !_displayLink) {
    return;
  }
  _displayLink.paused = NO;
  _callback = [callback copy];
}

- (void)invalidate {
  if (_displayLink) {
    ANIMAX_LOGI("AnimaXDisplayLinkVSync invalidate");
    [_displayLink invalidate];
    _displayLink = nil;
  }
  _callback = nil;
}

- (void)dealloc {
  ANIMAX_LOGI("AnimaXDisplayLinkVSync dealloc");
  [self invalidate];
}

@end

#pragma mark - VsyncDispatcherIOS

namespace lynx {
namespace animax {

class VsyncDispatcherIOS final : public VSyncDispatcher {
 public:
  VsyncDispatcherIOS() {
    ExecuteOnMainThread(^{
      vsync_source_ = [[AnimaXDisplayLinkVSync alloc] init];
    });
  }
  ~VsyncDispatcherIOS() override {
    ExecuteOnMainThread(^{
      if (vsync_source_) {
        [vsync_source_ invalidate];
        vsync_source_ = nil;
      }
    });
  }
  void RequestVSync() override {
    ExecuteOnMainThread(^{
      AnimaXDisplayLinkVSync* vsync = vsync_source_;
      if (!vsync) {
        return;
      }
      [vsync requestVSync:^(double timestampSeconds) {
        int64_t timestamp_ns = static_cast<int64_t>(llround(timestampSeconds * 1e9));
        VSyncDispatcher::Instance().OnVSync(timestamp_ns);
      }];
    });
  }

 private:
  template <typename Block>
  void ExecuteOnMainThread(Block block) {
    if (NSThread.isMainThread) {
      block();
    } else {
      dispatch_async(dispatch_get_main_queue(), block);
    }
  }
  AnimaXDisplayLinkVSync* vsync_source_ = nil;
};

VSyncDispatcher& VSyncDispatcher::Instance() {
  static base::NoDestructor<VsyncDispatcherIOS> inst;
  return *inst;
}

}  // namespace animax
}  // namespace lynx
