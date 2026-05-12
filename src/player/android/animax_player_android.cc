// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <android/native_window_jni.h>

#include <memory>

#include "base/include/fml/synchronization/waitable_event.h"
#include "base/include/platform/android/jni_convert_helper.h"
#include "include/player/animax_player.h"
#include "include/player/animax_player_builder.h"
#include "include/property/property_update_request.h"
#include "include/property/property_update_response.h"
#include "platform/android/animax_android/src/main/jni/gen/AnimaXPlayer_jni.h"
#include "platform/android/animax_android/src/main/jni/gen/AnimaXPlayer_register_jni.h"
#include "src/base/log/log.h"
#include "src/base/monitor/animax_metric_names.h"
#include "src/base/monitor/trace_event.h"
#include "src/base/util/android/java_only_map.h"
#include "src/base/util/android/metrics_callback_android.h"
#include "src/player/android/animax_ability_android.h"
#include "src/player/android/animax_context_android.h"
#include "src/player/android/animax_surface_android.h"
#include "src/player/android/animax_surface_drawable_android.h"
#include "src/player/animax_event.h"
#include "src/property/android/animax_key_path_android.h"
#include "src/property/android/animax_key_path_list_callback_android.h"
#include "src/property/android/animax_property_callback_android.h"
#include "src/property/android/animax_value_callback_android.h"
#include "src/property/android/animax_value_param_android.h"
#include "src/property/property_type.h"
#include "src/resource/resource_loader/android/animax_resource_loader_holder_android.h"

namespace animax {
namespace jni {
bool RegisterJNIForAnimaXPlayer(JNIEnv* env) {
  return RegisterNativesImpl(env);
}
}  // namespace jni
}  // namespace animax
namespace {
static void ConvertEventParamMapToJavaOnlyMap(
    const lynx::animax::EventParamMap& params,
    lynx::animax::android::JavaOnlyMap& jni_map) {
  for (const auto& pair : params) {
    const std::string& key = pair.first;
    const lynx::animax::EventParamValue& value = pair.second;
    switch (value.type) {
      case lynx::animax::EventParamValue::Type::kInt32:
        if (value.int_val) {
          jni_map.PushInt(key, *value.int_val);
        }
        break;
      case lynx::animax::EventParamValue::Type::kDouble:
        if (value.double_val) {
          jni_map.PushDouble(key, *value.double_val);
        }
        break;
      case lynx::animax::EventParamValue::Type::kString:
        if (value.string_val) {
          jni_map.PushString(key, *value.string_val);
        }
        break;
      case lynx::animax::EventParamValue::Type::kStringVector:
        if (value.string_vector_val) {
          lynx::animax::android::JavaOnlyArray jni_array;
          for (const auto& str : *value.string_vector_val) {
            jni_array.PushString(str);
          }
          jni_map.PushArray(key, &jni_array);
        }
        break;
      default:
        break;
    }
  }
}
}  // namespace

static jlong Create(JNIEnv* env, jobject jcaller, jlong loader,
                    jobject context) {
  using lynx::animax::AnimaXPlayer;
  using lynx::animax::Event;
  using lynx::animax::EventParamMap;

  lynx::animax::AnimaXContextAndroid animax_context{env, context};

  auto android_ability = std::make_shared<lynx::animax::AnimaXAbilityAndroid>(
      env, animax_context.GetAbility().Get());

  lynx::animax::AnimaXPlayerBuilder builder;
  builder.SetScale(animax_context.GetDeviceDensity())
      .EnableMultiThreadAccelerate(animax_context.MultiThreadAccelerate())
      .DisablePlaybackOnAssetLoadFailure(
          animax_context.DisablePlaybackOnAssetLoadFailure())
      .SetAbility(android_ability);

  auto* loader_holder =
      reinterpret_cast<lynx::animax::AnimaXResourceLoaderHolderAndroid*>(
          loader);
  if (loader_holder) {
    builder.SetResourceLoader(loader_holder->resource_loader)
        .SetUnzipLoader(loader_holder->unzip_loader);
  }

  builder.AddEventListener([android_ability = std::move(android_ability)](
                               AnimaXPlayer* /*player*/, const Event event,
                               const EventParamMap& params) {
    lynx::animax::android::JavaOnlyMap jni_map;
    ConvertEventParamMapToJavaOnlyMap(params, jni_map);
    android_ability->SendEvent(static_cast<uint8_t>(event),
                               jni_map.jni_object());
  });

  auto player_ptr = new std::shared_ptr<AnimaXPlayer>(builder.Build());
  return reinterpret_cast<jlong>(player_ptr);
}

