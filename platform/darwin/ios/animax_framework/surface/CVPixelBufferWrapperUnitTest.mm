// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <OCMock/OCMock.h>
#import <XCTest/XCTest.h>
#import "CVPixelBufferWrapper.h"

@interface TestView : UIView <AnimaXPixelBufferUpdateListener>
@property(nonatomic, assign) BOOL didDraw;
@property(nonatomic, strong) AnimaXScopedCVPixelBuffer *displayBufferScope;
@end

@implementation TestView
- (void)onBufferUpdated:(AnimaXScopedCVPixelBuffer *)bufferScope {
  self.didDraw = YES;
  self.displayBufferScope = bufferScope;
}
@end

@interface CVPixelBufferWrapperTests : XCTestCase
@property(nonatomic, strong) TestView *iv;
@end

@implementation CVPixelBufferWrapperTests

- (void)setUp {
  [super setUp];
  _iv = [[TestView alloc] initWithFrame:CGRectMake(0, 0, 30, 30)];
  self.iv.didDraw = NO;
}

- (void)tearDown {
  _iv = nil;
  [super tearDown];
}

- (void)testInit {
  CVPixelBufferWrapper *wrapper = [[CVPixelBufferWrapper alloc] init];
  XCTAssertNotNil(wrapper);
  XCTAssertEqual(wrapper.generation, 0);
  XCTAssertNil(wrapper.renderPixelBufferScope);
  XCTAssertNil(wrapper.metalTexture);
  XCTAssertEqual(wrapper.backend, AnimaXMetal);
}

- (void)testInitWithImageView {
  CVPixelBufferWrapper *wrapper = [[CVPixelBufferWrapper alloc] initWithView:self.iv];
  XCTAssertNotNil(wrapper);
  XCTAssertEqual(wrapper.generation, 0);
  XCTAssertNil(wrapper.renderPixelBufferScope);
  XCTAssertNil(wrapper.metalTexture);
  XCTAssertEqual(wrapper.backend, AnimaXMetal);
}

- (void)testResize {
  CVPixelBufferWrapper *wrapper = [[CVPixelBufferWrapper alloc] initWithView:self.iv];
  wrapper.backend = AnimaXMetal;
  size_t width = 64;
  size_t height = 32;
  NSUInteger genBefore = wrapper.generation;
  [wrapper resizePixelBufferWrapperWithWidth:width height:height];
  CVPixelBufferRef buffer = [wrapper acquirePixelBufferFromPool];
  wrapper.renderPixelBufferScope = [AnimaXScopedCVPixelBuffer newWrapOwned:buffer];
  XCTAssertNotNil(wrapper.renderPixelBufferScope);
  XCTAssertTrue(wrapper.generation > genBefore);
  XCTAssertEqual(CVPixelBufferGetWidth(wrapper.renderPixelBufferScope.object), width);
  XCTAssertEqual(CVPixelBufferGetHeight(wrapper.renderPixelBufferScope.object), height);
  XCTAssertNotNil(wrapper.metalTexture);
  XCTAssertEqual(wrapper.metalTexture.width, width);
  XCTAssertEqual(wrapper.metalTexture.height, height);
}

- (void)testNotifyBuffer {
  CVPixelBufferWrapper *wrapper = [[CVPixelBufferWrapper alloc] initWithView:self.iv];
  size_t width = 32;
  size_t height = 32;
  [wrapper resizePixelBufferWrapperWithWidth:width height:height];
  CVPixelBufferRef buffer = [wrapper acquirePixelBufferFromPool];
  wrapper.renderPixelBufferScope = [AnimaXScopedCVPixelBuffer newWrapOwned:buffer];
  XCTAssertFalse(self.iv.didDraw);

  [wrapper notifyBufferUpdateWithGeneration:wrapper.generation];

  XCTestExpectation *expect = [self expectationWithDescription:@"wait for image set"];
  dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.1 * NSEC_PER_SEC)),
                 dispatch_get_main_queue(), ^{
                   [expect fulfill];
                 });
  [self waitForExpectationsWithTimeout:1 handler:nil];

  XCTAssertTrue(self.iv.didDraw);
  XCTAssertEqual(wrapper.displayPixelBufferScope.object, self.iv.displayBufferScope.object);
}

- (void)testNotifyBufferWithExpiredGen {
  CVPixelBufferWrapper *wrapper = [[CVPixelBufferWrapper alloc] initWithView:self.iv];
  size_t width = 32;
  size_t height = 32;
  [wrapper resizePixelBufferWrapperWithWidth:width height:height];
  CVPixelBufferRef buffer = [wrapper acquirePixelBufferFromPool];
  wrapper.renderPixelBufferScope = [AnimaXScopedCVPixelBuffer newWrapOwned:buffer];
  XCTAssertFalse(self.iv.didDraw);
  [wrapper notifyBufferUpdateWithGeneration:wrapper.generation - 1];
  XCTestExpectation *expect = [self expectationWithDescription:@"wait for no image set"];
  dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.1 * NSEC_PER_SEC)),
                 dispatch_get_main_queue(), ^{
                   [expect fulfill];
                 });
  [self waitForExpectationsWithTimeout:1 handler:nil];

  XCTAssertFalse(self.iv.didDraw);
}
@end
