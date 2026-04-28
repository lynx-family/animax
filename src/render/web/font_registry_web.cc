// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/render/web/font_registry_web.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <mutex>
#include <optional>
#include <utility>
#include <vector>

#include "base/include/no_destructor.h"
#include "skity/io/data.hpp"
#include "skity/text/typeface.hpp"
#include "src/base/log/log.h"

namespace lynx {
namespace animax {
namespace {

std::string NormalizeFamilyName(const std::string& name) {
  auto begin = std::find_if_not(name.begin(), name.end(), [](unsigned char c) {
    return std::isspace(c);
  });
  auto end = std::find_if_not(name.rbegin(), name.rend(), [](unsigned char c) {
               return std::isspace(c);
             }).base();
  if (begin >= end) {
    return "";
  }

  std::string normalized(begin, end);
  std::transform(
      normalized.begin(), normalized.end(), normalized.begin(),
      [](unsigned char c) {
        return (c & 0x80) ? c : static_cast<unsigned char>(std::tolower(c));
      });
  return normalized;
}

bool IsDefaultFamilyName(const char* family_name) {
  return family_name == nullptr || *family_name == 0;
}

int StyleDistance(const skity::FontStyle& left, const skity::FontStyle& right) {
  return std::abs(left.weight() - right.weight()) * 10 +
         std::abs(left.width() - right.width()) * 100 +
         (left.slant() == right.slant() ? 0 : 1000);
}

struct FontEntryWeb {
  std::string family_name;
  std::shared_ptr<skity::Typeface> typeface;
  int fallback_priority = 0;
  uint64_t order = 0;
};

class FontRegistryWebImpl {
 public:
  bool RegisterFontData(const std::string& family_name, const void* bytes,
                        size_t length, bool is_default, int fallback_priority) {
    auto normalized_family = NormalizeFamilyName(family_name);
    if (normalized_family.empty() || bytes == nullptr || length == 0) {
      ANIMAX_LOGE(
          "FontRegistryWeb::RegisterFontData failed, invalid font "
          "descriptor, family_name: "
          << family_name << ", length: " << length);
      return false;
    }

    auto raw_data = skity::Data::MakeWithCopy(bytes, length);
    auto typeface = skity::Typeface::MakeFromData(raw_data);
    if (!typeface) {
      ANIMAX_LOGE(
          "FontRegistryWeb::RegisterFontData failed, MakeFromData "
          "failed, family_name: "
          << family_name);
      return false;
    }

    FontEntryWeb entry;
    entry.family_name = std::move(normalized_family);
    entry.typeface = typeface;
    entry.fallback_priority = fallback_priority;

    std::lock_guard<std::mutex> lock(mutex_);
    entry.order = next_order_++;
    entries_.push_back(std::move(entry));
    if (is_default) {
      default_typeface_ = typeface;
    }
    ANIMAX_LOGI("RegisterFontData success for " << family_name);
    return true;
  }

  std::shared_ptr<skity::Typeface> MatchFamilyStyle(
      const char* family_name, const skity::FontStyle& style) const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (IsDefaultFamilyName(family_name)) {
      return default_typeface_ ? default_typeface_ : MatchFallbackLocked(style);
    }

    auto normalized = NormalizeFamilyName(family_name);
    return MatchFamilyLocked(normalized, style);
  }

  std::shared_ptr<skity::Typeface> MatchFamilyStyleCharacter(
      const char* family_name, const skity::FontStyle& style,
      uint32_t character) const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!IsDefaultFamilyName(family_name)) {
      auto typeface = MatchFamilyCharacterLocked(
          NormalizeFamilyName(family_name), style, character);
      if (typeface) {
        return typeface;
      }
    }

    return MatchFallbackLocked(style, character);
  }