namespace {

std::shared_ptr<lynx::animax::AnimaXPlayer> GetAnimaXPlayerSharedPtr(
    jlong player) {
  auto* player_ptr_ptr =
      reinterpret_cast<std::shared_ptr<lynx::animax::AnimaXPlayer>*>(player);
  if (!player_ptr_ptr) {
    return nullptr;
  }
  return std::atomic_load(player_ptr_ptr);
}

std::unique_ptr<lynx::animax::AnimaXSurface> ResizeAnimaXSurface(
    std::unique_ptr<lynx::animax::AnimaXSurface> old_surface,
    lynx::animax::AnimaXSurfaceDrawableAndroid surface_drawable) {
  const auto desc =
      lynx::animax::AnimaXSurfaceAndroid::SurfaceDrawableDescription{
          .width = surface_drawable.GetWidth(),
          .height = surface_drawable.GetHeight(),
          .enable_anti_aliasing =
              static_cast<bool>(surface_drawable.IsAntiAliasingEnabled()),
      };
  auto new_surface = lynx::animax::AnimaXSurfaceAndroid::Reconfigure(
      std::move(old_surface), desc);
  return new_surface;
}

std::optional<lynx::animax::AnimaXSurfaceDrawableAndroid> ObtainSurfaceDrawable(
    lynx::base::android::ScopedWeakGlobalJavaRef<jobject>
        weak_java_surface_drawable) {
  auto j_surface_drawable = lynx::base::android::ScopedLocalJavaRef<jobject>(
      weak_java_surface_drawable);
  if (j_surface_drawable.IsNull()) {
    ANIMAX_LOGE("ObtainSurfaceDrawable failed, j_surface_drawable is null.");
    return std::nullopt;
  }

  JNIEnv* env = lynx::base::android::AttachCurrentThread();
  lynx::animax::AnimaXSurfaceDrawableAndroid surface_drawable{
      env, j_surface_drawable.Get()};

  return std::move(surface_drawable);
}
}  // namespace

static void CreateAnimaXSurface(JNIEnv* env, jobject jcaller, jlong player,
                                jobject java_surface_drawable) {
  ANIMAX_LOGI("CreateAnimaXSurface");
  auto player_ptr = GetAnimaXPlayerSharedPtr(player);
  if (!player_ptr) {
    return;
  }

  player_ptr->CreateSurface(
      [weak_java_surface_drawable =
           lynx::base::android::ScopedWeakGlobalJavaRef<jobject>{
               env, java_surface_drawable},
       weak_player = player_ptr->weak_from_this()]()
          -> std::unique_ptr<lynx::animax::AnimaXSurface> {
        auto player = weak_player.lock();
        if (!player) {
          return nullptr;
        }
        auto surface_drawable_opt =
            ObtainSurfaceDrawable(weak_java_surface_drawable);
        if (!surface_drawable_opt.has_value()) {
          return nullptr;
        }
        auto surface_drawable = std::move(surface_drawable_opt.value());
        player->MarkPlatformSurfaceAsInvalid(
            surface_drawable.IsPlatformSurfaceInitiallyInvalid());
        return lynx::animax::AnimaXSurfaceAndroid::Make(
            std::move(surface_drawable));
      });
}

static void UpdateAnimaXSurface(JNIEnv* env, jobject jcaller, jlong player,
                                jobject java_surface_drawable) {
  ANIMAX_LOGI("UpdateAnimaXSurface");
  auto player_ptr = GetAnimaXPlayerSharedPtr(player);
  if (!player_ptr) {
    return;
  }

  player_ptr->UpdateSurface(
      [weak_java_surface_drawable =
           lynx::base::android::ScopedWeakGlobalJavaRef<jobject>{
               env, java_surface_drawable},
       weak_player = player_ptr->weak_from_this()](
          std::unique_ptr<lynx::animax::AnimaXSurface> old_surface)
          -> std::unique_ptr<lynx::animax::AnimaXSurface> {
        auto player = weak_player.lock();
        if (!player) {
          return nullptr;
        }
        auto surface_drawable_opt =
            ObtainSurfaceDrawable(weak_java_surface_drawable);
        if (!surface_drawable_opt.has_value()) {
          return nullptr;
        }
        auto surface_drawable = std::move(surface_drawable_opt.value());
        player->MarkPlatformSurfaceAsInvalid(false);
        return ResizeAnimaXSurface(std::move(old_surface),
                                   std::move(surface_drawable));
      });
}

