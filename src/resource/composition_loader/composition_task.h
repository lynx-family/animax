// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RESOURCE_COMPOSITION_LOADER_COMPOSITION_TASK_H_
#define ANIMAX_SRC_RESOURCE_COMPOSITION_LOADER_COMPOSITION_TASK_H_

#include <memory>
#include <string>

#include "include/resource/loader_error.h"
#include "include/resource/uri_info.h"

namespace lynx {
namespace animax {

class CompositionModel;

enum class CompositionModelType {
  kInvalid = 0,
  kLottie = 1,
  kAlphaVideo = 2,
};

/**
 * @struct CompositionModelRequest
 * Encapsulates parameters necessary for requesting processing of a composition
 * model. Supports dual data sources: a URI or a raw JSON string, and includes a
 * scale factor for rendering adjustments.
 *
 * @param uri_info Contains the URI along with its scheme and content type. It
 * determines the type of composition to load:
 *                 - Loads an alpha video composition if the content type is
 * kZip.
 *                 - Can load a Lottie composition if the content type is kJson
 * or if the json_str is provided.
 * @param json_str Provides raw JSON data for inline model provisioning. When
 * provided, it indicates a request for a Lottie animation model.
 * @param scale    A scale factor that adjusts the rendering size of the model.
 */
struct CompositionModelRequest {
  UriInfo uri_info{};
  std::string json_str;
  float scale = 1.0f;
};

/**
 * @struct CompositionModelResponse
 * Wraps the processed composition model along with its metadata.
 *
 * @param model Shared pointer to the requested composition model.
 * @param model_type Specifies the type of composition model processed,
 * distinguishing between Lottie and Alpha Video.
 * @param base_uri Provides the base URI for accessing external resources
 * required by the model. This is left empty if the model was loaded directly
 * from a provided json_str in the request.
 *
 * Note: Assets for kLottie Composition Models are not loaded.
 */
struct CompositionModelResponse {
  std::shared_ptr<CompositionModel> model{};
  CompositionModelType model_type{};
  std::string base_uri{};
  std::string main_uri{};
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RESOURCE_COMPOSITION_LOADER_COMPOSITION_TASK_H_
