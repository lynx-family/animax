// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_INCLUDE_PROPERTY_ANIMAX_PLAYER_GLOBAL_H_
#define ANIMAX_INCLUDE_PROPERTY_ANIMAX_PLAYER_GLOBAL_H_

#include <mutex>
#include <unordered_map>

#include "base/include/no_destructor.h"
#include "include/base/macros.h"
#include "include/player/animax_player.h"

namespace lynx {
namespace animax {

class AnimaXPlayer;

/**
 * This class is designed to work with the NAPI model, facilitating the
 * retrieval of AnimaXPlayer instances using their unique identifier. It
 * supports registering, finding, and unregistering players in a thread-safe
 * manner, with the exception that the `Find` operation must be performed on the
 * JavaScript (JS) thread.
 */
class ANIMAX_EXPORT AnimaXPlayerGlobal {
 public:
  /**
   * Finds an AnimaXPlayer instance by its ID.
   * @param id A std::string representing the unique identifier of the
   * AnimaXPlayer instance to find.
   * @return std::weak_ptr<AnimaXPlayer> A weak pointer to the AnimaXPlayer
   * instance associated with the given ID.
   * @note This function must be called from the JS thread. The returned weak
   * pointer may be expired and should be checked before use. This function is
   * static and can be called without an object instance.
   */
  static std::weak_ptr<AnimaXPlayer> Find(const std::string& id);
  /**
   * Registers an AnimaXPlayer instance with a unique ID.
   * @param id A std::string representing the unique identifier to associate
   * with the given AnimaXPlayer instance.
   * @param player A std::weak_ptr<AnimaXPlayer> pointing to the AnimaXPlayer
   * instance to be registered.
   * @note This function is thread-safe and can be invoked from any thread. It
   * associates the provided ID with the given weak pointer to an AnimaXPlayer
   * instance, allowing it to be found later. This function is static and can be
   * called without an object instance.
   */
  static void Register(const std::string& id,
                       std::weak_ptr<AnimaXPlayer> player);
  /**
   * Unregisters an AnimaXPlayer instance associated with a given ID.
   * @param id A std::string representing the unique identifier of the
   * AnimaXPlayer instance to unregister.
   * @note This function is thread-safe and can be invoked from any thread. It
   * removes the association between the provided ID and its AnimaXPlayer
   * instance, effectively making it unfindable through the `Find` method.
   * "Unfindable" means that std::weak_ptr<AnimaXPlayer> returned by `Find`
   * will be nullptr. This function is static and can be called without an
   * object instance.
   */
  static void Unregister(const std::string& id);

 private:
  static AnimaXPlayerGlobal& Instance();
  static AnimaXPlayerGlobal& ThreadLocalInstance();
  std::unordered_map<std::string, std::weak_ptr<AnimaXPlayer>> player_map_{};
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_INCLUDE_PROPERTY_ANIMAX_PLAYER_GLOBAL_H_