static void SetAutoPlay(JNIEnv* env, jobject jcaller, jlong player,
                        jboolean enable) {
  auto player_ptr = GetAnimaXPlayerSharedPtr(player);
  if (!player_ptr) {
    return;
  }
  player_ptr->SetAutoplay(enable);
}

static void SetSpeed(JNIEnv* env, jobject jcaller, jlong player, jfloat speed) {
  auto player_ptr = GetAnimaXPlayerSharedPtr(player);
  if (!player_ptr) {
    return;
  }
  player_ptr->SetSpeed(speed);
}

static void SetProgress(JNIEnv* env, jobject jcaller, jlong player,
                        jfloat progress) {
  auto player_ptr = GetAnimaXPlayerSharedPtr(player);
  if (!player_ptr) {
    return;
  }
  player_ptr->SetProgress(progress);
}

static void SetObjectFit(JNIEnv* env, jobject jcaller, jlong player,
                         jint objectFit) {
  auto player_ptr = GetAnimaXPlayerSharedPtr(player);
  if (!player_ptr) {
    return;
  }
  player_ptr->SetObjectFit(static_cast<lynx::animax::ObjectFit>(objectFit));
}

static void SetObjectPosition(JNIEnv* env, jobject jcaller, jlong player,
                              jint objectPosition) {
  auto player_ptr = GetAnimaXPlayerSharedPtr(player);
  if (!player_ptr) {
    return;
  }
  player_ptr->SetObjectPosition(
      static_cast<lynx::animax::ObjectPosition>(objectPosition));
}

static void SetKeepLastFrame(JNIEnv* env, jobject jcaller, jlong player,
                             jboolean enable) {
  auto player_ptr = GetAnimaXPlayerSharedPtr(player);
  if (!player_ptr) {
    return;
  }
  player_ptr->SetKeepLastFrame(enable);
}

static void SetJson(JNIEnv* env, jobject jcaller, jlong player, jstring json) {
  auto player_ptr = GetAnimaXPlayerSharedPtr(player);
  if (!player_ptr || !json) {
    return;
  }
  const char* json_str = env->GetStringUTFChars(json, JNI_FALSE);
  player_ptr->SetJson(json_str);
  env->ReleaseStringUTFChars(json, json_str);
}

static void SetLoop(JNIEnv* env, jobject jcaller, jlong player,
                    jboolean enable) {
  auto player_ptr = GetAnimaXPlayerSharedPtr(player);
  if (!player_ptr) {
    return;
  }
  player_ptr->SetLoop(enable);
}

static void SetLoopCount(JNIEnv* env, jobject jcaller, jlong player,
                         jint loopCount) {
  auto player_ptr = GetAnimaXPlayerSharedPtr(player);
  if (!player_ptr) {
    return;
  }
  player_ptr->SetLoopCount(loopCount);
}

static void SetSrc(JNIEnv* env, jobject jcaller, jlong player, jstring src) {
  auto player_ptr = GetAnimaXPlayerSharedPtr(player);
  if (!player_ptr) {
    return;
  }
  std::string source =
      lynx::base::android::JNIConvertHelper::ConvertToString(env, src);
  player_ptr->SetSrc(source);
}

static void SetComposition(JNIEnv* env, jobject jcaller, jlong player,
                           jlong composition) {
  auto player_ptr = GetAnimaXPlayerSharedPtr(player);
  if (!player_ptr || !composition) {
    return;
  }
  auto* ptr_holder =
      reinterpret_cast<std::shared_ptr<lynx::animax::CompositionModel>*>(
          composition);
  player_ptr->SetComposition(*ptr_holder);
}

static void SetImageFolder(JNIEnv* env, jobject jcaller, jlong player,
                           jstring image_folder) {
  auto player_ptr = GetAnimaXPlayerSharedPtr(player);
  if (!player_ptr) {
    return;
  }
  std::string folder =
      lynx::base::android::JNIConvertHelper::ConvertToString(env, image_folder);
  player_ptr->SetImageFolder(std::move(folder));
}

