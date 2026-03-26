// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/resource_loader/ios/base_transform_loaders_ios.h"
#import <XCTest/XCTest.h>

using namespace lynx::animax;

@interface base_transform_loaders_ios_unittest : XCTestCase
@property(nonatomic) NSData *base64ImageData;
@property(nonatomic) NSData *txtData;
@property(nonatomic) NSString *tmpFilePath;
@end

@implementation base_transform_loaders_ios_unittest

- (void)setUp {
  std::string validDataURL =
      "data:image/"
      "png;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAAAXNSR0IArs4c6QAAAA1JREFUGFdj+M/"
      "A8B8ABQAB/6Zcm10AAAAASUVORK5CYII=";
  self.base64ImageData = [NSData
      dataWithContentsOfURL:[NSURL
                                URLWithString:[NSString stringWithUTF8String:validDataURL.data()]]];
  self.txtData = [@"good morning" dataUsingEncoding:NSUTF8StringEncoding];
}

- (void)tearDown {
  if ([self.tmpFilePath length] > 0) {
    [[NSFileManager defaultManager] removeItemAtPath:self.tmpFilePath error:nil];
  }
}

- (void)testNSDataBitmapTransformLoader_Succeed {
  auto ns_data_bitmap = NSDataBitmapTransformLoader::Make<NSDataBitmapTransformLoader>();
  XCTestExpectation *expectation = [self expectationWithDescription:@"Callback should be called."];
  ns_data_bitmap->Load(TransformRequestIOS{.input = self.base64ImageData,
                                           .input_type = TransformRequestInputType::kNSData,
                                           .output_type = ResourcePayloadType::kBitmap},
                       [expectation](ResourceResponse result, auto error) {
                         [expectation fulfill];
                         if (error) {
                           XCTFail("Image should convert successfully.");
                           return;
                         }
                         XCTAssertTrue(result.payload.bitmap != nullptr);
                       });
  [self waitForExpectationsWithTimeout:5 handler:nil];
}

- (void)testNSDataBitmapTransformLoader_Fail {
  auto ns_data_bitmap = NSDataBitmapTransformLoader::Make<NSDataBitmapTransformLoader>();
  XCTestExpectation *expectation = [self expectationWithDescription:@"Callback should be called."];
  ns_data_bitmap->Load(TransformRequestIOS{.input = self.txtData,
                                           .input_type = TransformRequestInputType::kNSData,
                                           .output_type = ResourcePayloadType::kBitmap},
                       [expectation](ResourceResponse result, auto error) {
                         [expectation fulfill];
                         if (!error) {
                           XCTFail("Should be an error.");
                           return;
                         }
                         XCTAssertTrue(result.payload.bitmap == nullptr);
                       });
  [self waitForExpectationsWithTimeout:5 handler:nil];
}

- (void)testNSDataRawDataTransformLoader_Succeed {
  auto ns_data_raw_data = NSDataBitmapTransformLoader::Make<NSDataRawDataTransformLoader>();
  XCTestExpectation *expectation = [self expectationWithDescription:@"Callback should be called."];
  ns_data_raw_data->Load(TransformRequestIOS{.input = self.txtData,
                                             .input_type = TransformRequestInputType::kNSData,
                                             .output_type = ResourcePayloadType::kRawData},
                         [expectation](ResourceResponse result, auto error) {
                           [expectation fulfill];
                           if (error) {
                             XCTFail("RawData should convert successfully.");
                             return;
                           }
                           XCTAssertTrue(result.payload.raw_data != nullptr);
                         });
  [self waitForExpectationsWithTimeout:5 handler:nil];
}

- (void)testNSDataRawDataTransformLoader_Fail {
  auto ns_data_raw_data = NSDataBitmapTransformLoader::Make<NSDataRawDataTransformLoader>();
  XCTestExpectation *expectation = [self expectationWithDescription:@"Callback should be called."];
  ns_data_raw_data->Load(TransformRequestIOS{.input = nil,
                                             .input_type = TransformRequestInputType::kNSData,
                                             .output_type = ResourcePayloadType::kRawData},
                         [expectation](ResourceResponse result, auto error) {
                           [expectation fulfill];
                           if (!error) {
                             XCTFail("Should be an error.");
                             return;
                           }
                           XCTAssertTrue(result.payload.raw_data == nullptr);
                         });
  [self waitForExpectationsWithTimeout:5 handler:nil];
}

- (void)testFilePathNSStringToFilePathStringLoader_Succeed {
  auto file_path_loader =
      FilePathNSStringToFilePathStringLoader::Make<FilePathNSStringToFilePathStringLoader>();
  XCTestExpectation *expectation = [self expectationWithDescription:@"Callback should be called."];
  NSString *validFilePath = @"/valid/path/to/file.txt";
  file_path_loader->Load(
      TransformRequestIOS{.input = validFilePath,
                          .input_type = TransformRequestInputType::kFilePathNSString,
                          .output_type = ResourcePayloadType::kFilePath},
      [expectation, validFilePath](TransformResponseIOS result, LoaderError error) {
        [expectation fulfill];
        if (error) {
          XCTFail("File path should be valid.");
          return;
        }
        XCTAssertTrue(result.payload.path == std::string{[validFilePath UTF8String]});
      });
  [self waitForExpectationsWithTimeout:5 handler:nil];
}

