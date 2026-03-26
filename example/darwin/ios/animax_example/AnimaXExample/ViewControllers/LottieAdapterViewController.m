// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import "LottieAdapterViewController.h"
#import <Foundation/Foundation.h>
#import <LottieAdapter/LottieAdapterFactory.h>
#import <LottieAdapter/LottieAdapterView.h>
#import <LottieAdapter/LottieKeyPath.h>
#import <LottieAdapter/LottieValueDelegate.h>
#import <UIKit/UIKit.h>
#import "ButtonsCollectionViewCell.h"
#import "ButtonsCollectionViewDataSource.h"
#import "KeyPathsViewController.h"

@interface ColorCallbackDelegate : NSObject <LottieColorValueDelegate>
@end

@interface OpacityCallbackDelegate : NSObject <LottieNumberValueDelegate>
@end

@interface LottieAdapterViewController () <UICollectionViewDelegate, UICollectionViewDataSource>
@property(nonatomic, strong) LottieAdapterView *animationView;
@property(nonatomic, strong) UIButton *playPauseButton;
@property(nonatomic, strong) UIButton *loopButton;
@property(nonatomic, strong) UISlider *progressSlider;
@property(nonatomic, strong) UILabel *frameLabel;
@property(nonatomic, strong) UIView *controlsContainer;
@property(nonatomic, strong) UICollectionView *buttonsCollectionView;
@property(nonatomic, strong) ColorCallbackDelegate *colorCallbackDelegate;
@property(nonatomic, strong) OpacityCallbackDelegate *opacityCallbackDelegate;

@property(nonatomic, assign) BOOL isPlaying;
@property(nonatomic, assign) BOOL isLooping;
@property(nonatomic, assign) CGFloat maxFrame;
@end

@implementation ColorCallbackDelegate

- (CGColorRef)colorForFrame:(CGFloat)frame
              startKeyframe:(CGFloat)startKeyframe
                endKeyframe:(CGFloat)endKeyframe
       interpolatedProgress:(CGFloat)interpolatedProgress
                 startColor:(CGColorRef)startColor
                   endColor:(CGColorRef)endColor
               currentColor:(CGColorRef)currentColor {
  // Cache color using member variable to extend lifetime
  CGFloat hue = interpolatedProgress;
  CGFloat saturation = 0.8f;
  CGFloat brightness = 0.9f;
  CGColorRef color =
      [UIColor colorWithHue:hue saturation:saturation brightness:brightness alpha:1.0f].CGColor;
  return CGColorRetain(color);
}

@end

@implementation OpacityCallbackDelegate

- (CGFloat)floatValueForFrame:(CGFloat)currentFrame
                startKeyframe:(CGFloat)startKeyframe
                  endKeyframe:(CGFloat)endKeyframe
         interpolatedProgress:(CGFloat)interpolatedProgress
                   startValue:(CGFloat)startValue
                     endValue:(CGFloat)endValue
                 currentValue:(CGFloat)interpolatedValue {
  // Generate opacity based on progress, ensuring 0-100 range
  return (0.5f + 0.5f * sinf(interpolatedProgress * M_PI * 2)) * 100;
}

@end

@implementation LottieAdapterViewController

- (void)viewDidLoad {
  [super viewDidLoad];
  self.view.backgroundColor = [UIColor whiteColor];

  [self setupUI];
  [self setupConstraints];
  [self setupAnimation];
}

- (void)viewDidAppear:(BOOL)animated {
  [super viewDidAppear:animated];
  // Auto-play the animation when the view appears
  [self loadDefaultAnimation];
}