static void SetSrcPolyfill(JNIEnv* env, jobject jcaller, jlong player,
                           jobject polyfillMap) {
  auto player_ptr = GetAnimaXPlayerSharedPtr(player);
  if (!player_ptr) {
    return;
  }

  std::unordered_map<std::string, std::string> polyfill;
  auto keys =
      lynx::animax::android::JavaOnlyMap::JavaOnlyMapGetKeys(env, polyfillMap);
  jclass cls_arraylist = env->GetObjectClass(keys.Get());
  lynx::base::android::ScopedGlobalJavaRef<jclass> cls_arraylist_global(
      env, cls_arraylist);
  env->DeleteLocalRef(cls_arraylist);

  jmethodID arraylist_get = env->GetMethodID(cls_arraylist_global.Get(), "get",
                                             "(I)Ljava/lang/Object;");
  jmethodID arraylist_size =
      env->GetMethodID(cls_arraylist_global.Get(), "size", "()I");
  jint module_len = env->CallIntMethod(keys.Get(), arraylist_size);

  for (int i = 0; i < module_len; i++) {
    auto key_str = static_cast<jstring>(
        env->CallObjectMethod(keys.Get(), arraylist_get, i));
    lynx::base::android::ScopedGlobalJavaRef<jstring> key_jstring(env, key_str);
    std::string key = lynx::base::android::JNIConvertHelper::ConvertToString(
        env, key_jstring.Get());

    lynx::animax::android::ReadableType type =
        lynx::animax::android::JavaOnlyMap::JavaOnlyMapGetTypeAtIndex(
            env, polyfillMap, key_jstring.Get());
    if (type == lynx::animax::android::ReadableType::String) {
      polyfill[key] =
          lynx::animax::android::JavaOnlyMap::JavaOnlyMapGetStringAtIndex(
              env, polyfillMap, key_jstring.Get());
    }
  }

  player_ptr->SetSrcPolyfill(polyfill);
}

static void SetAutoReverse(JNIEnv* env, jobject jcaller, jlong player,
                           jboolean isAutoReverse) {
  auto player_ptr = GetAnimaXPlayerSharedPtr(player);
  if (!player_ptr) {
    return;
  }
  player_ptr->SetAutoReverse(isAutoReverse);
}

static void SetDynamicResource(JNIEnv* env, jobject jcaller, jlong player,
                               jboolean dynamic) {
  auto player_ptr = GetAnimaXPlayerSharedPtr(player);
  if (!player_ptr) {
    return;
  }
  player_ptr->SetDynamicResource(dynamic);
}

static void SetMuted(JNIEnv* env, jobject jcaller, jlong player,
                     jboolean mute) {
  auto player_ptr = GetAnimaXPlayerSharedPtr(player);
  if (!player_ptr) {
    return;
  }
  player_ptr->SetMuted(mute);
}

static void SetEnableAudio(JNIEnv* env, jobject jcaller, jlong player,
                           jboolean enable) {
  auto player_ptr = GetAnimaXPlayerSharedPtr(player);
  if (!player_ptr) {
    return;
  }
  player_ptr->SetEnableAudio(enable);
}

static void SetStartFrame(JNIEnv* env, jobject jcaller, jlong player,
                          jint startFrame) {
  auto player_ptr = GetAnimaXPlayerSharedPtr(player);
  if (!player_ptr) {
    return;
  }
  player_ptr->SetStartFrame(startFrame);
}

static void SetEndFrame(JNIEnv* env, jobject jcaller, jlong player,
                        jint endFrame) {
  auto player_ptr = GetAnimaXPlayerSharedPtr(player);
  if (!player_ptr) {
    return;
  }
  player_ptr->SetEndFrame(endFrame);
}

static void Play(JNIEnv* env, jobject jcaller, jlong player) {
  auto player_ptr = GetAnimaXPlayerSharedPtr(player);
  if (!player_ptr) {
    return;
  }
  player_ptr->Play();
}

static void Pause(JNIEnv* env, jobject jcaller, jlong player) {
  auto player_ptr = GetAnimaXPlayerSharedPtr(player);
  if (!player_ptr) {
    return;
  }
  player_ptr->Pause();
}

