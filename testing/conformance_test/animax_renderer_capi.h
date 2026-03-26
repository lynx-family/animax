// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_TESTING_CONFORMANCE_TEST_ANIMAX_RENDERER_CAPI_H_
#define ANIMAX_TESTING_CONFORMANCE_TEST_ANIMAX_RENDERER_CAPI_H_

#include <stdint.h>

#define ANIMAX_EXPORT __attribute__((visibility("default")))

extern "C" {

struct ANIMAX_EXPORT _AnimaXRenderTask;
struct ANIMAX_EXPORT _AnimaXRenderResult;

typedef struct _AnimaXRenderTask* AnimaXRenderTask;
typedef struct _AnimaXRenderResult* AnimaXRenderResult;

ANIMAX_EXPORT AnimaXRenderTask AnimaXRenderTaskCreate(
    const char* lottie_json_file_path, int32_t width, int32_t height);
ANIMAX_EXPORT void AnimaXRenderTaskDestroy(AnimaXRenderTask task);
ANIMAX_EXPORT AnimaXRenderResult AnimaXRenderTaskRender(AnimaXRenderTask task,
                                                        int32_t frame);

ANIMAX_EXPORT void AnimaXRenderResultDestroy(AnimaXRenderResult result);
ANIMAX_EXPORT int32_t
AnimaXRenderResultGetRGBABitmapSize(AnimaXRenderResult result);
ANIMAX_EXPORT void AnimaXRenderResultCopyRGBABitmap(AnimaXRenderResult result,
                                                    uint8_t* bitmap);
}

#endif  // ANIMAX_TESTING_CONFORMANCE_TEST_ANIMAX_RENDERER_CAPI_H_
