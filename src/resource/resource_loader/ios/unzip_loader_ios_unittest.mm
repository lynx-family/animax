// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/resource_loader/ios/unzip_loader_ios.h"
#import <SSZipArchive/SSZipArchive.h>
#import <XCTest/XCTest.h>

using namespace lynx::animax;

@interface UnzipLoaderIOSTests : XCTestCase
@property(nonatomic) std::shared_ptr<Loader<UnzipRequest, UnzipResponse>> unzipLoader;
@end

@implementation UnzipLoaderIOSTests

- (void)setUp {
  [super setUp];
  self.unzipLoader = UnzipLoaderIOS::Make<UnzipLoaderIOS>();
}

- (void)tearDown {
  self.unzipLoader.reset();
  [super tearDown];
}

- (void)testValidUnzip {
  // Setup
  NSString *zipFilePath = [NSTemporaryDirectory() stringByAppendingPathComponent:@"valid.zip"];
  NSString *unzipFolderPath = [NSTemporaryDirectory() stringByAppendingPathComponent:@"unzip"];

  // Create a dummy zip file in the temporary directory
  NSString *contentPath = [NSTemporaryDirectory() stringByAppendingPathComponent:@"dummy.txt"];
  [@"Dummy content" writeToFile:contentPath atomically:YES encoding:NSUTF8StringEncoding error:nil];
  [SSZipArchive createZipFileAtPath:zipFilePath withFilesAtPaths:@[ contentPath ]];

  // Prepare the test expectation for asynchronous callback
  XCTestExpectation *expectation =
      [self expectationWithDescription:@"Unzip operation should succeed."];

  // Perform the test
  UnzipRequest request;
  request.zip_file_path = [zipFilePath UTF8String];
  request.unzip_folder_path = [unzipFolderPath UTF8String];

  self.unzipLoader->Load(
      request, [expectation, unzipFolderPath](UnzipResponse response, LoaderError error) {
        // Asserts
        XCTAssertFalse(!!error);  // Ensure there is no error
        XCTAssertEqualObjects([NSString stringWithUTF8String:response.path.c_str()],
                              unzipFolderPath);  // Check the response path
        [expectation fulfill];
      });

  // Wait for expectations
  [self waitForExpectationsWithTimeout:5 handler:nil];

  // Clean up: remove files created for the test
  [[NSFileManager defaultManager] removeItemAtPath:zipFilePath error:nil];
  [[NSFileManager defaultManager] removeItemAtPath:unzipFolderPath error:nil];
  [[NSFileManager defaultManager] removeItemAtPath:contentPath error:nil];
}

- (void)testUnzipRequestZipFilePathEmpty {
  // Prepare the test expectation for asynchronous callback
  XCTestExpectation *expectation =
      [self expectationWithDescription:@"Unzip operation should fail due to empty zip file path."];

  // Perform the test
  UnzipRequest request;
  request.zip_file_path = "";  // Provide an empty zip file path
  request.unzip_folder_path =
      [NSTemporaryDirectory() stringByAppendingPathComponent:@"unzip"].UTF8String;

  self.unzipLoader->Load(request, [expectation](UnzipResponse response, LoaderError error) {
    // Asserts
    XCTAssertTrue(
        !!error);  // Ensure there is an error, indicating failure due to empty zip file path
    [expectation fulfill];
  });

  // Wait for expectations
  [self waitForExpectationsWithTimeout:5 handler:nil];
}

- (void)testUnzipRequestFileURI {
  // Setup
  NSString *zipFilePath = [NSTemporaryDirectory() stringByAppendingPathComponent:@"valid.zip"];
  NSString *contentPath = [NSTemporaryDirectory() stringByAppendingPathComponent:@"dummy.txt"];
  [@"Dummy content" writeToFile:contentPath atomically:YES encoding:NSUTF8StringEncoding error:nil];
  [SSZipArchive createZipFileAtPath:zipFilePath withFilesAtPaths:@[ contentPath ]];

  // Convert local file path to a file URI
  NSURL *fileURL = [NSURL fileURLWithPath:zipFilePath];
  NSString *fileURI = [fileURL absoluteString];
  NSString *unzipFolderPath = [NSTemporaryDirectory() stringByAppendingPathComponent:@"unzip"];

  // Prepare the test expectation for asynchronous callback
  XCTestExpectation *expectation =
      [self expectationWithDescription:@"Unzip operation should succeed with file URI scheme."];

  // Perform the test
  UnzipRequest request;
  request.zip_file_path = [fileURI UTF8String];
  request.unzip_folder_path = [unzipFolderPath UTF8String];

  self.unzipLoader->Load(
      request, [expectation, unzipFolderPath](UnzipResponse response, LoaderError error) {
        // Asserts
        XCTAssertFalse(!!error);  // Ensure there is no error, indicating successful unzip
        // Verify that the directory was indeed created
        BOOL isDirectory;
        XCTAssertTrue([[NSFileManager defaultManager] fileExistsAtPath:unzipFolderPath
                                                           isDirectory:&isDirectory] &&
                      isDirectory);
        [expectation fulfill];
      });

  // Wait for expectations
  [self waitForExpectationsWithTimeout:5 handler:nil];

  // Clean up: remove files and directories created for the test
  [[NSFileManager defaultManager] removeItemAtPath:zipFilePath error:nil];
  [[NSFileManager defaultManager] removeItemAtPath:unzipFolderPath error:nil];
  [[NSFileManager defaultManager] removeItemAtPath:contentPath error:nil];
}

