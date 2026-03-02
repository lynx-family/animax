// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import "ViewController.h"
#import <AnimaX/AnimaXContext.h>
#import <AnimaX/AnimaXView.h>
#import <AnimaX/AnimaXViewProtocol.h>
#import <AnimaX/BaseAnimaXAbility.h>
#import <CoreGraphics/CoreGraphics.h>
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

#import "MultiAnimationViewController.h"
#import "OptionsViewController.h"
#import "PropertyCallbackController.h"
#import "PropertyUpdateController.h"
#import "SideMenuViewController.h"

@interface ViewController () <OptionsViewControllerDelegate, SideMenuDelegate>

@property(nonatomic, strong) UILabel *fileNameLabel;
@property(nonatomic, strong) UIButton *menuButton;
@property(nonatomic, strong) UIView *cardView;
@property(nonatomic, strong) AnimaXView *animaXView;

@property(nonatomic, strong) UISlider *progressSlider;
@property(nonatomic, strong) UILabel *frameCountLabel;
@property(nonatomic, strong) UILabel *progressLabel;

@property(nonatomic, strong) UIButton *loopButton;
@property(nonatomic, strong) UIButton *playPauseButton;

@property(nonatomic, assign) BOOL isPlaying;
@property(nonatomic, assign) BOOL isLooping;
@property(nonatomic, assign) NSInteger totalFrames;
@property(nonatomic, assign) BOOL isSeeking;

// Playlist
@property(nonatomic, strong) NSArray<NSURL *> *animationFiles;
@property(nonatomic, assign) NSInteger currentAnimationIndex;

@end

@implementation ViewController

- (void)viewDidLoad {
  [super viewDidLoad];
  [self.navigationController setNavigationBarHidden:YES animated:NO];

  self.view.backgroundColor = [UIColor colorWithRed:0.98
                                              green:0.98
                                               blue:0.98
                                              alpha:1.0];  // #FAFAFA

  // Init data
  self.isLooping = YES;
  self.isPlaying = YES;

  // Load all json files from bundle root
  NSMutableArray<NSURL *> *fileUrls = [NSMutableArray array];
  NSArray *urls = [[NSBundle mainBundle] URLsForResourcesWithExtension:@"json" subdirectory:nil];
  if (urls) {
    [fileUrls addObjectsFromArray:urls];
  }

  // Discover files in export_output
  NSString *downloadPath =
      [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"export_output"];
  NSFileManager *fm = [NSFileManager defaultManager];
  NSDirectoryEnumerator *enumerator = [fm enumeratorAtPath:downloadPath];
  NSString *file;
  while ((file = [enumerator nextObject])) {
    if ([file.pathExtension isEqualToString:@"json"] ||
        [file.pathExtension isEqualToString:@"zip"]) {
      NSURL *fileUrl = [NSURL fileURLWithPath:[downloadPath stringByAppendingPathComponent:file]];
      [fileUrls addObject:fileUrl];
    }
  }

  // Sort files for consistent order
  [fileUrls sortUsingComparator:^NSComparisonResult(NSURL *url1, NSURL *url2) {
    return [url1.lastPathComponent localizedStandardCompare:url2.lastPathComponent];
  }];
  self.animationFiles = fileUrls;

  self.currentAnimationIndex = 0;

  [self setupUI];
  [self loadCurrentAnimation];
}

