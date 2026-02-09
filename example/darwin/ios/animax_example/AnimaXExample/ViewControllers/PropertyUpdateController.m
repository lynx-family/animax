// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import "PropertyUpdateController.h"
#import <AnimaX/AnimaXContext.h>
#import <AnimaX/AnimaXKeyPath.h>
#import <AnimaX/AnimaXKeyPathListCallback.h>
#import <AnimaX/AnimaXPropertyCallback.h>
#import <AnimaX/AnimaXValueParam.h>
#import <AnimaX/AnimaXView.h>
#import <AnimaX/BaseAnimaXAbility.h>
#import <AnimaX/LayerPropertyType.h>

@interface PropertyUpdateController () <AnimaXPropertyCallback, AnimaXKeyPathListCallback>
@property(nonatomic, strong) AnimaXView *animaXView;
@property(nonatomic, strong) UIScrollView *scrollView;
@property(nonatomic, strong) UIStackView *buttonStackView;
@property(nonatomic, strong) AnimaXKeyPath *keyPath;
@property(nonatomic, strong) AnimaXKeyPath *fillKeyPath;

@end

@implementation PropertyUpdateController

- (void)viewDidLoad {
  [super viewDidLoad];
  self.view.backgroundColor = [UIColor whiteColor];
  self.title = @"Property Update";

  // Initialize key paths
  self.keyPath = [[AnimaXKeyPath alloc] initWithKeys:@[ @"**" ]];
  self.fillKeyPath = [[AnimaXKeyPath alloc] initWithKeys:@[ @"**" ]];

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

    // Animation view constraints (below close button)
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
  // Transform properties
  [self addButtonWithTitle:@"Toggle Visibility" action:@selector(toggleVisibility)];
  [self addButtonWithTitle:@"Set Opacity 50%" action:@selector(setOpacity50)];
  [self addButtonWithTitle:@"Set Anchor Point" action:@selector(setAnchorPoint)];
  [self addButtonWithTitle:@"Move Position" action:@selector(movePosition)];
  [self addButtonWithTitle:@"Scale 150%" action:@selector(scale150)];
  [self addButtonWithTitle:@"Rotate 45°" action:@selector(rotate45)];
  [self addButtonWithTitle:@"Skew 15°" action:@selector(skew15)];
  [self addButtonWithTitle:@"Skew Angle 30°" action:@selector(skewAngle30)];
  [self addButtonWithTitle:@"Start Opacity 80%" action:@selector(startOpacity80)];
  [self addButtonWithTitle:@"End Opacity 20%" action:@selector(endOpacity20)];
  [self addButtonWithTitle:@"Rotate X 90°" action:@selector(rotateX90)];
  [self addButtonWithTitle:@"Rotate Y 45°" action:@selector(rotateY45)];
  [self addButtonWithTitle:@"Rotate Z 180°" action:@selector(rotateZ180)];

  // Text properties
  [self addButtonWithTitle:@"Set Text \"UPDATED\"" action:@selector(setTextUpdated)];
  [self addButtonWithTitle:@"Text Size 72px" action:@selector(textSize72)];
  [self addButtonWithTitle:@"Text Color Red" action:@selector(textColorRed)];
  [self addButtonWithTitle:@"Text Tracking 5" action:@selector(textTracking5)];

  // Color and visual properties
  [self addButtonWithTitle:@"Fill Color Blue" action:@selector(fillColorBlue)];
  [self addButtonWithTitle:@"Color Filter Green" action:@selector(colorFilterGreen)];
  [self addButtonWithTitle:@"Stroke Color Purple" action:@selector(strokeColorPurple)];
  [self addButtonWithTitle:@"Stroke Width 8px" action:@selector(strokeWidth8)];
  [self addButtonWithTitle:@"Content Opacity 75%" action:@selector(contentOpacity75)];
  [self addButtonWithTitle:@"Blur Radius 10px" action:@selector(blurRadius10)];
  [self addButtonWithTitle:@"Ellipse Size 120x120" action:@selector(ellipseSize120)];
  [self addButtonWithTitle:@"Rectangle Size 150x100" action:@selector(rectangleSize150x100)];
  [self addButtonWithTitle:@"Corner Radius 25px" action:@selector(cornerRadius25)];
  [self addButtonWithTitle:@"Content Position 60x60" action:@selector(contentPosition60x60)];
  [self addButtonWithTitle:@"Repeater 4 Copies" action:@selector(repeater4Copies)];
  [self addButtonWithTitle:@"Repeater Offset 50x50" action:@selector(repeaterOffset50x50)];

  // PolyStar properties
  [self addButtonWithTitle:@"PolyStar 6 Points" action:@selector(polyStar6Points)];
  [self addButtonWithTitle:@"PolyStar Rotation 60°" action:@selector(polyStarRotation60)];
  [self addButtonWithTitle:@"PolyStar Inner Radius 30px" action:@selector(polyStarInnerRadius30)];
  [self addButtonWithTitle:@"PolyStar Outer Radius 60px" action:@selector(polyStarOuterRadius60)];
  [self addButtonWithTitle:@"PolyStar Inner Rounded 10px" action:@selector(polyStarInnerRounded10)];
  [self addButtonWithTitle:@"PolyStar Outer Rounded 15px" action:@selector(polyStarOuterRounded15)];

  // Drop Shadow properties
  [self addButtonWithTitle:@"Shadow Color Black" action:@selector(shadowColorBlack)];
  [self addButtonWithTitle:@"Shadow Opacity 80%" action:@selector(shadowOpacity80)];
  [self addButtonWithTitle:@"Shadow Direction 135°" action:@selector(shadowDirection135)];
  [self addButtonWithTitle:@"Shadow Distance 20px" action:@selector(shadowDistance20)];
  [self addButtonWithTitle:@"Shadow Blur 10px" action:@selector(shadowBlur10)];

  // Reset button
  [self addButtonWithTitle:@"Reset Animation" action:@selector(resetAnimation)];
  [self addButtonWithTitle:@"Get Keys for KeyPath '**'" action:@selector(getKeys)];
}

