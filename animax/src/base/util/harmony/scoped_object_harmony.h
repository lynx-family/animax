// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_BASE_UTIL_HARMONY_SCOPED_OBJECT_HARMONY_H_
#define ANIMAX_SRC_BASE_UTIL_HARMONY_SCOPED_OBJECT_HARMONY_H_

namespace lynx {
namespace animax {
template <typename T, auto DeleterPtr>
class ScopedOHObject {
 public:
  ScopedOHObject() noexcept = default;

  ~ScopedOHObject() noexcept {
    if (object) {
      DeleterPtr(object);
    }
  }

  T** WritablePtr() noexcept { return &object; }

  T* Ptr() const noexcept { return object; }

  void reset(T* new_object = nullptr) noexcept {
    if (object != new_object) {
      if (object) {
        DeleterPtr(object);
      }
      object = new_object;
    }
  }

  T* release() noexcept {
    T* temp = object;
    object = nullptr;
    return temp;
  }

  ScopedOHObject(ScopedOHObject&& other) noexcept : object(other.object) {
    other.object = nullptr;
  }

  ScopedOHObject& operator=(ScopedOHObject&& other) noexcept {
    if (this != &other) {
      reset(other.object);
      other.object = nullptr;
    }
    return *this;
  }

  ScopedOHObject(const ScopedOHObject&) = delete;
  ScopedOHObject& operator=(const ScopedOHObject&) = delete;

 private:
  T* object = nullptr;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_BASE_UTIL_HARMONY_SCOPED_OBJECT_HARMONY_H_