- (void)testZipFileDoesNotExist {
  // Setup
  NSString *nonExistentZipFilePath =
      [NSTemporaryDirectory() stringByAppendingPathComponent:@"nonexistent.zip"];
  NSString *unzipFolderPath = [NSTemporaryDirectory() stringByAppendingPathComponent:@"unzip"];

  // Ensure the zip file does not exist
  [[NSFileManager defaultManager] removeItemAtPath:nonExistentZipFilePath error:nil];

  // Prepare the test expectation for asynchronous callback
  XCTestExpectation *expectation = [self
      expectationWithDescription:@"Unzip operation should fail due to non-existent zip file."];

  // Perform the test
  UnzipRequest request;
  request.zip_file_path = [nonExistentZipFilePath UTF8String];
  request.unzip_folder_path = [unzipFolderPath UTF8String];

  self.unzipLoader->Load(request, [expectation](UnzipResponse response, LoaderError error) {
    // Asserts
    XCTAssertTrue(!!error);  // Ensure there is an error
    [expectation fulfill];
  });

  // Wait for expectations
  [self waitForExpectationsWithTimeout:5 handler:nil];
}

- (void)testZipFilePathIsADirectory {
  // Setup
  NSString *zipDirectoryPath =
      [NSTemporaryDirectory() stringByAppendingPathComponent:@"zipAsDirectory"];
  NSString *unzipFolderPath = [NSTemporaryDirectory() stringByAppendingPathComponent:@"unzip"];

  // Ensure the directory exists at the zip file path
  [[NSFileManager defaultManager] createDirectoryAtPath:zipDirectoryPath
                            withIntermediateDirectories:YES
                                             attributes:nil
                                                  error:nil];

  // Prepare the test expectation for asynchronous callback
  XCTestExpectation *expectation =
      [self expectationWithDescription:
                @"Unzip operation should fail because the zip file path is a directory."];

  // Perform the test
  UnzipRequest request;
  request.zip_file_path = [zipDirectoryPath UTF8String];
  request.unzip_folder_path = [unzipFolderPath UTF8String];

  self.unzipLoader->Load(request, [expectation](UnzipResponse response, LoaderError error) {
    // Asserts
    XCTAssertTrue(!!error);  // Ensure there is an error, indicating the path was a directory
    [expectation fulfill];
  });

  // Wait for expectations
  [self waitForExpectationsWithTimeout:5 handler:nil];

  // Clean up
  [[NSFileManager defaultManager] removeItemAtPath:zipDirectoryPath error:nil];
}

- (void)testUnzipToNonexistentDirectory {
  // Setup
  NSString *zipFilePath = [NSTemporaryDirectory() stringByAppendingPathComponent:@"valid.zip"];
  NSString *nonExistentUnzipPath =
      [NSTemporaryDirectory() stringByAppendingPathComponent:@"nonExistentUnzip"];

  // Ensure the directory does not exist by removing it if it does
  [[NSFileManager defaultManager] removeItemAtPath:nonExistentUnzipPath error:nil];

  // Create a dummy zip file in the temporary directory
  NSString *contentPath = [NSTemporaryDirectory() stringByAppendingPathComponent:@"dummy.txt"];
  [@"Dummy content" writeToFile:contentPath atomically:YES encoding:NSUTF8StringEncoding error:nil];
  [SSZipArchive createZipFileAtPath:zipFilePath withFilesAtPaths:@[ contentPath ]];

  // Prepare the test expectation for asynchronous callback
  XCTestExpectation *expectation =
      [self expectationWithDescription:
                @"Unzip operation should succeed even to a non-existent directory."];

  // Perform the test
  UnzipRequest request;
  request.zip_file_path = [zipFilePath UTF8String];
  request.unzip_folder_path = [nonExistentUnzipPath UTF8String];

  self.unzipLoader->Load(
      request, [expectation, nonExistentUnzipPath](UnzipResponse response, LoaderError error) {
        // Asserts
        XCTAssertFalse(!!error);  // Ensure there is no error indicating successful unzip
        XCTAssertEqualObjects([NSString stringWithUTF8String:response.path.c_str()],
                              nonExistentUnzipPath);  // Ensure the response path is as expected
        // Verify that the directory now exists
        BOOL isDirectory;
        XCTAssertTrue([[NSFileManager defaultManager] fileExistsAtPath:nonExistentUnzipPath
                                                           isDirectory:&isDirectory] &&
                      isDirectory);
        [expectation fulfill];
      });

  // Wait for expectations
  [self waitForExpectationsWithTimeout:5 handler:nil];

  // Clean up: remove files and directories created for the test
  [[NSFileManager defaultManager] removeItemAtPath:zipFilePath error:nil];
  [[NSFileManager defaultManager] removeItemAtPath:nonExistentUnzipPath error:nil];
  [[NSFileManager defaultManager] removeItemAtPath:contentPath error:nil];
}

