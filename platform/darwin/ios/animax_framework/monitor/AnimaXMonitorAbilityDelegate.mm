// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXMonitorAbilityDelegate.h>
#import <AnimaX/AnimaXMonitorService.h>
#import "include/player/animax_player.h"

#include "src/base/monitor/animax_metrics_manager.h"

#pragma mark - Constants

static NSString *const AnimaXMonitorTriggerPlay = @"onPlay";
static NSString *const AnimaXMonitorTriggerRelease = @"onRelease";
static NSString *const AnimaXMonitorMetricFPS = @"FPS";
static NSString *const AnimaXMonitorKey = @"default";

static NSString *const AnimaXMonitorPlatformPlayCount = @"play_count";
static NSString *const AnimaXMonitorPlatformStayDuration = @"stay_duration";
static NSString *const AnimaXMonitorPlatformDisplayMode = @"display_mode";
static NSString *const AnimaXMonitorPlatformTag = @"tag";

static const NSTimeInterval AnimaXMonitorMinimumIntervalTime = 5 * 60 * 1000;
static const NSTimeInterval AnimaXMonitorMaximumPlayCount = 5;

#pragma mark - Private Interface

@interface AnimaXMonitorAbilityDelegate ()

@property(nonatomic, strong, readonly) AnimaXServiceRegistry *serviceRegistry;
@property(nonatomic, assign) std::shared_ptr<lynx::animax::AnimaXPlayer> player;
@property(nonatomic, assign) BOOL hasReportedFirstPlay;

@property(nonatomic, strong, readonly) NSMutableDictionary *platformReportItems;
@property(nonatomic, assign, readonly) double stayDurationStartTimestamp;
@property(nonatomic, assign) double lastPlayReportTimestamp;
@property(nonatomic, assign) NSInteger playCount;
@property(nonatomic, assign) NSInteger playTriggerCount;

/**
 * Reports performance metrics with a specific trigger.
 *
 * @param trigger The event that triggered the performance report
 */
- (void)reportPerformance:(NSString *)trigger;

@end

#pragma mark - Implementation

@implementation AnimaXMonitorAbilityDelegate

#pragma mark - Lifecycle

- (instancetype)initWithServiceRegistry:(AnimaXServiceRegistry *)serviceRegistry {
  self = [super init];
  if (self) {
    _serviceRegistry = serviceRegistry;
    _hasReportedFirstPlay = NO;
    _platformReportItems = [NSMutableDictionary dictionary];
    _stayDurationStartTimestamp = CACurrentMediaTime() * 1000;
    _lastPlayReportTimestamp = CACurrentMediaTime() * 1000;
  }
  return self;
}

#pragma mark - Configuration

- (void)setAnimaXPlayer:(void *)player {
  _player = *(static_cast<std::shared_ptr<lynx::animax::AnimaXPlayer> *>(player));
}

- (void)updateUrl:(NSString *)url {
  id<AnimaXMonitorService> monitor =
      [self.serviceRegistry getService:@protocol(AnimaXMonitorService)];
  if (monitor) {
    [monitor.urlHolder updateUrl:url];
  }
}

- (void)setDisplayMode:(NSString *)mode {
  [[self platformReportItems] setObject:mode forKey:AnimaXMonitorPlatformDisplayMode];
}

- (void)setTag:(NSString *)tag {
  [[self platformReportItems] setObject:tag forKey:AnimaXMonitorPlatformTag];
}

#pragma mark - Event Handling

- (void)onError:(NSDictionary *)params {
  id<AnimaXMonitorService> monitor =
      [self.serviceRegistry getService:@protocol(AnimaXMonitorService)];
  if (monitor) {
    [monitor reportError:params];
  }
}

- (void)onRelease {
  double stayDuration = CACurrentMediaTime() * 1000 - _stayDurationStartTimestamp;
  [_platformReportItems setObject:@(stayDuration) forKey:AnimaXMonitorPlatformStayDuration];
  [self reportPerformance:AnimaXMonitorTriggerRelease];
  _player = nullptr;
}

