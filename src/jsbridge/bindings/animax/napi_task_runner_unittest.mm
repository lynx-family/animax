// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/jsbridge/bindings/animax/napi_task_runner.h"
#import <Lynx/LynxContext.h>
#import <Lynx/LynxTemplateRender.h>
#import <Lynx/LynxView+Internal.h>
#import <Lynx/LynxView.h>
#import <XCTest/XCTest.h>
#import "LynxTemplateRender+Internal.h"
#include "core/runtime/common/napi/napi_environment.h"
#include "core/runtime/js/lynx_runtime.h"
#include "core/shell/lynx_shell.h"

constexpr static NSInteger DEFAULT_LYNXVIEW_NUMBER = 1;

namespace {
thread_local int kCurrentThreadMark;
void MarkCurrentThread(int mark) { kCurrentThreadMark = mark; }

int GetCurrentThreadMark() { return kCurrentThreadMark; }
}  // namespace

@interface napi_task_runner_unittest : XCTestCase
@property(nonatomic, readwrite) NSMutableDictionary<NSNumber*, LynxView*>* lynxViews;
@property(nonatomic, readwrite) LynxView* lynxView;
@property(nonatomic, readwrite) lynx::shell::LynxShell* shellPtr;
@end

@implementation napi_task_runner_unittest

- (void)setUp {
  srand(static_cast<unsigned int>(time(NULL)));
  [self createNewLynxViewWithLynxGroupNumber:DEFAULT_LYNXVIEW_NUMBER];
  self.lynxView = [self getLynxViewWithLynxGroupNumber:DEFAULT_LYNXVIEW_NUMBER];
  self.shellPtr = [self getShellPtrWithLynxGroupNumber:DEFAULT_LYNXVIEW_NUMBER];
}

- (void)tearDown {
  self.lynxViews = nil;
  self.shellPtr = nullptr;
}

- (lynx::shell::LynxShell*)getShellPtrWithLynxGroupNumber:(NSInteger)number {
  return reinterpret_cast<lynx::shell::LynxShell*>(
      [self.lynxViews objectForKey:@(number)].templateRender.uiOwner.uiContext.shellPtr);
}

- (lynx::shell::LynxShell*)shellPtr {
  return [self getShellPtrWithLynxGroupNumber:1];
}

- (LynxView*)getLynxViewWithLynxGroupNumber:(NSInteger)number {
  return [self.lynxViews objectForKey:@(number)];
}

- (LynxView*)lynxView {
  return [self getLynxViewWithLynxGroupNumber:1];
}

- (void)createNewLynxViewWithLynxGroupNumber:(NSInteger)number {
  LynxView* view = [[LynxView alloc] initWithBuilderBlock:^(LynxViewBuilder* builder) {
    LynxGroupOption* option = [[LynxGroupOption alloc] init];
    option.enableCanvas = NO;
    option.enableJSGroupThread = YES;
    builder.group = [[LynxGroup alloc]
               initWithName:[NSString stringWithFormat:@"AnimaXNapiTestGroup%ld", number]
        withLynxGroupOption:option];
  }];
  if (self.lynxViews == nil) {
    self.lynxViews = [[NSMutableDictionary alloc] init];
  }
  [self.lynxViews setObject:view forKey:@(number)];
  if (number == 1) {
    self.lynxView = [self getLynxViewWithLynxGroupNumber:1];
    self.shellPtr = [self getShellPtrWithLynxGroupNumber:1];
  }
}

- (void)testPostTaskDefaultNapiTaskRunner {
  using namespace lynx;
  auto task_runner = animax::NapiTaskRunner();
  task_runner.PostTask(
      []() { XCTFail("Default constructed NapiTaskRunner PostTask should be noop"); });
}

