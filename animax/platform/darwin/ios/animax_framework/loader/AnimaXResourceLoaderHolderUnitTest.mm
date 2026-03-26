// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXResourceFactoryService.h>
#import <AnimaX/AnimaXServiceScope.h>
#import <AnimaX/DefaultAnimaXAbility.h>
#import <OCMock/OCMock.h>
#import <XCTest/XCTest.h>
#import "AnimaXResourceLoaderHolder+Internal.h"
#import "AnimaXResourceLoaderHolder.h"
#include "include/player/animax_player.h"
#include "include/player/animax_player_builder.h"
#include "include/player/vsync_monitor.h"

namespace {

class FakeVSyncMonitor : public lynx::animax::VSyncMonitor {
 public:
  ~FakeVSyncMonitor() override = default;

  using Callback = lynx::base::MoveOnlyClosure<void, int64_t>;
  void AsyncRequestVSync(Callback callback) override {}

  void RequestVSync(Callback callback) override {}
};

}  // namespace

@interface AnimaXResourceLoaderHolderTest : XCTestCase
@property(nonatomic, strong) id mockFactory;
@property(nonatomic, strong) id mockLoader;
@end

@implementation AnimaXResourceLoaderHolderTest

- (void)setUp {
  [super setUp];
  _mockFactory = OCMProtocolMock(@protocol(AnimaXResourceFactoryService));
  _mockLoader = OCMProtocolMock(@protocol(AnimaXLoaderProtocol));
}

- (void)tearDown {
  [_mockFactory stopMocking];
  [_mockLoader stopMocking];
  [super tearDown];
}

- (void)testloaderForCompositionFactoryWithValidScope {
  NSString *scope = @"testScope";
  NSArray *loaders = @[ _mockLoader ];

  OCMStub([_mockFactory createAnimaXLoaders]).andReturn(loaders);
  AnimaXContext *context =
      [[AnimaXContext alloc] initWithAbility:[[DefaultAnimaXAbility alloc] init]];
  AnimaXResourceLoaderHolder *holder =
      [AnimaXResourceLoaderHolder loaderForCompositionFactory:context];
  XCTAssertNotNil(holder, @"Holder should not be nil");
}

- (void)testLoaderForPlayerWithValidPlayer {
  NSArray *loaders = @[ _mockLoader ];

  OCMStub([_mockFactory createAnimaXLoaders]).andReturn(loaders);
  AnimaXContext *context =
      [[AnimaXContext alloc] initWithAbility:[[DefaultAnimaXAbility alloc] init]];
  lynx::animax::AnimaXPlayerBuilder builder;
  AnimaXResourceLoaderHolder *holder = [AnimaXResourceLoaderHolder loaderForPlayer:&builder
                                                                           context:context];
  XCTAssertNotNil(holder, @"Holder should not be nil");
}

- (void)testLoaderForPlayerWithNilPlayer {
  AnimaXContext *context =
      [[AnimaXContext alloc] initWithAbility:[[DefaultAnimaXAbility alloc] init]];
  AnimaXResourceLoaderHolder *holder = [AnimaXResourceLoaderHolder loaderForPlayer:NULL
                                                                           context:context];
  XCTAssertNil(holder, @"Holder should be nil for null player pointer");
}

- (void)testLoaderForPlayerWithNilFactory {
  AnimaXContext *context =
      [[AnimaXContext alloc] initWithAbility:[[DefaultAnimaXAbility alloc] init]];
  lynx::animax::AnimaXPlayerBuilder builder;
  AnimaXResourceLoaderHolder *holder = [AnimaXResourceLoaderHolder loaderForPlayer:&builder
                                                                           context:context];
  XCTAssertNotNil(holder, @"Holder should not be nil even with nil factory");
}

@end