- (void)setupUI {
  UILayoutGuide *guide = self.view.safeAreaLayoutGuide;

  // 0. Menu Button (Larger Icon)
  self.menuButton = [UIButton buttonWithType:UIButtonTypeSystem];
  UIImageSymbolConfiguration *config =
      [UIImageSymbolConfiguration configurationWithPointSize:24 weight:UIImageSymbolWeightMedium];
  [self.menuButton setImage:[[UIImage systemImageNamed:@"line.horizontal.3"]
                                imageByApplyingSymbolConfiguration:config]
                   forState:UIControlStateNormal];
  self.menuButton.tintColor = [UIColor blackColor];
  [self.menuButton addTarget:self
                      action:@selector(showSideMenu)
            forControlEvents:UIControlEventTouchUpInside];
  self.menuButton.translatesAutoresizingMaskIntoConstraints = NO;
  [self.view addSubview:self.menuButton];

  // 0.1 Title Label
  UILabel *titleLabel = [[UILabel alloc] init];
  titleLabel.text = @"Previewer";
  titleLabel.font = [UIFont boldSystemFontOfSize:22];
  titleLabel.textColor = [UIColor blackColor];
  titleLabel.translatesAutoresizingMaskIntoConstraints = NO;
  [self.view addSubview:titleLabel];

  // 1. Filename Label (Gray, Non-clickable)
  self.fileNameLabel = [[UILabel alloc] init];
  self.fileNameLabel.text = @"";  // Will be set by loadCurrentAnimation
  self.fileNameLabel.textColor = [UIColor colorWithWhite:0.4 alpha:1.0];
  self.fileNameLabel.font = [UIFont systemFontOfSize:14];
  self.fileNameLabel.translatesAutoresizingMaskIntoConstraints = NO;
  [self.view addSubview:self.fileNameLabel];

  // 2. Card View (Container)
  self.cardView = [[UIView alloc] init];
  self.cardView.backgroundColor = [UIColor whiteColor];
  self.cardView.layer.cornerRadius = 16;
  self.cardView.layer.shadowColor = [UIColor blackColor].CGColor;
  self.cardView.layer.shadowOpacity = 0.1;
  self.cardView.layer.shadowOffset = CGSizeMake(0, 4);
  self.cardView.layer.shadowRadius = 8;
  self.cardView.translatesAutoresizingMaskIntoConstraints = NO;
  [self.view addSubview:self.cardView];

  // 3. AnimaXView
  self.animaXView = [self createAnimaXView];
  self.animaXView.translatesAutoresizingMaskIntoConstraints = NO;
  [self.cardView addSubview:self.animaXView];

  // 4. Progress Section
  UIView *progressContainer = [[UIView alloc] init];
  progressContainer.translatesAutoresizingMaskIntoConstraints = NO;
  [self.view addSubview:progressContainer];

  self.progressSlider = [[UISlider alloc] init];
  self.progressSlider.tintColor = [UIColor darkGrayColor];
  self.progressSlider.translatesAutoresizingMaskIntoConstraints = NO;
  [self.progressSlider addTarget:self
                          action:@selector(sliderValueChanged:)
                forControlEvents:UIControlEventValueChanged];
  [self.progressSlider addTarget:self
                          action:@selector(sliderTouchDown:)
                forControlEvents:UIControlEventTouchDown];
  [self.progressSlider addTarget:self
                          action:@selector(sliderTouchUp:)
                forControlEvents:UIControlEventTouchUpInside | UIControlEventTouchUpOutside];
  [progressContainer addSubview:self.progressSlider];

  self.frameCountLabel = [[UILabel alloc] init];
  self.frameCountLabel.text = @"0/0";
  self.frameCountLabel.font = [UIFont systemFontOfSize:13];
  self.frameCountLabel.textColor = [UIColor darkGrayColor];
  self.frameCountLabel.translatesAutoresizingMaskIntoConstraints = NO;
  [progressContainer addSubview:self.frameCountLabel];

  self.progressLabel = [[UILabel alloc] init];
  self.progressLabel.text = @"0%";
  self.progressLabel.font = [UIFont systemFontOfSize:13];
  self.progressLabel.textColor = [UIColor darkGrayColor];
  self.progressLabel.translatesAutoresizingMaskIntoConstraints = NO;
  [progressContainer addSubview:self.progressLabel];

  // 5. Controls
  UIStackView *controlsStack = [[UIStackView alloc] init];
  controlsStack.axis = UILayoutConstraintAxisHorizontal;
  controlsStack.distribution = UIStackViewDistributionEqualSpacing;  // Distribute evenly
  controlsStack.alignment = UIStackViewAlignmentCenter;
  controlsStack.translatesAutoresizingMaskIntoConstraints = NO;
  [self.view addSubview:controlsStack];

  self.loopButton = [self createButtonWithIcon:@"repeat" action:@selector(toggleLoop)];
  [self updateLoopButtonState];

  UIButton *prevButton = [self createButtonWithIcon:@"backward.end.fill"
                                             action:@selector(prevAnimation)];

  self.playPauseButton = [UIButton buttonWithType:UIButtonTypeSystem];
  [self.playPauseButton setImage:[UIImage systemImageNamed:@"pause.fill"]
                        forState:UIControlStateNormal];
  [self.playPauseButton setBackgroundColor:[UIColor colorWithWhite:0.9
                                                             alpha:1.0]];  // Light gray circle
  self.playPauseButton.layer.cornerRadius = 26;
  self.playPauseButton.tintColor = [UIColor blackColor];
  [self.playPauseButton addTarget:self
                           action:@selector(togglePlayPause)
                 forControlEvents:UIControlEventTouchUpInside];
  self.playPauseButton.translatesAutoresizingMaskIntoConstraints = NO;
  [self.playPauseButton.widthAnchor constraintEqualToConstant:52].active = YES;
  [self.playPauseButton.heightAnchor constraintEqualToConstant:52].active = YES;

  UIButton *nextButton = [self createButtonWithIcon:@"forward.end.fill"
                                             action:@selector(nextAnimation)];
  UIButton *moreButton = [self createButtonWithIcon:@"ellipsis" action:@selector(showMoreOptions)];

  [controlsStack addArrangedSubview:self.loopButton];
  [controlsStack addArrangedSubview:prevButton];
  [controlsStack addArrangedSubview:self.playPauseButton];
  [controlsStack addArrangedSubview:nextButton];
  [controlsStack addArrangedSubview:moreButton];

  // Constraints
  [NSLayoutConstraint activateConstraints:@[
    // Menu Button
    [self.menuButton.topAnchor constraintEqualToAnchor:guide.topAnchor constant:12],
    [self.menuButton.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor constant:16],
    [self.menuButton.widthAnchor constraintEqualToConstant:44],
    [self.menuButton.heightAnchor constraintEqualToConstant:44],

    // Title Label
    [titleLabel.centerYAnchor constraintEqualToAnchor:self.menuButton.centerYAnchor],
    [titleLabel.leadingAnchor constraintEqualToAnchor:self.menuButton.trailingAnchor constant:8],

    // Filename
    [self.fileNameLabel.topAnchor constraintEqualToAnchor:self.menuButton.bottomAnchor constant:4],
    [self.fileNameLabel.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor constant:20],

    // Card
    [self.cardView.topAnchor constraintEqualToAnchor:self.fileNameLabel.bottomAnchor constant:12],
    [self.cardView.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor constant:16],
    [self.cardView.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor constant:-16],
    [self.cardView.bottomAnchor constraintEqualToAnchor:progressContainer.topAnchor constant:-16],

    // AnimaX View (Fill Card)
    [self.animaXView.topAnchor constraintEqualToAnchor:self.cardView.topAnchor],
    [self.animaXView.leadingAnchor constraintEqualToAnchor:self.cardView.leadingAnchor],
    [self.animaXView.trailingAnchor constraintEqualToAnchor:self.cardView.trailingAnchor],
    [self.animaXView.bottomAnchor constraintEqualToAnchor:self.cardView.bottomAnchor],

    // Progress Container
    [progressContainer.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor constant:16],
    [progressContainer.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor
                                                     constant:-16],
    [progressContainer.bottomAnchor constraintEqualToAnchor:controlsStack.topAnchor constant:-16],
    [progressContainer.heightAnchor constraintEqualToConstant:60],

    // Slider
    [self.progressSlider.topAnchor constraintEqualToAnchor:progressContainer.topAnchor],
    [self.progressSlider.leadingAnchor constraintEqualToAnchor:progressContainer.leadingAnchor],
    [self.progressSlider.trailingAnchor constraintEqualToAnchor:progressContainer.trailingAnchor],

    // Counter labels
    [self.frameCountLabel.topAnchor constraintEqualToAnchor:self.progressSlider.bottomAnchor
                                                   constant:4],
    [self.frameCountLabel.leadingAnchor constraintEqualToAnchor:progressContainer.leadingAnchor],

    [self.progressLabel.topAnchor constraintEqualToAnchor:self.progressSlider.bottomAnchor
                                                 constant:4],
    [self.progressLabel.trailingAnchor constraintEqualToAnchor:progressContainer.trailingAnchor],

    // Controls
    [controlsStack.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor constant:24],
    [controlsStack.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor constant:-24],
    [controlsStack.bottomAnchor constraintEqualToAnchor:guide.bottomAnchor constant:-20],
    [controlsStack.heightAnchor constraintEqualToConstant:60]
  ]];
}