- (void)addButtonWithTitle:(NSString *)title action:(SEL)action {
  UIButton *button = [UIButton buttonWithType:UIButtonTypeSystem];
  [button setTitle:title forState:UIControlStateNormal];
  [button setTitleColor:[UIColor whiteColor] forState:UIControlStateNormal];
  button.backgroundColor = [UIColor systemBlueColor];
  button.layer.cornerRadius = 8;
  button.titleLabel.font = [UIFont systemFontOfSize:16];

  [button addTarget:self action:action forControlEvents:UIControlEventTouchUpInside];

  NSLayoutConstraint *heightConstraint = [button.heightAnchor constraintEqualToConstant:44];
  heightConstraint.active = YES;

  [self.buttonStackView addArrangedSubview:button];
}

#pragma mark - Transform Properties

- (void)toggleVisibility {
  [self.animaXView updateLayerProperty:self.keyPath
                          propertyType:LayerPropertyTypeVisibility
                                 value:[AnimaXValueParam paramWithNumber:0.0]
                              callback:self];
}

- (void)setOpacity50 {
  [self.animaXView updateLayerProperty:self.keyPath
                          propertyType:LayerPropertyTypeTransformOpacity
                                 value:[AnimaXValueParam paramWithNumber:50.0]
                              callback:self];
}

- (void)setAnchorPoint {
  [self.animaXView updateLayerProperty:self.keyPath
                          propertyType:LayerPropertyTypeTransformAnchor
                                 value:[AnimaXValueParam paramWithCoordinateX:25.0 y:25.0]
                              callback:self];
}

- (void)movePosition {
  [self.animaXView updateLayerProperty:self.keyPath
                          propertyType:LayerPropertyTypeTransformPosition
                                 value:[AnimaXValueParam paramWithCoordinateX:50.0 y:50.0]
                              callback:self];
}

- (void)scale150 {
  [self.animaXView updateLayerProperty:self.keyPath
                          propertyType:LayerPropertyTypeTransformScale
                                 value:[AnimaXValueParam paramWithCoordinateX:1.5 y:1.5]
                              callback:self];
}

- (void)rotate45 {
  [self.animaXView updateLayerProperty:self.keyPath
                          propertyType:LayerPropertyTypeTransformRotation
                                 value:[AnimaXValueParam paramWithNumber:45.0]
                              callback:self];
}

- (void)skew15 {
  [self.animaXView updateLayerProperty:self.keyPath
                          propertyType:LayerPropertyTypeTransformSkew
                                 value:[AnimaXValueParam paramWithNumber:15.0]
                              callback:self];
}

- (void)skewAngle30 {
  [self.animaXView updateLayerProperty:self.keyPath
                          propertyType:LayerPropertyTypeTransformSkewAngle
                                 value:[AnimaXValueParam paramWithNumber:30.0]
                              callback:self];
}

- (void)startOpacity80 {
  [self.animaXView updateLayerProperty:self.keyPath
                          propertyType:LayerPropertyTypeTransformStartOpacity
                                 value:[AnimaXValueParam paramWithNumber:80.0]
                              callback:self];
}

- (void)endOpacity20 {
  [self.animaXView updateLayerProperty:self.keyPath
                          propertyType:LayerPropertyTypeTransformEndOpacity
                                 value:[AnimaXValueParam paramWithNumber:20.0]
                              callback:self];
}

