// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <OCMock/OCMock.h>
#import <XCTest/XCTest.h>
#import "CVPixelBufferWrapper.h"

#include <cstring>

@interface TestView : UIView <AnimaXPixelBufferUpdateListener>
@property(nonatomic, assign) BOOL didDraw;
@property(nonatomic, assign) NSUInteger updateCount;
@property(nonatomic, strong) AnimaXScopedCVPixelBuffer *displayBufferScope;
@end

@implementation TestView
- (void)onBufferUpdated:(AnimaXScopedCVPixelBuffer *)bufferScope {
  self.didDraw = YES;
  self.updateCount++;
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
  CVPixelBufferRef buffer = [CVPixelBufferWrapper createPixelBufferWithWidth:width
                                                                      height:height
                                                                     backend:AnimaXMetal];
  wrapper.renderPixelBufferScope = [AnimaXScopedCVPixelBuffer newWrapOwned:buffer];
  [wrapper resizePixelBufferWrapperWithWidth:width height:height];
  wrapper.generation++;
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

- (void)fillBuffer:(CVPixelBufferRef)buffer withByte:(uint8_t)value {
  CVReturn result = CVPixelBufferLockBaseAddress(buffer, 0);
  XCTAssertEqual(result, kCVReturnSuccess);
  if (result != kCVReturnSuccess) {
    return;
  }
  memset(CVPixelBufferGetBaseAddress(buffer), value,
         CVPixelBufferGetHeight(buffer) * CVPixelBufferGetBytesPerRow(buffer));
  CVPixelBufferUnlockBaseAddress(buffer, 0);
}

- (uint8_t)firstByteOfBuffer:(CVPixelBufferRef)buffer {
  CVReturn result = CVPixelBufferLockBaseAddress(buffer, kCVPixelBufferLock_ReadOnly);
  XCTAssertEqual(result, kCVReturnSuccess);
  if (result != kCVReturnSuccess) {
    return 0;
  }
  uint8_t value = ((uint8_t *)CVPixelBufferGetBaseAddress(buffer))[0];
  CVPixelBufferUnlockBaseAddress(buffer, kCVPixelBufferLock_ReadOnly);
  return value;
}

- (void)waitForPendingBufferNotifications {
  XCTestExpectation *expect = [self expectationWithDescription:@"pending buffer notifications"];
  dispatch_async(dispatch_get_main_queue(), ^{
    [expect fulfill];
  });
  [self waitForExpectationsWithTimeout:1 handler:nil];
}

- (void)testSoftwareBufferPreservesRenderTargetAndCallbackFrames {
  CVPixelBufferWrapper *wrapper = [[CVPixelBufferWrapper alloc] init];
  wrapper.backend = AnimaXSoftware;
  CVPixelBufferRef renderBuffer = [CVPixelBufferWrapper createPixelBufferWithWidth:4
                                                                            height:4
                                                                           backend:AnimaXSoftware];
  wrapper.renderPixelBufferScope = [AnimaXScopedCVPixelBuffer newWrapOwned:renderBuffer];
  [wrapper resizePixelBufferWrapperWithWidth:4 height:4];
  NSMutableArray<AnimaXScopedCVPixelBuffer *> *frames = [NSMutableArray new];
  XCTestExpectation *expect = [self expectationWithDescription:@"two completed software buffers"];
  expect.expectedFulfillmentCount = 2;
  wrapper.frameAvailableHandler = ^(CVPixelBufferRef buffer) {
    XCTAssertTrue([NSThread isMainThread]);
    [frames addObject:[AnimaXScopedCVPixelBuffer newWrap:buffer]];
    [expect fulfill];
  };
  uint8_t pixels[64];
  memset(pixels, 0x11, sizeof(pixels));
  [wrapper notifyBufferUpdateWithGeneration:wrapper.generation
                                  srcPixels:pixels
                                      width:4
                                     height:4
                                     stride:16];
  XCTAssertEqual(wrapper.renderPixelBufferScope.object, renderBuffer);
  XCTAssertEqual([self firstByteOfBuffer:renderBuffer], 0x11);
  memset(pixels, 0x22, sizeof(pixels));
  [wrapper notifyBufferUpdateWithGeneration:wrapper.generation
                                  srcPixels:pixels
                                      width:4
                                     height:4
                                     stride:16];
  [self waitForExpectationsWithTimeout:1 handler:nil];
  XCTAssertEqual(frames.count, 2u);
  if (frames.count == 2) {
    XCTAssertNotEqual(frames[0].object, renderBuffer);
    XCTAssertNotEqual(frames[1].object, renderBuffer);
    XCTAssertNotEqual(frames[0].object, frames[1].object);
    XCTAssertEqual([self firstByteOfBuffer:frames[0].object], 0x11);
    XCTAssertEqual([self firstByteOfBuffer:frames[1].object], 0x22);
  }
  XCTAssertEqual(wrapper.renderPixelBufferScope.object, renderBuffer);
  XCTAssertEqual([self firstByteOfBuffer:renderBuffer], 0x22);
}

- (void)testFrameHandlerSkipsExpiredGenerationAndInvalidPixels {
  CVPixelBufferWrapper *wrapper = [[CVPixelBufferWrapper alloc] init];
  __block BOOL notified = NO;
  wrapper.frameAvailableHandler = ^(CVPixelBufferRef buffer) {
    notified = YES;
  };
  uint8_t pixels[64] = {};
  [wrapper notifyBufferUpdateWithGeneration:wrapper.generation + 1
                                  srcPixels:pixels
                                      width:4
                                     height:4
                                     stride:16];
  [wrapper notifyBufferUpdateWithGeneration:wrapper.generation
                                  srcPixels:nullptr
                                      width:4
                                     height:4
                                     stride:16];
  [wrapper notifyBufferUpdateWithGeneration:wrapper.generation];
  [self waitForPendingBufferNotifications];
  XCTAssertFalse(notified);
  XCTAssertNil(wrapper.displayPixelBufferScope);
}

- (void)testSoftwareBufferWithoutHandlerKeepsOriginalBuffer {
  CVPixelBufferWrapper *wrapper = [[CVPixelBufferWrapper alloc] init];
  wrapper.backend = AnimaXSoftware;
  CVPixelBufferRef buffer = [CVPixelBufferWrapper createPixelBufferWithWidth:4
                                                                      height:4
                                                                     backend:AnimaXSoftware];
  wrapper.renderPixelBufferScope = [AnimaXScopedCVPixelBuffer newWrapOwned:buffer];
  id mock = OCMPartialMock(wrapper);
  OCMReject([mock acquirePixelBufferFromPool]);
  uint8_t pixels[64];
  memset(pixels, 0x33, sizeof(pixels));
  [wrapper notifyBufferUpdateWithGeneration:wrapper.generation
                                  srcPixels:pixels
                                      width:4
                                     height:4
                                     stride:16];
  XCTAssertEqual(wrapper.renderPixelBufferScope.object, buffer);
  XCTAssertEqual([self firstByteOfBuffer:buffer], 0x33);
  XCTAssertNil(wrapper.displayPixelBufferScope);
  OCMVerifyAll(mock);
  [mock stopMocking];
}

- (void)testFrameHandlerSkipsFailedCopy {
  CVPixelBufferWrapper *wrapper = [[CVPixelBufferWrapper alloc] initWithView:self.iv];
  wrapper.backend = AnimaXSoftware;
  AnimaXScopedCVPixelBuffer *previousFrame = [AnimaXScopedCVPixelBuffer
      newWrapOwned:[CVPixelBufferWrapper createPixelBufferWithWidth:4
                                                             height:4
                                                            backend:AnimaXMetal]];
  XCTAssertNotNil(previousFrame);
  wrapper.displayPixelBufferScope = previousFrame;
  __block BOOL notified = NO;
  wrapper.frameAvailableHandler = ^(CVPixelBufferRef buffer) {
    notified = YES;
  };
  CVPixelBufferRef wrongSizeBuffer = [CVPixelBufferWrapper createPixelBufferWithWidth:2
                                                                               height:2
                                                                              backend:AnimaXMetal];
  XCTAssertTrue(wrongSizeBuffer != nullptr);
  id mock = OCMPartialMock(wrapper);
  OCMStub([mock acquirePixelBufferFromPool]).andReturn(wrongSizeBuffer);
  uint8_t pixels[64] = {};
  [wrapper notifyBufferUpdateWithGeneration:wrapper.generation
                                  srcPixels:pixels
                                      width:4
                                     height:4
                                     stride:16];
  [self waitForPendingBufferNotifications];
  XCTAssertFalse(notified);
  XCTAssertEqual(wrapper.displayPixelBufferScope, previousFrame);
  [mock stopMocking];
}

- (void)testFrameHandlerSkipsGenerationChangedBeforeDelivery {
  CVPixelBufferWrapper *wrapper = [[CVPixelBufferWrapper alloc] initWithView:self.iv];
  wrapper.backend = AnimaXSoftware;
  __block BOOL notified = NO;
  wrapper.frameAvailableHandler = ^(CVPixelBufferRef buffer) {
    notified = YES;
  };
  uint8_t pixels[64] = {};
  [wrapper notifyBufferUpdateWithGeneration:wrapper.generation
                                  srcPixels:pixels
                                      width:4
                                     height:4
                                     stride:16];
  wrapper.generation++;
  [self waitForPendingBufferNotifications];
  XCTAssertFalse(notified);
}

- (void)testSoftwareViewReceivesFrameHandler {
  CVPixelBufferWrapper *wrapper = [[CVPixelBufferWrapper alloc] initWithView:self.iv];
  wrapper.backend = AnimaXSoftware;
  XCTestExpectation *expect = [self expectationWithDescription:@"software copied frame"];
  __block AnimaXScopedCVPixelBuffer *receivedBuffer;
  wrapper.frameAvailableHandler = ^(CVPixelBufferRef buffer) {
    XCTAssertTrue([NSThread isMainThread]);
    receivedBuffer = [AnimaXScopedCVPixelBuffer newWrap:buffer];
    [expect fulfill];
  };
  uint8_t pixels[64] = {};
  [wrapper notifyBufferUpdateWithGeneration:wrapper.generation
                                  srcPixels:pixels
                                      width:4
                                     height:4
                                     stride:16];
  [self waitForExpectationsWithTimeout:1 handler:nil];
  XCTAssertNotNil(receivedBuffer);
  XCTAssertTrue(self.iv.didDraw);
  XCTAssertEqual(receivedBuffer.object, self.iv.displayBufferScope.object);
}
- (CVPixelBufferWrapper *)newMetalWrapperWithView:(TestView *)view {
  CVPixelBufferWrapper *wrapper =
      view ? [[CVPixelBufferWrapper alloc] initWithView:view] : [[CVPixelBufferWrapper alloc] init];
  CVPixelBufferRef buffer = [CVPixelBufferWrapper createPixelBufferWithWidth:4
                                                                      height:4
                                                                     backend:AnimaXMetal];
  XCTAssertTrue(buffer != nullptr);
  wrapper.renderPixelBufferScope = [AnimaXScopedCVPixelBuffer newWrapOwned:buffer];
  [wrapper resizePixelBufferWrapperWithWidth:4 height:4];
  XCTAssertNotNil(wrapper.metalTexture);
  return wrapper;
}

- (void)clearMetalWrapper:(CVPixelBufferWrapper *)wrapper
                    color:(MTLClearColor)color
             commandQueue:(id<MTLCommandQueue>)queue {
  MTLRenderPassDescriptor *pass = [MTLRenderPassDescriptor renderPassDescriptor];
  pass.colorAttachments[0].texture = wrapper.metalTexture;
  pass.colorAttachments[0].loadAction = MTLLoadActionClear;
  pass.colorAttachments[0].storeAction = MTLStoreActionStore;
  pass.colorAttachments[0].clearColor = color;
  id<MTLCommandBuffer> command = [queue commandBuffer];
  id<MTLRenderCommandEncoder> encoder = [command renderCommandEncoderWithDescriptor:pass];
  XCTAssertNotNil(encoder);
  [encoder endEncoding];
  [command commit];
}

- (uint32_t)pixelOfBuffer:(CVPixelBufferRef)buffer {
  CVReturn result = CVPixelBufferLockBaseAddress(buffer, kCVPixelBufferLock_ReadOnly);
  XCTAssertEqual(result, kCVReturnSuccess);
  if (result != kCVReturnSuccess) {
    return 0;
  }
  uint32_t pixel = 0;
  memcpy(&pixel, CVPixelBufferGetBaseAddress(buffer), sizeof(pixel));
  CVPixelBufferUnlockBaseAddress(buffer, kCVPixelBufferLock_ReadOnly);
  return pixel;
}

- (void)verifyMetalFrameTimingForView:(BOOL)viewTarget {
  if (@available(iOS 12.0, *)) {
    CVPixelBufferWrapper *wrapper = [self newMetalWrapperWithView:viewTarget ? self.iv : nil];
    AnimaXScopedCVPixelBuffer *firstBuffer = wrapper.renderPixelBufferScope;
    [self fillBuffer:firstBuffer.object withByte:0x11];
    id<MTLTexture> firstTexture = wrapper.metalTexture;
    id<MTLCommandQueue> queue = [firstTexture.device newCommandQueue];
    id<MTLSharedEvent> gate = [firstTexture.device newSharedEvent];
    id<MTLCommandBuffer> blocker = [queue commandBuffer];
    [blocker encodeWaitForEvent:gate value:1];
    [blocker commit];
    // Prevent a deadlock if a regression reintroduces a synchronous wait.
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 2 * NSEC_PER_SEC),
                   dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0), ^{
                     gate.signaledValue = 1;
                   });
    NSMutableArray<AnimaXScopedCVPixelBuffer *> *frames = [NSMutableArray new];
    wrapper.frameAvailableHandler = ^(CVPixelBufferRef buffer) {
      XCTAssertTrue([NSThread isMainThread]);
      [frames addObject:[AnimaXScopedCVPixelBuffer newWrap:buffer]];
    };
    [self clearMetalWrapper:wrapper color:MTLClearColorMake(1, 0, 0, 1) commandQueue:queue];
    [wrapper notifyBufferUpdateWithGeneration:wrapper.generation commandQueue:queue];
    XCTAssertEqual(wrapper.renderPixelBufferScope, firstBuffer);
    XCTAssertEqual(wrapper.metalTexture, firstTexture);
    [self clearMetalWrapper:wrapper color:MTLClearColorMake(0, 1, 0, 1) commandQueue:queue];
    [wrapper notifyBufferUpdateWithGeneration:wrapper.generation commandQueue:queue];
    [self clearMetalWrapper:wrapper color:MTLClearColorMake(0, 0, 1, 1) commandQueue:queue];
    [self waitForPendingBufferNotifications];
    XCTAssertEqual(gate.signaledValue, 0u);
    XCTAssertEqual(frames.count, viewTarget ? 2u : 0u);
    if (viewTarget) {
      XCTAssertTrue(self.iv.didDraw);
      XCTAssertEqual(self.iv.updateCount, 2u);
      XCTAssertEqual([self pixelOfBuffer:self.iv.displayBufferScope.object], 0x11111111u);
    } else {
      XCTAssertNil(wrapper.displayPixelBufferScope);
    }
    XCTestExpectation *expect = [self expectationWithDescription:@"all GPU frames completed"];
    id<MTLCommandBuffer> marker = [queue commandBuffer];
    [marker addCompletedHandler:^(id<MTLCommandBuffer> completed) {
      dispatch_async(dispatch_get_main_queue(), ^{
        [expect fulfill];
      });
    }];
    [marker commit];
    gate.signaledValue = 1;
    [self waitForExpectationsWithTimeout:5 handler:nil];
    XCTAssertEqual(frames.count, 2u);
    if (frames.count == 2) {
      XCTAssertEqual([self pixelOfBuffer:frames[0].object], viewTarget ? 0x11111111u : 0xffff0000u);
      XCTAssertEqual([self pixelOfBuffer:frames[1].object], viewTarget ? 0x11111111u : 0xff00ff00u);
      XCTAssertNotEqual(frames[0].object, frames[1].object);
      XCTAssertNotEqual(frames[0].object, firstBuffer.object);
      XCTAssertNotEqual(frames[1].object, firstBuffer.object);
    }
    XCTAssertEqual(wrapper.renderPixelBufferScope, firstBuffer);
    XCTAssertEqual(wrapper.metalTexture, firstTexture);
    XCTAssertEqual([self pixelOfBuffer:firstBuffer.object], 0xff0000ffu);
    XCTAssertEqual(self.iv.updateCount, viewTarget ? 2u : 0u);
    if (viewTarget && frames.count == 2) {
      XCTAssertEqual(frames[1].object, self.iv.displayBufferScope.object);
    }
  }
}

