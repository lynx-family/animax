// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_PLATFORM_HARMONY_ANIMAX_LYNX_SRC_MAIN_CPP_UI_ANIMAX_H_
#define ANIMAX_PLATFORM_HARMONY_ANIMAX_LYNX_SRC_MAIN_CPP_UI_ANIMAX_H_

#include <string>

#include "include/base/macros.h"
#include "include/player/animax_player.h"
#include "include/player/animax_player_builder.h"
#include "include/player/vsync_monitor.h"
#include "platform/harmony/lynx_harmony/src/main/cpp/ui/ui_view.h"
#include "src/base/util/harmony/xcomponent_adapter.h"
#include "src/player/harmony/animax_ability_harmony.h"

namespace lynx {
namespace animax {

extern "C" {
ANIMAX_EXPORT lynx::tasm::harmony::UIBase* UIAnimaXCreateFunc(
    lynx::tasm::harmony::LynxContext* context, int sign,
    const std::string& tag);

ANIMAX_EXPORT void TryRegisterALogFunction();
}

class UIAnimaX : public tasm::harmony::UIBase, public XComponentCallback {
 public:
  using AnimaXPlayer = AnimaXPlayer;
  using Event = Event;

  using PropSetter = void (UIAnimaX::*)(const lepus::Value& value);
  using UIMethod = void (UIAnimaX::*)(
      const lepus::Value& args,
      base::MoveOnlyClosure<void, int32_t, const lepus::Value&> callback);

  UIAnimaX(tasm::harmony::LynxContext* context, int sign,
           const std::string& tag);
  ~UIAnimaX() override;

  void OnPropUpdate(const std::string& name,
                    const lepus::Value& value) override;
  void UpdateProps(tasm::PropBundleHarmony* props) override;

  void InvokeMethod(const std::string& method, const lepus::Value& args,
                    base::MoveOnlyClosure<void, int32_t, const lepus::Value&>
                        callback) override;

  void SetEvents(const std::vector<lepus::Value>& events) override;

  bool DispatchTouch(const ArkUI_UIInputEvent* event) override;

  void OnSurfaceCreated(OH_NativeXComponent* component, void* window) override;
  void OnSurfaceChanged(OH_NativeXComponent* component, void* window) override;
  void OnSurfaceDestroyed(OH_NativeXComponent* component,
                          void* window) override;

  void InitializeXComponentNode();
  void InitializeXComponentCallback();
  void InitializeXPlayer();
  bool HasContent() override { return true; }

 protected:
  void OnNodeEvent(ArkUI_NodeEvent* event) override;

 private:
  static std::unordered_map<std::string, PropSetter> prepared_prop_setters_;
  static std::unordered_map<std::string, PropSetter> prop_setters_;
  static std::unordered_map<std::string, PropSetter> delayed_prop_setters_;
  static std::unordered_map<std::string, UIMethod> ui_method_map_;
  static std::string GetEventName(Event event);

  void AddEventListener(AnimaXPlayerBuilder& builder);
  void UpdateVisibility(bool isVisible, animax::VisibilityState state);

  // props
  void SetAutoPlay(const lepus::Value& value);
  void SetSpeed(const lepus::Value& value);
  void SetProgress(const lepus::Value& value);
  void SetObjectFit(const lepus::Value& value);
  void SetKeepLastFrame(const lepus::Value& value);
  void SetJson(const lepus::Value& value);
  void SetSrc(const lepus::Value& value);
  void SetLoop(const lepus::Value& value);
  void SetLoopCount(const lepus::Value& value);
  void SetSrcFormat(const lepus::Value& value);
  void SetSrcPolyfill(const lepus::Value& value);
  void SetStartFrame(const lepus::Value& value);
  void SetEndFrame(const lepus::Value& value);
  void SetReverseMode(const lepus::Value& value);
  void SetAntiAliasing(const lepus::Value& value);
  void SetFpsEventInterval(const lepus::Value& value);
  void SetMaxFrameRate(const lepus::Value& value);
  void SetDynamicResource(const lepus::Value& value);
  void SetOpacity(const lepus::Value& value);
  void SetVisibility(const lepus::Value& value);
  void SetIgnoreAttachStatus(const lepus::Value& value);
  void SetIgnoreLynxLifecycle(const lepus::Value& value);
  void SetVideoFrameTimeout(const lepus::Value& value);
  void SetMultiThreadAccelerate(const lepus::Value& value);
  void SetEnableAudio(const lepus::Value& value);
  void SetMuted(const lepus::Value& value);

  // ui methods
  void Play(const lepus::Value& args,
            base::MoveOnlyClosure<void, int32_t, const lepus::Value&> callback);
  void Pause(
      const lepus::Value& args,
      base::MoveOnlyClosure<void, int32_t, const lepus::Value&> callback);
  void Resume(
      const lepus::Value& args,
      base::MoveOnlyClosure<void, int32_t, const lepus::Value&> callback);
  void Stop(const lepus::Value& args,
            base::MoveOnlyClosure<void, int32_t, const lepus::Value&> callback);
  void GetDuration(
      const lepus::Value& args,
      base::MoveOnlyClosure<void, int32_t, const lepus::Value&> callback);
  void IsAnimating(
      const lepus::Value& args,
      base::MoveOnlyClosure<void, int32_t, const lepus::Value&> callback);
  void Seek(const lepus::Value& args,
            base::MoveOnlyClosure<void, int32_t, const lepus::Value&> callback);
  void SubscribeUpdateEvent(
      const lepus::Value& args,
      base::MoveOnlyClosure<void, int32_t, const lepus::Value&> callback);
  void UnsubscribeUpdateEvent(
      const lepus::Value& args,
      base::MoveOnlyClosure<void, int32_t, const lepus::Value&> callback);
  void SubscribeUpdateEvents(
      const lepus::Value& args,
      base::MoveOnlyClosure<void, int32_t, const lepus::Value&> callback);
  void UnsubscribeUpdateEvents(
      const lepus::Value& args,
      base::MoveOnlyClosure<void, int32_t, const lepus::Value&> callback);
  void GetCurrentFrame(
      const lepus::Value& args,
      base::MoveOnlyClosure<void, int32_t, const lepus::Value&> callback);
  void PlaySegment(
      const lepus::Value& args,
      base::MoveOnlyClosure<void, int32_t, const lepus::Value&> callback);

  AnimaXPlayer& GetPlayer();

  std::shared_ptr<AnimaXPlayer> player_;
  bool anti_aliasing_ = true;
  bool is_opacity_visible_ = true;
  bool is_visibility_visible_ = true;
  bool enable_lynx_tap_event_ = false;
  bool ignore_attach_status_ = false;
  bool multi_thread_accelerate_ = false;

  std::shared_ptr<lynx::animax::AnimaXAbilityHarmony> ability_;
  OH_NativeXComponent* native_component_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_PLATFORM_HARMONY_ANIMAX_LYNX_SRC_MAIN_CPP_UI_ANIMAX_H_