- (void)showSideMenu {
  SideMenuViewController *menuVC = [[SideMenuViewController alloc] init];
  menuVC.delegate = self;
  [menuVC showInViewController:self];
}

- (void)showMoreOptions {
  OptionsViewController *optionsVC = [[OptionsViewController alloc] init];
  optionsVC.delegate = self;
  if (@available(iOS 15.0, *)) {
    if (optionsVC.sheetPresentationController) {
      if (@available(iOS 16.0, *)) {
        UISheetPresentationControllerDetent *customDetent = [UISheetPresentationControllerDetent
            customDetentWithIdentifier:@"small"
                              resolver:^CGFloat(
                                  id<UISheetPresentationControllerDetentResolutionContext> _Nonnull context) {
                                return 280.0;
                              }];
        optionsVC.sheetPresentationController.detents = @[ customDetent ];
      } else {
        optionsVC.sheetPresentationController.detents =
            @[ [UISheetPresentationControllerDetent mediumDetent] ];
      }
    }
  }
  [self presentViewController:optionsVC animated:YES completion:nil];
}

- (UIButton *)createButtonWithIcon:(NSString *)iconName action:(SEL)action {
  UIButton *btn = [UIButton buttonWithType:UIButtonTypeSystem];
  [btn setImage:[UIImage systemImageNamed:iconName] forState:UIControlStateNormal];
  [btn addTarget:self action:action forControlEvents:UIControlEventTouchUpInside];
  btn.tintColor = [UIColor darkGrayColor];
  btn.translatesAutoresizingMaskIntoConstraints = NO;
  [btn.widthAnchor constraintEqualToConstant:44].active = YES;
  [btn.heightAnchor constraintEqualToConstant:44].active = YES;
  return btn;
}