- (void)testMetalViewDeliversCopiedFrameWithViewUpdate {
  [self verifyMetalFrameTimingForView:YES];
}

- (void)testMetalBufferPreservesRenderTargetBeforeGPUFrameAvailable {
  [self verifyMetalFrameTimingForView:NO];
}

- (void)testMetalFrameHandlerDropsGenerationChangedBeforeGPUCompletion {
  if (@available(iOS 12.0, *)) {
    CVPixelBufferWrapper *wrapper = [self newMetalWrapperWithView:nil];
    id<MTLCommandQueue> queue = [wrapper.metalTexture.device newCommandQueue];
    id<MTLSharedEvent> gate = [wrapper.metalTexture.device newSharedEvent];
    id<MTLCommandBuffer> blocker = [queue commandBuffer];
    [blocker encodeWaitForEvent:gate value:1];
    [blocker commit];
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 2 * NSEC_PER_SEC),
                   dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0), ^{
                     gate.signaledValue = 1;
                   });
    __block BOOL notified = NO;
    wrapper.frameAvailableHandler = ^(CVPixelBufferRef buffer) {
      notified = YES;
    };
    [self clearMetalWrapper:wrapper color:MTLClearColorMake(1, 0, 0, 1) commandQueue:queue];
    [wrapper notifyBufferUpdateWithGeneration:wrapper.generation commandQueue:queue];
    wrapper.generation++;
    XCTestExpectation *expect = [self expectationWithDescription:@"GPU notifications drained"];
    id<MTLCommandBuffer> marker = [queue commandBuffer];
    [marker addCompletedHandler:^(id<MTLCommandBuffer> completed) {
      dispatch_async(dispatch_get_main_queue(), ^{
        [expect fulfill];
      });
    }];
    [marker commit];
    gate.signaledValue = 1;
    [self waitForExpectationsWithTimeout:5 handler:nil];
    XCTAssertFalse(notified);
    XCTAssertNil(wrapper.displayPixelBufferScope);
  }
}

