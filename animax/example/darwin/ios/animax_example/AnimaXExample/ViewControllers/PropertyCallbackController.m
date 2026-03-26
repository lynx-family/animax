// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import "PropertyCallbackController.h"
#import <AnimaX/AnimaXContext.h>
#import <AnimaX/AnimaXKeyPath.h>
#import <AnimaX/AnimaXPropertyCallback.h>
#import <AnimaX/AnimaXValueCallback.h>
#import <AnimaX/AnimaXValueParam.h>
#import <AnimaX/AnimaXView.h>
#import <AnimaX/BaseAnimaXAbility.h>
#import <AnimaX/LayerPropertyType.h>
#include <objc/runtime.h>

// Custom AnimaXValueCallback subclass
@interface CustomAnimaXValueCallback : AnimaXValueCallback
@property(nonatomic, copy) AnimaXValueParam * (^valueGenerator)(double progress);
- (instancetype)initWithValueGenerator:(AnimaXValueParam * (^)(double progress))valueGenerator;
@end

@implementation CustomAnimaXValueCallback

- (instancetype)initWithValueGenerator:(AnimaXValueParam * (^)(double progress))valueGenerator {
  if (self = [super init]) {
    _valueGenerator = valueGenerator;
  }
  return self;
}

- (nullable AnimaXValueParam *)getValue:(AnimaXFrameInfo *)frameInfo {
  if (self.valueGenerator) {
    return self.valueGenerator(frameInfo.overallProgress);
  }
  return nil;
}

@end

@interface PropertyCallbackController () <AnimaXPropertyCallback>
@property(nonatomic, strong) AnimaXView *animaXView;
@property(nonatomic, strong) UIScrollView *scrollView;
@property(nonatomic, strong) UIStackView *buttonStackView;
@property(nonatomic, strong) AnimaXKeyPath *keyPath;
@property(nonatomic, strong) AnimaXKeyPath *fillKeyPath;
@property(nonatomic, strong) NSMutableArray *callbacks;
@end

@implementation PropertyCallbackController

- (void)viewDidLoad {
  [super viewDidLoad];
  self.view.backgroundColor = [UIColor whiteColor];
  self.title = @"Property Callback";

  // Initialize key paths and callbacks array
  self.keyPath = [[AnimaXKeyPath alloc] initWithKeys:@[ @"**" ]];
  self.fillKeyPath = [[AnimaXKeyPath alloc] initWithKeys:@[ @"**" ]];
  self.callbacks = [[NSMutableArray alloc] init];

  [self setupUI];
  [self setupAnimation];
  [self setupButtons];
}

- (AnimaXView *)createAnimaXView {
  // Create BaseAnimaXAbility and add self as animation listener if needed
  BaseAnimaXAbility *ability = [[BaseAnimaXAbility alloc] init];

  // Create AnimaXContext with the ability
  AnimaXContext *animaxContext = [[AnimaXContext alloc] initWithAbility:ability];

  // Create AnimaXView with the context
  AnimaXView *view = [[AnimaXView alloc] initWithContext:animaxContext];

  // Configure the view settings
  view.enableNativeTapLayerEvent = NO;

  return view;
}

