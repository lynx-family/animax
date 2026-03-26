// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/base/util/event_warning_checker.h"

#include <string>

#include "src/base/log/log.h"
#include "src/resource/uri/uri_util.h"

namespace lynx {
namespace animax {

bool EventWarningChecker::CheckStartAndEndFrame(
    double start_frame, double end_frame, double current_frame,
    EventWarningChecker::WarningHandler&& warning_handler) {
  if (start_frame >= end_frame) {
    EventWarning warning = EventWarning::kStartFrameGreaterThanEndFrame;
    std::ostringstream warning_msg;
    warning_msg << "Invalid frame range: start-frame (" << start_frame
                << ") must be less than end-frame (" << end_frame
                << "). Please ensure you pass a valid range.";
    std::string warning_str = warning_msg.str();
    if (warning_handler) {
      warning_handler(warning, warning_str);
    }
    LogWarning(warning, warning_str);
    return false;
  }
  return true;
}

bool EventWarningChecker::CheckAssetCountOverLimit(
    int image_count, int video_count, int font_count,
    EventWarningChecker::WarningHandler&& warning_handler) {
  if (image_count > kRecommendedImageLimit ||
      video_count > kRecommendedVideoLimit ||
      font_count > kRecommendedFontLimit) {
    EventWarning warning = EventWarning::kAssetCountOverLimit;
    std::ostringstream warning_msg;
    warning_msg << "Asset count exceeds recommended limits: ";

    if (image_count > kRecommendedImageLimit) {
      warning_msg << "images = " << image_count << " (limit "
                  << kRecommendedImageLimit << "), ";
    }

    if (video_count > kRecommendedVideoLimit) {
      warning_msg << "alpha-videos = " << video_count << " (limit "
                  << kRecommendedVideoLimit << "), ";
    }

    if (font_count > kRecommendedFontLimit) {
      warning_msg << "fonts = " << font_count << " (limit "
                  << kRecommendedFontLimit << "), ";
    }

    warning_msg << "This does not affect functionality, but may reduce FPS and "
                   "cause stuttering.\n";

    if (image_count > kRecommendedImageLimit) {
      warning_msg << "The image count is high, consider converting to AP "
                     "format to improve performance.";
    }

    std::string warning_str = warning_msg.str();
    if (warning_handler) {
      warning_handler(warning, warning_str);
    }
    LogWarning(warning, warning_str);
    return false;
  }
  return true;
}

bool EventWarningChecker::CheckLottieFormat(
    const std::string src,
    EventWarningChecker::WarningHandler&& warning_handler) {
  auto content_type = ParseUriMainResourceContentType(src);
  if (content_type != UriInfo::ContentType::kJson &&
      content_type != UriInfo::ContentType::kZip) {
    EventWarning warning = EventWarning::kInvalidLottieFormat;
    std::ostringstream warning_msg;
    warning_msg
        << "Invalid lottie resource content type. The content type must be "
           "json or zip.";
    std::string warning_str = warning_msg.str();

    if (warning_handler) {
      warning_handler(warning, warning_str);
    }
    LogWarning(warning, warning_str);
    return false;
  }
  return true;
}

bool EventWarningChecker::CheckDynamicResourceLoadable(
    bool autoplay, bool dynamic_resource,
    EventWarningChecker::WarningHandler&& warning_handler) {
  if (autoplay && dynamic_resource) {
    EventWarning warning = EventWarning::kDynamicResourceCannotBeLoaded;
    std::string warning_str =
        "The animation is already playing so that dynamic resources "
        "cannot be loaded. Try turning off autoPlay if it is enabled.";

    if (warning_handler) {
      warning_handler(warning, warning_str);
    }
    LogWarning(warning, warning_str);
    return false;
  }
  return true;
}

bool EventWarningChecker::CheckExecuteBeforeReady(
    bool ready, const std::string action,
    EventWarningChecker::WarningHandler&& warning_handler) {
  if (!ready) {
    EventWarning warning = EventWarning::kExecuteBeforeReady;
    std::string warning_str =
        "Cannot perform \"" + action + "\": AnimaX's instance is not ready.";

    if (warning_handler) {
      warning_handler(warning, warning_str);
    }
    LogWarning(warning, warning_str);
    return false;
  }
  return true;
}

bool EventWarningChecker::CheckIllegalAlphaVideoSize(
    int width, int height,
    EventWarningChecker::WarningHandler&& warning_handler) {
  if (width % 16 != 0 || height % 16 != 0) {
    EventWarning warning = EventWarning::kIllegalAlphaVideoSize;
    std::ostringstream warning_msg;
    warning_msg << "Invalid resolution " << width << "," << height
                << " — both the width and height should be multiples of 16.";
    std::string warning_str = warning_msg.str();

    if (warning_handler) {
      warning_handler(warning, warning_str);
    }
    LogWarning(warning, warning_str);
    return false;
  }
  return true;
}

bool EventWarningChecker::CheckIllegalSubscribedStartFrame(
    double start_frame, EventWarningChecker::WarningHandler&& warning_handler) {
  if (start_frame != 0) {
    EventWarning warning = EventWarning::kIllegalSubscribedStartFrame;
    std::ostringstream warning_msg;
    warning_msg << "The Lottie animation starts at frame " << start_frame
                << " but should start at frame 0, so subscribing to frames may "
                   "not yield the updates you expect.";
    std::string warning_str = warning_msg.str();

    if (warning_handler) {
      warning_handler(warning, warning_str);
    }
    LogWarning(warning, warning_str);
    return false;
  }
  return true;
}

void EventWarningChecker::LogWarning(EventWarning warning,
                                     const std::string& warning_message) {
  ANIMAX_LOGW("AnimaXPlayer Warning,"
              << " type: " << StringifyEventWarning(warning)
              << ", message: " << warning_message);
}

}  // namespace animax
}  // namespace lynx