- (AnimaXView *)createAnimaXView {
  BaseAnimaXAbility *ability = [[BaseAnimaXAbility alloc] init];
  AnimaXContext *animaxContext = [[AnimaXContext alloc] initWithAbility:ability];
  AnimaXView *view = [[AnimaXView alloc] initWithContext:animaxContext];
  view.enableNativeTapLayerEvent = YES;
  [ability addAnimationListener:self];
  return view;
}

- (void)loadCurrentAnimation {
  if (self.animationFiles.count == 0 || self.currentAnimationIndex >= self.animationFiles.count) {
    self.fileNameLabel.text = @"";
    return;
  }
  NSURL *url = self.animationFiles[self.currentAnimationIndex];
  self.fileNameLabel.text = url.lastPathComponent;

  [self.animaXView setSrc:url.absoluteString];
  [self.animaXView setLoop:self.isLooping];
  [self.animaXView setAutoplay:YES];
  self.isPlaying = YES;
  [self updatePlayPauseButtonState];
}

#pragma mark - Actions

- (void)toggleLoop {
  self.isLooping = !self.isLooping;
  [self.animaXView setLoop:self.isLooping];
  [self updateLoopButtonState];
}

- (void)updateLoopButtonState {
  self.loopButton.alpha = self.isLooping ? 1.0 : 0.3;
}

