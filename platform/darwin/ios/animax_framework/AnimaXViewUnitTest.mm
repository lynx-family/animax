// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <AnimaX/AnimaXImageView.h>
#import <AnimaX/AnimaXView.h>

#import <XCTest/XCTest.h>
#import <objc/runtime.h>

#import "AnimaXSurfaceDrawable+Internal.h"

#pragma mark Test Classes

@interface AnimaXView (ApplicationLifecycleUnitTest)
- (void)onAnimaXApplicationDidBecomeActive;
- (BOOL)isAnimaXSurfaceCreationAllowed;
@end

@interface TestLifecycleAnimaXView : AnimaXView
@property(nonatomic, assign) BOOL surfaceCreationAllowed;
@end

@implementation TestLifecycleAnimaXView
- (BOOL)isAnimaXSurfaceCreationAllowed {
  return self.surfaceCreationAllowed;
}
@end

@interface TestPlayer : NSObject <AnimaXPlayerProtocol>
@property(nonatomic, assign) NSInteger adoptCount;
@property(nonatomic, strong) id lastDrawable;
@property(nonatomic, assign) int visibilityFlag;
@property(nonatomic, assign) BOOL hasHandleTap;
@end

@implementation TestPlayer
- (void)adoptAnimaXSurfaceDrawable:(id)drawable {
  self.lastDrawable = drawable;
  self.adoptCount++;
}
- (void)updateVisibilityState:(BOOL)visible forType:(VisibilityState)state {
  if (visible) {
    self.visibilityFlag &= ~state;
  } else {
    self.visibilityFlag |= state;
  }
}

- (void)handleTapAtPoint:(CGPoint)point {
  _hasHandleTap = YES;
}
- (int64_t)memoryUsageBytes {
  return 0;
}
- (double)progress {
  return 0;
}
@end

@interface TestStartListener : NSObject <AnimaXAnimationListener>
@property(nonatomic, copy) void (^onReadyBlock)(void);
@property(nonatomic, copy) void (^onStartBlock)(void);
@property(nonatomic, assign) BOOL didReady;
@property(nonatomic, assign) BOOL didStart;
@end

@implementation TestStartListener
- (void)onReady:(NSDictionary *)params {
  if (self.didReady) {
    return;
  }
  self.didReady = YES;
  if (self.onReadyBlock) {
    self.onReadyBlock();
  }
}

- (void)onStart:(NSDictionary *)params {
  if (self.didStart) {
    return;
  }
  self.didStart = YES;
  if (self.onStartBlock) {
    self.onStartBlock();
  }
}
@end

static NSString *TestEasyLottieJson() {
  return @"{\"v\":\"5.6.9\",\"fr\":25,\"ip\":0,\"op\":200,\"w\":1000,\"h\":1000,\"nm\":\""
         @"Composition 1\",\"ddd\":0,\"assets\":[],\"layers\":[{\"ddd\":0,\"ind\":1,"
         @"\"ty\":1,\"nm\":\"Intermediate Blue Solid Color 1\",\"sr\":1,\"ks\":{\"o\":{"
         @"\"a\":0,\"k\":100,\"ix\":11},\"r\":{\"a\":0,\"k\":0,\"ix\":10},\"p\":{"
         @"\"a\":0,\"k\":[500,500,0],\"ix\":2},\"a\":{\"a\":0,\"k\":[500,500,0],"
         @"\"ix\":1},\"s\":{\"a\":0,\"k\":[100,100,100],\"ix\":6}},\"ao\":0,"
         @"\"sw\":1000,\"sh\":1000,\"sc\":\"#505fec\",\"ip\":0,\"op\":200,\"st\":0,"
         @"\"bm\":0}],\"markers\":[]}";
}

@interface AnimaXViewTest : XCTestCase
@property(nonatomic, strong) UIView<AnimaXViewProtocol, AnimaXPlayerProtocol> *view;
@property(nonatomic, strong) id<AnimaXPlayerProtocol> player;
@end

@implementation AnimaXViewTest

#pragma mark Test: Set Up

- (void)tearDown {
  self.view = NULL;
  [super tearDown];
}