- (void)testUnzipToEmptyUnzipFolderPath {
  // Setup
  NSString *zipFilePath = [NSTemporaryDirectory() stringByAppendingPathComponent:@"valid.zip"];
  NSString *contentPath = [NSTemporaryDirectory() stringByAppendingPathComponent:@"dummy.txt"];
  [@"Dummy content" writeToFile:contentPath atomically:YES encoding:NSUTF8StringEncoding error:nil];
  [SSZipArchive createZipFileAtPath:zipFilePath withFilesAtPaths:@[ contentPath ]];

  // Ensure there's no specific path provided (i.e., an empty path)
  NSString *emptyUnzipPath = @"";

  // Prepare the test expectation for asynchronous callback
  XCTestExpectation *expectation =
      [self expectationWithDescription:
                @"Unzip operation should create a directory when no path is provided."];

  // Perform the test
  UnzipRequest request;
  request.zip_file_path = [zipFilePath UTF8String];
  request.unzip_folder_path = [emptyUnzipPath UTF8String];

  self.unzipLoader->Load(request, [expectation](UnzipResponse response, LoaderError error) {
    // Asserts
    XCTAssertFalse(!!error);  // Ensure there is no error, indicating unzip succeeded
    // Verify that a directory was indeed created
    BOOL isDirectory;
    NSString *generatedFolderPath = [NSString stringWithUTF8String:response.path.c_str()];
    XCTAssertTrue([[NSFileManager defaultManager] fileExistsAtPath:generatedFolderPath
                                                       isDirectory:&isDirectory] &&
                  isDirectory);
    [expectation fulfill];
  });

  // Wait for expectations
  [self waitForExpectationsWithTimeout:5 handler:nil];

  // Clean up: remove files and directories created for the test
  [[NSFileManager defaultManager] removeItemAtPath:zipFilePath error:nil];
  [[NSFileManager defaultManager] removeItemAtPath:contentPath error:nil];
}

- (void)testUnzipZipFileCorrupted {
  // Setup
  NSString *zipFilePath = [NSTemporaryDirectory() stringByAppendingPathComponent:@"corrupted.zip"];
  NSString *contentPath = [NSTemporaryDirectory() stringByAppendingPathComponent:@"dummy.txt"];
  [@"Dummy content" writeToFile:contentPath atomically:YES encoding:NSUTF8StringEncoding error:nil];
  [SSZipArchive createZipFileAtPath:zipFilePath withFilesAtPaths:@[ contentPath ]];

  // Corrupt the zip file by truncating it
  NSFileHandle *fileHandle = [NSFileHandle fileHandleForUpdatingAtPath:zipFilePath];
  [fileHandle truncateFileAtOffset:10];  // Truncate the file to make it likely corrupted
  [fileHandle closeFile];

  NSString *unzipFolderPath = [NSTemporaryDirectory() stringByAppendingPathComponent:@"unzip"];

  // Prepare the test expectation for asynchronous callback
  XCTestExpectation *expectation =
      [self expectationWithDescription:@"Unzip operation should fail due to corrupted zip file."];

  // Perform the test
  UnzipRequest request;
  request.zip_file_path = [zipFilePath UTF8String];
  request.unzip_folder_path = [unzipFolderPath UTF8String];

  self.unzipLoader->Load(request, [expectation](UnzipResponse response, LoaderError error) {
    // Asserts
    XCTAssertTrue(!!error);  // Ensure there is an error, indicating failure due to corruption
    [expectation fulfill];
  });

  // Wait for expectations
  [self waitForExpectationsWithTimeout:5 handler:nil];

  // Clean up: remove files and directories created for the test
  [[NSFileManager defaultManager] removeItemAtPath:zipFilePath error:nil];
  [[NSFileManager defaultManager] removeItemAtPath:contentPath error:nil];
}

@end
