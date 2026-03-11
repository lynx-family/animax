// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <XCTest/XCTest.h>
#import "src/resource/resource_loader/ios/AnimaXBase64Loader.h"

@interface AnimaXBase64LoaderUnitTest : XCTestCase
@property(strong, nonatomic) AnimaXBase64Loader *base64Loader;
@end

@implementation AnimaXBase64LoaderUnitTest

- (void)setUp {
  [super setUp];
  self.base64Loader = [[AnimaXBase64Loader alloc] init];
}

- (void)testBase64LoaderWithValidBase64String {
  // Encode sample data to Base64 string
  NSString *sampleText = @"Hello, world!";
  NSData *sampleData = [sampleText dataUsingEncoding:NSUTF8StringEncoding];
  NSString *base64String = [sampleData base64EncodedStringWithOptions:0];
  NSString *urlString = [NSString stringWithFormat:@"data:text/plain;base64,%@", base64String];

  AnimaXLoaderRequest *request = [AnimaXLoaderRequest requestWithURL:urlString];

  XCTestExpectation *expectation = [self expectationWithDescription:@"Completion handler called"];
  [self.base64Loader handleRequest:request
                        completion:^(AnimaXLoaderResponse *response) {
                          XCTAssertNotNil(response.payload, "Payload should not be nil");
                          NSString *decodedString =
                              [[NSString alloc] initWithData:response.payload
                                                    encoding:NSUTF8StringEncoding];
                          XCTAssertEqualObjects(decodedString, sampleText,
                                                "Decoded string should match the original");
                          XCTAssertNil(response.error, "There should be no error");
                          [expectation fulfill];
                        }];
  [self waitForExpectationsWithTimeout:5.0 handler:nil];
}

- (void)testBase64LoaderWithInvalidURL {
  NSString *invalidURL = @"data:text/plain;base64,????";

  AnimaXLoaderRequest *request = [AnimaXLoaderRequest requestWithURL:invalidURL];

  XCTestExpectation *expectation =
      [self expectationWithDescription:@"Completion handler called for invalid URL"];
  [self.base64Loader
      handleRequest:request
         completion:^(AnimaXLoaderResponse *response) {
           XCTAssertNotNil(response.error, "Error should be present for invalid URL");
           [expectation fulfill];
         }];
  [self waitForExpectationsWithTimeout:5.0 handler:nil];
}

- (void)testBase64LoaderWithEmptyURL {
  AnimaXLoaderRequest *request = [AnimaXLoaderRequest requestWithURL:@""];

  XCTestExpectation *expectation =
      [self expectationWithDescription:@"Completion handler called for empty URL"];
  [self.base64Loader handleRequest:request
                        completion:^(AnimaXLoaderResponse *response) {
                          XCTAssertNotNil(response.error, "Error should be present for empty URL");
                          [expectation fulfill];
                        }];
  [self waitForExpectationsWithTimeout:5.0 handler:nil];
}

@end
