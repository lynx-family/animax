// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/resource/resource_loader/ios/unzip_loader_ios.h"
#include "src/resource/loader/exec_loader.h"
#include "src/resource/loader/lambda_loader.h"
#include "src/resource/loader/pipe_loader.h"
#include "src/resource/resource_loader/ios/resource_util.h"
#include "src/resource/uri/uri_util.h"

#import <SSZipArchive/SSZipArchive.h>

@interface AnimaXSSZipArchiveDelegate : NSObject <SSZipArchiveDelegate>
+ (instancetype)sharedInstance;
@end

@implementation AnimaXSSZipArchiveDelegate

+ (instancetype)sharedInstance {
  static AnimaXSSZipArchiveDelegate* instance = nil;
  static dispatch_once_t onceToken;
  dispatch_once(&onceToken, ^{
    instance = [[AnimaXSSZipArchiveDelegate alloc] init];
  });
  return instance;
}

- (BOOL)zipArchiveShouldUnzipFileAtIndex:(NSInteger)fileIndex
                              totalFiles:(NSInteger)totalFiles
                             archivePath:(NSString*)archivePath
                                fileInfo:(unz_file_info)fileInfo {
  const unsigned long ZipUNIXVersion = 3;
  const unsigned long BSD_SFMT = 0170000;
  const unsigned long BSD_IFLNK = 0120000;
  BOOL fileIsSymbolicLink = ((fileInfo.version >> 8) == ZipUNIXVersion) &&
                            BSD_IFLNK == (BSD_SFMT & (fileInfo.external_fa >> 16));
  if (fileIsSymbolicLink) {
    return NO;
  }
  return YES;
}

@end

namespace lynx {
namespace animax {

namespace {

NSString* GetFilePathNSString(const std::string& zip_file_path) {
  NSString* ns_zip_file_path = [NSString stringWithUTF8String:zip_file_path.data()];
  if ([[NSURL URLWithString:ns_zip_file_path] isFileURL]) {
    // if the ns_zip_file_path is prefixed with file:// scheme, remove it.
    ns_zip_file_path = [[NSURL URLWithString:ns_zip_file_path] path];
  } else {
    ns_zip_file_path = [NSString stringWithUTF8String:zip_file_path.c_str()];
  }
  return ns_zip_file_path;
}

bool CheckZipFileExist(NSString* ns_zip_file_path) {
  BOOL is_directory;
  BOOL file_exists = [[NSFileManager defaultManager] fileExistsAtPath:ns_zip_file_path
                                                          isDirectory:&is_directory];
  return !is_directory && file_exists;
}

NSString* GetUnzipFolderPathNSString(const std::string& zip_file_path,
                                     const std::string& unzip_path) {
  if (unzip_path.empty()) {
    auto generated_unzip_path =
        ConcatFilePaths(ConcatFilePaths(EnsureCacheDirectory(), "animax_unzip_result_"),
                        std::to_string(std::hash<std::string>{}(zip_file_path)));
    return [NSString stringWithUTF8String:generated_unzip_path.data()];
  } else {
    return [NSString stringWithUTF8String:unzip_path.data()];
  }
}

LoaderError UnzipFileToPath(NSString* zip_file_path, NSString* unzip_path) {
  NSError* ns_error = nil;
  [SSZipArchive unzipFileAtPath:zip_file_path
                  toDestination:unzip_path
                      overwrite:YES
                       password:nil
                          error:&ns_error
                       delegate:[AnimaXSSZipArchiveDelegate sharedInstance]];
  if (ns_error) {
    auto* err_msg = [[ns_error description] UTF8String];
    return LoaderError{kUnzipFailed, err_msg ? std::string{err_msg} : std::string{}};
  } else {
    return LoaderError{};
  }
}

}  // namespace

UnzipLoaderIOS::UnzipLoaderIOS() {
  auto unzip =
      MakeLambdaLoader<UnzipRequest, UnzipResponse>([](UnzipRequest request, auto callback) {
        auto error = LoaderError{};
        auto response = UnzipResponse{};
        do {
          auto zip_file_path = request.zip_file_path;
          if (zip_file_path.empty()) {
            error = LoaderError{kFileDoesNotExist, "zip file uri is empty "};
            break;
          }

          NSString* ns_zip_file_path = GetFilePathNSString(zip_file_path);
          if (!CheckZipFileExist(ns_zip_file_path)) {
            error = LoaderError{kFileDoesNotExist, "zip file does not exist or is a folder."};
            break;
          }

          NSString* ns_unzip_path =
              GetUnzipFolderPathNSString(request.zip_file_path, request.unzip_folder_path);

          error = UnzipFileToPath(ns_zip_file_path, ns_unzip_path);
          if (!error) {
            response.path = std::string{[ns_unzip_path UTF8String]};
          }
        } while (false);
        callback(std::move(response), std::move(error));
      });
  loader_ = ResourceThreadExecLoader(unzip);
}

void UnzipLoaderIOS::Load(UnzipRequest request, CallbackType callback) {
  loader_->Load(std::move(request), std::move(callback));
}

}  // namespace animax
}  // namespace lynx
