// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/resource_loader/ios/loader_wrapper_ios.h"
#import <AnimaX/AnimaXLoaderResponse+Internal.h>
#import <OCMock/OCMock.h>
#import <XCTest/XCTest.h>
#include "src/resource/uri/uri_util.h"

@interface loader_wrapper_ios_unittest : XCTestCase
@property(nonatomic) id mockLoader;
@property(nonatomic) id mockResponse;
@property(nonatomic) NSData *txtData;
@end

@implementation loader_wrapper_ios_unittest

- (void)setUp {
  self.mockLoader = OCMProtocolMock(@protocol(AnimaXLoaderProtocol));
  self.mockResponse = OCMClassMock([AnimaXLoaderResponse class]);
  self.txtData = [@"good morning" dataUsingEncoding:NSUTF8StringEncoding];
}

- (void)tearDown {
  [self.mockLoader stopMocking];
  self.mockLoader = nil;
}

- (void)testInvalidLoaderWrapper {
  using namespace lynx::animax;
  XCTestExpectation *expectation = [self expectationWithDescription:@"Callback should be called."];
  auto wrapper = LoaderWrapperIOS::Make<LoaderWrapperIOS>(nil);
  wrapper->Load(ResourceRequest{}, [expectation](TransformRequestIOS result, LoaderError error) {
    [expectation fulfill];
    XCTAssertTrue(!!error);
  });

  [self waitForExpectationsWithTimeout:5 handler:nil];
}

- (void)testResourceRequestWithEmptyURL {
  using namespace lynx::animax;
  XCTestExpectation *expectation = [self expectationWithDescription:@"Callback should be called."];

  // Setup the loader wrapper with the mock loader
  auto wrapper = LoaderWrapperIOS::Make<LoaderWrapperIOS>(self.mockLoader);

  // Define an empty ResourceRequest for a scenario that should result in an error due to an invalid
  // URI
  auto uri = std::string{""};  // Empty URL
  ResourceRequest request = {
      .type = ResourceRequestType::kLoadRawData,
      .uri_info = UriInfo{
          .scheme = ParseUriScheme(uri),
          .content_type =
              UriInfo::ContentType::kUnknown,  // ContentType set to Unknown for empty uri
          .uri = uri,
      }};

  // Expect the loader to handle the request and invoke the completion block with an error
  OCMExpect([self.mockLoader
      handleRequest:[OCMArg checkWithBlock:^BOOL(AnimaXLoaderRequest *animaxRequest) {
        return animaxRequest.url == nil || [animaxRequest.url length] == 0;
      }]
         completion:([OCMArg invokeBlockWithArgs:[NSNull null], nil])]);

  // Perform the test
  wrapper->Load(request, [self, expectation](TransformRequestIOS result, LoaderError error) {
    [expectation fulfill];
    XCTAssertTrue(!!error);  // Check if the error is reported
    XCTAssertEqual(error.code, kInvalidArgument);
  });

  // Wait for expectations
  [self waitForExpectationsWithTimeout:5 handler:nil];
}

- (void)testLoadRawData {
  using namespace lynx::animax;
  XCTestExpectation *expectation = [self expectationWithDescription:@"Callback should be called."];
  auto wrapper = LoaderWrapperIOS::Make<LoaderWrapperIOS>(self.mockLoader);
  OCMStub([self.mockResponse payload]).andReturn(self.txtData);
  OCMStub([self.mockResponse payloadType]).andReturn(AnimaXLoaderPayloadTypeNSData);
  OCMStub([self.mockResponse error]).andReturn(nil);
  OCMExpect([self.mockLoader handleRequest:[OCMArg any]
                                completion:([OCMArg invokeBlockWithArgs:self.mockResponse, nil])]);
  auto uri = std::string{"https://example.com/a.json"};
  wrapper->Load(ResourceRequest{.type = ResourceRequestType::kLoadRawData,
                                .uri_info =
                                    UriInfo{
                                        .scheme = ParseUriScheme(uri),
                                        .content_type = ParseUriMainResourceContentType(uri),
                                        .uri = uri,
                                    }},
                [self, expectation](TransformRequestIOS result, LoaderError error) {
                  [expectation fulfill];
                  XCTAssertFalse(!!error);
                  XCTAssertEqual(result.input_type, TransformRequestInputType::kNSData);
                  XCTAssertEqual(result.output_type, ResourcePayloadType::kRawData);
                  XCTAssertTrue(result.input == self.txtData);
                });

  [self waitForExpectationsWithTimeout:5 handler:nil];
}