- (void)setupUI {
  [LottieAdapterFactory setDefaultType:AnimationTypeAnimaX];
  // Create animation view
  self.animationView = [[LottieAdapterView alloc] init];
  self.animationView.translatesAutoresizingMaskIntoConstraints = NO;
  [self.view addSubview:self.animationView];

  // Configure animation view
  self.animationView.layer.masksToBounds = YES;
  [self.animationView setContentMode:UIViewContentModeScaleAspectFit];
  self.animationView.loopAnimation = YES;
  self.animationView.userInteractionEnabled = NO;

  // Create playback controls
  self.playPauseButton = [self createButtonWithImage:[UIImage systemImageNamed:@"pause"]];
  [self.playPauseButton addTarget:self
                           action:@selector(togglePlayback)
                 forControlEvents:UIControlEventTouchUpInside];

  self.loopButton =
      [self createButtonWithImage:[UIImage systemImageNamed:@"arrow.3.arrows.circlepath"]];
  [self.loopButton addTarget:self
                      action:@selector(toggleLoop)
            forControlEvents:UIControlEventTouchUpInside];

  self.progressSlider = [[UISlider alloc] init];
  self.progressSlider.translatesAutoresizingMaskIntoConstraints = NO;
  [self.progressSlider addTarget:self
                          action:@selector(sliderValueChanged:)
                forControlEvents:UIControlEventValueChanged];

  self.frameLabel = [[UILabel alloc] init];
  self.frameLabel.translatesAutoresizingMaskIntoConstraints = NO;
  self.frameLabel.text = [NSString stringWithFormat:@"0/%.0f", self.maxFrame];
  self.frameLabel.font = [UIFont systemFontOfSize:12];
  self.frameLabel.textAlignment = NSTextAlignmentCenter;

  // Add playback controls to a container view
  UIView *controlsContainer = [[UIView alloc] init];
  controlsContainer.translatesAutoresizingMaskIntoConstraints = NO;
  [self.view addSubview:controlsContainer];

  [controlsContainer addSubview:self.playPauseButton];
  [controlsContainer addSubview:self.frameLabel];
  [controlsContainer addSubview:self.progressSlider];
  [controlsContainer addSubview:self.loopButton];

  // Store the controls container for use in constraints
  self.controlsContainer = controlsContainer;

  // Create buttons collection view
  UICollectionViewFlowLayout *layout = [[UICollectionViewFlowLayout alloc] init];
  layout.scrollDirection = UICollectionViewScrollDirectionHorizontal;
  layout.minimumInteritemSpacing = 10;
  layout.minimumLineSpacing = 10;
  layout.itemSize = CGSizeMake(120, 40);

  self.buttonsCollectionView = [[UICollectionView alloc] initWithFrame:CGRectZero
                                                  collectionViewLayout:layout];
  self.buttonsCollectionView.translatesAutoresizingMaskIntoConstraints = NO;
  self.buttonsCollectionView.backgroundColor = [UIColor clearColor];
  self.buttonsCollectionView.showsHorizontalScrollIndicator = NO;
  [self.buttonsCollectionView registerClass:[ButtonsCollectionViewCell class]
                 forCellWithReuseIdentifier:@"ButtonCell"];
  self.buttonsCollectionView.dataSource = self;
  self.buttonsCollectionView.delegate = self;

  [self.view addSubview:self.buttonsCollectionView];
}

- (void)setupConstraints {
  [NSLayoutConstraint activateConstraints:@[
    // Animation view constraints
    [self.animationView.centerXAnchor constraintEqualToAnchor:self.view.centerXAnchor],
    [self.animationView.topAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.topAnchor
                                                 constant:20],
    [self.animationView.widthAnchor constraintEqualToConstant:300],
    [self.animationView.heightAnchor constraintEqualToConstant:300],

    // Controls container constraints
    [self.controlsContainer.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor
                                                         constant:20],
    [self.controlsContainer.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor
                                                          constant:-20],
    [self.controlsContainer.topAnchor constraintEqualToAnchor:self.animationView.bottomAnchor
                                                     constant:20],
    [self.controlsContainer.heightAnchor constraintEqualToConstant:40],

    // Buttons collection view constraints
    [self.buttonsCollectionView.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor
                                                             constant:20],
    [self.buttonsCollectionView.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor
                                                              constant:-20],
    [self.buttonsCollectionView.bottomAnchor
        constraintEqualToAnchor:self.view.safeAreaLayoutGuide.bottomAnchor
                       constant:-20],
    [self.buttonsCollectionView.heightAnchor constraintEqualToConstant:60],
  ]];

  // Setup constraints for controls within the container
  UIButton *playButton = self.playPauseButton;
  UILabel *frameLabel = self.frameLabel;
  UISlider *slider = self.progressSlider;
  UIButton *loopButton = self.loopButton;
  UIView *controlsContainer = self.controlsContainer;

  [NSLayoutConstraint activateConstraints:@[
    [playButton.leadingAnchor constraintEqualToAnchor:controlsContainer.leadingAnchor constant:10],
    [playButton.centerYAnchor constraintEqualToAnchor:controlsContainer.centerYAnchor],
    [playButton.widthAnchor constraintEqualToConstant:30],
    [playButton.heightAnchor constraintEqualToConstant:30],

    [frameLabel.leadingAnchor constraintEqualToAnchor:playButton.trailingAnchor constant:8],
    [frameLabel.centerYAnchor constraintEqualToAnchor:controlsContainer.centerYAnchor],
    [frameLabel.widthAnchor constraintEqualToConstant:60],

    [slider.leadingAnchor constraintEqualToAnchor:frameLabel.trailingAnchor constant:8],
    [slider.centerYAnchor constraintEqualToAnchor:controlsContainer.centerYAnchor],
    [slider.trailingAnchor constraintEqualToAnchor:loopButton.leadingAnchor constant:-8],

    [loopButton.trailingAnchor constraintEqualToAnchor:controlsContainer.trailingAnchor
                                              constant:-10],
    [loopButton.centerYAnchor constraintEqualToAnchor:controlsContainer.centerYAnchor],
    [loopButton.widthAnchor constraintEqualToConstant:30],
    [loopButton.heightAnchor constraintEqualToConstant:30],
  ]];
}

