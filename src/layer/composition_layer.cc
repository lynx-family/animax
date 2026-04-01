// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/layer/composition_layer.h"

#include <unordered_map>

#include "src/layer/layer_factory.h"
#include "src/layer/text_layer.h"

namespace lynx {
namespace animax {

CompositionLayer::CompositionLayer(LayerModel& layer_model,
                                   CompositionModel& composition)
    : BaseLayer(layer_model, composition),
      layer_paint_(std::make_unique<Paint>()) {}

void CompositionLayer::Init() {
  BaseLayer::Init();

  auto* time_remapping = layer_model_.GetTimeRemapping();
  if (time_remapping) {
    time_remapping_animation_ = time_remapping->CreateAnimation();
    AddAnimation(time_remapping_animation_.get());
    time_remapping_animation_->AddUpdateListener(this);
  } else {
    time_remapping_animation_ = nullptr;
  }

  std::unordered_map<int32_t, size_t> layer_map;  // layerId to index of layers
  decltype(layers_) layers;
  for (auto it = layer_models_.rbegin(); it != layer_models_.rend(); it++) {
    auto& cur_model = **it;
    auto layer = LayerFactory::CreateLayer(cur_model, composition_);
    if (layer == nullptr) {
      continue;
    }
    layer->SetCompositionLayer(this);
    layer->SetEventListener(this);
    layer->SetPlayerContext(weak_context_);
    layer->Init();
    switch (cur_model.GetLayerType()) {
      case LayerType::kCamera:
        // we should pick the last camera in reverse iterating
        camera_layer_ = std::unique_ptr<CameraLayer>(
            static_cast<CameraLayer*>(layer.release()));
        break;
      default:
        layer_map[layer->GetLayerModel().GetId()] = layers.size();
        layers.push_back(std::move(layer));
        break;
    }
  }

  for (const auto [layer_id, layer_index] : layer_map) {
    auto& layer_model = layers[layer_index]->GetLayerModel();

    const auto parent_id = layer_model.GetParentId();
    if (layer_map.count(parent_id)) {
      auto parent_index = layer_map[parent_id];
      layers[layer_index]->SetParentLayer(layers[parent_index].get());
    }

    // matte
    const auto matte_type = layer_model.GetMatteType();
    if (MatteType::kAlpha == matte_type ||
        MatteType::kAlphaInverted == matte_type) {
      int32_t matte_parent = layer_model.GetMatteParent();
      matte_parent =
          (matte_parent == -1) ? layer_model.GetId() - 1 : matte_parent;
      if (layer_map.count(matte_parent)) {
        auto matte_parent_index = layer_map[matte_parent];
        layers[matte_parent_index]->GetLayerModel().SetIsMatteTarget(true);
        layers[layer_index]->SetMattedLayer(layers[matte_parent_index].get());
      }
    }
  }
  for (auto it = layers.rbegin(); it != layers.rend(); ++it) {
    if ((*it)->GetLayerModel().IsMatteTarget()) {
      matte_target_layers_.push_back(std::move(*it));
    } else {
      layers_.push_back(std::move(*it));
    }
  }

  if (time_remapping_animation_) {
    time_remapping_animation_->AddUpdateListener(this);
  }
}

inline std::unique_ptr<Matrix> CompositionLayer::GetCameraMatrix(
    Canvas& canvas) {
  if (!composition_.GetEnable3D()) {
    return nullptr;
  }

  if (camera_ == nullptr) {
    camera_ = std::make_unique<Camera>(layer_model_.GetPreCompWidth(),
                                       layer_model_.GetPreCompHeight(),
                                       camera_layer_ == nullptr);
  }

  if (camera_layer_ != nullptr) {
    camera_layer_->ConfigCamera(camera_.get());
  }

  return camera_->GetMatrix();
}

void CompositionLayer::DrawLayer(Canvas& canvas, Matrix& parent_matrix,
                                 int32_t parent_alpha) {
  canvas.Save();
  if (!GetLayerModel().GetEnable3D()) {
    DrawLayerInner(canvas, parent_matrix, parent_alpha);
  } else {
    canvas.Concat(parent_matrix);
    Matrix local_matrix;
    DrawLayerInner(canvas, local_matrix, parent_alpha);
  }
  canvas.Restore();
}

void CompositionLayer::DrawLayerInner(Canvas& canvas, Matrix& matrix,
                                      int32_t parent_alpha) {
  new_clip_rect_.Set(0, 0, layer_model_.GetPreCompWidth(),
                     layer_model_.GetPreCompHeight());
  matrix.MapRect(new_clip_rect_);

  // By default, translucency and drop-shadow compositing issues are not
  // resolved. When a group contains 2 or more contents with translucency or
  // drop-shadow effects, the rendering fidelity may differ from After Effects.
  // This is intentional, as fixing it requires expensive off-screen rendering.
  // We generally recommend that designers optimize and adjust at the design
  // level. If this issue receives significant feedback, we will consider adding
  // compatibility support.
  bool non_empty_rect = true;
  auto layer_name = layer_model_.GetName().data();
  bool ignore_clip = !clip_to_composition_bounds_ &&
                     std::strcmp("__container", layer_name) == 0;
  if (!ignore_clip && !new_clip_rect_.IsEmpty()) {
    non_empty_rect = canvas.ClipRect(new_clip_rect_);
  }

  auto camera_matrix = GetCameraMatrix(canvas);
  auto child_alpha = parent_alpha;
  for (auto it = layers_.rbegin(); it != layers_.rend(); it++) {
    if (!non_empty_rect) {
      continue;
    }
    if (!camera_matrix || !(*it)->GetLayerModel().GetEnable3D()) {
      (*it)->Draw(canvas, matrix, child_alpha);
    } else {
      canvas.Save();
      canvas.Concat(*camera_matrix);
      (*it)->Draw(canvas, matrix, child_alpha);
      canvas.Restore();
    }
  }
}

void CompositionLayer::SetProgress(float progress) {
  BaseLayer::SetProgress(progress);

  if (time_remapping_animation_) {
    auto duration_frames = composition_.GetDurationFrames() + 0.01;
    auto delay_frames = composition_.GetStartFrame();
    auto remapped_frames = time_remapping_animation_->GetValue().Get() *
                               composition_.GetFrameRate() -
                           delay_frames;
    progress = remapped_frames / duration_frames;
  }

  if (time_remapping_animation_ == nullptr) {
    progress -= layer_model_.GetStartProgress();
  }

  if (layer_model_.GetTimeStretch() != 0 &&
      layer_model_.GetName() != "__container") {
    progress /= layer_model_.GetTimeStretch();
  }

  for (auto it = layers_.rbegin(); it != layers_.rend(); it++) {
    (*it)->SetProgress(progress);
  }

  for (auto it = matte_target_layers_.rbegin();
       it != matte_target_layers_.rend(); it++) {
    (*it)->SetProgress(progress);
  }

  if (camera_layer_) {
    camera_layer_->SetProgress(progress);
  }
}

void CompositionLayer::GetBounds(RectF& out_bounds, Matrix& parent_matrix,
                                 bool apply_parent) {
  BaseLayer::GetBounds(out_bounds, parent_matrix, apply_parent);
  for (auto it = layers_.rbegin(); it != layers_.rend(); it++) {
    rect_.Set(0, 0, 0, 0);
    (*it)->GetBounds(rect_, *bounds_matrix_, true);
    out_bounds.Union(rect_);
  }
}

void CompositionLayer::OnLayerError(const EventError err,
                                    const std::string& err_msg) {
  if (listener_) {
    listener_->OnLayerError(err, err_msg);
  }
}

void CompositionLayer::OnLayerWarning(const lynx::animax::EventWarning warning,
                                      const std::string& warning_msg) {
  if (listener_) {
    listener_->OnLayerWarning(warning, warning_msg);
  }
}

void CompositionLayer::ResolveChildKeyPath(
    const AnimaXKeyPath& path, int32_t depth,
    ResolvedKeyPathElements& match_elements,
    const AnimaXKeyPath& current_partial_path) {
  for (auto& layer : layers_) {
    layer->ResolveKeyPath(path, depth, match_elements, current_partial_path);
  }
  // TODO(aiyongbiao.rick): support camera layer property update.
}

void CompositionLayer::HitTest(float x, float y,
                               std::unordered_set<std::string>& hit_layers) {
  for (auto& layer : layers_) {
    layer->HitTest(x, y, hit_layers);
  }
}

bool CompositionLayer::GetBoundsByName(const std::string& target_name,
                                       RectF& out_bounds) {
  bool result = BaseLayer::GetBoundsByName(target_name, out_bounds);
  if (result) {
    return true;
  }
  for (auto& layer : layers_) {
    result = layer->GetBoundsByName(target_name, out_bounds);
    if (result) {
      return true;
    }
  }
  return false;
}

}  // namespace animax
}  // namespace lynx