- (void)setupUI {
  // Close button
  UIButton *closeBtn = [UIButton buttonWithType:UIButtonTypeSystem];
  [closeBtn setImage:[UIImage systemImageNamed:@"xmark"] forState:UIControlStateNormal];
  [closeBtn addTarget:self
                action:@selector(closeTapped)
      forControlEvents:UIControlEventTouchUpInside];
  closeBtn.tintColor = [UIColor blackColor];
  closeBtn.translatesAutoresizingMaskIntoConstraints = NO;
  [self.view addSubview:closeBtn];

  // Title Label
  UILabel *titleLabel = [[UILabel alloc] init];
  titleLabel.text = self.title;
  titleLabel.font = [UIFont boldSystemFontOfSize:17];
  titleLabel.textAlignment = NSTextAlignmentCenter;
  titleLabel.translatesAutoresizingMaskIntoConstraints = NO;
  [self.view addSubview:titleLabel];

  // Create animation view (fixed at top)
  self.animaXView = [self createAnimaXView];
  self.animaXView.translatesAutoresizingMaskIntoConstraints = NO;
  [self.view addSubview:self.animaXView];

  // Create scroll view for buttons only
  self.scrollView = [[UIScrollView alloc] init];
  self.scrollView.translatesAutoresizingMaskIntoConstraints = NO;
  [self.view addSubview:self.scrollView];

  // Create button stack view
  self.buttonStackView = [[UIStackView alloc] init];
  self.buttonStackView.axis = UILayoutConstraintAxisVertical;
  self.buttonStackView.spacing = 8;
  self.buttonStackView.translatesAutoresizingMaskIntoConstraints = NO;
  [self.scrollView addSubview:self.buttonStackView];

  // Set up constraints
  [NSLayoutConstraint activateConstraints:@[
    // Close button
    [closeBtn.topAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.topAnchor
                                       constant:10],
    [closeBtn.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor constant:-20],
    [closeBtn.widthAnchor constraintEqualToConstant:44],
    [closeBtn.heightAnchor constraintEqualToConstant:44],

    // Title Label
    [titleLabel.centerYAnchor constraintEqualToAnchor:closeBtn.centerYAnchor],
    [titleLabel.centerXAnchor constraintEqualToAnchor:self.view.centerXAnchor],

    // Animation view constraints (fixed at top)
    [self.animaXView.topAnchor constraintEqualToAnchor:closeBtn.bottomAnchor constant:10],
    [self.animaXView.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor constant:16],
    [self.animaXView.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor constant:-16],
    [self.animaXView.heightAnchor constraintEqualToConstant:200],

    // Scroll view constraints (below animation view)
    [self.scrollView.topAnchor constraintEqualToAnchor:self.animaXView.bottomAnchor constant:16],
    [self.scrollView.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor],
    [self.scrollView.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor],
    [self.scrollView.bottomAnchor constraintEqualToAnchor:self.view.bottomAnchor],

    // Button stack view constraints
    [self.buttonStackView.topAnchor constraintEqualToAnchor:self.scrollView.topAnchor constant:16],
    [self.buttonStackView.leadingAnchor constraintEqualToAnchor:self.scrollView.leadingAnchor
                                                       constant:16],
    [self.buttonStackView.trailingAnchor constraintEqualToAnchor:self.scrollView.trailingAnchor
                                                        constant:-16],
    [self.buttonStackView.bottomAnchor constraintEqualToAnchor:self.scrollView.bottomAnchor
                                                      constant:-16],
    [self.buttonStackView.widthAnchor constraintEqualToAnchor:self.scrollView.widthAnchor
                                                     constant:-32]
  ]];
}

- (void)setupAnimation {
  [self.animaXView setAutoplay:YES];
  [self.animaXView setLoop:YES];
  NSBundle *bundle = [NSBundle mainBundle];
  NSString *jsonPath = [bundle pathForResource:@"data" ofType:@"json"];
  if (jsonPath) {
    NSURL *url = [NSURL fileURLWithPath:jsonPath];
    [_animaXView setSrc:url.absoluteString];
  }
}