- (void)testMetalFrameWithoutHandlerUsesOriginalViewNotification {
  CVPixelBufferWrapper *wrapper = [self newMetalWrapperWithView:self.iv];
  [self fillBuffer:wrapper.renderPixelBufferScope.object withByte:0x55];
  id<MTLCommandQueue> queue = OCMProtocolMock(@protocol(MTLCommandQueue));
  OCMReject([queue commandBuffer]);
  [wrapper notifyBufferUpdateWithGeneration:wrapper.generation commandQueue:queue];
  XCTAssertNotNil(wrapper.displayPixelBufferScope);
  XCTAssertFalse(self.iv.didDraw);
  [self waitForPendingBufferNotifications];
  XCTAssertTrue(self.iv.didDraw);
  XCTAssertEqual([self pixelOfBuffer:self.iv.displayBufferScope.object], 0x55555555u);
  OCMVerifyAll(queue);
}
- (void)testMetalBufferWithoutHandlerKeepsOriginalBuffer {
  CVPixelBufferWrapper *wrapper = [self newMetalWrapperWithView:nil];
  AnimaXScopedCVPixelBuffer *buffer = wrapper.renderPixelBufferScope;
  id<MTLTexture> texture = wrapper.metalTexture;
  id<MTLCommandQueue> queue = OCMProtocolMock(@protocol(MTLCommandQueue));
  OCMReject([queue commandBuffer]);
  [wrapper notifyBufferUpdateWithGeneration:wrapper.generation commandQueue:queue];
  XCTAssertEqual(wrapper.renderPixelBufferScope, buffer);
  XCTAssertEqual(wrapper.metalTexture, texture);
  XCTAssertNil(wrapper.displayPixelBufferScope);
  OCMVerifyAll(queue);
}

