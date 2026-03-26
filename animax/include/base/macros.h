// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_INCLUDE_BASE_MACROS_H_
#define ANIMAX_INCLUDE_BASE_MACROS_H_

#if defined(OS_MAC) || defined(OS_IOS)
#define ANIMAX_EXPORT
#elif defined(OS_WIN)
#define ANIMAX_EXPORT __declspec(dllexport)
#else
#define ANIMAX_EXPORT __attribute__((visibility("default")))
#endif

#endif  // ANIMAX_INCLUDE_BASE_MACROS_H_
