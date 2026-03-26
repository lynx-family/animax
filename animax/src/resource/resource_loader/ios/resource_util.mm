// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/resource_loader/ios/resource_util.h"
#include "src/base/log/log.h"
#include "src/base/thread/task_runner.h"
#include "src/base/thread/thread_assert.h"
#include "src/resource/resource_loader.h"

#import <Accelerate/Accelerate.h>
#import <MobileCoreServices/MobileCoreServices.h>

namespace lynx {
namespace animax {
namespace {

static constexpr const char* PNG_1x1_RGBA50505050_BASE64_DATA_URL =
    "data:image/"
    "png;base64,"
    "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABAQMAAAAl21bKAAAAA1BMVEUyMjKlMgnVAAAAAXRSTlMyiDGJ5gAAAApJREFUCN"
    "djYAAAAAIAAeIhvDMAAAAASUVORK5CYII=";

static constexpr const char* BASE64_PREFIX = "base64,";
static constexpr const char* DATA_URL_PREFIX = "data:";

bool CheckDeviceHasIOS17IndexedPNGDecodingBug() {
  CFDataRef cfData = nil;
  bool hasBug = false;
  if (@available(iOS 17, *)) {
    do {
      UIImage* image = [UIImage
          imageWithData:[NSData
                            dataWithContentsOfURL:
                                [NSURL
                                    URLWithString:[NSString
                                                      stringWithUTF8String:
                                                          PNG_1x1_RGBA50505050_BASE64_DATA_URL]]]];
      if (image == nil || image.CGImage == nil) {
        break;
      }
      CGDataProviderRef dataProvider = CGImageGetDataProvider(image.CGImage);
      if (dataProvider == nil) {
        break;
      }
      cfData = CGDataProviderCopyData(dataProvider);
      if (cfData == nil) {
        break;
      }
      GLubyte* pixels = (GLubyte*)CFDataGetBytePtr(cfData);
      if (!pixels) {
        break;
      }
      if (!(pixels[0] == 50 && pixels[1] == 50 && pixels[2] == 50)) {
        hasBug = true;
      }
    } while (0);
  }
  if (cfData != nil) {
    CFRelease(cfData);
  }
  return hasBug;
}

}  // namespace

bool DeviceHasIOS17IndexedPNGDecodingBug() {
  static bool has_bug = CheckDeviceHasIOS17IndexedPNGDecodingBug();
  return has_bug;
}

namespace {

CGImageRef CGImageCreateMutableCopy(CGImageRef image, CGBitmapInfo bitmapInfo) {
  if (!image) {
    return nil;
  }
  size_t width = CGImageGetWidth(image);
  size_t height = CGImageGetHeight(image);
  size_t bitsPerComponent = CGImageGetBitsPerComponent(image);
  size_t bitsPerPixel = CGImageGetBitsPerPixel(image);
  size_t bytesPerRow = CGImageGetBytesPerRow(image);
  CGColorSpaceRef space = CGImageGetColorSpace(image);
  CGDataProviderRef provider = CGImageGetDataProvider(image);
  const CGFloat* decode = CGImageGetDecode(image);
  bool shouldInterpolate = CGImageGetShouldInterpolate(image);
  CGColorRenderingIntent intent = CGImageGetRenderingIntent(image);
  CGImageRef newImage =
      CGImageCreate(width, height, bitsPerComponent, bitsPerPixel, bytesPerRow, space, bitmapInfo,
                    provider, decode, shouldInterpolate, intent);
  return newImage;
}

CGImageRef FixCGImageAlphaInfoIfNeeded(CGImageRef cg_image_ref) CF_RETURNS_RETAINED {
  do {
    if (!DeviceHasIOS17IndexedPNGDecodingBug() || !cg_image_ref) {
      break;
    }

    // Device has iOS17 Indexed PNG decoding bug.
    const auto is_png =
        CFStringCompare(CGImageGetUTType(cg_image_ref), kUTTypePNG, 0) == kCFCompareEqualTo;
    // The CGImageRef retrieved from UIImage should have a CGAlphaInfo of either
    // kCGImageAlphaPremultipliedLast or kCGImageAlphaPremultipliedFirst. However, due to a
    // iOS-version-specific bug, it may incorrectly be set to kCGImageAlphaLast or
    // kCGImageAlphaFirst instead.
    const auto cg_image_alpha_info_is_wrong =
        (CGImageGetAlphaInfo(cg_image_ref) == kCGImageAlphaLast ||
         CGImageGetAlphaInfo(cg_image_ref) == kCGImageAlphaFirst);

    if (!is_png || !cg_image_alpha_info_is_wrong) {
      break;
    }

    // If CGImageAlphaInfo is incorrect and we continue using kCGImageAlphaLast/kCGImageAlphaFirst,
    // alpha premultiplication will be applied twice, leading to unintended results.
    CGBitmapInfo bitmap_info = CGImageGetBitmapInfo(cg_image_ref);
    CGBitmapInfo alpha_info = bitmap_info & kCGBitmapAlphaInfoMask;
    CGBitmapInfo new_alpha_info = alpha_info;
    if (alpha_info == kCGImageAlphaLast) {
      new_alpha_info = kCGImageAlphaPremultipliedLast;
    } else if (alpha_info == kCGImageAlphaFirst) {
      new_alpha_info = kCGImageAlphaPremultipliedFirst;
    }
    CGBitmapInfo byte_order_info = bitmap_info & kCGBitmapByteOrderMask;
    const auto has_float_component =
        (bitmap_info & kCGBitmapFloatComponents) == kCGBitmapFloatComponents;
    CGBitmapInfo new_bitmap_info = new_alpha_info | byte_order_info;
    if (has_float_component) {
      new_bitmap_info = new_bitmap_info | kCGBitmapFloatComponents;
    }

    CGImageRef new_cg_image_ref = CGImageCreateMutableCopy(cg_image_ref, new_bitmap_info);
    return new_cg_image_ref;
  } while (0);

  CGImageRetain(cg_image_ref);
  return cg_image_ref;
}

std::unique_ptr<Bitmap> CGImageRefToBitmap(CGImageRef cg_image_ref) {
  if (!cg_image_ref) {
    return nullptr;
  }

  const auto width = static_cast<uint32_t>(CGImageGetWidth(cg_image_ref));
  const auto height = static_cast<uint32_t>(CGImageGetHeight(cg_image_ref));
  if (width == 0 || height == 0) {
    ANIMAX_LOGE("Invalid image: Either the width or the height is 0.");
    return nullptr;
  }

  CGContextRef context = NULL;
  do {
    CGBitmapInfo contextBitmapInfo = kCGBitmapByteOrderDefault | kCGImageAlphaPremultipliedLast;
    context = CGBitmapContextCreate(NULL, width, height, 8, width * 4,
                                    CGColorSpaceCreateDeviceRGB(), contextBitmapInfo);
    if (!context) {
      ANIMAX_LOGE("Failed to create CGContext for bitmap");
      break;
    }

    CGContextDrawImage(context, CGRectMake(0, 0, width, height), cg_image_ref);
    size_t bpr = CGBitmapContextGetBytesPerRow(context);
    size_t length = height * bpr;
    void* data = CGBitmapContextGetData(context);
    if (length == 0 || !data) {
      ANIMAX_LOGE("Failed to retrieve pixel data from the CGContext.");
      break;
    }

    return Bitmap::MakeRGBA(
        width, height, data,
        [](const void* data) {
          CGContextRef context = static_cast<CGContextRef>(const_cast<void*>(data));
          CGContextRelease(context);
        },
        context);
  } while (0);

  CGContextRelease(context);
  return nullptr;
}

}  // namespace

std::unique_ptr<Bitmap> NSDataToBitmap(NSData* data) {
  if (!data) {
    return nullptr;
  }

  UIImage* ui_image = [UIImage imageWithData:data];
  if (!ui_image) {
    return nullptr;
  }

  return UIImageToBitmap(ui_image);
}

std::unique_ptr<Bitmap> UIImageToBitmap(UIImage* ui_image) {
  CGImageRef cg_image_ref = ui_image.CGImage;

  if (!cg_image_ref) {
    ANIMAX_LOGE("Failed to convert NSData into UIImage.");
    return nullptr;
  }

  CGImageRef fixed_cg_image_ref = FixCGImageAlphaInfoIfNeeded(cg_image_ref);

  auto bitmap = CGImageRefToBitmap(fixed_cg_image_ref);
  CGImageRelease(fixed_cg_image_ref);
  return bitmap;
}

std::unique_ptr<RawData> NSDataToRawData(NSData* ns_data) {
  auto* ns_data_ptr = [ns_data bytes];
  auto data_length = [ns_data length];
  if (!ns_data || !ns_data_ptr || data_length == 0) {
    ANIMAX_LOGE("Invalid NSData.");
    return nullptr;
  }

  auto* data_ptr = std::malloc(data_length);
  if (!data_ptr) {
    ANIMAX_LOGE("Failed to allocate memory for RawData.");
    return nullptr;
  }

  std::memcpy(data_ptr, ns_data_ptr, data_length);
  return RawData::MakeRawData(
      data_ptr, data_length, [](const void* data_ptr) { std::free(const_cast<void*>(data_ptr)); },
      data_ptr);
}

bool TryCreateDirectory(NSString* directory) {
  BOOL is_dir = NO;
  BOOL is_dir_exist = [[NSFileManager defaultManager] fileExistsAtPath:directory
                                                           isDirectory:&is_dir];
  if (is_dir_exist && is_dir) {
    return true;
  }

  if (is_dir_exist && !is_dir) {
    ANIMAX_LOGE("File exists at path" << [directory UTF8String] << ", but not a directory.");
    return false;
  }

  if (!is_dir_exist && is_dir) {
    ANIMAX_LOGE("Invalid directory path: " << [directory UTF8String]);
    return false;
  }

  NSError* dirError = nil;
  if (![[NSFileManager defaultManager] createDirectoryAtPath:directory
                                 withIntermediateDirectories:YES
                                                  attributes:nil
                                                       error:&dirError]) {
    ANIMAX_LOGE("Failed to create dir at path: " << [directory UTF8String]);
    return false;
  }

  return true;
}

std::string EnsureCacheDirectory() {
  NSString* cache_directory =
      [[NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES) lastObject]
          stringByAppendingPathComponent:@"com.bytedance.animax.caches"];
  if ([cache_directory length] == 0) {
    return std::string{};
  }