- (void)rotateX90 {
  [self.animaXView updateLayerProperty:self.keyPath
                          propertyType:LayerPropertyTypeTransformRotationX
                                 value:[AnimaXValueParam paramWithNumber:90.0]
                              callback:self];
}

- (void)rotateY45 {
  [self.animaXView updateLayerProperty:self.keyPath
                          propertyType:LayerPropertyTypeTransformRotationY
                                 value:[AnimaXValueParam paramWithNumber:45.0]
                              callback:self];
}

- (void)rotateZ180 {
  [self.animaXView updateLayerProperty:self.keyPath
                          propertyType:LayerPropertyTypeTransformRotationZ
                                 value:[AnimaXValueParam paramWithNumber:180.0]
                              callback:self];
}

#pragma mark - Text Properties

- (void)setTextUpdated {
  [self.animaXView updateLayerProperty:self.keyPath
                          propertyType:LayerPropertyTypeTextValue
                                 value:[AnimaXValueParam paramWithString:@"UPDATED"]
                              callback:self];
}

- (void)textSize72 {
  [self.animaXView updateLayerProperty:self.keyPath
                          propertyType:LayerPropertyTypeTextSize
                                 value:[AnimaXValueParam paramWithNumber:72.0]
                              callback:self];
}

- (void)textColorRed {
  [self.animaXView updateLayerProperty:self.keyPath
                          propertyType:LayerPropertyTypeTextColor
                                 value:[AnimaXValueParam paramWithColor:0xFFFF0000]
                              callback:self];
}

- (void)textTracking5 {
  [self.animaXView updateLayerProperty:self.keyPath
                          propertyType:LayerPropertyTypeTextTracking
                                 value:[AnimaXValueParam paramWithNumber:5.0]
                              callback:self];
}

#pragma mark - Color and Visual Properties

- (void)fillColorBlue {
  [self.animaXView updateLayerProperty:self.fillKeyPath
                          propertyType:LayerPropertyTypeColor
                                 value:[AnimaXValueParam paramWithColor:0xFF0000FF]
                              callback:self];
}

- (void)colorFilterGreen {
  [self.animaXView updateLayerProperty:self.fillKeyPath
                          propertyType:LayerPropertyTypeColorFilter
                                 value:[AnimaXValueParam paramWithColorFilter:0x8000FF00
                                                                         mode:1]  // MULTIPLY mode
                              callback:self];
}

- (void)strokeColorPurple {
  [self.animaXView updateLayerProperty:self.fillKeyPath
                          propertyType:LayerPropertyTypeStrokeColor
                                 value:[AnimaXValueParam paramWithColor:0xFF800080]
                              callback:self];
}

- (void)strokeWidth8 {
  [self.animaXView updateLayerProperty:self.fillKeyPath
                          propertyType:LayerPropertyTypeStrokeWidth
                                 value:[AnimaXValueParam paramWithNumber:8.0]
                              callback:self];
}

- (void)contentOpacity75 {
  [self.animaXView updateLayerProperty:self.fillKeyPath
                          propertyType:LayerPropertyTypeOpacity
                                 value:[AnimaXValueParam paramWithNumber:75.0]
                              callback:self];
}

- (void)blurRadius10 {
  [self.animaXView updateLayerProperty:self.fillKeyPath
                          propertyType:LayerPropertyTypeBlurRadius
                                 value:[AnimaXValueParam paramWithNumber:10.0]
                              callback:self];
}

- (void)ellipseSize120 {
  [self.animaXView updateLayerProperty:self.fillKeyPath
                          propertyType:LayerPropertyTypeEllipseSize
                                 value:[AnimaXValueParam paramWithCoordinateX:120.0 y:120.0]
                              callback:self];
}

- (void)rectangleSize150x100 {
  [self.animaXView updateLayerProperty:self.fillKeyPath
                          propertyType:LayerPropertyTypeRectangleSize
                                 value:[AnimaXValueParam paramWithCoordinateX:150.0 y:100.0]
                              callback:self];
}

- (void)cornerRadius25 {
  [self.animaXView updateLayerProperty:self.fillKeyPath
                          propertyType:LayerPropertyTypeCornerRadius
                                 value:[AnimaXValueParam paramWithNumber:25.0]
                              callback:self];
}

- (void)contentPosition60x60 {
  [self.animaXView updateLayerProperty:self.fillKeyPath
                          propertyType:LayerPropertyTypePosition
                                 value:[AnimaXValueParam paramWithCoordinateX:60.0 y:60.0]
                              callback:self];
}

- (void)repeater4Copies {
  [self.animaXView updateLayerProperty:self.fillKeyPath
                          propertyType:LayerPropertyTypeRepeaterCopies
                                 value:[AnimaXValueParam paramWithNumber:4.0]
                              callback:self];
}

