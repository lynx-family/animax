// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/base/util/ios/ca_util.h"
#import <OCMock/OCMock.h>
#import <XCTest/XCTest.h>

@interface ca_util_Unittest : XCTestCase
@end

@implementation ca_util_Unittest

- (void)testCreateCAMetalLayer {
  using namespace lynx::animax;
  CGRect frame = CGRectMake(100, 100, 16, 16);
  CGFloat scale = 2;
  CAMetalLayer* layer = CreateCAMetalLayer(frame, scale);
  XCTAssertEqual(layer.frame.origin.x, 0);
  XCTAssertEqual(layer.frame.origin.y, 0);
  XCTAssertEqual(layer.frame.size.width, frame.size.width);
  XCTAssertEqual(layer.frame.size.height, frame.size.height);
  XCTAssertEqual(layer.opaque, NO);
  XCTAssertEqual(layer.pixelFormat, MTLPixelFormatBGRA8Unorm);
  XCTAssertEqual(layer.contentsScale, scale);
  XCTAssertNotNil(layer.device);
}

bool approximatelyEqualCGFloat(CGFloat a, CGFloat b) {
  return fabs(a - b) <= ((fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * CGFLOAT_EPSILON);
}

- (void)testResizeCALayer {
  using namespace lynx::animax;
  // 1. Setup initial conditions
  CGSize initialSize = CGSizeMake(10, 10);
  CGSize newSize = CGSizeMake(20, 15);
  CGFloat scale = [UIScreen mainScreen].scale;  // Assuming the test runs on a device

  // 2. Test with CALayer
  CALayer* layer = [CALayer layer];
  layer.frame = CGRectMake(0, 0, initialSize.width, initialSize.height);

  ResizeCALayer(layer, newSize);

  // 3. Verify CALayer size change
  XCTAssertTrue(approximatelyEqualCGFloat(layer.frame.size.width, newSize.width));
  XCTAssertTrue(approximatelyEqualCGFloat(layer.frame.size.height, newSize.height));
}

- (void)testResizeCAMetalLayerDrawable {
  using namespace lynx::animax;

  CGSize initialSize = CGSizeMake(10, 10);
  CGSize newSize = CGSizeMake(20, 15);
  CGFloat scale = [UIScreen mainScreen].scale;  // Assuming the test runs on a device

  CAMetalLayer* metalLayer = [CAMetalLayer layer];
  metalLayer.frame = CGRectMake(0, 0, initialSize.width, initialSize.height);

  ResizeCALayer(metalLayer, newSize);
  ResizeCAMetalLayerDrawable(metalLayer, newSize);

  // Verify CAMetalLayer size and drawable size change
  XCTAssertTrue(approximatelyEqualCGFloat(metalLayer.frame.size.width, newSize.width));
  XCTAssertTrue(approximatelyEqualCGFloat(metalLayer.frame.size.height, newSize.height));

  CGSize expectedDrawableSize = CGSizeMake(newSize.width, newSize.height);
  XCTAssertTrue(
      approximatelyEqualCGFloat(metalLayer.drawableSize.width, expectedDrawableSize.width));
  XCTAssertTrue(
      approximatelyEqualCGFloat(metalLayer.drawableSize.height, expectedDrawableSize.height));
}

@end
