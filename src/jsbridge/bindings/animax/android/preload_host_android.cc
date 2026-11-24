// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include <cmath>
#include <memory>

#include "base/include/platform/android/jni_utils.h"
#include "base/include/platform/android/scoped_java_ref.h"
#include "include/player/animax_player.h"
#include "platform/android/animax_napi/src/main/jni/gen/AnimaXPreloadHost_jni.h"
#include "platform/android/animax_napi/src/main/jni/gen/AnimaXPreloadHost_register_jni.h"
#include "src/jsbridge/bindings/animax/preload_host.h"
#include "src/resource/resource_loader/android/animax_resource_loader_holder_android.h"

using lynx::animax::AnimaXPlayer;
using lynx::animax::PreloadHost;
using lynx::animax::PreloadHostCapabilities;

namespace animax {
namespace jni {
bool RegisterJNIForAnimaXPreloadHost(JNIEnv* env) {
  return RegisterNativesImpl(env);
}
}  // namespace jni
}  // namespace animax

static jlong Create(JNIEnv* env, jobject caller) {
  auto host = std::make_shared<PreloadHost>(
      [weak_java_host = lynx::base::android::ScopedWeakGlobalJavaRef<jobject>(
           env, caller)](PreloadHost::Callback callback) {
        auto java_host =
            lynx::base::android::ScopedLocalJavaRef<jobject>(weak_java_host);
        if (java_host.IsNull()) {
          callback({}, "Preload host is unavailable");
          return;
        }
        auto pending =
            std::make_unique<PreloadHost::Callback>(std::move(callback));
        if (Java_AnimaXPreloadHost_requestCapabilities(
                lynx::base::android::AttachCurrentThread(), java_host.Get(),
                reinterpret_cast<jlong>(pending.get()))) {
          pending.release();
        } else {
          (*pending)({}, "Preload host could not dispatch to the UI thread");
        }
      });
  return reinterpret_cast<jlong>(
      new std::shared_ptr<PreloadHost>(std::move(host)));
}

static void Destroy(JNIEnv*, jclass, jlong pointer) {
  auto host = std::unique_ptr<std::shared_ptr<PreloadHost>>(
      reinterpret_cast<std::shared_ptr<PreloadHost>*>(pointer));
  (*host)->Invalidate();
}

static jboolean Attach(JNIEnv*, jclass, jlong pointer, jlong env) {
  auto& host = *reinterpret_cast<std::shared_ptr<PreloadHost>*>(pointer);
  return env && PreloadHost::AttachToEnv(reinterpret_cast<napi_env>(env), host);
}

static void Detach(JNIEnv*, jclass, jlong env) {
  PreloadHost::DetachFromEnv(reinterpret_cast<napi_env>(env));
}

static jboolean AttachPlayer(JNIEnv* env, jclass, jlong pointer,
                             jobject player) {
  auto player_pointer = Java_AnimaXPreloadHost_getPlayerPointer(env, player);
  if (!player_pointer) return false;
  auto native_player = std::atomic_load(
      reinterpret_cast<std::shared_ptr<AnimaXPlayer>*>(player_pointer));
  auto& host = *reinterpret_cast<std::shared_ptr<PreloadHost>*>(pointer);
  if (!native_player || !host->IsValid()) return false;
  std::weak_ptr<PreloadHost> weak_host = host;
  native_player->SetCompositionHost(
      weak_host, [weak_host](std::function<void()> commit) {
        auto host = weak_host.lock();
        return host && host->RunIfValid(std::move(commit));
      });
  return true;
}

static void CompleteCapabilities(JNIEnv*, jclass, jlong pointer, jlong loader,
                                 jfloat density) {
  auto callback = std::unique_ptr<PreloadHost::Callback>(
      reinterpret_cast<PreloadHost::Callback*>(pointer));
  auto* holder =
      reinterpret_cast<lynx::animax::AnimaXResourceLoaderHolderAndroid*>(
          loader);
  if (!holder || !holder->resource_loader || !holder->unzip_loader ||
      !std::isfinite(density) || density <= 0) {
    (*callback)({},
                "Preload resource services or screen scale are unavailable");
    return;
  }
  (*callback)(PreloadHostCapabilities{holder->resource_loader,
                                      holder->unzip_loader, density},
              {});
}
