// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * Enumeration of resource property types that can be modified.
 * These values correspond to the ResourcePropertyType enum in the native code.
 */
typedef NS_ENUM(NSUInteger, ResourcePropertyType) {
  /**
   * Unknown resource property type.
   */
  ResourcePropertyTypeUnknown = 0,

  /**
   * Directory name for image resources
   */
  ResourcePropertyTypeImageDirName = 1,

  /**
   * File name for image resources
   */
  ResourcePropertyTypeImageFileName,

  /**
   * Width of image resources
   */
  ResourcePropertyTypeImageWidth,

  /**
   * Height of image resources
   */
  ResourcePropertyTypeImageHeight,

  /**
   * Font family for text elements
   */
  ResourcePropertyTypeFontFamily = 101,

  /**
   * Font style for text elements
   */
  ResourcePropertyTypeFontStyle,

  /**
   * Font ascent for text elements
   */
  ResourcePropertyTypeFontAscent,

  /**
   * Font file path for text elements
   */
  ResourcePropertyTypeFontPath,

  /**
   * Directory name for video resources
   */
  ResourcePropertyTypeVideoDirName = 201,

  /**
   * File name for video resources
   */
  ResourcePropertyTypeVideoFileName,

  /**
   * Width of video resources
   */
  ResourcePropertyTypeVideoWidth,

  /**
   * Height of video resources
   */
  ResourcePropertyTypeVideoHeight
};

NS_ASSUME_NONNULL_END
