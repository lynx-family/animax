// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import "MultiAnimationViewController.h"
#import <AnimaX/AnimaXAnimationListener.h>
#import <AnimaX/AnimaXContext.h>
#import <AnimaX/AnimaXPlayerProtocol.h>
#import <AnimaX/AnimaXView.h>
#import <AnimaX/BaseAnimaXAbility.h>

@interface MultiAnimationCell : UICollectionViewCell <AnimaXAnimationListener>
@property(nonatomic, strong) AnimaXView *animaxView;
@property(nonatomic, strong) UILabel *fpsLabel;
@end

@implementation MultiAnimationCell

- (instancetype)initWithFrame:(CGRect)frame {
  self = [super initWithFrame:frame];
  if (self) {
    // Create AnimaXView
    BaseAnimaXAbility *ability = [[BaseAnimaXAbility alloc] init];
    [ability addAnimationListener:self];

    AnimaXContext *context = [[AnimaXContext alloc] initWithAbility:ability];
    _animaxView = [[AnimaXView alloc] initWithContext:context];
    _animaxView.translatesAutoresizingMaskIntoConstraints = NO;
    [_animaxView setLoop:YES];
    [_animaxView setAutoplay:YES];
    [_animaxView setFPSEventInterval:1000];

    [self.contentView addSubview:_animaxView];

    // FPS Label
    _fpsLabel = [[UILabel alloc] init];
    _fpsLabel.text = @"FPS(--)";
    _fpsLabel.font = [UIFont systemFontOfSize:10];
    _fpsLabel.textColor = [UIColor darkGrayColor];
    _fpsLabel.textAlignment = NSTextAlignmentCenter;
    _fpsLabel.translatesAutoresizingMaskIntoConstraints = NO;
    [self.contentView addSubview:_fpsLabel];

    [NSLayoutConstraint activateConstraints:@[
      [_animaxView.topAnchor constraintEqualToAnchor:self.contentView.topAnchor],
      [_animaxView.leadingAnchor constraintEqualToAnchor:self.contentView.leadingAnchor],
      [_animaxView.trailingAnchor constraintEqualToAnchor:self.contentView.trailingAnchor],
      [_animaxView.bottomAnchor constraintEqualToAnchor:_fpsLabel.topAnchor],

      [_fpsLabel.leadingAnchor constraintEqualToAnchor:self.contentView.leadingAnchor],
      [_fpsLabel.trailingAnchor constraintEqualToAnchor:self.contentView.trailingAnchor],
      [_fpsLabel.bottomAnchor constraintEqualToAnchor:self.contentView.bottomAnchor],
      [_fpsLabel.heightAnchor constraintEqualToConstant:20]
    ]];
  }
  return self;
}

- (void)prepareForReuse {
  [super prepareForReuse];
  // Reset view state if needed
  _fpsLabel.text = @"FPS(--)";
}

#pragma mark - AnimaXAnimationListener

- (void)onFps:(NSDictionary *)params {
  NSNumber *fps = params[@"fps"];
  if (fps) {
    int fpsValue = [fps intValue];
    dispatch_async(dispatch_get_main_queue(), ^{
      self.fpsLabel.text = [NSString stringWithFormat:@"FPS(%d)", fpsValue];
    });
  }
}

- (void)onCompletion:(NSDictionary *)params {
}
- (void)onStart:(NSDictionary *)params {
}
- (void)onRepeat:(NSDictionary *)params {
}
- (void)onCancel:(NSDictionary *)params {
}
- (void)onReady:(NSDictionary *)params {
}
- (void)onUpdate:(NSDictionary *)params {
}
- (void)onError:(NSDictionary *)params {
}
- (void)onWarning:(NSDictionary *)params {
}
- (void)onTapLayers:(NSDictionary *)params {
}
- (void)onFirstFrame:(NSDictionary *)params {
}
- (void)onCompositionReady:(NSDictionary *)params {
}

@end

@interface MultiAnimationViewController () <UICollectionViewDataSource,
                                            UICollectionViewDelegateFlowLayout>