  std::shared_ptr<skity::Typeface> GetDefaultTypeface(
      const skity::FontStyle& style) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return default_typeface_ ? default_typeface_ : MatchFallbackLocked(style);
  }

 private:
  std::shared_ptr<skity::Typeface> MatchFamilyLocked(
      const std::string& normalized_family,
      const skity::FontStyle& style) const {
    const FontEntryWeb* best_entry = nullptr;
    int best_distance = 0;
    for (const auto& entry : entries_) {
      if (entry.family_name != normalized_family) {
        continue;
      }

      auto distance = StyleDistance(entry.typeface->GetFontStyle(), style);
      if (best_entry == nullptr || distance < best_distance) {
        best_entry = &entry;
        best_distance = distance;
      }
    }
    return best_entry ? best_entry->typeface : nullptr;
  }

  std::shared_ptr<skity::Typeface> MatchFamilyCharacterLocked(
      const std::string& normalized_family, const skity::FontStyle& style,
      uint32_t character) const {
    const FontEntryWeb* best_entry = nullptr;
    int best_distance = 0;
    for (const auto& entry : entries_) {
      if (entry.family_name != normalized_family ||
          entry.typeface->UnicharToGlyph(character) == 0) {
        continue;
      }

      auto distance = StyleDistance(entry.typeface->GetFontStyle(), style);
      if (best_entry == nullptr || distance < best_distance) {
        best_entry = &entry;
        best_distance = distance;
      }
    }
    return best_entry ? best_entry->typeface : nullptr;
  }

  std::shared_ptr<skity::Typeface> MatchFallbackLocked(
      const skity::FontStyle& style,
      std::optional<uint32_t> character = std::nullopt) const {
    std::vector<const FontEntryWeb*> fallbacks;
    fallbacks.reserve(entries_.size());
    for (const auto& entry : entries_) {
      fallbacks.push_back(&entry);
    }

    std::stable_sort(
        fallbacks.begin(), fallbacks.end(),
        [](const FontEntryWeb* left, const FontEntryWeb* right) {
          if (left->fallback_priority != right->fallback_priority) {
            return left->fallback_priority < right->fallback_priority;
          }
          return left->order < right->order;
        });

    size_t index = 0;
    while (index < fallbacks.size()) {
      auto priority = fallbacks[index]->fallback_priority;
      const FontEntryWeb* best_entry = nullptr;
      int best_distance = 0;

      while (index < fallbacks.size() &&
             fallbacks[index]->fallback_priority == priority) {
        const auto* entry = fallbacks[index++];
        if (!entry->typeface) {
          continue;
        }
        if (character && entry->typeface->UnicharToGlyph(*character) == 0) {
          continue;
        }

        auto distance = StyleDistance(entry->typeface->GetFontStyle(), style);
        if (best_entry == nullptr || distance < best_distance) {
          best_entry = entry;
          best_distance = distance;
        }
      }

      if (best_entry != nullptr) {
        return best_entry->typeface;
      }
    }

    ANIMAX_LOGE("FontRegistryWeb::MatchFallbackLocked failed, character: "
                << (character ? static_cast<int64_t>(*character) : -1));
    return nullptr;
  }

  mutable std::mutex mutex_;
  std::vector<FontEntryWeb> entries_;
  std::shared_ptr<skity::Typeface> default_typeface_;
  uint64_t next_order_ = 0;
};

FontRegistryWebImpl& GetRegistryImpl() {
  static base::NoDestructor<FontRegistryWebImpl> registry;
  return *registry;
}

}  // namespace

FontRegistryWeb& FontRegistryWeb::Get() {
  static base::NoDestructor<FontRegistryWeb> registry;
  return *registry;
}

bool FontRegistryWeb::RegisterFontData(const std::string& family_name,
                                       const void* bytes, size_t length,
                                       bool is_default, int fallback_priority) {
  return GetRegistryImpl().RegisterFontData(family_name, bytes, length,
                                            is_default, fallback_priority);
}

std::shared_ptr<skity::Typeface> FontRegistryWeb::MatchFamilyStyle(
    const char* family_name, const skity::FontStyle& style) const {
  return GetRegistryImpl().MatchFamilyStyle(family_name, style);
}

std::shared_ptr<skity::Typeface> FontRegistryWeb::MatchFamilyStyleCharacter(
    const char* family_name, const skity::FontStyle& style,
    uint32_t character) const {
  return GetRegistryImpl().MatchFamilyStyleCharacter(family_name, style,
                                                     character);
}

std::shared_ptr<skity::Typeface> FontRegistryWeb::GetDefaultTypeface(
    const skity::FontStyle& style) const {
  return GetRegistryImpl().GetDefaultTypeface(style);
}

}  // namespace animax
}  // namespace lynx

extern "C" skity::Typeface* AnimaXWebFontMatchFamilyStyle(
    const char* family_name, int weight, int width, int slant) {
  auto typeface = lynx::animax::FontRegistryWeb::Get().MatchFamilyStyle(
      family_name,
      skity::FontStyle(weight, width,
                       static_cast<skity::FontStyle::Slant>(slant)));
  if (!typeface) {
    ANIMAX_LOGE("AnimaXWebFontMatchFamilyStyle failed, family_name: "
                << (family_name == nullptr ? "" : family_name));
    return nullptr;
  }
  return typeface.get();
}

extern "C" skity::Typeface* AnimaXWebFontMatchFamilyStyleCharacter(
    const char* family_name, int weight, int width, int slant,
    uint32_t character) {
  auto typeface =
      lynx::animax::FontRegistryWeb::Get().MatchFamilyStyleCharacter(
          family_name,
          skity::FontStyle(weight, width,
                           static_cast<skity::FontStyle::Slant>(slant)),
          character);
  if (!typeface) {
    ANIMAX_LOGE(
        "AnimaXWebFontMatchFamilyStyleCharacter failed, "
        "family_name: "
        << (family_name == nullptr ? "" : family_name)
        << ", character: " << character);
    return nullptr;
  }
  return typeface.get();
}