- (void)createViewIsImageView:(BOOL)useImageView {
  BaseAnimaXAbility *ability = [[BaseAnimaXAbility alloc] init];
  AnimaXContext *ctx = [[AnimaXContext alloc] initWithAbility:ability];
  if (useImageView) {
    self.view = [[AnimaXImageView alloc] initWithContext:ctx];
    XCTAssertTrue([self.view isKindOfClass:AnimaXImageView.class]);
  } else {
    TestLifecycleAnimaXView *view = [[TestLifecycleAnimaXView alloc] initWithContext:ctx];
    view.surfaceCreationAllowed = YES;
    self.view = view;
    XCTAssertTrue([self.view isKindOfClass:AnimaXView.class]);
  }
  XCTAssertNotNil(self.view, @"Failed to create the target view.");
  XCTAssertNotNil([self.view valueForKey:@"player"]);
  self.player = [[TestPlayer alloc] init];
  [self.view setValue:self.player forKey:@"player"];
  XCTAssertNil([self.view valueForKey:@"drawable"]);
}

- (void)testInit {
  [self createViewIsImageView:NO];
}

- (void)testInitImageView {
  [self createViewIsImageView:YES];
}

#pragma mark Test: Frame Changed

- (void)generalTestFrameChanged {
  [self.view setFrame:CGRectMake(0, 0, 100, 60)];
  id drawable = [self.view valueForKey:@"drawable"];
  XCTAssertNotNil(drawable, @"Drawable should be created on valid frame.");
  XCTAssertEqual([(TestPlayer *)self.player adoptCount], 1,
                 @"Player should adopt drawable after creating.");
  XCTAssertEqualObjects([(TestPlayer *)self.player lastDrawable], drawable);
  CGFloat scale = UIScreen.mainScreen.scale;
  XCTAssertEqual(100 * scale, [(AnimaXSurfaceDrawable *)drawable size].width);
  XCTAssertEqual(60 * scale, [(AnimaXSurfaceDrawable *)drawable size].height);
  self.view.bounds = CGRectMake(0, 0, 110, 80);
  XCTAssertEqual([(TestPlayer *)self.player adoptCount], 2,
                 @"Player should adopt drawable after creating.");
  XCTAssertEqual([self.view valueForKey:@"drawable"], drawable);
  XCTAssertEqual(110 * scale, [(AnimaXSurfaceDrawable *)drawable size].width);
  XCTAssertEqual(80 * scale, [(AnimaXSurfaceDrawable *)drawable size].height);
}

- (void)generalTestInvalidFrameChanged {
  self.view.frame = CGRectMake(0, 0, 0, 0);
  id drawable = [self.view valueForKey:@"drawable"];
  XCTAssertNil(drawable);
  XCTAssertEqual([(TestPlayer *)self.player adoptCount], 0);
  self.view.ignoreAttachStatus = YES;
  [self.view onCompositionReady];
  drawable = [self.view valueForKey:@"drawable"];
  XCTAssertNotNil(drawable);
  XCTAssertEqual([(TestPlayer *)self.player adoptCount], 1);
}

- (void)testFrameChanged {
  [self createViewIsImageView:NO];
  [self generalTestFrameChanged];
  [self createViewIsImageView:NO];
  [self generalTestInvalidFrameChanged];
}

- (void)testFrameChangedDefersDrawableUntilApplicationBecomesActive {
  [self createViewIsImageView:NO];
  TestLifecycleAnimaXView *view = (TestLifecycleAnimaXView *)self.view;
  view.surfaceCreationAllowed = NO;

  self.view.frame = CGRectMake(0, 0, 100, 60);
  XCTAssertNil([self.view valueForKey:@"drawable"]);
  XCTAssertEqual([(TestPlayer *)self.player adoptCount], 0);

  view.surfaceCreationAllowed = YES;
  [view onAnimaXApplicationDidBecomeActive];

  XCTAssertNotNil([self.view valueForKey:@"drawable"]);
  XCTAssertEqual([(TestPlayer *)self.player adoptCount], 1);
}

- (void)testFrameChangedImage {
  [self createViewIsImageView:YES];
  [self generalTestFrameChanged];
  [self createViewIsImageView:YES];
  [self generalTestInvalidFrameChanged];
}

#pragma mark Test: Visibility

- (void)generalTestVisibility {
  XCTAssertEqual([(TestPlayer *)self.player visibilityFlag], 0);
  [self.view didMoveToWindow];
  XCTAssertEqual([(TestPlayer *)self.player visibilityFlag], kAttach);
  UIWindow *win = [[UIWindow alloc] initWithFrame:CGRectMake(0, 0, 320, 480)];
  [win addSubview:self.view];
  XCTAssertEqual([(TestPlayer *)self.player visibilityFlag], 0);
  self.view.hidden = YES;
  XCTAssertEqual([(TestPlayer *)self.player visibilityFlag], kVisible);
  self.view.hidden = NO;
  XCTAssertEqual([(TestPlayer *)self.player visibilityFlag], 0);
  self.view.alpha = 0;
  XCTAssertEqual([(TestPlayer *)self.player visibilityFlag], kOpacity);
  self.view.alpha = 1;
  XCTAssertEqual([(TestPlayer *)self.player visibilityFlag], 0);
  self.view.layer.opacity = 0;
  XCTAssertEqual([(TestPlayer *)self.player visibilityFlag], kOpacity);
  self.view.layer.opacity = 1;
  XCTAssertEqual([(TestPlayer *)self.player visibilityFlag], 0);
  self.view.frame = CGRectMake(0, 0, 0, 0);
  XCTAssertEqual([(TestPlayer *)self.player visibilityFlag], kSize);
  self.view.frame = CGRectMake(0, 0, 50, 50);
  XCTAssertEqual([(TestPlayer *)self.player visibilityFlag], 0);
}