- (void)repeaterOffset50x50 {
  [self.animaXView updateLayerProperty:self.fillKeyPath
                          propertyType:LayerPropertyTypeRepeaterOffset
                                 value:[AnimaXValueParam paramWithCoordinateX:50.0 y:50.0]
                              callback:self];
}

#pragma mark - PolyStar Properties

- (void)polyStar6Points {
  [self.animaXView updateLayerProperty:self.fillKeyPath
                          propertyType:LayerPropertyTypePolyStarPoints
                                 value:[AnimaXValueParam paramWithNumber:6.0]
                              callback:self];
}

- (void)polyStarRotation60 {
  [self.animaXView updateLayerProperty:self.fillKeyPath
                          propertyType:LayerPropertyTypePolyStarRotation
                                 value:[AnimaXValueParam paramWithNumber:60.0]
                              callback:self];
}

- (void)polyStarInnerRadius30 {
  [self.animaXView updateLayerProperty:self.fillKeyPath
                          propertyType:LayerPropertyTypePolyStarInnerRadius
                                 value:[AnimaXValueParam paramWithNumber:30.0]
                              callback:self];
}

- (void)polyStarOuterRadius60 {
  [self.animaXView updateLayerProperty:self.fillKeyPath
                          propertyType:LayerPropertyTypePolyStarOuterRadius
                                 value:[AnimaXValueParam paramWithNumber:60.0]
                              callback:self];
}

- (void)polyStarInnerRounded10 {
  [self.animaXView updateLayerProperty:self.fillKeyPath
                          propertyType:LayerPropertyTypePolyStarInnerRounded
                                 value:[AnimaXValueParam paramWithNumber:10.0]
                              callback:self];
}

- (void)polyStarOuterRounded15 {
  [self.animaXView updateLayerProperty:self.fillKeyPath
                          propertyType:LayerPropertyTypePolyStarOuterRounded
                                 value:[AnimaXValueParam paramWithNumber:15.0]
                              callback:self];
}

#pragma mark - Drop Shadow Properties

- (void)shadowColorBlack {
  [self.animaXView updateLayerProperty:self.fillKeyPath
                          propertyType:LayerPropertyTypeDropShadowColor
                                 value:[AnimaXValueParam paramWithColor:0xFF000000]
                              callback:self];
}

- (void)shadowOpacity80 {
  [self.animaXView updateLayerProperty:self.fillKeyPath
                          propertyType:LayerPropertyTypeDropShadowOpacity
                                 value:[AnimaXValueParam paramWithNumber:80.0]
                              callback:self];
}

- (void)shadowDirection135 {
  [self.animaXView updateLayerProperty:self.fillKeyPath
                          propertyType:LayerPropertyTypeDropShadowDirection
                                 value:[AnimaXValueParam paramWithNumber:135.0]
                              callback:self];
}

- (void)shadowDistance20 {
  [self.animaXView updateLayerProperty:self.fillKeyPath
                          propertyType:LayerPropertyTypeDropShadowDistance
                                 value:[AnimaXValueParam paramWithNumber:20.0]
                              callback:self];
}

- (void)shadowBlur10 {
  [self.animaXView updateLayerProperty:self.fillKeyPath
                          propertyType:LayerPropertyTypeDropShadowRadius
                                 value:[AnimaXValueParam paramWithNumber:10.0]
                              callback:self];
}

#pragma mark - Reset

- (void)resetAnimation {
  [self.animaXView reload];
  NSLog(@"Animation reset");
}

#pragma mark - AnimaXPropertyCallback

- (void)onSuccess {
  NSLog(@"Property updated successfully");
}

- (void)onError:(NSString *)errorMessage {
  NSLog(@"Property update error: %@", errorMessage);
}

#pragma mark - Actions

- (void)closeTapped {
  [self dismissViewControllerAnimated:YES completion:nil];
}

#pragma mark - Button Actions

- (void)getKeys {
  AnimaXKeyPath *allKeyPath = [[AnimaXKeyPath alloc] initWithKeys:@[ @"**" ]];
  [self.animaXView getKeysForKeyPath:allKeyPath callback:self];
}

#pragma mark - AnimaXKeyPathListCallback

- (void)onCallback:(NSArray<AnimaXKeyPath *> *)keyPaths {
  if (keyPaths) {
    NSLog(@"Found %lu keyPaths:", (unsigned long)keyPaths.count);
    for (AnimaXKeyPath *kp in keyPaths) {
      NSLog(@"- %@", kp.description);
    }
  } else {
    NSLog(@"No keyPaths found or error occurred.");
  }
}

@end
