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

- (void)testDownloadToLocal_OnlySnapshotsZipFilePath {
  using namespace lynx::animax;
  OCMStub([self.mockResponse payload]).andReturn(@"/host/resource");
  OCMStub([self.mockResponse payloadType]).andReturn(AnimaXLoaderPayloadTypeFilePathNSString);
  OCMStub([self.mockResponse error]).andReturn(nil);
  OCMStub([self.mockLoader handleRequest:[OCMArg any]
                              completion:([OCMArg invokeBlockWithArgs:self.mockResponse, nil])]);
  auto wrapper = LoaderWrapperIOS::Make<LoaderWrapperIOS>(self.mockLoader);
  auto cache_by_content = [&wrapper](std::string uri, UriInfo::ContentType content_type,
                                     bool requested) {
    auto result = std::make_shared<bool>(!requested);
    wrapper->Load(ResourceRequest{.type = ResourceRequestType::kDownloadToLocal,
                                  .uri_info = UriInfo{.scheme = ParseUriScheme(uri),
                                                      .content_type = content_type,
                                                      .uri = uri},
                                  .cache_by_content = requested},
                  [result](TransformRequestIOS request, LoaderError error) {
                    *result = request.cache_by_content;
                  });
    return *result;
  };

  XCTAssertTrue(cache_by_content("https://example.com/a.zip", UriInfo::ContentType::kZip, true));
  XCTAssertFalse(cache_by_content("https://example.com/a.mp4", UriInfo::ContentType::kVideo, true));
  XCTAssertFalse(cache_by_content("https://example.com/a.zip", UriInfo::ContentType::kZip, false));
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

using namespace lynx::animax;

@interface ResourceLoadRecorder : NSObject <AnimaXLoaderProtocol, AnimaXMonitorService>
@property(nonatomic, strong) NSMutableArray<AnimaXLoaderRequest *> *requests;
@property(nonatomic, strong)
    NSMutableDictionary<NSNumber *, AnimaXLoaderCompletionHandler> *completions;
@property(nonatomic, strong) NSMutableArray<NSDictionary *> *events;
- (void)finish:(NSUInteger)index response:(AnimaXLoaderResponse *)response;
@end

@implementation ResourceLoadRecorder
@dynamic urlHolder;
- (instancetype)init {
  if ((self = [super init])) {
    _requests = [NSMutableArray new];
    _completions = [NSMutableDictionary new];
    _events = [NSMutableArray new];
  }
  return self;
}
- (AnimaXLoaderScheme)getScheme {
  return AnimaXLoaderSchemeHttp;
}
- (void)handleRequest:(AnimaXLoaderRequest *)request completion:(AnimaXLoaderCompletionHandler)cb {
  self.completions[@(self.requests.count)] = [cb copy];
  [self.requests addObject:request];
}
- (void)finish:(NSUInteger)index response:(AnimaXLoaderResponse *)response {
  AnimaXLoaderCompletionHandler completion = self.completions[@(index)];
  [self.completions removeObjectForKey:@(index)];
  completion(response);
}
- (void)reportResourceLoad:(NSDictionary *)params {
  [self.events addObject:params];
}
- (void)reportError:(NSDictionary *)params {
}
- (void)reportPerformance:(NSDictionary *)params {
}
@end

namespace {
ResourceRequest Request(UriInfo::ContentType type) {
  return {.type = ResourceRequestType::kLoadRawData,
          .uri_info = {.scheme = UriInfo::Scheme::kHttp,
                       .content_type = type,
                       .uri = "https://example.org/resource?signature=test"}};
}
}  // namespace

@interface AnimaXResourceLoadReportingTest : XCTestCase
@end

@implementation AnimaXResourceLoadReportingTest
- (void)testOverlappingRequestsKeepIndependentSourcesAndRunPlaybackFirst {
  ResourceLoadRecorder *host = [ResourceLoadRecorder new];
  LoaderWrapperIOS loader(host, host);
  auto request = Request(UriInfo::ContentType::kJson);
  loader.Load(request, [self, host](TransformRequestIOS, LoaderError error) {
    XCTAssertFalse(!!error);
    XCTAssertEqual(host.events.count, 1u);
  });
  loader.Load(request, [self, host](TransformRequestIOS, LoaderError error) {
    XCTAssertFalse(!!error);
    XCTAssertEqual(host.events.count, 0u);
  });
  NSDictionary *first = host.requests[0].resourceLoadInfo;
  NSDictionary *second = host.requests[1].resourceLoadInfo;
  XCTAssertNotNil(first[@"request_id"]);
  XCTAssertNotEqualObjects(first[@"request_id"], second[@"request_id"]);
  host.requests[1].resourceLoadInfo[@"cache_source"] = @"in_flight";
  AnimaXLoaderResponse *response =
      [AnimaXLoaderResponse responseWithPayload:[@"{}" dataUsingEncoding:NSUTF8StringEncoding]];
  [host finish:1 response:response];
  host.requests[0].resourceLoadInfo[@"cache_source"] = @"http_cache";
  [host finish:0 response:response];
  XCTAssertEqual(host.events.count, 2u);
  XCTAssertEqualObjects(host.events[0][@"request_id"], second[@"request_id"]);
  XCTAssertEqualObjects(host.events[0][@"phase"], @"consume");
  XCTAssertEqualObjects(host.events[0][@"resource_url_hash"],
                        @"Q7HaREAHGj520CD7v47QfuEwazrPZ/wiGUikE4yzmbQ=");
  XCTAssertEqualObjects(host.events[1][@"resource_url_hash"], host.events[0][@"resource_url_hash"]);
  XCTAssertEqualObjects(host.events[0][@"cache_source"], @"in_flight");
  XCTAssertEqualObjects(host.events[1][@"cache_source"], @"http_cache");
  XCTAssertEqualObjects(host.events[1][@"status"], @"success");
  XCTAssertEqualObjects(host.events[1][AnimaXMonitorSourceUrl], @"https://example.org/resource");
}

- (void)testResourceTypeComesFromTheModelAndMissingSourceStaysUnknown {
  ResourceLoadRecorder *host = [ResourceLoadRecorder new];
  LoaderWrapperIOS loader(host, host);
  NSArray *expected = @[ @"json", @"zip", @"font", @"image", @"video", @"audio" ];
  for (NSUInteger i = 0; i < expected.count; ++i) {
    loader.Load(Request(static_cast<UriInfo::ContentType>(i)),
                [](TransformRequestIOS, LoaderError) {});
    // An older host leaves the request untouched. An unrecognized source is also unknown.
    if (i == 1) {
      host.requests[i].resourceLoadInfo[@"cache_source"] = @"unrecognized-source";
    }
    [host finish:i
        response:[AnimaXLoaderResponse
                     responseWithPayload:[@"asset" dataUsingEncoding:NSUTF8StringEncoding]]];
    XCTAssertEqualObjects(host.events[i][@"resource_type"], expected[i]);
    XCTAssertEqualObjects(host.events[i][@"cache_source"], @"unknown");
  }
}

- (void)testFailuresAndCancellationAreNeverCacheHits {
  ResourceLoadRecorder *host = [ResourceLoadRecorder new];
  LoaderWrapperIOS loader(host, host);
  for (NSString *status in @[ @"failed", @"cancelled" ]) {
    loader.Load(Request(UriInfo::ContentType::kImage),
                [self](TransformRequestIOS, LoaderError error) { XCTAssertTrue(!!error); });
    host.requests.lastObject.resourceLoadInfo[@"cache_source"] = @"memory";
    host.requests.lastObject.resourceLoadInfo[@"cancelled"] =
        @([status isEqualToString:@"cancelled"]);
    [host finish:host.requests.count - 1
        response:[AnimaXLoaderResponse responseWithErrorMessage:@"load failed"]];
    XCTAssertEqualObjects(host.events.lastObject[@"status"], status);
    XCTAssertEqualObjects(host.events.lastObject[@"cache_source"], @"unknown");
  }
}

- (void)testUnmonitoredRequestHasNoDiagnostics {
  ResourceLoadRecorder *host = [ResourceLoadRecorder new];
  LoaderWrapperIOS loader(host);
  loader.Load(Request(UriInfo::ContentType::kFont), [](TransformRequestIOS, LoaderError) {});
  XCTAssertNil(host.requests.lastObject.resourceLoadInfo);
  [host finish:0 response:[AnimaXLoaderResponse responseWithPayload:[NSData data]]];
  XCTAssertEqual(host.events.count, 0u);
}

- (void)testEmptyPayloadIsExcludedFromCacheHits {
  ResourceLoadRecorder *host = [ResourceLoadRecorder new];
  LoaderWrapperIOS loader(host, host);
  loader.Load(Request(UriInfo::ContentType::kJson), [](TransformRequestIOS, LoaderError) {});
  host.requests.lastObject.resourceLoadInfo[@"cache_source"] = @"memory";
  [host finish:0 response:[AnimaXLoaderResponse responseWithPayload:[NSData data]]];
  XCTAssertEqualObjects(host.events.lastObject[@"status"], @"failed");
  XCTAssertEqualObjects(host.events.lastObject[@"cache_source"], @"unknown");
}
@end
