// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <OCMock/OCMock.h>
#import <XCTest/XCTest.h>

#include <cmath>
#include <memory>
#include <string>
#include "include/resource/bitmap.h"
#include "include/resource/raw_data.h"
#include "src/base/thread/thread_assert.h"
#include "src/resource/resource_loader/ios/resource_util.h"

using namespace lynx::animax;

@interface resource_util_unittest : XCTestCase

@end

@implementation resource_util_unittest

- (void)setUp {
}

- (void)tearDown {
  NSString *cacheDirPath =
      [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES).lastObject
          stringByAppendingPathComponent:@"com.bytedance.animax.caches"];
  [[NSFileManager defaultManager] removeItemAtPath:cacheDirPath error:nil];
}

- (void)testTryCreateDirectory_WhenDirectoryExists {
  // Create a temporary directory
  NSString *tempDir = NSTemporaryDirectory();
  XCTAssertTrue(TryCreateDirectory(tempDir), @"Should return true if directory already exists.");
}

- (void)testTryCreateDirectory_WhenFileExistsAtPath {
  // Create a temporary file
  NSString *tempFile = [NSTemporaryDirectory() stringByAppendingPathComponent:@"tempFile"];
  [@"" writeToFile:tempFile atomically:YES encoding:NSUTF8StringEncoding error:nil];

  XCTAssertFalse(TryCreateDirectory(tempFile), @"Should return false if a file exists at path.");

  // Clean up
  [[NSFileManager defaultManager] removeItemAtPath:tempFile error:nil];
}

- (void)testTryCreateDirectory_WithInvalidPath {
  NSString *invalidPath = @"/System/InvalidDirectory";
  XCTAssertFalse(TryCreateDirectory(invalidPath),
                 @"Should return false for an invalid directory path.");
}

- (void)testTryCreateDirectory_Success {
  // Use a unique directory name in the temporary directory
  NSString *uniqueDir =
      [NSTemporaryDirectory() stringByAppendingPathComponent:[[NSUUID UUID] UUIDString]];
  XCTAssertTrue(TryCreateDirectory(uniqueDir),
                @"Should return true when directory is successfully created.");

  // Verify the directory exists
  BOOL isDir = NO;
  BOOL exists = [[NSFileManager defaultManager] fileExistsAtPath:uniqueDir isDirectory:&isDir];
  XCTAssertTrue(exists && isDir, @"A new directory should exist at the path.");

  // Clean up
  [[NSFileManager defaultManager] removeItemAtPath:uniqueDir error:nil];
}

- (void)testEnsureCacheDirectory_WhenDirectoryExists {
  // Ensure the cache directory exists
  NSString *expectedCacheDir =
      [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES).lastObject
          stringByAppendingPathComponent:@"com.bytedance.animax.caches"];
  [NSFileManager.defaultManager createDirectoryAtPath:expectedCacheDir
                          withIntermediateDirectories:YES
                                           attributes:nil
                                                error:nil];

  // Call the function
  std::string cacheDir = EnsureCacheDirectory();

  // Verify
  XCTAssertFalse(cacheDir.empty(), @"Cache directory path should not be empty.");
  XCTAssertEqualObjects(@(cacheDir.c_str()), expectedCacheDir,
                        @"Returned path should match the expected cache directory path.");
  [NSFileManager.defaultManager removeItemAtPath:expectedCacheDir error:nil];
}

- (void)testEnsureCacheDirectory_WhenDirectoryCreationFails {
  // Create a file at the expected directory path
  NSString *cacheDirPath =
      [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES).lastObject
          stringByAppendingPathComponent:@"com.bytedance.animax.caches"];
  [@"" writeToFile:cacheDirPath atomically:YES encoding:NSUTF8StringEncoding error:nil];
  // Call the function
  std::string cacheDir = EnsureCacheDirectory();

  // Verify
  XCTAssertTrue(
      cacheDir.empty(),
      @"Cache directory path should be empty when directory creation fails due to existing file.");

  // Clean up: Remove the file
  [NSFileManager.defaultManager removeItemAtPath:cacheDirPath error:nil];
}

- (void)testEnsureCacheDirectory_WhenDirectoryNeedsCreation {
  // Remove the cache directory if it exists
  NSString *cacheDirPath =
      [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES).lastObject
          stringByAppendingPathComponent:@"com.bytedance.animax.caches"];
  [NSFileManager.defaultManager removeItemAtPath:cacheDirPath error:nil];

  // Call the function
  std::string cacheDir = EnsureCacheDirectory();
  BOOL isDir = NO;
  // Verify
  XCTAssertFalse(cacheDir.empty(), @"Cache directory path should not be empty.");
  XCTAssertTrue([NSFileManager.defaultManager fileExistsAtPath:cacheDirPath isDirectory:&isDir],
                @"Cache directory should exist after calling GetCacheDirectory.");
  XCTAssertTrue(isDir, @"Cache directory should be a directory.");

  // Clean up: Remove the created directory
  [NSFileManager.defaultManager removeItemAtPath:cacheDirPath error:nil];
}