- (void)togglePlayPause {
  if (self.isPlaying) {
    [self.animaXView pause];
  } else {
    [self.animaXView resume];
  }
  self.isPlaying = !self.isPlaying;
  [self updatePlayPauseButtonState];
}

- (void)updatePlayPauseButtonState {
  NSString *icon = self.isPlaying ? @"pause.fill" : @"play.fill";
  [self.playPauseButton setImage:[UIImage systemImageNamed:icon] forState:UIControlStateNormal];
}

- (void)prevAnimation {
  if (self.currentAnimationIndex > 0) {
    self.currentAnimationIndex--;
  } else {
    self.currentAnimationIndex = self.animationFiles.count - 1;
  }
  [self loadCurrentAnimation];
}

- (void)nextAnimation {
  if (self.currentAnimationIndex < self.animationFiles.count - 1) {
    self.currentAnimationIndex++;
  } else {
    self.currentAnimationIndex = 0;
  }
  [self loadCurrentAnimation];
}

#pragma mark - Slider

- (void)sliderTouchDown:(UISlider *)slider {
  self.isSeeking = YES;
  [self.animaXView pause];
}

- (void)sliderValueChanged:(UISlider *)slider {
  // Optional: distinct from seeking action if needed
  // Update progress label immediately
  float progress = slider.value;
  self.progressLabel.text = [NSString stringWithFormat:@"%d%%", (int)(progress * 100)];

  // Also update frame count text if total frames known
  if (self.totalFrames > 0) {
    int currentFrame = (int)(progress * self.totalFrames);
    self.frameCountLabel.text =
        [NSString stringWithFormat:@"%d/%ld", currentFrame, (long)self.totalFrames];
  }

  [self.animaXView setProgress:progress];
}

- (void)sliderTouchUp:(UISlider *)slider {
  self.isSeeking = NO;
  if (self.isPlaying) {
    [self.animaXView play];
  }
}

#pragma mark - AnimaXAnimationListener

- (void)onStart:(NSDictionary *)param {
  // Valid start
}

- (void)onReady:(NSDictionary *)param {
  NSNumber *total = param[@"total"];
  self.totalFrames = total ? [total integerValue] : 0;

  // Subscribe to all frame updates for smooth seek bar progress
  if (self.totalFrames > 0) {
    NSMutableArray *frames = [NSMutableArray arrayWithCapacity:self.totalFrames];
    for (int i = 0; i < self.totalFrames; i++) {
      [frames addObject:@(i)];
    }
    [self.animaXView subscribeUpdateEvents:frames subscribe:YES];
  }

  dispatch_async(dispatch_get_main_queue(), ^{
    self.frameCountLabel.text = [NSString stringWithFormat:@"0/%ld", (long)self.totalFrames];
    self.progressSlider.value = 0;
    self.progressLabel.text = @"0%";
  });
}

- (void)onUpdate:(NSDictionary *)param {
  if (self.isSeeking) return;

  NSNumber *current = param[@"current"];
  NSInteger currentFrame = current ? [current integerValue] : 0;
  float progress = 0;
  if (self.totalFrames > 0) {
    progress = (float)currentFrame / (float)self.totalFrames;
  }

  dispatch_async(dispatch_get_main_queue(), ^{
    self.progressSlider.value = progress;
    self.frameCountLabel.text =
        [NSString stringWithFormat:@"%ld/%ld", (long)currentFrame, (long)self.totalFrames];
    self.progressLabel.text = [NSString stringWithFormat:@"%d%%", (int)(progress * 100)];
  });
}