- (void)testMetalBufferPreparationFailureDoesNotPublishOrChangeRenderTarget {
  CVPixelBufferWrapper *wrapper = [self newMetalWrapperWithView:nil];
  AnimaXScopedCVPixelBuffer *buffer = wrapper.renderPixelBufferScope;
  id<MTLTexture> texture = wrapper.metalTexture;
  __block BOOL notified = NO;
  wrapper.frameAvailableHandler = ^(CVPixelBufferRef frame) {
    notified = YES;
  };
  CVPixelBufferRef wrongSizeBuffer = [CVPixelBufferWrapper createPixelBufferWithWidth:2
                                                                               height:2
                                                                              backend:AnimaXMetal];
  id mock = OCMPartialMock(wrapper);
  OCMStub([mock acquirePixelBufferFromPool]).andReturn(wrongSizeBuffer);
  id<MTLCommandQueue> queue = [texture.device newCommandQueue];
  [wrapper notifyBufferUpdateWithGeneration:wrapper.generation commandQueue:queue];
  [self waitForPendingBufferNotifications];
  XCTAssertFalse(notified);
  XCTAssertEqual(wrapper.renderPixelBufferScope, buffer);
  XCTAssertEqual(wrapper.metalTexture, texture);
  [mock stopMocking];
}
@end