- (void)testGetTempFilePathForUrl_WhenCacheDirUnavailable {
  // Create a file at the expected cache directory path
  NSString *cacheDirPath =
      [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES).lastObject
          stringByAppendingPathComponent:@"com.bytedance.animax.caches"];
  [@"" writeToFile:cacheDirPath atomically:YES encoding:NSUTF8StringEncoding error:nil];

  // Call the function
  std::string url = "http://example.com/resource";
  std::string filePath = GetTempFilePathForUrl(url);

  // Verify that the function returns an empty string
  XCTAssertTrue(filePath.empty(),
                @"Function should return an empty string when cache directory is unavailable.");

  // Clean up - Remove the file
  [[NSFileManager defaultManager] removeItemAtPath:cacheDirPath error:nil];
}

- (void)testGetTempFilePathForUrl_Success {
  // Ensure the cache directory exists or can be created
  NSString *cacheDirPath =
      [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES).lastObject
          stringByAppendingPathComponent:@"com.bytedance.animax.caches"];

  // Call the function
  std::string url = "http://example.com/resource";
  std::string filePath = GetTempFilePathForUrl(url);

  // Verify that the function returns a non-empty string
  XCTAssertFalse(filePath.empty(),
                 @"Function should return a non-empty string when cache directory is available.");

  // Construct the expected file path
  NSString *expectedFilePath = [cacheDirPath
      stringByAppendingPathComponent:[NSString stringWithFormat:@"tmp_%lu",
                                                                (unsigned long)
                                                                    std::hash<std::string>{}(url)]];
  XCTAssertEqualObjects(@(filePath.c_str()), expectedFilePath,
                        @"Returned path should match the expected file path.");

  // Clean up - Optionally remove the cache directory if created specifically for this test
  [[NSFileManager defaultManager] removeItemAtPath:cacheDirPath error:nil];
}

- (void)testNSDataToBitmap_WhenDataIsNull {
  std::unique_ptr<Bitmap> bitmap = NSDataToBitmap(nil);
  XCTAssert(bitmap == nullptr, @"Bitmap should be nullptr when NSData is nil.");
}

- (void)testNSDataToBitmap_WhenUIImageIsInvalid {
  NSData *invalidData = [NSData data];  // empty data
  std::unique_ptr<Bitmap> bitmap = NSDataToBitmap(invalidData);
  XCTAssert(bitmap == nullptr, @"Bitmap should be nullptr when UIImage is invalid.");
}

- (NSData *)createValidImageData10x10Red {
  CGSize imageSize = CGSizeMake(10, 10);  // Example size
  UIGraphicsBeginImageContextWithOptions(imageSize, NO, 0.0);
  [[UIColor redColor] setFill];  // Example color
  UIRectFill(CGRectMake(0, 0, imageSize.width, imageSize.height));
  UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
  UIGraphicsEndImageContext();

  return UIImagePNGRepresentation(image);
}

- (NSData *)createRGBWhite_2x2_RGBAPNG {
  // 2x2 RGBA PNG Image with four pixels
  // RGBA(255, 0, 0, 128) | RGBA(0, 255, 0, 128)
  // ---------------------|-------------------------
  // RGBA(0, 0, 255, 128) | RGBA(255, 255, 255, 128)
  return
      [NSData dataWithContentsOfURL:
                  [NSURL URLWithString:@"data:image/"
                                       @"png;base64,"
                                       @"iVBORw0KGgoAAAANSUhEUgAAAAIAAAACCAYAAABytg0kAAAAGklEQVR4nG"
                                       @"P8z8DQwPifgYGFkeE/w38GBgYALjYEhLiBifoAAAAASUVORK5CYII="]];
}

