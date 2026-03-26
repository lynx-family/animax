// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_IOS_RESOURCE_UTIL_H_
#define ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_IOS_RESOURCE_UTIL_H_

#include <memory>
#include "base/include/closure.h"
#include "include/resource/bitmap.h"
#include "include/resource/raw_data.h"

namespace lynx {
namespace animax {

bool TryCreateDirectory(NSString* directory);
std::string EnsureCacheDirectory();
std::string GetTempFilePathForUrl(const std::string& url);

bool DeviceHasIOS17IndexedPNGDecodingBug();
std::unique_ptr<Bitmap> NSDataToBitmap(NSData* data);
std::unique_ptr<Bitmap> UIImageToBitmap(UIImage* ui_image);
std::unique_ptr<RawData> NSDataToRawData(NSData* ns_data);
bool WriteNSDataToFile(NSData* data, const std::string& path);

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_RESOURCE_LOADER_IOS_RESOURCE_UTIL_H_