- (void)testFilePathNSStringToFilePathStringLoader_Fail {
  auto file_path_loader =
      FilePathNSStringToFilePathStringLoader::Make<FilePathNSStringToFilePathStringLoader>();
  XCTestExpectation *expectation = [self expectationWithDescription:@"Callback should be called."];
  file_path_loader->Load(
      TransformRequestIOS{.input = nil,  // Invalid input as nil
                          .input_type = TransformRequestInputType::kFilePathNSString,
                          .output_type = ResourcePayloadType::kFilePath},
      [expectation](TransformResponseIOS result, LoaderError error) {
        [expectation fulfill];
        if (!error) {
          XCTFail("Should return an error for nil input.");
          return;
        }
        XCTAssertEqualObjects([NSString stringWithUTF8String:result.payload.path.c_str()], @"");
      });
  [self waitForExpectationsWithTimeout:5 handler:nil];
}

- (void)testFilePathNSStringToNSDataLoader_Success {
  auto file_path_loader = FilePathNSStringToNSDataLoader::Make<FilePathNSStringToNSDataLoader>();
  XCTestExpectation *expectation = [self expectationWithDescription:@"Callback should be called."];
  NSString *tempDirectoryPath = NSTemporaryDirectory();

  // Create a unique file name
  NSString *fileName = [NSString stringWithFormat:@"%@.data", [[NSUUID UUID] UUIDString]];
  self.tmpFilePath = [tempDirectoryPath stringByAppendingPathComponent:fileName];

  // Write NSData to the file
  NSError *error = nil;
  BOOL success = [self.txtData writeToFile:self.tmpFilePath
                                   options:NSDataWritingAtomic
                                     error:&error];
  XCTAssertTrue(success);
  file_path_loader->Load(
      TransformRequestIOS{.input = self.tmpFilePath,
                          .input_type = TransformRequestInputType::kFilePathNSString,
                          .output_type = ResourcePayloadType::kFilePath},
      [expectation, self](TransformRequestIOS result, LoaderError error) {
        [expectation fulfill];
        XCTAssertFalse(!!error);
        XCTAssertEqualObjects(result.input, self.txtData, "NSData content should be the same");
      });
  [self waitForExpectationsWithTimeout:5 handler:nil];
}

- (void)testFilePathNSStringToNSDataLoader_Fail_NilInput {
  auto file_path_loader = FilePathNSStringToNSDataLoader::Make<FilePathNSStringToNSDataLoader>();
  XCTestExpectation *expectation = [self expectationWithDescription:@"Callback should be called."];
  file_path_loader->Load(
      TransformRequestIOS{.input = nil,  // Nil input to trigger the error
                          .input_type = TransformRequestInputType::kFilePathNSString,
                          .output_type = ResourcePayloadType::kFilePath},
      [expectation](TransformRequestIOS result, LoaderError error) {
        [expectation fulfill];
        XCTAssertEqual(error.code, LoaderErrorCode::kInvalidArgument);
      });
  [self waitForExpectationsWithTimeout:5 handler:nil];
}

- (void)testFilePathNSStringToNSDataLoader_Fail_FileNotExist {
  auto file_path_loader = FilePathNSStringToNSDataLoader::Make<FilePathNSStringToNSDataLoader>();
  XCTestExpectation *expectation = [self expectationWithDescription:@"Callback should be called."];
  NSString *invalidFilePath = @"/invalid/path/to/file.txt";

  file_path_loader->Load(
      TransformRequestIOS{.input = invalidFilePath,
                          .input_type = TransformRequestInputType::kFilePathNSString,
                          .output_type = ResourcePayloadType::kFilePath},
      [expectation](TransformRequestIOS result, LoaderError error) {
        [expectation fulfill];
        XCTAssertEqual(error.code, LoaderErrorCode::kFileDoesNotExist);
      });
  [self waitForExpectationsWithTimeout:5 handler:nil];
}

- (void)testNSDataToFilePathStringLoader_Success_UseDefaultFilePath {
  auto loader = NSDataToFilePathStringLoader::Make<NSDataToFilePathStringLoader>();
  NSString *tempDirectoryPath = NSTemporaryDirectory();
  NSString *fileName = [NSString stringWithFormat:@"%@.data", [[NSUUID UUID] UUIDString]];
  self.tmpFilePath = [tempDirectoryPath stringByAppendingPathComponent:fileName];
  XCTestExpectation *expectation = [self expectationWithDescription:@"Callback should be called."];
  loader->Load(
      TransformRequestIOS{
          .input = self.txtData,
          .input_type = TransformRequestInputType::kNSData,
          .output_type = ResourcePayloadType::kFilePath,
          .default_file_path_for_ns_data_file_path_conversion = [self.tmpFilePath UTF8String],
      },
      [expectation, self](TransformResponseIOS result, LoaderError error) {
        [expectation fulfill];
        XCTAssertFalse(!!error);
        XCTAssertTrue([[NSFileManager defaultManager] fileExistsAtPath:self.tmpFilePath],
                      "File should be created");
      });
  [self waitForExpectationsWithTimeout:5 handler:nil];
}

- (void)testNSDataToFilePathStringLoader_Success_UseHashedFilePath {
  auto loader = NSDataToFilePathStringLoader::Make<NSDataToFilePathStringLoader>();
  XCTestExpectation *expectation = [self expectationWithDescription:@"Callback should be called."];
  loader->Load(
      TransformRequestIOS{
          .input = self.txtData,
          .input_type = TransformRequestInputType::kNSData,
          .output_type = ResourcePayloadType::kFilePath,
          // leave this as empty to make it use hashed file path
          .default_file_path_for_ns_data_file_path_conversion = std::string{},
      },
      [expectation, self](TransformResponseIOS result, LoaderError error) {
        [expectation fulfill];
        XCTAssertFalse(!!error);
        self.tmpFilePath = [NSString stringWithUTF8String:result.payload.path.data()];
        XCTAssertTrue([[NSFileManager defaultManager] fileExistsAtPath:self.tmpFilePath],
                      "File should be created");
      });
  [self waitForExpectationsWithTimeout:5 handler:nil];
}

@end
