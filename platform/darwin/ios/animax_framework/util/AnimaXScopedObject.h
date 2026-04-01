// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/// Generic RAII wrapper that brings ARC lifetime management to CF / CV objects.
///
/// Subclasses expose a typed `object` property and a type-specific `wrap:` factory,
/// while the base class owns the raw pointer and runs the supplied deleter in dealloc.
///
/// Use macros to generate typed ObjC wrappers for CF/CV refs.
/// ObjC lightweight generics cannot take CV/CF ref types, and C++ templates do not fit
/// ObjC API boundaries such as @property, protocol methods, or framework headers.
typedef void (*AnimaXScopedObjectDeleter)(const void *);

@interface AnimaXScopedObject : NSObject

+ (instancetype)new NS_UNAVAILABLE;
- (instancetype)init NS_UNAVAILABLE;

- (nullable instancetype)initWithObject:(nullable const void *)object
                                deleter:(AnimaXScopedObjectDeleter _Nonnull)deleter
    NS_DESIGNATED_INITIALIZER;

@property(nonatomic, readonly, nullable) const void *rawObject;

@end

#define ANIMAX_SCOPED_OBJECT_INTERFACE(ClassName, RefType)         \
  @interface ClassName : AnimaXScopedObject                        \
  @property(nonatomic, readonly, nullable) RefType object;         \
  +(nullable instancetype)newWrap : (nullable RefType)object;      \
  +(nullable instancetype)newWrapOwned : (nullable RefType)object; \
  @end

#define ANIMAX_SCOPED_OBJECT_IMPLEMENTATION(ClassName, RefType, RetainFn, ReleaseFn)          \
  @implementation ClassName                                                                   \
  +(nullable instancetype)newWrap : (nullable RefType)object {                                \
    if (object == NULL) {                                                                     \
      return nil;                                                                             \
    }                                                                                         \
    RetainFn(object);                                                                         \
    return [[self alloc] initWithObject:object deleter:(AnimaXScopedObjectDeleter)ReleaseFn]; \
  }                                                                                           \
  +(nullable instancetype)newWrapOwned : (nullable RefType)object {                           \
    if (object == NULL) {                                                                     \
      return nil;                                                                             \
    }                                                                                         \
    return [[self alloc] initWithObject:object deleter:(AnimaXScopedObjectDeleter)ReleaseFn]; \
  }                                                                                           \
  -(RefType)object {                                                                          \
    return (RefType)self.rawObject;                                                           \
  }                                                                                           \
  @end

NS_ASSUME_NONNULL_END