  if (!TryCreateDirectory(cache_directory)) {
    return std::string{};
  }

  const char* cache_directory_ptr = [cache_directory UTF8String];
  if (!cache_directory_ptr) {
    return std::string{};
  }
  return std::string(cache_directory_ptr);
}

std::string GetTempFilePathForUrl(const std::string& url) {
  NSString* ns_url_string = [NSString stringWithUTF8String:url.c_str()];
  if (!ns_url_string) {
    return std::string{};
  }
  NSURL* ns_url = [NSURL URLWithString:ns_url_string];
  NSString* ns_path_extension = [ns_url pathExtension];
  std::string path_extension;
  if ([ns_path_extension length] > 0) {
    path_extension = "." + std::string{[ns_path_extension UTF8String]};
  }

  const auto cache_dir = EnsureCacheDirectory();
  if (cache_dir.empty()) {
    ANIMAX_LOGE("Cannot get cache directory");
    return std::string{};
  }
  const auto file =
      cache_dir + "/tmp_" + std::to_string(std::hash<std::string>{}(url)) + path_extension;
  return file;
}

bool WriteNSDataToFile(NSData* data, const std::string& path) {
  NSString* path_ns_string = [NSString stringWithUTF8String:path.data()];
  if (![data writeToFile:path_ns_string atomically:YES]) {
    ANIMAX_LOGE("Failed to write to file: " << path);
    return false;
  }
  return true;
}

}  // namespace animax
}  // namespace lynx