- (void)setupAnimation {
  self.isPlaying = YES;
  self.isLooping = YES;

  // Set initial loop state
  [self.animationView setLoopAnimation:self.isLooping];
  self.loopButton.alpha = self.isLooping ? 1.0 : 0.5;

  // Add animation progress observer
  [NSTimer scheduledTimerWithTimeInterval:0.016f
                                  repeats:YES
                                    block:^(NSTimer *_Nonnull timer) {
                                      if (self.animationView.isAnimationPlaying) {
                                        CGFloat progress = self.animationView.animationProgress;
                                        [self updateProgress:progress];
                                      }
                                    }];
}

- (void)loadDefaultAnimation {
  [self.animationView setAnimationNamed:@"dp/data"];

  // Update max frame based on loaded animation
  self.maxFrame = self.animationView.animationDuration * 30;

  // Update UI
  [self updateProgress:0];
  [self updatePlayState:YES];

  // Play animation
  [self.animationView play];
}

- (UIButton *)createButtonWithImage:(UIImage *)image {
  UIButton *button = [UIButton buttonWithType:UIButtonTypeSystem];
  [button setImage:image forState:UIControlStateNormal];
  button.translatesAutoresizingMaskIntoConstraints = NO;
  return button;
}

- (void)togglePlayback {
  if (self.isPlaying) {
    [self.animationView pause];
    [self updatePlayState:NO];
  } else {
    [self.animationView play];
    [self updatePlayState:YES];
  }
}

- (void)toggleLoop {
  self.isLooping = !self.isLooping;
  [self.animationView setLoopAnimation:self.isLooping];
  self.loopButton.alpha = self.isLooping ? 1.0 : 0.5;
}

- (void)sliderValueChanged:(UISlider *)slider {
  CGFloat progress = slider.value;
  [self.animationView setAnimationProgress:progress];
  [self updateProgress:progress];
}

- (void)updateProgress:(CGFloat)progress {
  // Update slider position
  self.progressSlider.value = progress;

  // Update frame label
  NSInteger maxFrame = (NSInteger)self.maxFrame;
  NSInteger currentFrame = (NSInteger)(progress * maxFrame);
  self.frameLabel.text = [NSString stringWithFormat:@"%ld/%ld", (long)currentFrame, (long)maxFrame];
}

- (void)updatePlayState:(BOOL)playing {
  self.isPlaying = playing;
  UIImage *image =
      playing ? [UIImage systemImageNamed:@"pause"] : [UIImage systemImageNamed:@"play"];
  [self.playPauseButton setImage:image forState:UIControlStateNormal];
}

#pragma mark - UICollectionViewDataSource

- (NSInteger)collectionView:(UICollectionView *)collectionView
     numberOfItemsInSection:(NSInteger)section {
  // We'll have 4 buttons: Show KeyPaths, Load Asset, Color Callback, and Opacity Callback
  return 4;
}

- (UICollectionViewCell *)collectionView:(UICollectionView *)collectionView
                  cellForItemAtIndexPath:(NSIndexPath *)indexPath {
  ButtonsCollectionViewCell *cell =
      [collectionView dequeueReusableCellWithReuseIdentifier:@"ButtonCell" forIndexPath:indexPath];

  if (indexPath.item == 0) {
    cell.buttonTitle = @"Show KeyPaths";
  } else if (indexPath.item == 1) {
    cell.buttonTitle = @"Load Asset";
  } else if (indexPath.item == 2) {
    cell.buttonTitle = @"Color Callback";
  } else if (indexPath.item == 3) {
    cell.buttonTitle = @"Opacity Callback";
  }

  [cell.button addTarget:self
                  action:@selector(buttonTapped:)
        forControlEvents:UIControlEventTouchUpInside];

  return cell;
}

#pragma mark - UICollectionViewDelegate

