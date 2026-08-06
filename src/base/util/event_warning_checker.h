// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_BASE_UTIL_EVENT_WARNING_CHECKER_H_
#define ANIMAX_SRC_BASE_UTIL_EVENT_WARNING_CHECKER_H_

#include <string>

#include "base/include/closure.h"
#include "src/player/animax_event.h"

namespace lynx {
namespace animax {

class EventWarningChecker {
 public:
  EventWarningChecker() = delete;
  EventWarningChecker(const EventWarningChecker&) = delete;
  EventWarningChecker& operator=(const EventWarningChecker&) = delete;
  using WarningHandler =
      base::MoveOnlyClosure<void, EventWarning, const std::string&>;

  /**
   * Check if start_frame is less than end_frame.
   * Will trigger warning if start_frame >= end_frame.
   * @param start_frame    The value of start frame number
   * @param end_frame      The value of end frame number
   * @param current_frame  The value of current frame number
   * @param warning_handler Callback to handle warning events
   * @return               true if frame is valid, false if warning was
   * triggered
   */
  static bool CheckStartAndEndFrame(double start_frame, double end_frame,
                                    double current_frame,
                                    WarningHandler&& warning_handler);

  /**
   * Check if resource counts exceed performance thresholds.
   * Will trigger warning if any resource count exceeds recommended limits.
   * @param image_count   Number of images being used
   * @param video_count   Number of videos being used
   * @param font_count    Number of fonts being used
   * @param warning_handler Callback to handle warning events
   * @return             true if counts are within limits, false if warning was
   * triggered
   */
  static bool CheckAssetCountOverLimit(int image_count, int video_count,
                                       int font_count,
                                       WarningHandler&& warning_handler);

  /**
   * Validate Lottie animation source file format.
   * Will trigger warning if source format is invalid or unsupported.
   * @param src           Path or URL to the Lottie animation source
   * @param warning_handler Callback to handle warning events
   * @return             true if format is valid, false if warning was triggered
   */
  static bool CheckLottieFormat(const std::string src,
                                WarningHandler&& warning_handler,
                                bool allow_extensionless_json = false);

  /**
   * Check if dynamic resources can be loaded in current state.
   * Will trigger warning if resources cannot be loaded with current autoplay
   * setting.
   * @param autoplay            Autoplay status
   * @param dynamic_resource    Dynamic resource status
   * @param warning_handler     Callback to handle warning events
   * @return             true if resources can be loaded, false if warning was
   * triggered
   */
  static bool CheckDynamicResourceLoadable(bool autoplay, bool dynamic_resource,
                                           WarningHandler&& warning_handler);

  /**
   * Validate if action can be executed in current ready state.
   * Will trigger warning if action cannot be performed in current state.
   * @param ready         Current ready status
   * @param action        The action name to validate
   * @param warning_handler Callback to handle warning events
   * @return             true if action can be executed, false if warning was
   * triggered
   */
  static bool CheckExecuteBeforeReady(bool ready, const std::string action,
                                      WarningHandler&& warning_handler);

  /**
   * Check if alpha video size is legal.
   * Will trigger warning if width or height are not multiples of 16.
   * @param width        The width in pixels
   * @param height       The height in pixels
   * @param warning_handler Callback to handle warning events
   * @return             true if resolution is valid, false if warning was
   * triggered
   */
  static bool CheckIllegalAlphaVideoSize(int width, int height,
                                         WarningHandler&& warning_handler);

  /**
   * Check if start frame number support subscribe.
   * Will trigger warning if start_frame is not zero.
   * @param start_frame  The value of start frame number
   * @param warning_handler Callback to handle warning events
   * @return             true if start frame number is zero, false if warning
   * was triggered
   */
  static bool CheckIllegalSubscribedStartFrame(
      double start_frame, WarningHandler&& warning_handler);

 private:
  static void LogWarning(EventWarning warning,
                         const std::string& warning_message);
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_BASE_UTIL_EVENT_WARNING_CHECKER_H_