@property(nonatomic, strong) UICollectionView *collectionView;
@property(nonatomic, assign) NSInteger itemCount;
@end

@implementation MultiAnimationViewController

- (void)viewDidLoad {
  [super viewDidLoad];
  self.view.backgroundColor = [UIColor whiteColor];
  self.title = @"Multi Animation";

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

  [NSLayoutConstraint activateConstraints:@[
    [closeBtn.topAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.topAnchor
                                       constant:10],
    [closeBtn.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor constant:-20],
    [closeBtn.widthAnchor constraintEqualToConstant:44],
    [closeBtn.heightAnchor constraintEqualToConstant:44],

    // Title Label
    [titleLabel.centerYAnchor constraintEqualToAnchor:closeBtn.centerYAnchor],
    [titleLabel.centerXAnchor constraintEqualToAnchor:self.view.centerXAnchor],
  ]];

  UICollectionViewFlowLayout *layout = [[UICollectionViewFlowLayout alloc] init];
  layout.minimumInteritemSpacing = 0;
  layout.minimumLineSpacing = 0;

  self.collectionView = [[UICollectionView alloc] initWithFrame:CGRectZero
                                           collectionViewLayout:layout];
  self.collectionView.backgroundColor = [UIColor whiteColor];
  self.collectionView.dataSource = self;
  self.collectionView.delegate = self;
  self.collectionView.translatesAutoresizingMaskIntoConstraints = NO;
  [self.collectionView registerClass:[MultiAnimationCell class] forCellWithReuseIdentifier:@"Cell"];
  [self.view addSubview:self.collectionView];

  [NSLayoutConstraint activateConstraints:@[
    [self.collectionView.topAnchor constraintEqualToAnchor:closeBtn.bottomAnchor constant:10],
    [self.collectionView.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor],
    [self.collectionView.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor],
    [self.collectionView.bottomAnchor constraintEqualToAnchor:self.view.bottomAnchor]
  ]];

  // Calculate items to fill screen
  [self calculateGrid];
}

- (void)calculateGrid {
  CGFloat kMinItemSize = 100.0;
  CGFloat width = CGRectGetWidth(self.view.bounds);
  CGFloat height = CGRectGetHeight(self.view.bounds);

  NSInteger cols = MAX(1, (NSInteger)(width / kMinItemSize));
  NSInteger rows = MAX(1, (NSInteger)(height / kMinItemSize));
  self.itemCount = cols * rows;

  [self.collectionView reloadData];
}

- (void)closeTapped {
  [self dismissViewControllerAnimated:YES completion:nil];
}

#pragma mark - UICollectionViewDataSource

- (NSInteger)collectionView:(UICollectionView *)collectionView
     numberOfItemsInSection:(NSInteger)section {
  return self.itemCount;
}

- (__kindof UICollectionViewCell *)collectionView:(UICollectionView *)collectionView
                           cellForItemAtIndexPath:(NSIndexPath *)indexPath {
  MultiAnimationCell *cell = [collectionView dequeueReusableCellWithReuseIdentifier:@"Cell"
                                                                       forIndexPath:indexPath];

  // Load animation
  NSBundle *bundle = [NSBundle mainBundle];
  NSString *jsonPath = [bundle pathForResource:@"simple_shape" ofType:@"json"];
  if (jsonPath) {
    NSURL *url = [NSURL fileURLWithPath:jsonPath];
    [cell.animaxView setSrc:url.absoluteString];
  }

  return cell;
}

#pragma mark - UICollectionViewDelegateFlowLayout

- (CGSize)collectionView:(UICollectionView *)collectionView
                    layout:(UICollectionViewLayout *)collectionViewLayout
    sizeForItemAtIndexPath:(NSIndexPath *)indexPath {
  CGFloat width = CGRectGetWidth(collectionView.frame);
  NSInteger cols = MAX(1, (NSInteger)(width / 100.0));
  CGFloat itemWidth = width / cols;
  return CGSizeMake(itemWidth, itemWidth + 20);
}

@end