// Tests the ability of the NAPI Task Runner to receive and execute a simple task.
// This is verified by posting a task to the NAPI Task Runner and fulfilling an XCTestExpectation
// when the task is invoked.
- (void)testPostTaskToNapiTaskRunner {
  using namespace lynx;
  XCTestExpectation* expectation = [self expectationWithDescription:@"task should be invoked"];
  self.shellPtr->GetRuntimeActor()->ActAsync(
      [expectation](std::unique_ptr<lynx::runtime::LynxRuntime>& runtime) {
        auto env = runtime->GetNapiEnvironment()->proxy()->Env();
        auto task_runner =
            std::shared_ptr<lynx::animax::NapiTaskRunner>(new lynx::animax::NapiTaskRunner{env});
        dispatch_async(dispatch_get_main_queue(), ^{
          task_runner->PostTask([expectation]() { [expectation fulfill]; });
        });
      });
  [self waitForExpectationsWithTimeout:5 handler:nil];
}

- (void)testPostTasktoMovedConstructedNapiTaskRunner {
  using namespace lynx;
  XCTestExpectation* expectation = [self expectationWithDescription:@"task should be invoked"];
  self.shellPtr->GetRuntimeActor()->ActAsync(
      [expectation](std::unique_ptr<lynx::runtime::LynxRuntime>& runtime) {
        auto env = runtime->GetNapiEnvironment()->proxy()->Env();
        auto task_runner = lynx::animax::NapiTaskRunner{env};
        auto task_runner2 = std::move(task_runner);
        task_runner2.PostTask([expectation]() { [expectation fulfill]; });
        task_runner.PostTask(
            []() { XCTFail("Default constructed NapiTaskRunner PostTask should be noop"); });
      });
  [self waitForExpectationsWithTimeout:5 handler:nil];
}

- (void)testPostTasktoMoveAssignedNapiTaskRunner {
  using namespace lynx;
  XCTestExpectation* expectation = [self expectationWithDescription:@"task should be invoked"];
  self.shellPtr->GetRuntimeActor()->ActAsync(
      [expectation](std::unique_ptr<lynx::runtime::LynxRuntime>& runtime) {
        auto env = runtime->GetNapiEnvironment()->proxy()->Env();
        auto task_runner = lynx::animax::NapiTaskRunner{env};
        auto task_runner2 = lynx::animax::NapiTaskRunner{};
        task_runner2 = std::move(task_runner);
        task_runner2.PostTask([expectation]() { [expectation fulfill]; });
        task_runner.PostTask(
            []() { XCTFail("Default constructed NapiTaskRunner PostTask should be noop"); });
      });
  [self waitForExpectationsWithTimeout:5 handler:nil];
}

// Ensures that resources associated with a task are correctly released after the task is executed.
// A unique_ptr is used to manage a resource, with a custom deleter that fulfills an
// XCTestExpectation upon deletion, indicating resource release.
- (void)testTaskResourceReleaseAfterInvocation {
  using namespace lynx;
  XCTestExpectation* releaseExpectation =
      [self expectationWithDescription:@"this resource should be released"];
  auto releaseExpectationDeleter = [releaseExpectation](int* resource) {
    [releaseExpectation fulfill];
  };
  releaseExpectation = nil;
  // Create a unique resource managed by a unique_ptr, using the custom deleter to monitor its
  // release.
  auto unique_resource = std::unique_ptr<int, decltype(releaseExpectationDeleter)>(
      new int{1}, std::move(releaseExpectationDeleter));

  self.shellPtr->GetRuntimeActor()->ActAsync(
      [unique_resource = std::move(unique_resource)](
          std::unique_ptr<lynx::runtime::LynxRuntime>& runtime) mutable {
        auto env = runtime->GetNapiEnvironment()->proxy()->Env();
        auto task_runner =
            std::shared_ptr<lynx::animax::NapiTaskRunner>(new lynx::animax::NapiTaskRunner{env});
        task_runner->PostTask([unique_resource = std::move(unique_resource)]() {
          // Post an empty task that captures the unique_resource. The task itself doesn't need to
          // perform any action; the focus is on the release of the unique_resource upon task
          // completion.
        });
      });
  [self waitForExpectationsWithTimeout:5 handler:nil];
}