- (void)generalTestVisibilityMixed {
  [self.view didMoveToWindow];
  self.view.alpha = 0;
  XCTAssertEqual([(TestPlayer *)self.player visibilityFlag], kAttach | kOpacity);
}

- (void)testVisibility {
  [self createViewIsImageView:NO];
  [self generalTestVisibility];
  [self createViewIsImageView:NO];
  [self generalTestVisibilityMixed];
}

- (void)testVisibilityImage {
  [self createViewIsImageView:YES];
  [self generalTestVisibility];
  [self createViewIsImageView:YES];
  [self generalTestVisibilityMixed];
}

#pragma mark Test: Tap

- (UITouch *)makeFakeTouchAt:(CGPoint)pt inView:(UIView *)view {
  UITouch *touch = [[UITouch alloc] init];
  @try {
    [touch setValue:view forKey:@"view"];
    if (view.window) {
      [touch setValue:view.window forKey:@"window"];
    }
    [touch setValue:@(UITouchPhaseEnded) forKey:@"phase"];
    CGPoint winPt = view.window ? [view convertPoint:pt toView:view.window] : pt;
    NSValue *ptValue = [NSValue valueWithCGPoint:winPt];
    [touch setValue:ptValue forKey:@"_locationInWindow"];
  } @catch (NSException *exception) {
  }
  return touch;
}

- (void)emitTouchesEndedAt:(CGPoint)pt {
  UITouch *touch = [self makeFakeTouchAt:pt inView:self.view];
  NSSet<UITouch *> *touches = [NSSet setWithObject:touch];
  UIEvent *event = NULL;
  [self.view touchesEnded:touches withEvent:event];
}

- (void)generalTestTouch {
  TestPlayer *player = (TestPlayer *)self.player;
  self.view.frame = CGRectMake(0, 0, 100, 60);
  [self emitTouchesEndedAt:CGPointMake(10, 20)];
  XCTAssertEqual([(TestPlayer *)self.player hasHandleTap], NO);
  self.view.enableNativeTapLayerEvent = YES;
  self.view.userInteractionEnabled = YES;
  [self emitTouchesEndedAt:CGPointMake(10, 20)];
  XCTAssertEqual([(TestPlayer *)self.player hasHandleTap], YES);
}

- (void)testTouch {
  [self createViewIsImageView:NO];
  [self generalTestTouch];
}

- (void)testTouchImage {
  [self createViewIsImageView:YES];
  [self generalTestTouch];
}

#pragma mark Test: Player Delegate

- (NSArray<NSString *> *)selectorsFromProtocolStrict:(Protocol *)proto {
  unsigned int count = 0;
  struct objc_method_description *list =
      protocol_copyMethodDescriptionList(proto, YES, YES, &count);
  NSMutableArray<NSString *> *names = [NSMutableArray arrayWithCapacity:count];
  for (unsigned int i = 0; i < count; i++) {
    SEL sel = list[i].name;
    if (sel != NULL) {
      [names addObject:NSStringFromSelector(sel)];
    }
  }
  free(list);
  return names.copy;
}

- (void)generalTestPlayerDelegate {
  NSArray<NSString *> *sels = [self selectorsFromProtocolStrict:@protocol(AnimaXPlayerProtocol)];
  NSMutableArray<NSString *> *missing = [NSMutableArray array];
  for (NSString *name in sels) {
    if (![self.view respondsToSelector:NSSelectorFromString(name)]) {
      [missing addObject:name];
    }
  }
  XCTAssertTrue(missing.count == 0, @"Missing implementation: %@",
                [missing componentsJoinedByString:@", "]);
}

- (void)testPlayerDelegate {
  [self createViewIsImageView:NO];
  [self generalTestPlayerDelegate];
}

- (void)testPlayerDelegateImage {
  [self createViewIsImageView:YES];
  [self generalTestPlayerDelegate];
}

@end