static void Resume(JNIEnv* env, jobject jcaller, jlong player) {
  auto player_ptr = GetAnimaXPlayerSharedPtr(player);
  if (!player_ptr) {
    return;
  }
  player_ptr->Resume();
}

static void Stop(JNIEnv* env, jobject jcaller, jlong player) {
  auto player_ptr = GetAnimaXPlayerSharedPtr(player);
  if (!player_ptr) {
    return;
  }
  player_ptr->Stop();
}

static jdouble GetDurationMs(JNIEnv* env, jobject jcaller, jlong player) {
  auto player_ptr = GetAnimaXPlayerSharedPtr(player);
  if (!player_ptr) {
    return 0;
  }
  return player_ptr->GetDurationMs();
}

static jboolean IsAnimating(JNIEnv* env, jobject jcaller, jlong player) {
  auto player_ptr = GetAnimaXPlayerSharedPtr(player);
  if (!player_ptr) {
    return false;
  }
  return player_ptr->IsAnimating();
}

static void Seek(JNIEnv* env, jobject jcaller, jlong player, jint frame) {
  auto player_ptr = GetAnimaXPlayerSharedPtr(player);
  if (!player_ptr) {
    return;
  }
  player_ptr->Seek(frame);
}

static void SubscribeUpdateEvent(JNIEnv* env, jobject jcaller, jlong player,
                                 jint frame) {
  auto player_ptr = GetAnimaXPlayerSharedPtr(player);
  if (!player_ptr) {
    return;
  }
  player_ptr->SubscribeUpdateEvent(frame);
}

static void UnsubscribeUpdateEvent(JNIEnv* env, jobject jcaller, jlong player,
                                   jint frame) {
  auto player_ptr = GetAnimaXPlayerSharedPtr(player);
  if (!player_ptr) {
    return;
  }
  player_ptr->UnsubscribeUpdateEvent(frame);
}

static void SubscribeUpdateEvents(JNIEnv* env, jobject jcaller, jlong player,
                                  jintArray frames, jboolean subscribe) {
  jsize frame_array_len = env->GetArrayLength(frames);
  jint* frame_players = env->GetIntArrayElements(frames, nullptr);
  std::unordered_set<int> frame_set(frame_players,
                                    frame_players + frame_array_len);
  env->ReleaseIntArrayElements(frames, frame_players, JNI_ABORT);

  auto player_ptr = GetAnimaXPlayerSharedPtr(player);
  if (!player_ptr) {
    return;
  }
  player_ptr->SubscribeUpdateEvents(std::move(frame_set), subscribe);
}

static jdouble GetCurrentFrame(JNIEnv* env, jobject jcaller, jlong player) {
  auto player_ptr = GetAnimaXPlayerSharedPtr(player);
  if (!player_ptr) {
    return 0;
  }
  return player_ptr->GetCurrentFrame();
}

static void PlaySegment(JNIEnv* env, jobject jcaller, jlong player,
                        jint start_frame, jint end_frame) {
  auto player_ptr = GetAnimaXPlayerSharedPtr(player);
  if (!player_ptr) {
    return;
  }
  player_ptr->PlaySegment(start_frame, end_frame);
}

static void Destroy(JNIEnv* env, jclass jcaller, jlong player) {
  auto* player_ptr =
      reinterpret_cast<std::shared_ptr<lynx::animax::AnimaXPlayer>*>(player);
  if (!player_ptr) {
    return;
  }

  auto player_shared_ptr = std::atomic_exchange(
      player_ptr, std::shared_ptr<lynx::animax::AnimaXPlayer>{nullptr});
  if (player_shared_ptr) {
    player_shared_ptr->Destroy();
  }
}

static void DeletePtr(JNIEnv* env, jclass jcaller, jlong player) {
  auto* player_ptr =
      reinterpret_cast<std::shared_ptr<lynx::animax::AnimaXPlayer>*>(player);
  delete player_ptr;
}

static void Reload(JNIEnv* env, jobject jcaller, jlong player) {
  auto player_ptr = GetAnimaXPlayerSharedPtr(player);
  if (!player_ptr) {
    return;
  }
  player_ptr->Reload();
}

