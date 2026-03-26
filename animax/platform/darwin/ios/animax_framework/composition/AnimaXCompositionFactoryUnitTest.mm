// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXCompositionFactory.h>
#import <AnimaX/AnimaXContext.h>
#import <AnimaX/AnimaXServiceScope.h>
#import <AnimaX/DefaultAnimaXAbility.h>
#import <XCTest/XCTest.h>

@interface AnimaXCompositionFactoryTest : XCTestCase
@property(nonatomic, strong) AnimaXCompositionFactory *factory;
@property(nonatomic, strong) AnimaXServiceScope *scope;
@end

@implementation AnimaXCompositionFactoryTest

- (void)setUp {
  [super setUp];
  _factory = [AnimaXCompositionFactory shared];
  _scope = [AnimaXServiceScope defaultScope];
}

- (void)testSharedInstance {
  AnimaXCompositionFactory *instance1 = [AnimaXCompositionFactory shared];
  AnimaXCompositionFactory *instance2 = [AnimaXCompositionFactory shared];
  XCTAssertEqual(instance1, instance2, @"Shared instance should be singleton");
}

- (void)testLoadJsonWithValidJson {
  NSString *validJson = @"{\"test\": \"data\"}";
  XCTestExpectation *expectation = [self expectationWithDescription:@"Load JSON completion"];
  AnimaXContext *context =
      [[AnimaXContext alloc] initWithAbility:[[DefaultAnimaXAbility alloc] init]];

  [_factory loadJson:validJson
         withContext:context
          completion:^(AnimaXComposition *composition, NSError *error) {
            XCTAssertNotNil(composition, @"Composition should not be nil for valid JSON");
            [expectation fulfill];
          }];

  [self waitForExpectationsWithTimeout:5.0 handler:nil];
}

- (void)testLoadJsonWithInvalidJson {
  NSString *invalidJson = @"invalid";
  XCTestExpectation *expectation =
      [self expectationWithDescription:@"Load invalid JSON completion"];
  AnimaXContext *context =
      [[AnimaXContext alloc] initWithAbility:[[DefaultAnimaXAbility alloc] init]];

  [_factory loadJson:invalidJson
         withContext:context
          completion:^(AnimaXComposition *composition, NSError *error) {
            XCTAssertNil(composition, @"Composition should be nil for invalid JSON");
            [expectation fulfill];
          }];

  [self waitForExpectationsWithTimeout:5.0 handler:nil];
}

- (void)testReleaseAll {
  [_factory releaseAll];
  // Verify that subsequent operations still work
  XCTestExpectation *expectation = [self expectationWithDescription:@"Load after release"];
  AnimaXContext *context =
      [[AnimaXContext alloc] initWithAbility:[[DefaultAnimaXAbility alloc] init]];

  [_factory loadJson:@"{}"
         withContext:context
          completion:^(AnimaXComposition *composition, NSError *error) {
            [expectation fulfill];
          }];

  [self waitForExpectationsWithTimeout:5.0 handler:nil];
}

@end
