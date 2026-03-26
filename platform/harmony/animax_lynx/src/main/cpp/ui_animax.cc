// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "platform/harmony/animax_lynx/src/main/cpp/ui_animax.h"

#include <arkui/native_type.h>
#include <dlfcn.h>

#include "animax/platform/harmony/animax_lynx/src/main/cpp/ui_animax.h"
#include "base/include/fml/make_copyable.h"
#include "base/include/no_destructor.h"
#include "base/include/platform/harmony/harmony_vsync_manager.h"
#include "core/renderer/dom/lynx_get_ui_result.h"
#include "include/base/visibility_state.h"
#include "include/player/animax_player_builder.h"
#include "platform/harmony/lynx_harmony/src/main/cpp/ui/base/node_manager.h"
#include "platform/harmony/lynx_harmony/src/main/cpp/ui/ui_root.h"
#include "src/base/gl/harmony/animax_surface_harmony.h"
#include "src/base/log/log.h"
#include "src/base/util/animax_native.h"
#include "src/player/vsync_dispatcher.h"
#include "src/resource/resource_loader/harmony/resource_loader_harmony.h"
#include "src/resource/resource_loader/harmony/unzip_loader_harmony.h"

namespace lynx {
namespace animax {

namespace {
static constexpr const char* kData = "data";
static constexpr const char* const kTapLayers = "taplayers";
}  // namespace

extern "C" {
lynx::tasm::harmony::UIBase* UIAnimaXCreateFunc(
    lynx::tasm::harmony::LynxContext* context, int sign,
    const std::string& tag) {
  return new UIAnimaX(static_cast<lynx::tasm::harmony::LynxContext*>(context),
                      sign, tag);
}

void TryRegisterALogFunction() {
  void* handle = dlopen("libalog.so", RTLD_LAZY);
  if (handle == nullptr) {
    return;
  }
  ALogFunction sym_default_instance = reinterpret_cast<ALogFunction>(
      dlsym(handle, "alog_write_default_instance"));
  if (sym_default_instance == nullptr) {
    return;
  }
  AnimaXNative::Instance().RegisterALogFunction(sym_default_instance);
}
}

class VsyncDispatcherHarmony final : public VSyncDispatcher {
 public:
  void RequestVSync() override {
    base::HarmonyVsyncManager::GetInstance().RequestVSync(
        fml::MakeCopyable([](long long timestamp) mutable {
          VSyncDispatcher::Instance().OnVSync(timestamp);
        }));
  }
};

VSyncDispatcher& VSyncDispatcher::Instance() {
  static base::NoDestructor<VsyncDispatcherHarmony> inst;
  return *inst;
}

std::unordered_map<std::string, UIAnimaX::PropSetter> UIAnimaX::prop_setters_{
    {"autoplay", &UIAnimaX::SetAutoPlay},
    {"speed", &UIAnimaX::SetSpeed},
    {"progress", &UIAnimaX::SetProgress},
    {"objectfit", &UIAnimaX::SetObjectFit},
    {"keeplastframe", &UIAnimaX::SetKeepLastFrame},
    {"loop", &UIAnimaX::SetLoop},
    {"loop-count", &UIAnimaX::SetLoopCount},
    {"src-polyfill", &UIAnimaX::SetSrcPolyfill},
    {"start-frame", &UIAnimaX::SetStartFrame},
    {"end-frame", &UIAnimaX::SetEndFrame},
    {"auto-reverse", &UIAnimaX::SetReverseMode},
    {"anti-aliasing", &UIAnimaX::SetAntiAliasing},
    {"fps-event-interval", &UIAnimaX::SetFpsEventInterval},
    {"max-frame-rate", &UIAnimaX::SetMaxFrameRate},
    {"dynamic-resource", &UIAnimaX::SetDynamicResource},
    {"opacity", &UIAnimaX::SetOpacity},
    {"visibility", &UIAnimaX::SetVisibility},
    {"ignore-attach-status", &UIAnimaX::SetIgnoreAttachStatus},
    {"ignore-lynx-lifecycle", &UIAnimaX::SetIgnoreLynxLifecycle},
    {"video-frame-timeout", &UIAnimaX::SetVideoFrameTimeout},
    {"enable-audio", &UIAnimaX::SetEnableAudio},
    {"muted", &UIAnimaX::SetMuted},
};

std::unordered_map<std::string, UIAnimaX::PropSetter>
    UIAnimaX::delayed_prop_setters_{
        {"json", &UIAnimaX::SetJson},
        {"src", &UIAnimaX::SetSrc},
        {"src-format", &UIAnimaX::SetSrcFormat},
    };

std::unordered_map<std::string, UIAnimaX::PropSetter>
    UIAnimaX::prepared_prop_setters_{
        {"multi-thread-accelerate", &UIAnimaX::SetMultiThreadAccelerate},
    };

std::unordered_map<std::string, UIAnimaX::UIMethod> UIAnimaX::ui_method_map_ = {
    {"play", &UIAnimaX::Play},
    {"pause", &UIAnimaX::Pause},
    {"resume", &UIAnimaX::Resume},
    {"stop", &UIAnimaX::Stop},
    {"getDuration", &UIAnimaX::GetDuration},
    {"isAnimating", &UIAnimaX::IsAnimating},
    {"seek", &UIAnimaX::Seek},
    {"subscribeUpdateEvent", &UIAnimaX::SubscribeUpdateEvent},
    {"unsubscribeUpdateEvent", &UIAnimaX::UnsubscribeUpdateEvent},
    {"subscribeUpdateEvents", &UIAnimaX::SubscribeUpdateEvents},
    {"unsubscribeUpdateEvents", &UIAnimaX::UnsubscribeUpdateEvents},
    {"getCurrentFrame", &UIAnimaX::GetCurrentFrame},
    {"playSegment", &UIAnimaX::PlaySegment},
};

UIAnimaX::UIAnimaX(tasm::harmony::LynxContext* context, int sign,
                   const std::string& tag)
    : tasm::harmony::UIBase(context, ARKUI_NODE_XCOMPONENT, sign, tag) {
  ANIMAX_LOGI("UIAnimaX constructor this:" << this);

  InitializeXComponentNode();
  InitializeXComponentCallback();
}

void UIAnimaX::InitializeXComponentNode() {
  // Set xcomponent background color to transparent
  tasm::harmony::NodeManager::Instance().SetAttributeWithNumberValue(
      Node(), NODE_BACKGROUND_COLOR, 0x00000000);

  // Set xcomponent type to surface
  tasm::harmony::NodeManager::Instance().SetAttributeWithNumberValue(
      Node(), NODE_XCOMPONENT_TYPE,
      static_cast<int32_t>(ARKUI_XCOMPONENT_TYPE_SURFACE));
  tasm::harmony::NodeManager::Instance().RegisterNodeEvent(
      Node(), NODE_EVENT_ON_ATTACH, NODE_EVENT_ON_ATTACH, this);
  tasm::harmony::NodeManager::Instance().RegisterNodeEvent(
      Node(), NODE_EVENT_ON_DETACH, NODE_EVENT_ON_DETACH, this);
  tasm::harmony::NodeManager::Instance().RegisterNodeEvent(
      Node(), NODE_EVENT_ON_AREA_CHANGE, NODE_EVENT_ON_AREA_CHANGE, this);
}

void UIAnimaX::InitializeXComponentCallback() {
  native_component_ = OH_NativeXComponent_GetNativeXComponent(Node());
  XComponentAdapter::GetInstance().RegisterCallback(native_component_, this);
}

void UIAnimaX::InitializeXPlayer() {
  auto scale = context_->ScaledDensity();

  // Initialize resource loaders
  auto resource_loader = std::shared_ptr<animax::ResourceLoaderHarmony>(
      new animax::ResourceLoaderHarmony(context_->GetResourceLoader()));
  auto unzip_loader = std::shared_ptr<animax::UnzipLoaderHarmony>(
      new animax::UnzipLoaderHarmony());

  AnimaXPlayerBuilder builder;
  builder.SetScale(scale)
      .SetResourceLoader(std::move(resource_loader))
      .SetUnzipLoader(std::move(unzip_loader))
      .EnableMultiThreadAccelerate(multi_thread_accelerate_);

  // Ensure event listener is added before build
  AddEventListener(builder);

  ability_ = std::make_shared<lynx::animax::AnimaXAbilityHarmony>();
  builder.SetAbility(ability_);

  // Build player
  player_ = builder.Build();
}

void UIAnimaX::AddEventListener(AnimaXPlayerBuilder& builder) {
  builder.AddEventListener([context = GetContext(), sign = Sign()](
                               AnimaXPlayer* player, const Event event,
                               const animax::EventParamMap& params) {
    auto params_out = lepus::Dictionary::Create();
    for (const auto& pair : params) {
      const auto& key = pair.first;
      const auto& value = pair.second;
      switch (value.type) {
        case animax::EventParamValue::Type::kInt32:
          if (value.int_val) {
            params_out->SetValue(key, *value.int_val);
          }
          break;
        case animax::EventParamValue::Type::kDouble:
          if (value.double_val) {
            params_out->SetValue(key, *value.double_val);
          }
          break;
        case animax::EventParamValue::Type::kString:
          if (value.string_val) {
            params_out->SetValue(key, *value.string_val);
          }
          break;
        case animax::EventParamValue::Type::kStringVector:
          if (value.string_vector_val) {
            auto lepus_array = lepus::CArray::Create();
            for (const auto& str : *value.string_vector_val) {
              lepus_array->emplace_back(str);
            }
            params_out->SetValue(key, std::move(lepus_array));
          }
          break;
        default:
          break;
      }
    }
    tasm::harmony::CustomEvent custom_event{sign, UIAnimaX::GetEventName(event),
                                            "detail", lepus_value(params_out)};
    context->SendEvent(custom_event);
  });
}

std::string UIAnimaX::GetEventName(Event event) {
  std::string name;
  if (Event::kCompletion == event) {
    name = "completion";
  } else if (Event::kStart == event) {
    name = "start";
  } else if (Event::kRepeat == event) {
    name = "repeat";
  } else if (Event::kCancel == event) {
    name = "cancel";
  } else if (Event::kReady == event) {
    name = "ready";
  } else if (Event::kUpdate == event) {
    name = "update";
  } else if (Event::kError == event) {
    name = "error";
  } else if (Event::kFps == event) {
    name = "fps";
  } else if (Event::kTapLayer == event) {
    name = "taplayers";
  } else if (Event::kFirstFrame == event) {
    name = "firstframe";
  } else if (Event::kCompositionReady == event) {
    name = "compositionready";
  } else if (Event::kWarning == event) {
    name = "warning";
  }
  return name;
}

UIAnimaX::~UIAnimaX() {
  ANIMAX_LOGI("~UIAnimaX, this:" << this);

  tasm::harmony::NodeManager::Instance().UnregisterNodeEvent(
      Node(), NODE_EVENT_ON_ATTACH);
  tasm::harmony::NodeManager::Instance().UnregisterNodeEvent(
      Node(), NODE_EVENT_ON_DETACH);
  tasm::harmony::NodeManager::Instance().UnregisterNodeEvent(
      Node(), NODE_EVENT_ON_AREA_CHANGE);

  if (native_component_) {
    XComponentAdapter::GetInstance().UnregisterCallback(native_component_);
    native_component_ = nullptr;
  }

  GetPlayer().Destroy();
}

void UIAnimaX::OnSurfaceCreated(OH_NativeXComponent* component, void* window) {
  uint64_t width;
  uint64_t height;
  OH_NativeXComponent_GetXComponentSize(component, window, &width, &height);
  ANIMAX_LOGI("OnSurfaceCreated width:" << width << ", height:" << height);

  GetPlayer().CreateSurface(
      [width, height, anti_aliasing = anti_aliasing_, window]() {
        const auto desc = AnimaXSurfaceHarmony::Description{
            .native_window = static_cast<OHNativeWindow*>(window),
            .width = static_cast<int32_t>(width),
            .height = static_cast<int32_t>(height),
            .backend = AnimaXBackend::kGL,
            .enable_anti_aliasing = anti_aliasing,
        };
        return AnimaXSurfaceHarmony::Make(desc);
      });
}

void UIAnimaX::OnSurfaceChanged(OH_NativeXComponent* component, void* window) {
  uint64_t width;
  uint64_t height;
  OH_NativeXComponent_GetXComponentSize(component, window, &width, &height);
  ANIMAX_LOGI("OnSurfaceChanged width:" << width << ", height:" << height);

  if (width <= 0 || height <= 0) {
    return;
  }

  GetPlayer().UpdateSurface([width, height, anti_aliasing = anti_aliasing_](
                                std::unique_ptr<AnimaXSurface> old_surface) {
    const auto desc = AnimaXSurfaceHarmony::ReconfigureDescription{
        .width = static_cast<int32_t>(width),
        .height = static_cast<int32_t>(height),
        .enable_anti_aliasing = anti_aliasing};
    return AnimaXSurfaceHarmony::Reconfigure(std::move(old_surface), desc);
  });
}

void UIAnimaX::OnSurfaceDestroyed(OH_NativeXComponent* component,
                                  void* window) {
  ANIMAX_LOGI("OnSurfaceDestroyed");
}

void UIAnimaX::OnPropUpdate(const std::string& name,
                            const lepus::Value& value) {
  tasm::harmony::UIBase::OnPropUpdate(name, value);
  if (auto it = prop_setters_.find(name); it != prop_setters_.end()) {
    UIAnimaX::PropSetter setter = it->second;
    (this->*setter)(value);
  }
}

void UIAnimaX::UpdateProps(tasm::PropBundleHarmony* props) {
  for (const auto& [id, value] : props->GetProps()) {
    if (auto it = prepared_prop_setters_.find(id);
        it != prepared_prop_setters_.end()) {
      UIAnimaX::PropSetter setter = it->second;
      (this->*setter)(value);
    }
  }
  if (!player_) {
    // UIOwner will call UpdateProps immediately after UIAnimaX is created.
    // So we can ensure that player_ is created before being used.
    InitializeXPlayer();
  }
  tasm::harmony::UIBase::UpdateProps(props);
  for (const auto& [id, value] : props->GetProps()) {
    if (auto it = delayed_prop_setters_.find(id);
        it != delayed_prop_setters_.end()) {
      UIAnimaX::PropSetter setter = it->second;
      (this->*setter)(value);
    }
  }
}

void UIAnimaX::InvokeMethod(
    const std::string& method, const lepus::Value& args,
    base::MoveOnlyClosure<void, int32_t, const lepus::Value&> callback) {
  if (auto it = ui_method_map_.find(method); it != ui_method_map_.end()) {
    (this->*it->second)(args, std::move(callback));
  } else {
    tasm::harmony::UIBase::InvokeMethod(method, args, std::move(callback));
  }
}

void UIAnimaX::SetEvents(const std::vector<lepus::Value>& events) {
  tasm::harmony::UIBase::SetEvents(events);
  if (events_.empty()) {
    return;
  }

  enable_lynx_tap_event_ =
      std::find(events_.begin(), events_.end(), kTapLayers) != events_.end();
}

bool UIAnimaX::DispatchTouch(const ArkUI_UIInputEvent* event) {
  if (!enable_lynx_tap_event_ || player_ == nullptr || context_ == nullptr) {
    return false;
  }

  if (OH_ArkUI_UIInputEvent_GetAction(event) != UI_TOUCH_EVENT_ACTION_DOWN) {
    return false;
  }

  float view_rect[4] = {0};
  GetBoundingClientRect(view_rect, false);
  float root_view_rect[4] = {0};
  GetContext()->Root()->GetBoundingClientRect(root_view_rect, false);

  auto changed_x = OH_ArkUI_PointerEvent_GetX(event);
  auto changed_y = OH_ArkUI_PointerEvent_GetY(event);

  auto scale = context_->ScaledDensity();
  auto x = (changed_x + root_view_rect[0] - view_rect[0]) * scale;
  auto y = (changed_y + root_view_rect[1] - view_rect[1]) * scale;

  GetPlayer().OnTap(x, y);

  return tasm::harmony::UIBase::DispatchTouch(event);
}

void UIAnimaX::SetAutoPlay(const lepus::Value& value) {
  if (!value.IsBool()) {
    return;
  }
  GetPlayer().SetAutoplay(value.Bool());
}

void UIAnimaX::SetSpeed(const lepus::Value& value) {
  if (!value.IsNumber()) {
    return;
  }
  GetPlayer().SetSpeed(value.Number());
}

void UIAnimaX::SetProgress(const lepus::Value& value) {
  if (!value.IsNumber()) {
    return;
  }
  GetPlayer().SetProgress(value.Number());
}

void UIAnimaX::SetObjectFit(const lepus::Value& value) {
  if (!value.IsString()) {
    return;
  }
  auto& object_fit_str = value.StdString();
  auto object_fit = animax::ObjectFit::kContain;
  if (object_fit_str == "cover") {
    object_fit = animax::ObjectFit::kCover;
  } else if (object_fit_str == "center") {
    object_fit = animax::ObjectFit::kCenter;
  } else if (object_fit_str == "fill") {
    object_fit = animax::ObjectFit::kFill;
  }
  GetPlayer().SetObjectFit(object_fit);
}

void UIAnimaX::SetKeepLastFrame(const lepus::Value& value) {
  if (!value.IsBool()) {
    return;
  }
  GetPlayer().SetKeepLastFrame(value.Bool());
}

void UIAnimaX::SetJson(const lepus::Value& value) {
  if (!value.IsString()) {
    return;
  }
  GetPlayer().SetJson(value.StdString());
}

void UIAnimaX::SetSrc(const lepus::Value& value) {
  if (!value.IsString()) {
    return;
  }
  GetPlayer().SetSrc(value.StdString());
}

void UIAnimaX::SetLoop(const lepus::Value& value) {
  if (!value.IsBool()) {
    return;
  }
  GetPlayer().SetLoop(value.Bool());
}

void UIAnimaX::SetLoopCount(const lepus::Value& value) {
  if (!value.IsNumber()) {
    return;
  }
  GetPlayer().SetLoopCount(static_cast<long>(value.Number()));
}

void UIAnimaX::SetSrcFormat(const lepus::Value& value) {
  if (!value.IsString()) {
    return;
  }
  GetPlayer().SetSrc(value.StdString());
}

void UIAnimaX::SetSrcPolyfill(const lepus::Value& value) {
  if (!value.IsTable()) {
    return;
  }
  auto table = value.Table();
  std::unordered_map<std::string, std::string> map;
  for (const auto& entry : *table) {
    if (entry.second.IsString()) {
      map[entry.first.str()] = entry.second.StdString();
    }
  }
  GetPlayer().SetSrcPolyfill(map);
}

void UIAnimaX::SetStartFrame(const lepus::Value& value) {
  if (!value.IsNumber()) {
    return;
  }
  GetPlayer().SetStartFrame(value.Number());
}

void UIAnimaX::SetEndFrame(const lepus::Value& value) {
  if (!value.IsNumber()) {
    return;
  }
  GetPlayer().SetEndFrame(value.Number());
}

void UIAnimaX::SetReverseMode(const lepus::Value& value) {
  if (!value.IsBool()) {
    return;
  }
  GetPlayer().SetAutoReverse(value.Bool());
}

void UIAnimaX::SetAntiAliasing(const lepus::Value& value) {
  if (!value.IsString()) {
    return;
  }
  anti_aliasing_ = value.String() != "none";
}

void UIAnimaX::SetFpsEventInterval(const lepus::Value& value) {
  if (!value.IsNumber()) {
    return;
  }
  GetPlayer().SetFpsEventInterval(static_cast<long>(value.Number()));
}

void UIAnimaX::SetMaxFrameRate(const lepus::Value& value) {
  if (!value.IsNumber()) {
    return;
  }
  GetPlayer().SetMaxFrameRate(value.Number());
}

void UIAnimaX::SetDynamicResource(const lepus::Value& value) {
  if (!value.IsBool()) {
    return;
  }
  GetPlayer().SetDynamicResource(value.Bool());
}

void UIAnimaX::OnNodeEvent(ArkUI_NodeEvent* event) {
  tasm::harmony::UIBase::OnNodeEvent(event);
  ArkUI_NodeEventType type = OH_ArkUI_NodeEvent_GetEventType(event);
  if (type == NODE_EVENT_ON_ATTACH) {
    ANIMAX_LOGI("On Attach to Window: " << this);
    UpdateVisibility(true, animax::VisibilityState::kAttach);
  } else if (type == NODE_EVENT_ON_DETACH) {
    ANIMAX_LOGI("On Detach from Window: " << this);
    UpdateVisibility(false, animax::VisibilityState::kAttach);
  } else if (type == NODE_EVENT_ON_AREA_CHANGE) {
    auto data = OH_ArkUI_NodeEvent_GetNodeComponentEvent(event)->data;
    float width = data[0].f32;
    float height = data[1].f32;
    float new_width = data[6].f32;
    float new_height = data[7].f32;
    bool is_size_visible = width > 0 && height > 0;
    bool is_new_size_visible = new_width > 0 && new_height > 0;
    if (is_size_visible != is_new_size_visible) {
      ANIMAX_LOGI("On Size Change: " << this << ". width:" << width
                                     << ", height: " << height
                                     << ". new width:" << new_width
                                     << ", new height: " << new_height);
      UpdateVisibility(is_new_size_visible, animax::VisibilityState::kSize);
    }
  }
}

void UIAnimaX::SetOpacity(const lepus::Value& value) {
  double new_opacity;
  if (value.IsNil()) {
    // follow UIBase::SetOpacity
    new_opacity = 1;
  } else {
    new_opacity = value.Number();
  }
  bool is_new_opacity_visible = new_opacity > 0;
  if (is_new_opacity_visible != is_opacity_visible_) {
    UpdateVisibility(is_new_opacity_visible, animax::VisibilityState::kOpacity);
  }
  is_opacity_visible_ = is_new_opacity_visible;
}

void UIAnimaX::SetVisibility(const lepus::Value& value) {
  starlight::VisibilityType new_visibility_state;
  if (value.IsNil()) {
    // follow UIBase::SetVisibility
    new_visibility_state = starlight::VisibilityType::kVisible;
  } else {
    new_visibility_state =
        static_cast<starlight::VisibilityType>(value.Number());
  }
  bool is_new_visibility_visible =
      new_visibility_state == starlight::VisibilityType::kVisible;
  if (is_visibility_visible_ != is_new_visibility_visible) {
    UpdateVisibility(is_new_visibility_visible,
                     animax::VisibilityState::kVisible);
  }
  is_visibility_visible_ = is_new_visibility_visible;
}

void UIAnimaX::SetIgnoreAttachStatus(const lepus::Value& value) {
  if (!value.IsBool()) {
    return;
  }
  ignore_attach_status_ = value.Bool();
}

void UIAnimaX::SetIgnoreLynxLifecycle(const lepus::Value& value) {
  // TODO(aiyongbiao.rick): implement it after foreground and background event
  // was supported.
}

void UIAnimaX::SetVideoFrameTimeout(const lepus::Value& value) {
  if (!value.IsNumber()) {
    return;
  }
  if (ability_) {
    ability_->SetVideoFrameTimeout(static_cast<int32_t>(value.Number()));
  }
}

void UIAnimaX::SetMultiThreadAccelerate(const lepus::Value& value) {
  if (!value.IsBool()) {
    return;
  }
  multi_thread_accelerate_ = value.Bool();
}

void UIAnimaX::SetEnableAudio(const lepus::Value& value) {
  if (!value.IsBool()) {
    return;
  }
  GetPlayer().SetEnableAudio(value.Bool());
}

void UIAnimaX::SetMuted(const lepus::Value& value) {
  if (!value.IsBool()) {
    return;
  }
  GetPlayer().SetMuted(value.Bool());
}

void UIAnimaX::Play(
    const lepus::Value& args,
    base::MoveOnlyClosure<void, int32_t, const lepus::Value&> callback) {
  GetPlayer().Play();
  callback(tasm::LynxGetUIResult::SUCCESS, lepus::Value());
}

void UIAnimaX::Pause(
    const lepus::Value& args,
    base::MoveOnlyClosure<void, int32_t, const lepus::Value&> callback) {
  GetPlayer().Pause();
  callback(tasm::LynxGetUIResult::SUCCESS, lepus::Value());
}

void UIAnimaX::Resume(
    const lepus::Value& args,
    base::MoveOnlyClosure<void, int32_t, const lepus::Value&> callback) {
  GetPlayer().Resume();
  callback(tasm::LynxGetUIResult::SUCCESS, lepus::Value());
}

void UIAnimaX::Stop(
    const lepus::Value& args,
    base::MoveOnlyClosure<void, int32_t, const lepus::Value&> callback) {
  GetPlayer().Stop();
  callback(tasm::LynxGetUIResult::SUCCESS, lepus::Value());
}

void UIAnimaX::GetDuration(
    const lepus::Value& args,
    base::MoveOnlyClosure<void, int32_t, const lepus::Value&> callback) {
  auto duration = GetPlayer().GetDurationMs();
  auto dict = lepus::Dictionary::Create();
  dict->SetValue(kData, duration);
  callback(tasm::LynxGetUIResult::SUCCESS, lepus::Value(std::move(dict)));
}

void UIAnimaX::IsAnimating(
    const lepus::Value& args,
    base::MoveOnlyClosure<void, int32_t, const lepus::Value&> callback) {
  auto animating = GetPlayer().IsAnimating();
  auto dict = lepus::Dictionary::Create();
  dict->SetValue(kData, animating);
  callback(tasm::LynxGetUIResult::SUCCESS, lepus::Value(std::move(dict)));
}

void UIAnimaX::Seek(
    const lepus::Value& args,
    base::MoveOnlyClosure<void, int32_t, const lepus::Value&> callback) {
  const auto params = args.Table();
  auto frame = params->GetValue("frame")->Number();
  GetPlayer().Seek(frame);
  callback(tasm::LynxGetUIResult::SUCCESS, lepus::Value());
}

void UIAnimaX::SubscribeUpdateEvent(
    const lepus::Value& args,
    base::MoveOnlyClosure<void, int32_t, const lepus::Value&> callback) {
  const auto params = args.Table();
  auto frame = params->GetValue("frame")->Number();
  GetPlayer().SubscribeUpdateEvent(frame);
  callback(tasm::LynxGetUIResult::SUCCESS, lepus::Value());
}

void UIAnimaX::UnsubscribeUpdateEvent(
    const lepus::Value& args,
    base::MoveOnlyClosure<void, int32_t, const lepus::Value&> callback) {
  const auto params = args.Table();
  auto frame = params->GetValue("frame")->Number();
  GetPlayer().UnsubscribeUpdateEvent(frame);
  callback(tasm::LynxGetUIResult::SUCCESS, lepus::Value());
}

static void GetFramesFromArg(const lepus::Value& args,
                             std::unordered_set<int>& frames_set) {
  const auto params = args.Table();
  auto frames = params->GetValue("frames")->Array();
  for (size_t i = 0; i < frames->size(); i++) {
    frames_set.insert(frames->get(i).Number());
  }
}

void UIAnimaX::SubscribeUpdateEvents(
    const lepus::Value& args,
    base::MoveOnlyClosure<void, int32_t, const lepus::Value&> callback) {
  std::unordered_set<int> frames_set;
  GetFramesFromArg(args, frames_set);
  GetPlayer().SubscribeUpdateEvents(std::move(frames_set), true);
  callback(tasm::LynxGetUIResult::SUCCESS, lepus::Value());
}

void UIAnimaX::UnsubscribeUpdateEvents(
    const lepus::Value& args,
    base::MoveOnlyClosure<void, int32_t, const lepus::Value&> callback) {
  std::unordered_set<int> frames_set;
  GetFramesFromArg(args, frames_set);
  GetPlayer().SubscribeUpdateEvents(std::move(frames_set), false);
  callback(tasm::LynxGetUIResult::SUCCESS, lepus::Value());
}

void UIAnimaX::GetCurrentFrame(
    const lepus::Value& args,
    base::MoveOnlyClosure<void, int32_t, const lepus::Value&> callback) {
  auto frame = GetPlayer().GetCurrentFrame();
  callback(tasm::LynxGetUIResult::SUCCESS, lepus::Value(frame));
}

void UIAnimaX::PlaySegment(
    const lepus::Value& args,
    base::MoveOnlyClosure<void, int32_t, const lepus::Value&> callback) {
  const auto params = args.Table();
  auto start_frame = params->GetValue("startFrame")->Number();
  auto end_frame = params->GetValue("endFrame")->Number();
  GetPlayer().PlaySegment(start_frame, end_frame);
  callback(tasm::LynxGetUIResult::SUCCESS, lepus::Value());
}

void UIAnimaX::UpdateVisibility(bool isVisible, animax::VisibilityState state) {
  if (ignore_attach_status_ && state == animax::VisibilityState::kAttach) {
    return;
  }
  if (isVisible) {
    GetPlayer().OnShow(state);
  } else {
    GetPlayer().OnHide(state);
  }
}

AnimaXPlayer& UIAnimaX::GetPlayer() {
  ANIMAX_CHECK(player_);
  return *player_;
}

}  // namespace animax
}  // namespace lynx