- (void)setupButtons {
  // Transform properties (matching Android exactly)
  [self addCallbackButtonWithTitle:@"Visibility"
                      propertyType:LayerPropertyTypeVisibility
                           keyPath:self.keyPath
                    valueGenerator:^AnimaXValueParam *(double progress) {
                      int visible = sin(progress * M_PI * 4) > 0 ? 1 : 0;
                      return [AnimaXValueParam paramWithNumber:visible];
                    }];

  [self addCallbackButtonWithTitle:@"Transform Opacity"
                      propertyType:LayerPropertyTypeTransformOpacity
                           keyPath:self.keyPath
                    valueGenerator:^AnimaXValueParam *(double progress) {
                      double opacity = fabs(sin(progress * M_PI)) * 100;
                      return [AnimaXValueParam paramWithNumber:opacity];
                    }];

  [self addCallbackButtonWithTitle:@"Transform Anchor"
                      propertyType:LayerPropertyTypeTransformAnchor
                           keyPath:self.keyPath
                    valueGenerator:^AnimaXValueParam *(double progress) {
                      double radius = 20.0;
                      double x = radius * cos(progress * 2 * M_PI);
                      double y = radius * sin(progress * 2 * M_PI);
                      return [AnimaXValueParam paramWithCoordinateX:x y:y];
                    }];

  [self addCallbackButtonWithTitle:@"Transform Position"
                      propertyType:LayerPropertyTypeTransformPosition
                           keyPath:self.keyPath
                    valueGenerator:^AnimaXValueParam *(double progress) {
                      double radius = 50.0;
                      double x = radius * cos(progress * 2 * M_PI);
                      double y = radius * sin(progress * 2 * M_PI);
                      return [AnimaXValueParam paramWithCoordinateX:x y:y];
                    }];

  [self addCallbackButtonWithTitle:@"Transform Scale"
                      propertyType:LayerPropertyTypeTransformScale
                           keyPath:self.keyPath
                    valueGenerator:^AnimaXValueParam *(double progress) {
                      double scale = 0.5 + fabs(sin(progress * M_PI)) * 1.0;
                      return [AnimaXValueParam paramWithCoordinateX:scale y:scale];
                    }];

  [self addCallbackButtonWithTitle:@"Transform Rotation"
                      propertyType:LayerPropertyTypeTransformRotation
                           keyPath:self.keyPath
                    valueGenerator:^AnimaXValueParam *(double progress) {
                      double degrees = progress * 360;
                      return [AnimaXValueParam paramWithNumber:degrees];
                    }];

  [self addCallbackButtonWithTitle:@"Transform Skew"
                      propertyType:LayerPropertyTypeTransformSkew
                           keyPath:self.keyPath
                    valueGenerator:^AnimaXValueParam *(double progress) {
                      double skew = sin(progress * 2 * M_PI) * 30;
                      return [AnimaXValueParam paramWithNumber:skew];
                    }];

  [self addCallbackButtonWithTitle:@"Transform Skew Angle"
                      propertyType:LayerPropertyTypeTransformSkewAngle
                           keyPath:self.keyPath
                    valueGenerator:^AnimaXValueParam *(double progress) {
                      double skewAngle = progress * 90;
                      return [AnimaXValueParam paramWithNumber:skewAngle];
                    }];

  [self addCallbackButtonWithTitle:@"Transform Start Opacity"
                      propertyType:LayerPropertyTypeTransformStartOpacity
                           keyPath:self.keyPath
                    valueGenerator:^AnimaXValueParam *(double progress) {
                      double opacity = progress * 100;
                      return [AnimaXValueParam paramWithNumber:opacity];
                    }];

  [self addCallbackButtonWithTitle:@"Transform End Opacity"
                      propertyType:LayerPropertyTypeTransformEndOpacity
                           keyPath:self.keyPath
                    valueGenerator:^AnimaXValueParam *(double progress) {
                      double opacity = (1 - progress) * 100;
                      return [AnimaXValueParam paramWithNumber:opacity];
                    }];

  [self addCallbackButtonWithTitle:@"Transform Rotation X"
                      propertyType:LayerPropertyTypeTransformRotationX
                           keyPath:self.keyPath
                    valueGenerator:^AnimaXValueParam *(double progress) {
                      double degrees = sin(progress * M_PI) * 180;
                      return [AnimaXValueParam paramWithNumber:degrees];
                    }];

  [self addCallbackButtonWithTitle:@"Transform Rotation Y"
                      propertyType:LayerPropertyTypeTransformRotationY
                           keyPath:self.keyPath
                    valueGenerator:^AnimaXValueParam *(double progress) {
                      double degrees = cos(progress * M_PI) * 180;
                      return [AnimaXValueParam paramWithNumber:degrees];
                    }];

  [self addCallbackButtonWithTitle:@"Transform Rotation Z"
                      propertyType:LayerPropertyTypeTransformRotationZ
                           keyPath:self.keyPath
                    valueGenerator:^AnimaXValueParam *(double progress) {
                      double degrees = progress * 360;
                      return [AnimaXValueParam paramWithNumber:degrees];
                    }];

  // Text properties
  [self addCallbackButtonWithTitle:@"Text Value"
                      propertyType:LayerPropertyTypeTextValue
                           keyPath:self.keyPath
                    valueGenerator:^AnimaXValueParam *(double progress) {
                      int progressPercent = (int)(progress * 100);
                      NSString *text =
                          [NSString stringWithFormat:@"Progress: %d%%", progressPercent];
                      return [AnimaXValueParam paramWithString:text];
                    }];

  [self addCallbackButtonWithTitle:@"Text Size"
                      propertyType:LayerPropertyTypeTextSize
                           keyPath:self.keyPath
                    valueGenerator:^AnimaXValueParam *(double progress) {
                      double size = 50 + fabs(sin(progress * M_PI)) * 24;
                      return [AnimaXValueParam paramWithNumber:size];
                    }];

  [self addCallbackButtonWithTitle:@"Text Color"
                      propertyType:LayerPropertyTypeTextColor
                           keyPath:self.keyPath
                    valueGenerator:^AnimaXValueParam *(double progress) {
                      return [self generateColorValue:progress];
                    }];

  [self addCallbackButtonWithTitle:@"Text Tracking"
                      propertyType:LayerPropertyTypeTextTracking
                           keyPath:self.keyPath
                    valueGenerator:^AnimaXValueParam *(double progress) {
                      double tracking = sin(progress * 2 * M_PI) * 10;
                      return [AnimaXValueParam paramWithNumber:tracking];
                    }];

  // Color and visual properties
  [self addCallbackButtonWithTitle:@"Color"
                      propertyType:LayerPropertyTypeColor
                           keyPath:self.fillKeyPath
                    valueGenerator:^AnimaXValueParam *(double progress) {
                      return [self generateColorValue:progress];
                    }];

  [self addCallbackButtonWithTitle:@"Color Filter"
                      propertyType:LayerPropertyTypeColorFilter
                           keyPath:self.fillKeyPath
                    valueGenerator:^AnimaXValueParam *(double progress) {
                      return [self generateColorFilterValue:progress];
                    }];

  [self addCallbackButtonWithTitle:@"Stroke Color"
                      propertyType:LayerPropertyTypeStrokeColor
                           keyPath:self.fillKeyPath
                    valueGenerator:^AnimaXValueParam *(double progress) {
                      return [self generateColorValue:progress];
                    }];

  [self addCallbackButtonWithTitle:@"Stroke Width"
                      propertyType:LayerPropertyTypeStrokeWidth
                           keyPath:self.fillKeyPath
                    valueGenerator:^AnimaXValueParam *(double progress) {
                      double width = 1 + fabs(sin(progress * M_PI)) * 10;
                      return [AnimaXValueParam paramWithNumber:width];
                    }];

  [self addCallbackButtonWithTitle:@"Content Opacity"
                      propertyType:LayerPropertyTypeOpacity
                           keyPath:self.fillKeyPath
                    valueGenerator:^AnimaXValueParam *(double progress) {
                      double opacity = fabs(sin(progress * M_PI)) * 100;
                      return [AnimaXValueParam paramWithNumber:opacity];
                    }];

  [self addCallbackButtonWithTitle:@"Blur Radius"
                      propertyType:LayerPropertyTypeBlurRadius
                           keyPath:self.fillKeyPath
                    valueGenerator:^AnimaXValueParam *(double progress) {
                      double radius = fabs(sin(progress * M_PI)) * 20;
                      return [AnimaXValueParam paramWithNumber:radius];
                    }];

  [self addCallbackButtonWithTitle:@"Ellipse Size"
                      propertyType:LayerPropertyTypeEllipseSize
                           keyPath:self.fillKeyPath
                    valueGenerator:^AnimaXValueParam *(double progress) {
                      double size = 50 + fabs(sin(progress * M_PI)) * 100;
                      return [AnimaXValueParam paramWithCoordinateX:size y:size];
                    }];

  [self addCallbackButtonWithTitle:@"Rectangle Size"
                      propertyType:LayerPropertyTypeRectangleSize
                           keyPath:self.fillKeyPath
                    valueGenerator:^AnimaXValueParam *(double progress) {
                      double width = 50 + fabs(sin(progress * M_PI)) * 100;
                      double height = 50 + fabs(cos(progress * M_PI)) * 100;
                      return [AnimaXValueParam paramWithCoordinateX:width y:height];
                    }];

  [self addCallbackButtonWithTitle:@"Corner Radius"
                      propertyType:LayerPropertyTypeCornerRadius
                           keyPath:self.fillKeyPath
                    valueGenerator:^AnimaXValueParam *(double progress) {
                      double radius = fabs(sin(progress * M_PI)) * 50;
                      return [AnimaXValueParam paramWithNumber:radius];
                    }];

  [self addCallbackButtonWithTitle:@"Content Position"
                      propertyType:LayerPropertyTypePosition
                           keyPath:self.fillKeyPath
                    valueGenerator:^AnimaXValueParam *(double progress) {
                      double radius = 30.0;
                      double x = radius * cos(progress * 2 * M_PI);
                      double y = radius * sin(progress * 2 * M_PI);
                      return [AnimaXValueParam paramWithCoordinateX:x y:y];
                    }];

  [self addCallbackButtonWithTitle:@"Repeater Copies"
                      propertyType:LayerPropertyTypeRepeaterCopies
                           keyPath:self.fillKeyPath
                    valueGenerator:^AnimaXValueParam *(double progress) {
                      int copies = 1 + (int)(progress * 5);
                      return [AnimaXValueParam paramWithNumber:copies];
                    }];

  [self addCallbackButtonWithTitle:@"Repeater Offset"
                      propertyType:LayerPropertyTypeRepeaterOffset
                           keyPath:self.fillKeyPath
                    valueGenerator:^AnimaXValueParam *(double progress) {
                      double offset = progress * 100;
                      return [AnimaXValueParam paramWithCoordinateX:offset y:offset];
                    }];

  // PolyStar properties
  [self addCallbackButtonWithTitle:@"PolyStar Points"
                      propertyType:LayerPropertyTypePolyStarPoints
                           keyPath:self.fillKeyPath
                    valueGenerator:^AnimaXValueParam *(double progress) {
                      int points = 3 + (int)(progress * 5);
                      return [AnimaXValueParam paramWithNumber:points];
                    }];

  [self addCallbackButtonWithTitle:@"PolyStar Rotation"
                      propertyType:LayerPropertyTypePolyStarRotation
                           keyPath:self.fillKeyPath
                    valueGenerator:^AnimaXValueParam *(double progress) {
                      double rotation = progress * 360;
                      return [AnimaXValueParam paramWithNumber:rotation];
                    }];

  [self addCallbackButtonWithTitle:@"PolyStar Inner Radius"
                      propertyType:LayerPropertyTypePolyStarInnerRadius
                           keyPath:self.fillKeyPath
                    valueGenerator:^AnimaXValueParam *(double progress) {
                      double radius = 10 + fabs(sin(progress * M_PI)) * 40;
                      return [AnimaXValueParam paramWithNumber:radius];
                    }];

  [self addCallbackButtonWithTitle:@"PolyStar Outer Radius"
                      propertyType:LayerPropertyTypePolyStarOuterRadius
                           keyPath:self.fillKeyPath
                    valueGenerator:^AnimaXValueParam *(double progress) {
                      double radius = 30 + fabs(cos(progress * M_PI)) * 50;
                      return [AnimaXValueParam paramWithNumber:radius];
                    }];

  [self addCallbackButtonWithTitle:@"PolyStar Inner Rounded"
                      propertyType:LayerPropertyTypePolyStarInnerRounded
                           keyPath:self.fillKeyPath
                    valueGenerator:^AnimaXValueParam *(double progress) {
                      double rounded = fabs(sin(progress * M_PI)) * 20;
                      return [AnimaXValueParam paramWithNumber:rounded];
                    }];

  [self addCallbackButtonWithTitle:@"PolyStar Outer Rounded"
                      propertyType:LayerPropertyTypePolyStarOuterRounded
                           keyPath:self.fillKeyPath
                    valueGenerator:^AnimaXValueParam *(double progress) {
                      double rounded = fabs(cos(progress * M_PI)) * 20;
                      return [AnimaXValueParam paramWithNumber:rounded];
                    }];

  // Drop Shadow properties
  [self addCallbackButtonWithTitle:@"Drop Shadow Color"
                      propertyType:LayerPropertyTypeDropShadowColor
                           keyPath:self.fillKeyPath
                    valueGenerator:^AnimaXValueParam *(double progress) {
                      return [self generateColorValue:progress];
                    }];

  [self addCallbackButtonWithTitle:@"Drop Shadow Opacity"
                      propertyType:LayerPropertyTypeDropShadowOpacity
                           keyPath:self.fillKeyPath
                    valueGenerator:^AnimaXValueParam *(double progress) {
                      double opacity = fabs(sin(progress * M_PI)) * 100;
                      return [AnimaXValueParam paramWithNumber:opacity];
                    }];

  [self addCallbackButtonWithTitle:@"Drop Shadow Direction"
                      propertyType:LayerPropertyTypeDropShadowDirection
                           keyPath:self.fillKeyPath
                    valueGenerator:^AnimaXValueParam *(double progress) {
                      double direction = progress * 360;
                      return [AnimaXValueParam paramWithNumber:direction];
                    }];

  [self addCallbackButtonWithTitle:@"Drop Shadow Distance"
                      propertyType:LayerPropertyTypeDropShadowDistance
                           keyPath:self.fillKeyPath
                    valueGenerator:^AnimaXValueParam *(double progress) {
                      double distance = fabs(sin(progress * M_PI)) * 50;
                      return [AnimaXValueParam paramWithNumber:distance];
                    }];

  [self addCallbackButtonWithTitle:@"Drop Shadow Radius"
                      propertyType:LayerPropertyTypeDropShadowRadius
                           keyPath:self.fillKeyPath
                    valueGenerator:^AnimaXValueParam *(double progress) {
                      double radius = fabs(cos(progress * M_PI)) * 20;
                      return [AnimaXValueParam paramWithNumber:radius];
                    }];

  // Reset button
  [self addButtonWithTitle:@"Reset All Callbacks" action:@selector(resetCallbacks)];
}

