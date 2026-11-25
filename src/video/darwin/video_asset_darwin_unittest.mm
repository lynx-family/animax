// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AVFoundation/AVFoundation.h>
#import <XCTest/XCTest.h>
#import <objc/runtime.h>

#include <memory>
#include <string>

#include "src/resource/asset/video_asset_model.h"
#include "src/video/darwin/video_asset_darwin.h"

using namespace lynx::animax;

namespace {

constexpr int kFrameCount = 12;
constexpr int kVideoSize = 128;

enum class SampleChange { kNone, kSplit, kUnreadable };

SampleChange g_sample_change = SampleChange::kNone;
// A negative index changes every sample.
int g_changed_sample_index = -1;
int g_sample_index = 0;
IMP g_copy_next_sample_buffer = nullptr;

void CopyAttachment(const void* key, const void* value, void* context) {
  CFDictionarySetValue(static_cast<CFMutableDictionaryRef>(context), key, value);
}

// Copies the sample data into three separate memory blocks, or references a
// block that is never allocated so that copying the sample data fails.
CMSampleBufferRef CopySample(CMSampleBufferRef sample, SampleChange change) {
  CMBlockBufferRef source = CMSampleBufferGetDataBuffer(sample);
  size_t length = CMBlockBufferGetDataLength(source);
  CMBlockBufferRef data = nullptr;
  if (change == SampleChange::kUnreadable) {
    CMBlockBufferCreateWithMemoryBlock(kCFAllocatorDefault, nullptr, length, kCFAllocatorDefault,
                                       nullptr, 0, length, 0, &data);
  } else {
    CMBlockBufferCreateEmpty(kCFAllocatorDefault, 3, 0, &data);
    const size_t sizes[] = {length / 3, length / 3, length - 2 * (length / 3)};
    size_t offset = 0;
    for (size_t size : sizes) {
      void* block = malloc(size);
      CMBlockBufferCopyDataBytes(source, offset, size, block);
      CMBlockBufferAppendMemoryBlock(data, block, size, kCFAllocatorMalloc, nullptr, 0, size, 0);
      offset += size;
    }
  }
  if (!data) {
    return nullptr;
  }
  CMSampleTimingInfo timing{CMSampleBufferGetDuration(sample),
                            CMSampleBufferGetPresentationTimeStamp(sample),
                            CMSampleBufferGetDecodeTimeStamp(sample)};
  CMSampleBufferRef copy = nullptr;
  CMSampleBufferCreateReady(kCFAllocatorDefault, data, CMSampleBufferGetFormatDescription(sample),
                            1, 1, &timing, 1, &length, &copy);
  CFRelease(data);
  CFArrayRef attachments = CMSampleBufferGetSampleAttachmentsArray(sample, false);
  if (copy && attachments && CFArrayGetCount(attachments) > 0) {
    CFArrayRef copied = CMSampleBufferGetSampleAttachmentsArray(copy, true);
    CFDictionaryApplyFunction(static_cast<CFDictionaryRef>(CFArrayGetValueAtIndex(attachments, 0)),
                              CopyAttachment, const_cast<void*>(CFArrayGetValueAtIndex(copied, 0)));
  }
  return copy;
}

CMSampleBufferRef CopyNextSampleBuffer(id self, SEL command) {
  auto sample =
      reinterpret_cast<CMSampleBufferRef (*)(id, SEL)>(g_copy_next_sample_buffer)(self, command);
  if (!sample || !CMSampleBufferGetDataBuffer(sample)) {
    return sample;
  }
  const int index = g_sample_index++;
  if (g_sample_change == SampleChange::kNone ||
      (g_changed_sample_index >= 0 && index != g_changed_sample_index)) {
    return sample;
  }
  CMSampleBufferRef copy = CopySample(sample, g_sample_change);
  if (!copy) {
    return sample;
  }
  CFRelease(sample);
  return copy;
}

NSString* WriteVideo() {
  NSString* path = [NSTemporaryDirectory()
      stringByAppendingPathComponent:[[NSUUID UUID].UUIDString stringByAppendingString:@".mp4"]];
  AVAssetWriter* writer = [AVAssetWriter assetWriterWithURL:[NSURL fileURLWithPath:path]
                                                   fileType:AVFileTypeMPEG4
                                                      error:nil];
  // Without reordering, a prefix of the decode order is a decodable prefix.
  NSDictionary* settings = @{
    AVVideoCodecKey : AVVideoCodecTypeH264,
    AVVideoWidthKey : @(kVideoSize),
    AVVideoHeightKey : @(kVideoSize),
    AVVideoCompressionPropertiesKey :
        @{AVVideoMaxKeyFrameIntervalKey : @4, AVVideoAllowFrameReorderingKey : @NO},
  };
  AVAssetWriterInput* input = [AVAssetWriterInput assetWriterInputWithMediaType:AVMediaTypeVideo
                                                                 outputSettings:settings];
  NSDictionary* attributes = @{
    (id)kCVPixelBufferPixelFormatTypeKey : @(kCVPixelFormatType_32BGRA),
    (id)kCVPixelBufferWidthKey : @(kVideoSize),
    (id)kCVPixelBufferHeightKey : @(kVideoSize),
  };
  AVAssetWriterInputPixelBufferAdaptor* adaptor = [AVAssetWriterInputPixelBufferAdaptor
      assetWriterInputPixelBufferAdaptorWithAssetWriterInput:input
                                 sourcePixelBufferAttributes:attributes];
  [writer addInput:input];
  [writer startWriting];
  [writer startSessionAtSourceTime:kCMTimeZero];
  for (int i = 0; i < kFrameCount; ++i) {
    while (!input.readyForMoreMediaData) {
      [NSThread sleepForTimeInterval:0.01];
    }
    CVPixelBufferRef pixels = nullptr;
    CVPixelBufferPoolCreatePixelBuffer(nullptr, adaptor.pixelBufferPool, &pixels);
    CVPixelBufferLockBaseAddress(pixels, 0);
    memset(CVPixelBufferGetBaseAddress(pixels), i * 20,
           CVPixelBufferGetBytesPerRow(pixels) * kVideoSize);
    CVPixelBufferUnlockBaseAddress(pixels, 0);
    [adaptor appendPixelBuffer:pixels withPresentationTime:CMTimeMake(i, 30)];
    CVPixelBufferRelease(pixels);
  }
  [input markAsFinished];
  dispatch_semaphore_t finished = dispatch_semaphore_create(0);
  [writer finishWritingWithCompletionHandler:^{
    dispatch_semaphore_signal(finished);
  }];
  dispatch_semaphore_wait(finished, DISPATCH_TIME_FOREVER);
  return writer.status == AVAssetWriterStatusCompleted ? path : nil;
}

// Loads through the preload entry when `error` is not null.
std::shared_ptr<VideoAssetDarwin> Load(NSString* path, SampleChange change, int index,
                                       LoaderError* error = nullptr) {
  g_sample_change = change;
  g_changed_sample_index = index;
  g_sample_index = 0;
  auto asset = std::make_shared<VideoAssetDarwin>(VideoAssetModel{});
  if (error) {
    *error = asset->PrepareFrameDataForPreload(path.UTF8String);
  } else {
    asset->PrepareFrameData(std::string(path.UTF8String));
  }
  g_sample_change = SampleChange::kNone;
  return asset;
}

NSData* FrameData(const VideoAssetDarwin& asset) {
  return [NSData dataWithBytes:asset.GetFrameData() length:asset.GetFrameDataLength()];
}

}  // namespace