- (NSData *)createRGBWhite_2x2_IndexedPNG {
  // 2x2 Indexed PNG Image with four pixels
  // RGBA(255, 0, 0, 128) | RGBA(0, 255, 0, 128)
  // ---------------------|-------------------------
  // RGBA(0, 0, 255, 128) | RGBA(255, 255, 255, 128)
  return [NSData
      dataWithContentsOfURL:
          [NSURL
              URLWithString:
                  @"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAIAAAACCAMAAABFaP0WAAADAFBMVEX//"
                  @"/8AAP//AAAA/"
                  @"wAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                  @"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                  @"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                  @"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                  @"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                  @"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                  @"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                  @"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                  @"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                  @"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                  @"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                  @"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                  @"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABj44qgAAABAHRSTlOA"
                  @"gICAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                  @"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                  @"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                  @"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                  @"AAAAAAAAAAAAAAAAAAAAAAAAXEJUrwAAAA5JREFUeJxjYGJmYGQAAAAeAAcKdEOzAAAAAElFTkSuQmC"
                  @"C"]];
}

- (BOOL)canDecodeR50G50B50A50_1x1_IndexedPNGCorrectly {
  NSData *data = [self createR50G50B50A50_1x1_IndexedPNGImage];
  UIImage *ui_image = [UIImage imageWithData:data];
  CGImageRef cg_image_ref = ui_image.CGImage;
  auto width = static_cast<uint32_t>(CGImageGetWidth(cg_image_ref));
  auto height = static_cast<uint32_t>(CGImageGetHeight(cg_image_ref));
  CGContextRef context = NULL;
  CGBitmapInfo contextBitmapInfo = kCGBitmapByteOrderDefault | kCGImageAlphaPremultipliedLast;
  context = CGBitmapContextCreate(NULL, width, height, 8, width * 4, CGColorSpaceCreateDeviceRGB(),
                                  contextBitmapInfo);
  CGContextDrawImage(context, CGRectMake(0, 0, width, height), cg_image_ref);
  size_t bpr = CGBitmapContextGetBytesPerRow(context);
  uint8_t *bitmap = static_cast<uint8_t *>(CGBitmapContextGetData(context));
  // This RGBA(50, 50, 50, 50) image should be decoded as (10, 10, 10, 50)
  // because kCGImageAlphaPremultipliedLast.
  // However, on iOS 17.1 and iOS 17.2, it will be premultiplied twice.
  BOOL correct = (bitmap[0] == 10) && (bitmap[1] == 10) && (bitmap[2] == 10);
  CGContextRelease(context);
  return correct;
}

- (NSData *)createR50G50B50A50_1x1_IndexedPNGImage {
  // 1x1 Indexed PNG Image with one pixel RGBA(50, 50, 50, 50)
  return [NSData
      dataWithContentsOfURL:
          [NSURL URLWithString:@"data:image/"
                               @"png;base64,"
                               @"iVBORw0KGgoAAAANSUhEUgAAAAEAAAABAQMAAAAl21bKAAAAA1BMVEUyMjKlMgnVAA"
                               @"AAAXRSTlMyiDGJ5gAAAApJREFUCNdjYAAAAAIAAeIhvDMAAAAASUVORK5CYII="]];
}

- (NSData *)createR255G255B255A10_1x1_IndexedPNGImage {
  // 1x1 Indexed PNG Image with one pixel RGBA(255, 255, 255, 10)
  // This pixel has very small alpha. If this pixel got alpha premultiplied twice, it will become
  // RGBA(0, 0, 0, 10). In this case, you can't use unpremultiply to recover it to the original
  // pixel.
  return [NSData
      dataWithContentsOfURL:
          [NSURL URLWithString:
                     @"data:image/"
                     @"png;base64,"
                     @"iVBORw0KGgoAAAANSUhEUgAAAAEAAAABAQMAAAAl21bKAAAAA1BMVEX///"
                     @"+nxBvIAAAAAXRSTlMKoDMxeAAAAApJREFUCNdjYAAAAAIAAeIhvDMAAAAASUVORK5CYII="]];
}

- (void)testDeviceHasIOS17IndexedPNGDecodingBug_Correct {
  XCTAssertEqual([self canDecodeR50G50B50A50_1x1_IndexedPNGCorrectly],
                 !DeviceHasIOS17IndexedPNGDecodingBug());
}

- (void)testNSDataToBitmap_IOS17IndexedPNGBug_R255G255B255A10_1x1_IndexedPNGImage {
  std::unique_ptr<Bitmap> bitmap = NSDataToBitmap([self createR255G255B255A10_1x1_IndexedPNGImage]);
  XCTAssertEqual(bitmap->Width(), 1u, @"Width should be 1.");
  XCTAssertEqual(bitmap->Height(), 1u, @"Height should be 1.");
  // Check color of the first pixel
  const uint8_t *pixels = reinterpret_cast<const uint8_t *>(bitmap->Pixels());
  if (pixels) {
    const auto expected = static_cast<uint8_t>(std::ceil(255. * (10. / 255.)));
    XCTAssertEqual(expected, pixels[0], "R should be 10");
    XCTAssertEqual(expected, pixels[1], "G should be 10.");
    XCTAssertEqual(expected, pixels[2], "B should be 10.");
    XCTAssertEqual(expected, pixels[3], "A should be 10.");
  } else {
    XCTFail(@"Pixels should not be null.");
  }
}