- (void)addCallbackButtonWithTitle:(NSString *)title
                      propertyType:(LayerPropertyType)propertyType
                           keyPath:(AnimaXKeyPath *)keyPath
                    valueGenerator:(AnimaXValueParam * (^)(double progress))valueGenerator {
  UIButton *button = [UIButton buttonWithType:UIButtonTypeSystem];
  [button setTitle:title forState:UIControlStateNormal];
  [button setTitleColor:[UIColor whiteColor] forState:UIControlStateNormal];
  button.backgroundColor = [UIColor systemBlueColor];
  button.layer.cornerRadius = 8;
  button.titleLabel.font = [UIFont systemFontOfSize:16];

  [button addTarget:self
                action:@selector(addPropertyCallback:)
      forControlEvents:UIControlEventTouchUpInside];

  // Store the configuration in the button's tag and associated objects
  button.tag = propertyType;
  objc_setAssociatedObject(button, "keyPath", keyPath, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
  objc_setAssociatedObject(button, "valueGenerator", valueGenerator,
                           OBJC_ASSOCIATION_COPY_NONATOMIC);
  objc_setAssociatedObject(button, "title", title, OBJC_ASSOCIATION_COPY_NONATOMIC);

  NSLayoutConstraint *heightConstraint = [button.heightAnchor constraintEqualToConstant:44];
  heightConstraint.active = YES;

  [self.buttonStackView addArrangedSubview:button];
}

- (void)addButtonWithTitle:(NSString *)title action:(SEL)action {
  UIButton *button = [UIButton buttonWithType:UIButtonTypeSystem];
  [button setTitle:title forState:UIControlStateNormal];
  [button setTitleColor:[UIColor whiteColor] forState:UIControlStateNormal];
  button.backgroundColor = [UIColor systemGreenColor];
  button.layer.cornerRadius = 8;
  button.titleLabel.font = [UIFont systemFontOfSize:16];

  [button addTarget:self action:action forControlEvents:UIControlEventTouchUpInside];

  NSLayoutConstraint *heightConstraint = [button.heightAnchor constraintEqualToConstant:44];
  heightConstraint.active = YES;

  [self.buttonStackView addArrangedSubview:button];
}

- (void)addPropertyCallback:(UIButton *)sender {
  LayerPropertyType propertyType = sender.tag;
  AnimaXKeyPath *keyPath = objc_getAssociatedObject(sender, "keyPath");
  AnimaXValueParam * (^valueGenerator)(double progress) =
      objc_getAssociatedObject(sender, "valueGenerator");
  NSString *title = objc_getAssociatedObject(sender, "title");

  CustomAnimaXValueCallback *callback =
      [[CustomAnimaXValueCallback alloc] initWithValueGenerator:valueGenerator];

  [self.animaXView addLayerPropertyCallback:propertyType
                                    keyPath:keyPath
                              valueCallback:callback
                                   callback:self];

  [self.callbacks addObject:callback];
  NSLog(@"Added callback for %@", title);
}

- (void)resetCallbacks {
  [self.callbacks removeAllObjects];
  [self.animaXView reload];
  NSLog(@"All callbacks reset");
}

#pragma mark - AnimaXPropertyCallback

- (void)onSuccess {
  NSLog(@"Property callback added successfully");
}

- (void)onError:(NSString *)errorMessage {
  NSLog(@"Property callback error: %@", errorMessage);
}

#pragma mark - Actions

- (void)closeTapped {
  [self dismissViewControllerAnimated:YES completion:nil];
}

// Helper methods for color generation (matching Android implementation)
- (AnimaXValueParam *)generateColorValue:(double)progress {
  if (progress < 0.2) {
    return [AnimaXValueParam paramWithColor:0xFFFF0000];  // Red
  } else if (progress < 0.4) {
    return [AnimaXValueParam paramWithColor:0xFF00FF00];  // Green
  } else if (progress < 0.6) {
    return [AnimaXValueParam paramWithColor:0xFF0000FF];  // Blue
  } else if (progress < 0.8) {
    return [AnimaXValueParam paramWithColor:0xFFFF00FF];  // Purple
  } else {
    return [AnimaXValueParam paramWithColor:0xFFFFA500];  // Orange
  }
}

- (AnimaXValueParam *)generateColorFilterValue:(double)progress {
  if (progress < 0.125) {
    return [AnimaXValueParam paramWithColorFilter:0x00000000 mode:0];  // SRC_OVER
  } else if (progress < 0.25) {
    return [AnimaXValueParam paramWithColorFilter:0xFF0000FF mode:1];  // MULTIPLY
  } else if (progress < 0.375) {
    return [AnimaXValueParam paramWithColorFilter:0x80FFFFFF mode:2];  // SCREEN
  } else if (progress < 0.5) {
    return [AnimaXValueParam paramWithColorFilter:0x80000000 mode:1];  // MULTIPLY
  } else if (progress < 0.75) {
    return [AnimaXValueParam paramWithColorFilter:0xFF00FF00 mode:3];  // OVERLAY
  } else if (progress < 0.875) {
    return [AnimaXValueParam paramWithColorFilter:0xCC0080FF mode:4];  // DST
  } else {
    return [AnimaXValueParam paramWithColorFilter:0x80FFFF00 mode:5];  // SRC
  }
}

@end