- (void)testLoadRawData_ResponseWithAnimaXLoaderPayloadTypeFilePathNSString {
  using namespace lynx::animax;
  XCTestExpectation *expectation = [self expectationWithDescription:@"Callback should be called."];

  // Setup the loader wrapper with the mock loader
  auto wrapper = LoaderWrapperIOS::Make<LoaderWrapperIOS>(self.mockLoader);

  // Prepare a mock response with a file path
  NSString *filePath = @"/path/to/data.txt";
  OCMStub([self.mockResponse payload]).andReturn(filePath);
  OCMStub([self.mockResponse payloadType]).andReturn(AnimaXLoaderPayloadTypeFilePathNSString);
  OCMStub([self.mockResponse error]).andReturn(nil);
  OCMExpect([self.mockLoader handleRequest:[OCMArg any]
                                completion:([OCMArg invokeBlockWithArgs:self.mockResponse, nil])]);

  // Define a ResourceRequest for loading raw data that expects a file path as response
  auto uri = std::string{"https://example.com/data.json"};
  wrapper->Load(
      ResourceRequest{.type = ResourceRequestType::kLoadRawData,
                      .uri_info =
                          UriInfo{
                              .scheme = ParseUriScheme(uri),
                              .content_type = UriInfo::ContentType::kJson,
                              .uri = uri,
                          }},
      [self, expectation, filePath](TransformRequestIOS result, LoaderError error) {
        [expectation fulfill];
        XCTAssertFalse(!!error);  // Verify no errors occurred
        XCTAssertEqual(
            result.input_type,
            TransformRequestInputType::kFilePathNSString);  // Check that the input type is correct
        XCTAssertEqual(result.output_type, ResourcePayloadType::kRawData);  // Verify output type
        XCTAssertEqualObjects(result.input,
                              filePath);  // Check if the input matches the expected file path
      });

  // Expect the loader to handle the request correctly and invoke the completion block
  OCMExpect([self.mockLoader handleRequest:[OCMArg any]
                                completion:([OCMArg invokeBlockWithArgs:self.mockResponse, nil])]);

  // Wait for expectations
  [self waitForExpectationsWithTimeout:5 handler:nil];
}

- (void)testLoadRawData_nilResponse {
  using namespace lynx::animax;
  auto wrapper = LoaderWrapperIOS::Make<LoaderWrapperIOS>(self.mockLoader);
  OCMExpect([self.mockLoader handleRequest:[OCMArg any]
                                completion:([OCMArg invokeBlockWithArgs:[NSNull null], nil])]);
  auto uri = std::string{"https://example.com/a.json"};
  wrapper->Load(ResourceRequest{.type = ResourceRequestType::kLoadRawData,
                                .uri_info =
                                    UriInfo{
                                        .scheme = ParseUriScheme(uri),
                                        .content_type = ParseUriMainResourceContentType(uri),
                                        .uri = uri,
                                    }},
                [self](TransformRequestIOS result, LoaderError error) { XCTAssertTrue(!!error); });
}

- (void)testLoadRawData_errorResponse {
  using namespace lynx::animax;
  auto wrapper = LoaderWrapperIOS::Make<LoaderWrapperIOS>(self.mockLoader);
  OCMStub([self.mockResponse payload]).andReturn(nil);
  OCMStub([self.mockResponse error])
      .andReturn([NSError errorWithDomain:@"com.lynx.animax.test" code:1 userInfo:@{}]);
  OCMExpect([self.mockLoader handleRequest:[OCMArg any]
                                completion:([OCMArg invokeBlockWithArgs:self.mockResponse, nil])]);
  auto uri = std::string{"https://example.com/a.json"};
  wrapper->Load(ResourceRequest{.type = ResourceRequestType::kLoadRawData,
                                .uri_info =
                                    UriInfo{
                                        .scheme = ParseUriScheme(uri),
                                        .content_type = ParseUriMainResourceContentType(uri),
                                        .uri = uri,
                                    }},
                [self](TransformRequestIOS result, LoaderError error) { XCTAssertTrue(!!error); });
}