- (void)testNSDataToBitmap_IOS17IndexedPNGBug_R50G50B50A50_1x1_IndexedPNGImage {
  std::unique_ptr<Bitmap> bitmap = NSDataToBitmap([self createR50G50B50A50_1x1_IndexedPNGImage]);
  XCTAssertEqual(bitmap->Width(), 1u, @"Width should be 1.");
  XCTAssertEqual(bitmap->Height(), 1u, @"Height should be 1.");
  // Check color of the first pixel
  const uint8_t *pixels = reinterpret_cast<const uint8_t *>(bitmap->Pixels());
  if (pixels) {
    const auto expected = static_cast<uint8_t>(std::ceil(50 * (50. / 255.)));
    XCTAssertEqual(expected, pixels[0], "R should be 10");
    XCTAssertEqual(expected, pixels[1], "G should be 10.");
    XCTAssertEqual(expected, pixels[2], "B should be 10.");
    XCTAssertEqual(50, pixels[3], "A should be 50.");
  } else {
    XCTFail(@"Pixels should not be null.");
  }
}

- (void)testNSDataToBitmap_IOS17IndexedPNGBug_RGBWhite_2x2_IndexedPNG {
  std::unique_ptr<Bitmap> bitmap = NSDataToBitmap([self createRGBWhite_2x2_IndexedPNG]);
  XCTAssertEqual(bitmap->Width(), 2u, @"Width should be 1.");
  XCTAssertEqual(bitmap->Height(), 2u, @"Height should be 1.");
  // Check color of the first pixel
  const uint8_t *pixels = reinterpret_cast<const uint8_t *>(bitmap->Pixels());
  if (pixels) {
    const auto expected = static_cast<uint8_t>(std::ceil(255 * (128. / 255.)));
    XCTAssertEqual(expected, pixels[0], "R should be 128");
    XCTAssertEqual(0, pixels[1], "G should be 0");
    XCTAssertEqual(0, pixels[2], "B should be 0");
    XCTAssertEqual(128, pixels[3], "A should be 128.");
    pixels = pixels + 4;
    XCTAssertEqual(0, pixels[0], "R should be 0");
    XCTAssertEqual(expected, pixels[1], "G should be 128");
    XCTAssertEqual(0, pixels[2], "B should be 0");
    XCTAssertEqual(128, pixels[3], "A should be 128.");
    pixels = pixels + 4;
    XCTAssertEqual(0, pixels[0], "R should be 0");
    XCTAssertEqual(0, pixels[1], "G should be 0");
    XCTAssertEqual(expected, pixels[2], "B should be 128");
    XCTAssertEqual(128, pixels[3], "A should be 128.");
    pixels = pixels + 4;
    XCTAssertEqual(expected, pixels[0], "R should be 128");
    XCTAssertEqual(expected, pixels[1], "G should be 128");
    XCTAssertEqual(expected, pixels[2], "B should be 128");
    XCTAssertEqual(128, pixels[3], "A should be 128.");
  } else {
    XCTFail(@"Pixels should not be null.");
  }
}

- (void)testNSDataToBitmap_IOS17IndexedPNGBug_RGBWhite_2x2_RGBAPNG {
  std::unique_ptr<Bitmap> bitmap = NSDataToBitmap([self createRGBWhite_2x2_RGBAPNG]);
  XCTAssertEqual(bitmap->Width(), 2u, @"Width should be 1.");
  XCTAssertEqual(bitmap->Height(), 2u, @"Height should be 1.");
  // Check color of the first pixel
  const uint8_t *pixels = reinterpret_cast<const uint8_t *>(bitmap->Pixels());
  if (pixels) {
    const auto expected = static_cast<uint8_t>(std::ceil(255 * (128. / 255.)));
    XCTAssertEqual(expected, pixels[0], "R should be 128");
    XCTAssertEqual(0, pixels[1], "G should be 0");
    XCTAssertEqual(0, pixels[2], "B should be 0");
    XCTAssertEqual(128, pixels[3], "A should be 128.");
    pixels = pixels + 4;
    XCTAssertEqual(0, pixels[0], "R should be 0");
    XCTAssertEqual(expected, pixels[1], "G should be 128");
    XCTAssertEqual(0, pixels[2], "B should be 0");
    XCTAssertEqual(128, pixels[3], "A should be 128.");
    pixels = pixels + 4;
    XCTAssertEqual(0, pixels[0], "R should be 0");
    XCTAssertEqual(0, pixels[1], "G should be 0");
    XCTAssertEqual(expected, pixels[2], "B should be 128");
    XCTAssertEqual(128, pixels[3], "A should be 128.");
    pixels = pixels + 4;
    XCTAssertEqual(expected, pixels[0], "R should be 128");
    XCTAssertEqual(expected, pixels[1], "G should be 128");
    XCTAssertEqual(expected, pixels[2], "B should be 128");
    XCTAssertEqual(128, pixels[3], "A should be 128.");
  } else {
    XCTFail(@"Pixels should not be null.");
  }
}