// Verifies the NAPI Task Runner's ability to handle multiple tasks being posted.
// Six separate tasks are posted, each fulfilling its own XCTestExpectation upon invocation to
// confirm successful execution.
- (void)testPostingMultipleTasksToNapiTaskRunner {
  using namespace lynx;
  // These two tasks(e1, e2) are posted as soon as the task runner is created.
  XCTestExpectation* e1 = [self expectationWithDescription:@"task1 should be invoked"];
  XCTestExpectation* e2 = [self expectationWithDescription:@"task2 should be invoked"];

  self.shellPtr->GetRuntimeActor()->ActAsync(
      [e1, e2](std::unique_ptr<lynx::runtime::LynxRuntime>& runtime) {
        auto env = runtime->GetNapiEnvironment()->proxy()->Env();
        auto task_runner =
            std::shared_ptr<lynx::animax::NapiTaskRunner>(new lynx::animax::NapiTaskRunner{env});
        dispatch_async(dispatch_get_main_queue(), ^{
          task_runner->PostTask([e1]() { [e1 fulfill]; });
          task_runner->PostTask([e2]() { [e2 fulfill]; });
        });
      });
  [self waitForExpectationsWithTimeout:5 handler:nil];

  // These two tasks (e3, e4) are posted separately in another ActAsync call.
  XCTestExpectation* e3 = [self expectationWithDescription:@"task3 should be invoked"];
  XCTestExpectation* e4 = [self expectationWithDescription:@"task4 should be invoked"];

  self.shellPtr->GetRuntimeActor()->ActAsync(
      [e3](std::unique_ptr<lynx::runtime::LynxRuntime>& runtime) {
        auto env = runtime->GetNapiEnvironment()->proxy()->Env();
        auto task_runner =
            std::shared_ptr<lynx::animax::NapiTaskRunner>(new lynx::animax::NapiTaskRunner{env});
        dispatch_async(dispatch_get_main_queue(), ^{
          task_runner->PostTask([e3]() { [e3 fulfill]; });
        });
      });
  self.shellPtr->GetRuntimeActor()->ActAsync(
      [e4](std::unique_ptr<lynx::runtime::LynxRuntime>& runtime) {
        auto env = runtime->GetNapiEnvironment()->proxy()->Env();
        auto task_runner =
            std::shared_ptr<lynx::animax::NapiTaskRunner>(new lynx::animax::NapiTaskRunner{env});
        dispatch_async(dispatch_get_main_queue(), ^{
          task_runner->PostTask([e4]() { [e4 fulfill]; });
        });
      });
  [self waitForExpectationsWithTimeout:5 handler:nil];
}

- (void)testPostTaskWhenNapiEnvWasDestoried {
  using namespace lynx;
  XCTestExpectation* napiTaskRunnerCreatedExpectation =
      [self expectationWithDescription:@"napi task runner should be created."];
  auto old_task_runner = std::shared_ptr<lynx::animax::NapiTaskRunner>{};
  self.shellPtr->GetRuntimeActor()->ActAsync(
      [napiTaskRunnerCreatedExpectation,
       &old_task_runner](std::unique_ptr<lynx::runtime::LynxRuntime>& runtime) {
        auto env = runtime->GetNapiEnvironment()->proxy()->Env();
        old_task_runner =
            std::shared_ptr<lynx::animax::NapiTaskRunner>(new lynx::animax::NapiTaskRunner{env});
        [napiTaskRunnerCreatedExpectation fulfill];
      });
  [self waitForExpectationsWithTimeout:5 handler:nil];
  // Destroy the old LynxView and create a new one. This will make the Napi Env recreated
  // (Async'ly).
  [self createNewLynxViewWithLynxGroupNumber:DEFAULT_LYNXVIEW_NUMBER];
  XCTestExpectation* dispatchExp =
      [self expectationWithDescription:@"dispatch should be performed"];
  dispatch_async(dispatch_get_main_queue(), ^{
    self.shellPtr->GetRuntimeActor()->ActAsync(
        [old_task_runner, dispatchExp](std::unique_ptr<lynx::runtime::LynxRuntime>& runtime) {
          // The old NapiTaskRunner should be invalidated
          // At this point, we create a new NapiTaskRunner
          auto env = runtime->GetNapiEnvironment()->proxy()->Env();
          std::shared_ptr<lynx::animax::NapiTaskRunner>(new lynx::animax::NapiTaskRunner{env});
          // we try to PostTask with the old task runner, the task may or may not run.
          dispatch_async(dispatch_get_main_queue(), ^{
            [dispatchExp fulfill];
            old_task_runner->PostTask([]() {
              // There should be no issue on PostTask.
            });
          });
        });
  });
  [self waitForExpectationsWithTimeout:5 handler:nil];
}