@interface VideoAssetDarwinUnitTest : XCTestCase
@property(nonatomic, copy) NSString* videoPath;
@end

@implementation VideoAssetDarwinUnitTest

- (void)setUp {
  self.videoPath = WriteVideo();
  XCTAssertNotNil(self.videoPath);
  Method method =
      class_getInstanceMethod([AVAssetReaderTrackOutput class], @selector(copyNextSampleBuffer));
  g_copy_next_sample_buffer =
      method_setImplementation(method, reinterpret_cast<IMP>(CopyNextSampleBuffer));
}

- (void)tearDown {
  Method method =
      class_getInstanceMethod([AVAssetReaderTrackOutput class], @selector(copyNextSampleBuffer));
  method_setImplementation(method, g_copy_next_sample_buffer);
  [[NSFileManager defaultManager] removeItemAtPath:self.videoPath error:nil];
}

- (void)testCopiesSamplesSplitAcrossMemoryBlocks {
  auto expected = Load(self.videoPath, SampleChange::kNone, -1);
  XCTAssertTrue(expected->IsValid());
  XCTAssertEqual(expected->GetFrameCount(), kFrameCount);

  auto split = Load(self.videoPath, SampleChange::kSplit, -1);
  XCTAssertTrue(split->IsValid());
  XCTAssertEqual(split->GetFrameCount(), kFrameCount);
  XCTAssertEqualObjects(FrameData(*split), FrameData(*expected));

  LoaderError error;
  auto preloaded = Load(self.videoPath, SampleChange::kSplit, -1, &error);
  XCTAssertFalse(!!error);
  XCTAssertEqualObjects(FrameData(*preloaded), FrameData(*expected));
}

- (void)testStopsBeforeUnreadableSample {
  constexpr int kUnreadableIndex = 6;
  auto expected = Load(self.videoPath, SampleChange::kNone, -1);
  XCTAssertEqual(expected->GetFrameCount(), kFrameCount);

  // Playback keeps the frames before the unreadable sample and does not
  // record an empty frame that the player cannot decode.
  auto truncated = Load(self.videoPath, SampleChange::kUnreadable, kUnreadableIndex);
  XCTAssertTrue(truncated->IsValid());
  XCTAssertEqual(truncated->GetFrameCount(), kUnreadableIndex);
  for (int i = 0; i < truncated->GetFrameCount(); ++i) {
    XCTAssertLessThan(truncated->GetFrameInfo(i).beg_, truncated->GetFrameInfo(i).end_);
  }
  const NSRange prefix = NSMakeRange(0, expected->GetFrameInfo(kUnreadableIndex).beg_);
  XCTAssertEqualObjects(FrameData(*truncated), [FrameData(*expected) subdataWithRange:prefix]);

  LoaderError error;
  Load(self.videoPath, SampleChange::kUnreadable, kUnreadableIndex, &error);
  XCTAssertTrue(!!error);
}

- (void)testRejectsUnreadableFirstSample {
  auto asset = Load(self.videoPath, SampleChange::kUnreadable, 0);
  XCTAssertFalse(asset->IsValid());

  LoaderError error;
  Load(self.videoPath, SampleChange::kUnreadable, 0, &error);
  XCTAssertTrue(!!error);
}

@end