lynx::animax::android::JavaOnlyMap GetEventTrackingMapInternal(JNIEnv* env,
                                                               jlong player) {
  auto player_ptr = GetAnimaXPlayerSharedPtr(player);
  lynx::animax::android::JavaOnlyMap jni_map;
  if (!player_ptr) {
    return jni_map;
  }
  std::array<bool, lynx::animax::PlayerEventTracker::kNumEventTypes> events =
      player_ptr->GetEventTrackingArray();
  const std::array<std::string,
                   lynx::animax::PlayerEventTracker::kNumEventTypes>&
      event_names = player_ptr->GetEventNames();

  for (size_t i = 0; i < lynx::animax::PlayerEventTracker::kNumEventTypes;
       i++) {
    jni_map.PushInt(event_names[i], events[i]);
  }
  return jni_map;
}

static jobject GetEventTrackingMap(JNIEnv* env, jobject jcaller, jlong player) {
  return env->NewLocalRef(  // NOLINT
      GetEventTrackingMapInternal(env, player).jni_object());
}

static void SetFpsEventInterval(JNIEnv* env, jobject jcaller, jlong player,
                                jlong interval) {
  auto player_ptr = GetAnimaXPlayerSharedPtr(player);
  if (!player_ptr) {
    return;
  }
  player_ptr->SetFpsEventInterval(interval);
}

static void SetMaxFrameRate(JNIEnv* env, jobject jcaller, jlong player,
                            jdouble maxFrameRate) {
  auto player_ptr = GetAnimaXPlayerSharedPtr(player);
  if (!player_ptr) {
    return;
  }
  player_ptr->SetMaxFrameRate(maxFrameRate);
}

static void OnShow(JNIEnv* env, jobject jcaller, jlong player, jlong state) {
  auto player_ptr = GetAnimaXPlayerSharedPtr(player);
  if (!player_ptr) {
    return;
  }
  player_ptr->OnShow(static_cast<lynx::animax::VisibilityState>(state));
}

static void OnHide(JNIEnv* env, jobject jcaller, jlong player, jlong state) {
  auto player_ptr = GetAnimaXPlayerSharedPtr(player);
  if (!player_ptr) {
    return;
  }
  player_ptr->OnHide(static_cast<lynx::animax::VisibilityState>(state));
}

static void Cancel(JNIEnv* env, jobject jcaller, jlong player) {
  auto player_ptr = GetAnimaXPlayerSharedPtr(player);
  if (!player_ptr) {
    return;
  }
  player_ptr->Stop();
}

static void OnTap(JNIEnv* env, jobject jcaller, jlong player, jfloat x,
                  jfloat y) {
  auto player_ptr = GetAnimaXPlayerSharedPtr(player);
  if (!player_ptr) {
    return;
  }
  player_ptr->OnTap(x, y);
}

static void GetMetricsAsync(JNIEnv* env, jobject jcaller, jlong player,
                            jobject callback) {
  auto player_ptr = GetAnimaXPlayerSharedPtr(player);
  if (!player_ptr) {
    return;
  }
  lynx::base::android::ScopedGlobalJavaRef<jobject> global_ref_to_callback{
      env, callback};
  lynx::base::android::ScopedGlobalJavaRef<jobject>
      global_ref_to_java_events_map{
          env, GetEventTrackingMapInternal(env, player).jni_object()};

  std::shared_ptr<lynx::animax::MetricsCallbackAndroid> shared_java_callback =
      std::make_shared<lynx::animax::MetricsCallbackAndroid>(
          std::move(global_ref_to_callback),
          std::move(global_ref_to_java_events_map));

  auto java_callback_wrapper =
      [player_ptr, shared_java_callback = std::move(shared_java_callback)](
          const lynx::animax::MetricsMap& metrics_map) mutable {
        lynx::animax::android::JavaOnlyMap java_metrics_map;
        for (const auto& [key, value] : metrics_map) {
          // fps is a float type, and all others are int types
          if (key == lynx::animax::AnimaXMetricNames::kFps) {
            java_metrics_map.PushDouble(key, value);
            continue;
          }
          java_metrics_map.PushInt(key, value);
        }

        shared_java_callback->OnMetricsReady(java_metrics_map.jni_object());
      };

  player_ptr->ExportDataFromMetricsManager(std::move(java_callback_wrapper));
}