- (void)onCompletion:(NSDictionary *)param {
  if (!self.isLooping) {
    dispatch_async(dispatch_get_main_queue(), ^{
      self.isPlaying = NO;
      [self updatePlayPauseButtonState];
    });
  }
}

- (void)onCancel:(NSDictionary *)param {
}
- (void)onRepeat:(NSDictionary *)param {
}
- (void)onError:(NSDictionary *)param {
}

- (void)onTapLayers:(NSDictionary *)param {
  // Handle tap layers if overlay enabled
  NSLog(@"Tapped layer: %@", param);
}

#pragma mark - OptionsViewControllerDelegate

- (void)didSelectBackgroundColor {
  // Toggle background color for demo
  if ([self.cardView.backgroundColor isEqual:[UIColor whiteColor]]) {
    self.cardView.backgroundColor = [UIColor blackColor];
  } else {
    self.cardView.backgroundColor = [UIColor whiteColor];
  }
  [self dismissViewControllerAnimated:YES completion:nil];
}

- (void)didSelectAnimationList {
  [self dismissViewControllerAnimated:YES
                           completion:^{
                             [self showAnimationList];
                           }];
}

- (void)showAnimationList {
  UIAlertController *alert =
      [UIAlertController alertControllerWithTitle:@"Select Animation"
                                          message:nil
                                   preferredStyle:UIAlertControllerStyleActionSheet];

  for (int i = 0; i < self.animationFiles.count; i++) {
    NSURL *url = self.animationFiles[i];
    [alert addAction:[UIAlertAction actionWithTitle:url.lastPathComponent
                                              style:UIAlertActionStyleDefault
                                            handler:^(UIAlertAction *_Nonnull action) {
                                              self.currentAnimationIndex = i;
                                              [self loadCurrentAnimation];
                                            }]];
  }

  [alert addAction:[UIAlertAction actionWithTitle:@"Cancel"
                                            style:UIAlertActionStyleCancel
                                          handler:nil]];

  // For iPad support
  if (alert.popoverPresentationController) {
    alert.popoverPresentationController.sourceView = self.fileNameLabel;
    alert.popoverPresentationController.sourceRect = self.fileNameLabel.bounds;
  }

  [self presentViewController:alert animated:YES completion:nil];
}

- (void)didSelectKeyPaths {
  NSLog(@"KeyPaths requested");
  [self dismissViewControllerAnimated:YES completion:nil];
}

- (void)didToggleEvents:(BOOL)enabled {
  // Toggle overlay logic if implemented
  NSLog(@"Events toggled: %d", enabled);
  [self dismissViewControllerAnimated:YES completion:nil];
}

- (void)didSelectPropertyUpdate {
  [self dismissViewControllerAnimated:YES
                           completion:^{
                             PropertyUpdateController *vc = [[PropertyUpdateController alloc] init];
                             vc.modalPresentationStyle = UIModalPresentationFullScreen;
                             [self presentViewController:vc animated:YES completion:nil];
                           }];
}

- (void)didSelectPropertyCallback {
  [self dismissViewControllerAnimated:YES
                           completion:^{
                             PropertyCallbackController *vc =
                                 [[PropertyCallbackController alloc] init];
                             vc.modalPresentationStyle = UIModalPresentationFullScreen;
                             [self presentViewController:vc animated:YES completion:nil];
                           }];
}

- (void)didSelectMultiAnimation {
  [self dismissViewControllerAnimated:YES
                           completion:^{
                             MultiAnimationViewController *vc =
                                 [[MultiAnimationViewController alloc] init];
                             vc.modalPresentationStyle = UIModalPresentationFullScreen;
                             [self presentViewController:vc animated:YES completion:nil];
                           }];
}

@end