- (void)testLoadBitmap {
  using namespace lynx::animax;
  XCTestExpectation *expectation = [self expectationWithDescription:@"Callback should be called."];

  // Setup the loader wrapper with the mock loader
  auto wrapper = LoaderWrapperIOS::Make<LoaderWrapperIOS>(self.mockLoader);

  // Convert base64 string to NSData for bitmap data
  NSString *base64String = @"iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAAAXNSR0IArs4c6QAAAA1JRE"
                           @"FUGFdj+M/A8B8ABQAB/6Zcm10AAAAASUVORK5CYII=";
  NSData *bitmapData = [[NSData alloc] initWithBase64EncodedString:base64String options:0];

  // Mock response setup for bitmap data
  OCMStub([self.mockResponse payload]).andReturn(bitmapData);
  OCMStub([self.mockResponse payloadType]).andReturn(AnimaXLoaderPayloadTypeNSData);
  OCMStub([self.mockResponse error]).andReturn(nil);

  // Expect the loader to handle the request and invoke the completion block
  NSDictionary *expectedProperties =
      @{@"image_width" : @(1), @"image_height" : @(1)};  // Assuming dimensions match the 1x1 image
  OCMExpect([self.mockLoader
      handleRequest:[OCMArg checkWithBlock:^BOOL(AnimaXLoaderRequest *request) {
        return [request.url isEqualToString:@"https://example.com/image.png"] &&
               [request.requestProperties isEqualToDictionary:expectedProperties];
      }]
         completion:([OCMArg invokeBlockWithArgs:self.mockResponse, nil])]);

  // Define a ResourceRequest for loading a bitmap
  auto uri = std::string{"https://example.com/image.png"};
  wrapper->Load(ResourceRequest{.type = ResourceRequestType::kLoadBitmap,
                                .width = 1,
                                .height = 1,
                                .uri_info =
                                    UriInfo{
                                        .scheme = ParseUriScheme(uri),
                                        .content_type = lynx::animax::UriInfo::ContentType::kImage,
                                        .uri = uri,
                                    }},
                [self, expectation, bitmapData](TransformRequestIOS result, LoaderError error) {
                  [expectation fulfill];
                  XCTAssertEqual(result.input_type, TransformRequestInputType::kNSData);
                  XCTAssertEqual(result.output_type, ResourcePayloadType::kBitmap);
                  XCTAssertFalse(!!error);
                  XCTAssertEqualObjects(result.input, bitmapData);
                });

  // Wait for expectations
  [self waitForExpectationsWithTimeout:5 handler:nil];
}

- (void)testDownloadToLocal {
  using namespace lynx::animax;
  XCTestExpectation *expectation = [self expectationWithDescription:@"Callback should be called."];

  // Setup the loader wrapper with the mock loader
  auto wrapper = LoaderWrapperIOS::Make<LoaderWrapperIOS>(self.mockLoader);

  // Mock the response setup with a file path to simulate a downloaded file
  NSString *downloadedFilePath = @"/temporary/downloadedfile.txt";
  OCMStub([self.mockResponse payload]).andReturn(downloadedFilePath);
  OCMStub([self.mockResponse payloadType]).andReturn(AnimaXLoaderPayloadTypeFilePathNSString);
  OCMStub([self.mockResponse error]).andReturn(nil);

  // Define a ResourceRequest for downloading to local storage
  auto uri = std::string{"https://example.com/file.json"};
  ResourceRequest request = {.type = ResourceRequestType::kDownloadToLocal,
                             .uri_info = UriInfo{
                                 .scheme = ParseUriScheme(uri),
                                 .content_type = UriInfo::ContentType::kJson,
                                 .uri = uri,
                             }};

  // Expect the loader to handle the request and invoke the completion block with the file path
  OCMExpect([self.mockLoader
      handleRequest:[OCMArg checkWithBlock:^BOOL(AnimaXLoaderRequest *animaxRequest) {
        return [animaxRequest.url isEqualToString:@"https://example.com/file.json"];
      }]
         completion:([OCMArg invokeBlockWithArgs:self.mockResponse, nil])]);

  // Perform the test
  wrapper->Load(request, [self, expectation, downloadedFilePath](TransformRequestIOS result,
                                                                 LoaderError error) {
    [expectation fulfill];
    XCTAssertFalse(!!error);  // Verify no errors occurred
    XCTAssertEqual(
        result.input_type,
        TransformRequestInputType::kFilePathNSString);  // Check that the input type is correct
    XCTAssertEqual(result.output_type, ResourcePayloadType::kFilePath);  // Verify output type
    XCTAssertEqualObjects(
        result.input,
        downloadedFilePath);  // Check if the input matches the expected downloaded file path
  });

  // Wait for expectations
  [self waitForExpectationsWithTimeout:5 handler:nil];
}

- (void)testInvalidResourceRequest {
  using namespace lynx::animax;
  XCTestExpectation *expectation = [self expectationWithDescription:@"Callback should be called."];

  // Setup the loader wrapper with the mock loader
  auto wrapper = LoaderWrapperIOS::Make<LoaderWrapperIOS>(self.mockLoader);

  // Define a ResourceRequest with an invalid type
  auto uri = std::string{"https://example.com/invalid"};
  ResourceRequest request = {.type = ResourceRequestType::kInvalid,  // Setting the type to invalid
                             .uri_info = UriInfo{
                                 .scheme = ParseUriScheme(uri),
                                 .content_type = UriInfo::ContentType::kUnknown,
                                 .uri = uri,
                             }};

  // No need to set up mock response because the handler should ideally not be invoked with an
  // invalid request type
  OCMReject([self.mockLoader handleRequest:[OCMArg any] completion:[OCMArg any]]);

  // Perform the test
  wrapper->Load(request, [self, expectation](TransformRequestIOS result, LoaderError error) {
    [expectation fulfill];
    XCTAssertTrue(!!error);  // Expect an error because the request type is invalid
  });

  // Wait for expectations
  [self waitForExpectationsWithTimeout:5 handler:nil];
}

@end