- (void)onRepeat:(NSDictionary *)params {
  [self reportOnPlayAndTryTrigger:NO];
}

- (void)onResume {
  [self reportOnPlayAndTryTrigger:YES];
}

- (void)onPlaySegment {
  [self reportOnPlayAndTryTrigger:YES];
}

- (void)onPlay {
  [self reportOnPlayAndTryTrigger:YES];
}

#pragma mark - Private Methods

static void TransferNativeMetricsIntoPlatform(
    const lynx::animax::MetricsMap &native_metrics_map, NSMutableDictionary *platform_metrics_map,
    const lynx::animax::PlayerEventTracker::EventArray &event_array,
    const lynx::animax::PlayerEventTracker::EventNameArray &event_name_array) {
  // Collect metrics from native player
  for (const auto &[key, value] : native_metrics_map) {
    if (key.empty()) {
      continue;
    }

    NSString *platformAdaptedKey = [NSString stringWithUTF8String:key.c_str()];

    // Convert metric to NSNumber based on type
    NSNumber *platformAdaptedValue;
    if ([platformAdaptedKey isEqualToString:AnimaXMonitorMetricFPS]) {
      platformAdaptedValue = [NSNumber numberWithDouble:value];
    } else {
      platformAdaptedValue = [NSNumber numberWithLongLong:static_cast<long long>(value)];
    }
    [platform_metrics_map setObject:platformAdaptedValue forKey:platformAdaptedKey];
  }

  // Collect event tracking data
  for (size_t i = 0; i < lynx::animax::PlayerEventTracker::kNumEventTypes; ++i) {
    [platform_metrics_map setObject:@(event_array[i])
                             forKey:[NSString stringWithUTF8String:event_name_array[i].c_str()]];
  }
}
/**
 * Called when onRepeat, resume, play or play segment.
 * If the last report was within MinimumIntervalTime and the cumulative play count is less than
 * MaximumPlayCount, do not report.
 */
- (void)reportOnPlayAndTryTrigger:(BOOL)trigger {
  _playCount++;
  if (trigger) {
    _playTriggerCount++;
  }
  double currentTime = CACurrentMediaTime() * 1000;
  if (!_hasReportedFirstPlay || _playTriggerCount >= AnimaXMonitorMaximumPlayCount ||
      (currentTime - _lastPlayReportTimestamp) >= AnimaXMonitorMinimumIntervalTime) {
    [self reportPerformance:AnimaXMonitorTriggerPlay];
    _hasReportedFirstPlay = YES;
    _lastPlayReportTimestamp = currentTime;
  }
}

- (void)reportPerformance:(NSString *)trigger {
  if (_player == nullptr) {
    return;
  }

  id<AnimaXMonitorService> monitor =
      [self.serviceRegistry getService:@protocol(AnimaXMonitorService)];
  if (monitor == nullptr) {
    return;
  }
  auto metricsDict = [_platformReportItems mutableCopy];
  if (_playCount > 0) {
    [metricsDict setObject:[NSNumber numberWithInteger:_playCount]
                    forKey:AnimaXMonitorPlatformPlayCount];
    _playCount = 0;
    _playTriggerCount = 0;
  }

  lynx::animax::ExternalMetricsReadyCallback callback =
      [trigger, monitor, player = _player,
       metricsDict](const lynx::animax::MetricsMap &native_metrics_map) {
        TransferNativeMetricsIntoPlatform(native_metrics_map, metricsDict,
                                          player->GetEventTrackingArray(), player->GetEventNames());

        // Add trigger information
        [metricsDict setObject:trigger forKey:AnimaXMonitorKey];

        [monitor reportPerformance:[metricsDict copy]];
      };

  _player->ExportDataFromMetricsManager(std::move(callback));
}

@end
