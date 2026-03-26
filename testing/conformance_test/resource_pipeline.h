// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_TESTING_CONFORMANCE_TEST_RESOURCE_PIPELINE_H_
#define ANIMAX_TESTING_CONFORMANCE_TEST_RESOURCE_PIPELINE_H_

#include "include/resource/loader.h"
#include "include/resource/resource_task.h"
#include "src/resource/composition_loader/composition_model_pipeline.h"

namespace lynx {
namespace animax {

Loader<ResourceRequest, ResourceResponse>::Ptr GetSharedResourcePipeline();
std::shared_ptr<CompositionModelPipeline> GetSharedCompoistionPipeline();

#endif  // ANIMAX_TESTING_CONFORMANCE_TEST_RESOURCE_PIPELINE_H_

}  // namespace animax
}  // namespace lynx