- (void)testNSDataToBitmap_SuccessfulConversion {
  NSData *validData = [self createValidImageData10x10Red];  // Create valid image data
  std::unique_ptr<Bitmap> bitmap = NSDataToBitmap(validData);

  XCTAssert(bitmap != nullptr, @"Bitmap should not be nullptr for valid image data.");

  const auto scale = UIScreen.mainScreen.scale;
  // Check dimensions
  XCTAssertEqual(bitmap->Width(), 10 * scale, @"Width should be 10 * scale.");
  XCTAssertEqual(bitmap->Height(), 10 * scale, @"Height should be 10 * scale.");

  // Check color of the first pixel
  const uint8_t *pixels = reinterpret_cast<const uint8_t *>(bitmap->Pixels());
  if (pixels) {
    uint32_t firstPixel = (pixels[0] << 24) | (pixels[1] << 16) | (pixels[2] << 8) | pixels[3];
    uint32_t expectedRedPixel = 0xFF0000FF;  // Assuming RGBA format and red color
    XCTAssertEqual(firstPixel, expectedRedPixel, @"First pixel should be red.");
  } else {
    XCTFail(@"Pixels should not be null.");
  }
}

- (void)testNSDataToRawData_WhenNSDataIsNilOrEmpty {
  // Test with nil NSData
  std::unique_ptr<RawData> rawData = NSDataToRawData(nil);
  XCTAssert(rawData == nullptr, @"RawData should be nullptr when NSData is nil.");

  // Test with empty NSData
  NSData *emptyData = [NSData data];
  rawData = NSDataToRawData(emptyData);
  XCTAssert(rawData == nullptr, @"RawData should be nullptr when NSData is empty.");
}

- (void)testNSDataToRawData_SuccessfulConversion {
  // Create valid NSData
  const char *testString = "Test data";
  NSData *validData = [NSData dataWithBytes:testString length:strlen(testString)];

  std::unique_ptr<RawData> rawData = NSDataToRawData(validData);
  XCTAssert(rawData != nullptr, @"RawData should not be nullptr for valid NSData.");

  // Additional checks: Verify data length and content
  XCTAssertEqual(rawData->Length(), strlen(testString),
                 @"Data length should match the input NSData length.");
  int memcmpResult = std::memcmp(rawData->Data(), [validData bytes], [validData length]);
  XCTAssertEqual(memcmpResult, 0, @"Data content should match the input NSData content.");
}

- (void)testWriteNSDataToFile_Success {
  // Create test data
  NSData *testData = [@"Test Data" dataUsingEncoding:NSUTF8StringEncoding];
  std::string testPath =
      [NSTemporaryDirectory() stringByAppendingPathComponent:@"testfile.txt"].UTF8String;

  // Call the function
  bool success = WriteNSDataToFile(testData, testPath);

  // Verify
  XCTAssertTrue(success, @"The function should return true on successful write.");
  NSData *writtenData =
      [NSData dataWithContentsOfFile:[NSString stringWithUTF8String:testPath.c_str()]];
  XCTAssertEqualObjects(testData, writtenData, @"The written data should match the original data.");

  // Clean up
  [[NSFileManager defaultManager] removeItemAtPath:[NSString stringWithUTF8String:testPath.c_str()]
                                             error:nil];
}

- (void)testWriteNSDataToFile_Failure {
  // Create test data
  id testData = OCMClassMock([NSData class]);
  OCMStub([testData writeToFile:[OCMArg any] atomically:YES]).andReturn(NO);

  std::string testPath =
      [NSTemporaryDirectory() stringByAppendingPathComponent:@"testfile.txt"].UTF8String;

  // Call the function
  bool success = WriteNSDataToFile(testData, testPath);

  // Verify
  XCTAssertFalse(success, @"The function should return false on write failure.");

  // Clean up
  [testData stopMocking];
}

@end