static void UpdateLayerProperty(JNIEnv* env, jobject jcaller, jlong player,
                                jint type, jobject key_path, jobject param,
                                jobject callback) {
  DCHECK(key_path && param);
  auto player_ptr = GetAnimaXPlayerSharedPtr(player);
  if (!player_ptr) {
    return;
  }

  auto layer_type = static_cast<lynx::animax::LayerPropertyType>(type);
  auto key_path_android =
      std::make_unique<lynx::animax::AnimaXKeyPathAndroid>(env, key_path);
  auto value =
      std::make_unique<lynx::animax::AnimaXValueParamAndroid>(env, param);
  auto animax_callback =
      callback ? std::make_unique<lynx::animax::AnimaXPropertyCallbackAndroid>(
                     env, callback)
               : nullptr;

  player_ptr->UpdateLayerProperty(lynx::animax::LayerStaticRequest::Make(
      layer_type, std::move(key_path_android), std::move(value),
      [callback = std::move(animax_callback)](
          const lynx::animax::PropertyUpdateResponse& response) {
        if (callback) {
          callback->OnPropertyCallback(response);
        }
      }));
}

static void AddLayerPropertyCallback(JNIEnv* env, jobject jcaller, jlong player,
                                     jint type, jobject key_path,
                                     jobject value_callback, jobject callback) {
  DCHECK(key_path && value_callback);
  auto player_ptr = GetAnimaXPlayerSharedPtr(player);
  if (!player_ptr) {
    return;
  }

  auto layer_type = static_cast<lynx::animax::LayerPropertyType>(type);
  auto key_path_android =
      std::make_unique<lynx::animax::AnimaXKeyPathAndroid>(env, key_path);
  auto typed_callback = lynx::animax::AnimaXValueCallbackAndroid::Create(
      env, value_callback, layer_type);
  auto animax_callback =
      callback ? std::make_unique<lynx::animax::AnimaXPropertyCallbackAndroid>(
                     env, callback)
               : nullptr;

  player_ptr->AddLayerPropertyCallback(lynx::animax::LayerCallbackRequest::Make(
      layer_type, std::move(key_path_android), std::move(typed_callback),
      [callback = std::move(animax_callback)](
          const lynx::animax::PropertyUpdateResponse& response) {
        if (callback) {
          callback->OnPropertyCallback(response);
        }
      }));
}

static void SetResourceProperty(JNIEnv* env, jobject jcaller, jlong player,
                                jint type, jstring resourceId, jobject param,
                                jobject callback) {
  DCHECK(param);
  auto player_ptr = GetAnimaXPlayerSharedPtr(player);
  if (!player_ptr) {
    return;
  }

  auto resource_type = static_cast<lynx::animax::ResourcePropertyType>(type);
  auto resource_id =
      lynx::base::android::JNIConvertHelper::ConvertToString(env, resourceId);
  auto value =
      std::make_unique<lynx::animax::AnimaXValueParamAndroid>(env, param);
  auto animax_callback =
      callback ? std::make_unique<lynx::animax::AnimaXPropertyCallbackAndroid>(
                     env, callback)
               : nullptr;

  player_ptr->SetResourceProperty(lynx::animax::ResourceUpdateRequest::Make(
      resource_type, std::move(resource_id), std::move(value),
      [callback = std::move(animax_callback)](
          const lynx::animax::PropertyUpdateResponse& response) {
        if (callback) {
          callback->OnPropertyCallback(response);
        }
      }));
}

static void GetKeysForKeyPath(JNIEnv* env, jobject jcaller, jlong player,
                              jobject key_path, jobject callback) {
  DCHECK(key_path && callback);
  auto player_ptr = GetAnimaXPlayerSharedPtr(player);
  if (!player_ptr) {
    return;
  }

  auto key_path_android =
      std::make_unique<lynx::animax::AnimaXKeyPathAndroid>(env, key_path);
  auto animax_callback =
      std::make_unique<lynx::animax::AnimaXKeyPathListCallbackAndroid>(
          env, callback);

  player_ptr->GetKeysForKeyPath(
      std::move(key_path_android),
      [callback = std::move(animax_callback)](
          const std::vector<lynx::animax::AnimaXKeyPath>& key_paths) {
        callback->OnKeyPathCallback(key_paths);
      });
}

static jlong GetMemoryUsageBytes(JNIEnv* env, jobject jcaller, jlong player) {
  auto player_ptr = GetAnimaXPlayerSharedPtr(player);
  if (!player_ptr) {
    return 0;
  }

  return static_cast<jlong>(player_ptr->GetEstimatedMemoryUsage());
}
