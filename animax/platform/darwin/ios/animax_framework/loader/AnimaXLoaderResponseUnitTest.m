// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXLoaderResponse+Internal.h>
#import <AnimaX/AnimaXLoaderResponse.h>
#import <XCTest/XCTest.h>

@interface AnimaXLoaderResponseTests : XCTestCase

@end

@implementation AnimaXLoaderResponseTests

// Tests response creation with a valid NSString payload
- (void)testResponseWithValidNSStringPayload {
  NSString *testString = @"Test String";
  AnimaXLoaderResponse<NSString *> *response =
      [AnimaXLoaderResponse responseWithPayload:testString];

  XCTAssertNotNil(response);
  XCTAssertEqual(response.payloadType, AnimaXLoaderPayloadTypeFilePathNSString);
  XCTAssertEqualObjects(response.payload, testString);
  XCTAssertNil(response.error);
}

// Tests response creation with a valid NSData payload
- (void)testResponseWithValidNSDataPayload {
  NSData *testData = [@"Test Data" dataUsingEncoding:NSUTF8StringEncoding];
  AnimaXLoaderResponse<NSData *> *response = [AnimaXLoaderResponse responseWithPayload:testData];

  XCTAssertNotNil(response);
  XCTAssertEqual(response.payloadType, AnimaXLoaderPayloadTypeNSData);
  XCTAssertEqualObjects(response.payload, testData);
  XCTAssertNil(response.error);
}

// Tests response creation with an invalid payload type
- (void)testResponseWithInvalidPayload {
  NSNumber *invalidPayload = @42;
  AnimaXLoaderResponse *response = [AnimaXLoaderResponse responseWithPayload:invalidPayload];

  XCTAssertNotNil(response);
  XCTAssertEqual(response.payloadType, AnimaXLoaderPayloadTypeInvalid);
  XCTAssertNil(response.payload);
  XCTAssertNotNil(response.error);
  XCTAssertEqualObjects(response.error.localizedDescription,
                        @"Invalid payload: payload must be either NSString* or NSData*.");
}

// Tests response creation with a valid error object
- (void)testResponseWithError {
  NSError *testError = [NSError errorWithDomain:@"TestErrorDomain" code:100 userInfo:nil];
  AnimaXLoaderResponse *response = [AnimaXLoaderResponse responseWithError:testError];

  XCTAssertNotNil(response);
  XCTAssertEqual(response.payloadType, AnimaXLoaderPayloadTypeInvalid);
  XCTAssertNil(response.payload);
  XCTAssertEqualObjects(response.error, testError);
}

// Tests response creation with a nil error
- (void)testResponseWithNilError {
  AnimaXLoaderResponse *response = [AnimaXLoaderResponse responseWithError:nil];

  XCTAssertNotNil(response);
  XCTAssertEqual(response.payloadType, AnimaXLoaderPayloadTypeInvalid);
  XCTAssertNil(response.payload);
  XCTAssertNotNil(response.error);
  XCTAssertEqualObjects(response.error.localizedDescription,
                        @"Invalid error: error must not be nil.");
}

@end
