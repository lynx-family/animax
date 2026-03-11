// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <XCTest/XCTest.h>
#import "src/resource/resource_loader/ios/AnimaXFileLoader.h"

@interface AnimaXFileLoaderUnittest : XCTestCase
@property(nonatomic) AnimaXFileLoader *fileLoader;
@property(nonatomic) NSString *tempFilePath;
@end

@implementation AnimaXFileLoaderUnittest

- (void)setUp {
  [super setUp];
  self.fileLoader = [[AnimaXFileLoader alloc] init];
  // Create a temporary file
  NSString *tempDirectory = NSTemporaryDirectory();
  self.tempFilePath = [tempDirectory stringByAppendingPathComponent:@"tempFile.txt"];
  NSString *content = @"Sample content for testing.";
  [content writeToFile:self.tempFilePath atomically:YES encoding:NSUTF8StringEncoding error:nil];
}

- (void)tearDown {
  // Clean up: Remove the temporary file
  [[NSFileManager defaultManager] removeItemAtPath:self.tempFilePath error:nil];
  [super tearDown];
}

- (void)testFileLoaderWithValidFile {
  AnimaXLoaderRequest *request = [AnimaXLoaderRequest requestWithURL:self.tempFilePath];

  XCTestExpectation *expectation = [self expectationWithDescription:@"Completion handler called"];
  [self.fileLoader handleRequest:request
                      completion:^(AnimaXLoaderResponse *response) {
                        XCTAssertNotNil(response.payload, "Payload should not be nil");
                        XCTAssertNil(response.error, "There should be no error");
                        NSString *responseString =
                            [[NSString alloc] initWithData:response.payload
                                                  encoding:NSUTF8StringEncoding];
                        XCTAssertEqualObjects(responseString, @"Sample content for testing.",
                                              "Content should match the written file");
                        [expectation fulfill];
                      }];
  [self waitForExpectationsWithTimeout:5.0 handler:nil];
}

- (void)testFileLoaderWithNilRequest {
  XCTestExpectation *expectation =
      [self expectationWithDescription:@"Completion handler called for nil request"];
  [self.fileLoader handleRequest:nil
                      completion:^(AnimaXLoaderResponse *response) {
                        XCTAssertNotNil(response.error, "Error should be present for nil request");
                        [expectation fulfill];
                      }];
  [self waitForExpectationsWithTimeout:5.0 handler:nil];
}

- (void)testFileLoaderWithNonexistentFile {
  AnimaXLoaderRequest *request = [AnimaXLoaderRequest
      requestWithURL:[self.tempFilePath stringByAppendingString:@"nonexistent"]];

  XCTestExpectation *expectation =
      [self expectationWithDescription:@"Completion handler called for nonexistent file"];
  [self.fileLoader
      handleRequest:request
         completion:^(AnimaXLoaderResponse *response) {
           XCTAssertNotNil(response.error, "Error should be present for nonexistent file");
           [expectation fulfill];
         }];
  [self waitForExpectationsWithTimeout:5.0 handler:nil];
}

@end
