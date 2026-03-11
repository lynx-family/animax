// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXComposition.h>
#import <XCTest/XCTest.h>
#import "AnimaXComposition+Internal.h"

@interface AnimaXCompositionTest : XCTestCase
@end

@implementation AnimaXCompositionTest

- (void)testInitWithNullModel {
  AnimaXComposition *composition = [[AnimaXComposition alloc] initWithSharedModel:NULL];
  XCTAssertNotNil(composition, @"Should create composition even with NULL model");
  XCTAssertEqual([composition startFrame], 0, @"Start frame should be 0 for null model");
  XCTAssertEqual([composition endFrame], 0, @"End frame should be 0 for null model");
  XCTAssertNil([composition internalModel], @"Internal model should be null");
}

@end