// Tests task execution within specified Lynx Group contexts to ensure proper task routing.
// Tasks are posted to two different Lynx Groups, with each task verifying execution in its intended
// group by checking thread marks.
- (void)testTaskExecutionInSpecifiedLynxGroup {
  using namespace lynx;
  XCTestExpectation* e1 = [self expectationWithDescription:@"task1 should be invoked"];
  XCTestExpectation* e2 = [self expectationWithDescription:@"task2 should be invoked"];

  auto random1 = rand() % 100;
  auto random2 = rand() % 100;

  [self createNewLynxViewWithLynxGroupNumber:2];

  [self getShellPtrWithLynxGroupNumber:1]->GetRuntimeActor()
      -> ActAsync([random1](std::unique_ptr<lynx::runtime::LynxRuntime>& runtime) {
    auto env = runtime->GetNapiEnvironment()->proxy()->Env();
    std::shared_ptr<lynx::animax::NapiTaskRunner>(new lynx::animax::NapiTaskRunner{env});
    MarkCurrentThread(random1);
  });
  [self getShellPtrWithLynxGroupNumber:2]->GetRuntimeActor()
      -> ActAsync([random2](std::unique_ptr<lynx::runtime::LynxRuntime>& runtime) {
    auto env = runtime->GetNapiEnvironment()->proxy()->Env();
    std::shared_ptr<lynx::animax::NapiTaskRunner>(new lynx::animax::NapiTaskRunner{env});
    MarkCurrentThread(random2);
  });
  [self getShellPtrWithLynxGroupNumber:1]->GetRuntimeActor()
      -> ActAsync([e1, random1](std::unique_ptr<lynx::runtime::LynxRuntime>& runtime) {
    auto env = runtime->GetNapiEnvironment()->proxy()->Env();
    auto task_runner =
        std::shared_ptr<lynx::animax::NapiTaskRunner>(new lynx::animax::NapiTaskRunner{env});
    dispatch_async(dispatch_get_main_queue(), ^{
      task_runner->PostTask([e1, random1]() {
        XCTAssertEqual(GetCurrentThreadMark(), random1,
                       "Current thread should be marked as random1.");
        [e1 fulfill];
      });
    });
  });
  [self getShellPtrWithLynxGroupNumber:2]->GetRuntimeActor()
      -> ActAsync([e2, random2](std::unique_ptr<lynx::runtime::LynxRuntime>& runtime) {
    auto env = runtime->GetNapiEnvironment()->proxy()->Env();
    auto task_runner =
        std::shared_ptr<lynx::animax::NapiTaskRunner>(new lynx::animax::NapiTaskRunner{env});
    dispatch_async(dispatch_get_main_queue(), ^{
      task_runner->PostTask([e2, random2]() {
        XCTAssertEqual(GetCurrentThreadMark(), random2,
                       "Current thread should be marked as random2.");
        [e2 fulfill];
      });
    });
  });
  [self waitForExpectationsWithTimeout:5 handler:nil];
}

@end