- (void)collectionView:(UICollectionView *)collectionView
    didSelectItemAtIndexPath:(NSIndexPath *)indexPath {
  [collectionView deselectItemAtIndexPath:indexPath animated:YES];

  if (indexPath.item == 0) {
    [self showKeyPaths];
  } else if (indexPath.item == 1) {
    [self showLoadAssetDialog];
  } else if (indexPath.item == 2) {
    [self applyColorCallback];
  } else if (indexPath.item == 3) {
    [self applyOpacityCallback];
  }
}

- (void)buttonTapped:(UIButton *)sender {
  // Find the index path of the cell containing this button
  CGPoint buttonPosition = [sender convertPoint:CGPointZero toView:self.buttonsCollectionView];
  NSIndexPath *indexPath = [self.buttonsCollectionView indexPathForItemAtPoint:buttonPosition];

  if (indexPath) {
    [self collectionView:self.buttonsCollectionView didSelectItemAtIndexPath:indexPath];
  }
}

#pragma mark - Button Actions

- (void)showKeyPaths {
  LottieKeyPath *keyPath = [LottieKeyPath keypathWithKeypath:@"**"];
  NSArray *keys = [self.animationView keysForKeyPath:keyPath];

  if (keys.count > 0) {
    KeyPathsViewController *keyPathsVC = [[KeyPathsViewController alloc] initWithKeyPaths:keys];
    UINavigationController *navController =
        [[UINavigationController alloc] initWithRootViewController:keyPathsVC];
    [self presentViewController:navController animated:YES completion:nil];
  } else {
    UIAlertController *alert =
        [UIAlertController alertControllerWithTitle:@"KeyPaths"
                                            message:@"No key paths found."
                                     preferredStyle:UIAlertControllerStyleAlert];
    [alert addAction:[UIAlertAction actionWithTitle:@"OK"
                                              style:UIAlertActionStyleDefault
                                            handler:nil]];
    [self presentViewController:alert animated:YES completion:nil];
  }
}

- (void)showLoadAssetDialog {
  // Get list of JSON files from the main bundle
  NSBundle *mainBundle = [NSBundle mainBundle];
  NSArray *allResources = [mainBundle pathsForResourcesOfType:@"json" inDirectory:nil];

  // Extract just the filenames
  NSMutableArray *jsonFiles = [[NSMutableArray alloc] init];
  for (NSString *path in allResources) {
    NSString *filename = [path lastPathComponent];
    [jsonFiles addObject:filename];
  }

  if (jsonFiles.count > 0) {
    UIAlertController *alert =
        [UIAlertController alertControllerWithTitle:@"Load Asset"
                                            message:nil
                                     preferredStyle:UIAlertControllerStyleActionSheet];

    for (NSString *filename in jsonFiles) {
      [alert addAction:[UIAlertAction actionWithTitle:filename
                                                style:UIAlertActionStyleDefault
                                              handler:^(UIAlertAction *_Nonnull action) {
                                                [self loadAnimation:filename];
                                              }]];
    }

    [alert addAction:[UIAlertAction actionWithTitle:@"Cancel"
                                              style:UIAlertActionStyleCancel
                                            handler:nil]];
    [self presentViewController:alert animated:YES completion:nil];
  } else {
    UIAlertController *alert =
        [UIAlertController alertControllerWithTitle:@"Load Asset"
                                            message:@"No JSON files found in bundle."
                                     preferredStyle:UIAlertControllerStyleAlert];
    [alert addAction:[UIAlertAction actionWithTitle:@"OK"
                                              style:UIAlertActionStyleDefault
                                            handler:nil]];
    [self presentViewController:alert animated:YES completion:nil];
  }
}

- (void)loadAnimation:(NSString *)assetName {
  [self.animationView stop];
  [self.animationView setAnimationNamed:assetName];

  // Update max frame based on new animation
  self.maxFrame = self.animationView.animationDuration * 30;

  // Update UI
  [self updateProgress:0];
  [self updatePlayState:YES];

  // Play animation
  [self.animationView play];
}

- (void)applyColorCallback {
  LottieKeyPath *keyPath = [LottieKeyPath keypathWithKeypath:@"**.Color"];

  if (!self.colorCallbackDelegate) {
    self.colorCallbackDelegate = [[ColorCallbackDelegate alloc] init];
  }
  [self.animationView setValueDelegate:self.colorCallbackDelegate forKeypath:keyPath];
}

- (void)applyOpacityCallback {
  LottieKeyPath *keyPath = [LottieKeyPath keypathWithKeypath:@"**.Opacity"];

  if (!self.opacityCallbackDelegate) {
    self.opacityCallbackDelegate = [[OpacityCallbackDelegate alloc] init];
  }
  [self.animationView setValueDelegate:self.opacityCallbackDelegate forKeypath:keyPath];
}

@end
