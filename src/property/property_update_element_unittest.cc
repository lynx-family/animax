// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/property/property_update_element.h"

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "gtest/gtest.h"
#include "include/property/animax_key_path.h"
#include "include/property/animax_value_param.h"
#include "include/property/property_update_request.h"
#include "src/animation/base_keyframe_animation.h"
#include "src/animation/interpolator/interpolator.h"
#include "src/animation/text_keyframe_animation.h"
#include "src/model/composition_model.h"
#include "src/model/keyframe/keyframe_model.h"
#include "src/model/rect_model.h"
#include "src/model/value/base_value.h"
#include "src/model/value/document_data.h"
#include "src/property/animax_value_callback.h"
#include "src/property/property_type.h"
#include "src/property/property_update_context.h"

using namespace lynx::animax;

namespace {

constexpr int32_t kDocumentColor0 = static_cast<int32_t>(0xFF101010);
constexpr int32_t kDocumentColor1 = static_cast<int32_t>(0xFF202020);
constexpr int32_t kDocumentColor2 = static_cast<int32_t>(0xFF303030);
constexpr int32_t kDocumentColor3 = static_cast<int32_t>(0xFF404040);
constexpr int32_t kAnimatorColor0 = static_cast<int32_t>(0xFFAAAAAA);
constexpr int32_t kAnimatorColor1 = static_cast<int32_t>(0xFFBBBBBB);
constexpr int32_t kAnimatorColor2 = static_cast<int32_t>(0xFFCCCCCC);
constexpr int32_t kAnimatorColor3 = static_cast<int32_t>(0xFFDDDDDD);
constexpr int32_t kUpdatedColor = static_cast<int32_t>(0xFFACBEEF);
constexpr int32_t kUpdatedStrokeColor = static_cast<int32_t>(0xFF123456);
constexpr float kUpdatedSize = 42.5f;
constexpr int32_t kUpdatedTracking = 21;
constexpr float kUpdatedStrokeWidth = 8.5f;

std::unique_ptr<DocumentData> MakeDocumentData(std::string text, int32_t color,
                                               float size, int32_t tracking,
                                               int32_t stroke_color,
                                               float stroke_width) {
  return std::make_unique<DocumentData>(
      std::move(text), "", size, DocumentJustification::kLeftAlign, tracking, 0,
      0, color, stroke_color, stroke_width, false, nullptr, nullptr);
}

std::unique_ptr<KeyframeModel> MakeDocumentKeyframe(std::string text,
                                                    int32_t color, float size,
                                                    int32_t tracking,
                                                    int32_t stroke_color,
                                                    float stroke_width) {
  auto keyframe = std::make_unique<KeyframeModel>(MakeDocumentData(
      std::move(text), color, size, tracking, stroke_color, stroke_width));
  keyframe->SetEndValue(keyframe->CopyStartValue());
  return keyframe;
}

std::unique_ptr<KeyframeModel> MakeColorKeyframe(int32_t color) {
  auto keyframe =
      std::make_unique<KeyframeModel>(ValueFactory::Make<Color>(color));
  keyframe->SetEndValue(keyframe->CopyStartValue());
  return keyframe;
}

std::shared_ptr<KeyframeModelList> MakeDocumentFrames() {
  auto keyframes = std::make_shared<KeyframeModelList>();
  keyframes->push_back(MakeDocumentKeyframe("text-0", kDocumentColor0, 10.f, 1,
                                            kDocumentColor1, 1.f));
  keyframes->push_back(MakeDocumentKeyframe("text-1", kDocumentColor1, 20.f, 2,
                                            kDocumentColor2, 2.f));
  keyframes->push_back(MakeDocumentKeyframe("text-2", kDocumentColor2, 30.f, 3,
                                            kDocumentColor3, 3.f));
  keyframes->push_back(MakeDocumentKeyframe("text-3", kDocumentColor3, 40.f, 4,
                                            kDocumentColor0, 4.f));
  return keyframes;
}

std::shared_ptr<KeyframeModelList> MakeDefaultColorDocumentFrames() {
  auto keyframes = std::make_shared<KeyframeModelList>();
  keyframes->push_back(MakeDocumentKeyframe("text-0", 0, 0.f, 0, 0, 0.f));
  keyframes->push_back(MakeDocumentKeyframe("text-1", 0, 0.f, 0, 0, 0.f));
  keyframes->push_back(MakeDocumentKeyframe("text-2", 0, 0.f, 0, 0, 0.f));
  keyframes->push_back(MakeDocumentKeyframe("text-3", 0, 0.f, 0, 0, 0.f));
  return keyframes;
}

std::shared_ptr<KeyframeModelList> MakeAnimatorColorFrames() {
  auto keyframes = std::make_shared<KeyframeModelList>();
  keyframes->push_back(MakeColorKeyframe(kAnimatorColor0));
  keyframes->push_back(MakeColorKeyframe(kAnimatorColor1));
  keyframes->push_back(MakeColorKeyframe(kAnimatorColor2));
  keyframes->push_back(MakeColorKeyframe(kAnimatorColor3));
  return keyframes;
}

std::shared_ptr<KeyframeModelList> MakeAnimatedDocumentFrames(
    CompositionModel& composition) {
  auto keyframes = std::make_shared<KeyframeModelList>();
  keyframes->push_back(std::make_unique<KeyframeModel>(
      composition,
      MakeDocumentData("start", kDocumentColor0, 10.f, 1, kDocumentColor1, 1.f),
      MakeDocumentData("end", kDocumentColor3, 40.f, 4, kDocumentColor0, 4.f),
      LinearInterpolator::Make(), 0.f, 10.f));
  return keyframes;
}

std::shared_ptr<KeyframeModelList> MakeSegmentedDocumentFrames(
    CompositionModel& composition) {
  auto keyframes = std::make_shared<KeyframeModelList>();
  keyframes->push_back(std::make_unique<KeyframeModel>(
      composition,
      MakeDocumentData("text-0", kDocumentColor0, 10.f, 1, kDocumentColor1,
                       1.f),
      MakeDocumentData("text-1", kDocumentColor1, 20.f, 2, kDocumentColor2,
                       2.f),
      LinearInterpolator::Make(), 0.f, 10.f));
  keyframes->push_back(std::make_unique<KeyframeModel>(
      composition,
      MakeDocumentData("text-1", kDocumentColor1, 20.f, 2, kDocumentColor2,
                       2.f),
      MakeDocumentData("text-2", kDocumentColor2, 30.f, 3, kDocumentColor3,
                       3.f),
      LinearInterpolator::Make(), 10.f, 18.f));
  keyframes->push_back(std::make_unique<KeyframeModel>(
      composition,
      MakeDocumentData("text-2", kDocumentColor2, 30.f, 3, kDocumentColor3,
                       3.f),
      MakeDocumentData("text-3", kDocumentColor3, 40.f, 4, kDocumentColor0,
                       4.f),
      LinearInterpolator::Make(), 18.f, 25.f));
  return keyframes;
}

std::unique_ptr<AnimaXKeyPath> MakeTextKeyPath() {
  return std::make_unique<AnimaXKeyPath>(std::vector<std::string>{"text"});
}

AnimaXPropertyCallback MakeNoopCallback() {
  return [](const PropertyUpdateResponse&) {};
}

class NoopValueCallback : public AnimaXValueCallback {
 public:
  std::unique_ptr<Value> GetValue(const Value*,
                                  const AnimaXFrameInfo&) override {
    return nullptr;
  }
};

PropertyUpdateContext MakeLayerContext(
    LayerPropertyType type, std::unique_ptr<AnimaXValueParam> value) {
  auto request = LayerStaticRequest::Make(type, MakeTextKeyPath(),
                                          std::move(value), MakeNoopCallback());
  return PropertyUpdateContext(std::move(request));
}

PropertyUpdateContext MakeLayerCallbackContext(LayerPropertyType type) {
  auto request = LayerCallbackRequest::Make(
      type, MakeTextKeyPath(), std::make_shared<NoopValueCallback>(),
      MakeNoopCallback());
  return PropertyUpdateContext(std::move(request));
}

template <typename T>
T* GetStartValue(KeyframeAnimation& animation, int32_t index) {
  auto* keyframe = animation.GetKeyframe(index);
  return keyframe ? keyframe->GetStartValue<T>() : nullptr;
}

template <typename T>
T* GetEndValue(KeyframeAnimation& animation, int32_t index) {
  auto* keyframe = animation.GetKeyframe(index);
  return keyframe ? keyframe->GetEndValue<T>() : nullptr;
}

class TestPropertyUpdateElement : public PropertyUpdateElement,
                                  public AnimationHost,
                                  public AnimationListener {
 public:
  explicit TestPropertyUpdateElement(
      std::shared_ptr<KeyframeModelList> document_frames,
      std::unique_ptr<ColorKeyframeAnimation> color_animation = nullptr)
      : document_animation_(std::make_unique<TextKeyframeAnimation>(
            std::move(document_frames))),
        color_animation_(std::move(color_animation)) {
    AddAnimation(document_animation_.get());
    if (color_animation_) {
      AddAnimation(color_animation_.get());
    }
  }

  KeyframeAnimation* GetAnimationForProperty(LayerPropertyType type) override {
    switch (type) {
      case LayerPropertyType::kTextValue:
        return document_animation_.get();
      case LayerPropertyType::kTextColor:
      case LayerPropertyType::kColor:
        return GetOrCreateAnimation<ColorKeyframeAnimation>(color_animation_,
                                                            *this, this, type);
      case LayerPropertyType::kStrokeColor:
        return GetOrCreateAnimation<ColorKeyframeAnimation>(
            stroke_color_animation_, *this, this, type);
      case LayerPropertyType::kStrokeWidth:
        return GetOrCreateAnimation<FloatKeyframeAnimation>(
            stroke_width_animation_, *this, this, type);
      case LayerPropertyType::kTextTracking:
        return GetOrCreateAnimation<FloatKeyframeAnimation>(tracking_animation_,
                                                            *this, this, type);
      case LayerPropertyType::kTextSize:
        return GetOrCreateAnimation<FloatKeyframeAnimation>(size_animation_,
                                                            *this, this, type);
      default:
        return nullptr;
    }
  }

  void OnValueChanged() override {}

  int32_t AnimationCount() const {
    return static_cast<int32_t>(GetAnimations().size());
  }

 private:
  std::unique_ptr<TextKeyframeAnimation> document_animation_;
  std::unique_ptr<ColorKeyframeAnimation> color_animation_;
  std::unique_ptr<ColorKeyframeAnimation> stroke_color_animation_;
  std::unique_ptr<FloatKeyframeAnimation> stroke_width_animation_;
  std::unique_ptr<FloatKeyframeAnimation> tracking_animation_;
  std::unique_ptr<FloatKeyframeAnimation> size_animation_;
};

}  // namespace

TEST(PropertyUpdateElementTest,
     TextColorUpdateCreatesDocumentDerivedKeyframes) {
  TestPropertyUpdateElement element(MakeDocumentFrames());
  auto context =
      MakeLayerContext(LayerPropertyType::kTextColor,
                       std::make_unique<AnimaXValueParam>(kUpdatedColor, 3));

  EXPECT_EQ(element.AnimationCount(), 1);
  EXPECT_EQ(element.UpdateLayerProperty(context),
            PropertyUpdateResult::kSuccess);
  EXPECT_EQ(element.AnimationCount(), 2);

  auto* color_animation =
      element.GetAnimationForProperty(LayerPropertyType::kTextColor);
  auto* color_animation_again =
      element.GetAnimationForProperty(LayerPropertyType::kTextColor);
  ASSERT_NE(color_animation, nullptr);
  EXPECT_EQ(color_animation, color_animation_again);
  EXPECT_EQ(element.AnimationCount(), 2);
  EXPECT_EQ(color_animation->Type(), ValueType::kColor);
  EXPECT_EQ(color_animation->GetSize(), 4);
  ASSERT_NE(GetStartValue<Color>(*color_animation, 3), nullptr);
  ASSERT_NE(GetEndValue<Color>(*color_animation, 2), nullptr);
  EXPECT_EQ(GetStartValue<Color>(*color_animation, 3)->GetInt(), kUpdatedColor);
  EXPECT_EQ(GetEndValue<Color>(*color_animation, 2)->GetInt(), kUpdatedColor);
  EXPECT_EQ(GetStartValue<Color>(*color_animation, 0)->GetInt(),
            kDocumentColor0);
  EXPECT_EQ(GetStartValue<Color>(*color_animation, 1)->GetInt(),
            kDocumentColor1);
  EXPECT_EQ(GetStartValue<Color>(*color_animation, 2)->GetInt(),
            kDocumentColor2);
  EXPECT_EQ(GetEndValue<Color>(*color_animation, 3)->GetInt(), kDocumentColor3);

  auto* text_animation =
      element.GetAnimationForProperty(LayerPropertyType::kTextValue);
  ASSERT_NE(text_animation, nullptr);
  ASSERT_NE(GetStartValue<DocumentData>(*text_animation, 3), nullptr);
  ASSERT_NE(GetEndValue<DocumentData>(*text_animation, 2), nullptr);
  EXPECT_EQ(GetStartValue<DocumentData>(*text_animation, 3)->GetColor(),
            kDocumentColor3);
  EXPECT_EQ(GetEndValue<DocumentData>(*text_animation, 2)->GetColor(),
            kDocumentColor2);
}

TEST(PropertyUpdateElementTest,
     MissingDocumentColorUsesShapedDefaultKeyframes) {
  TestPropertyUpdateElement element(MakeDefaultColorDocumentFrames());
  auto context =
      MakeLayerContext(LayerPropertyType::kTextColor,
                       std::make_unique<AnimaXValueParam>(kUpdatedColor, 3));

  EXPECT_EQ(element.UpdateLayerProperty(context),
            PropertyUpdateResult::kSuccess);
  auto* color_animation =
      element.GetAnimationForProperty(LayerPropertyType::kTextColor);

  ASSERT_NE(color_animation, nullptr);
  EXPECT_EQ(color_animation->Type(), ValueType::kColor);
  EXPECT_EQ(color_animation->GetSize(), 4);
  ASSERT_NE(GetStartValue<Color>(*color_animation, 3), nullptr);
  ASSERT_NE(GetEndValue<Color>(*color_animation, 2), nullptr);
  EXPECT_EQ(GetStartValue<Color>(*color_animation, 3)->GetInt(), kUpdatedColor);
  EXPECT_EQ(GetEndValue<Color>(*color_animation, 2)->GetInt(), kUpdatedColor);
  for (auto index = 0; index < color_animation->GetSize(); ++index) {
    if (index == 3) {
      continue;
    }
    ASSERT_NE(GetStartValue<Color>(*color_animation, index), nullptr);
    EXPECT_EQ(GetStartValue<Color>(*color_animation, index)->GetInt(), 0);
  }
}

TEST(PropertyUpdateElementTest, TextDocumentPropertiesUseDerivedKeyframes) {
  TestPropertyUpdateElement element(MakeDocumentFrames());

  auto size_context = MakeLayerContext(
      LayerPropertyType::kTextSize,
      std::make_unique<AnimaXValueParam>(static_cast<double>(kUpdatedSize), 3));
  EXPECT_EQ(element.UpdateLayerProperty(size_context),
            PropertyUpdateResult::kSuccess);

  auto stroke_color_context = MakeLayerContext(
      LayerPropertyType::kStrokeColor,
      std::make_unique<AnimaXValueParam>(kUpdatedStrokeColor, 3));
  EXPECT_EQ(element.UpdateLayerProperty(stroke_color_context),
            PropertyUpdateResult::kSuccess);

  auto stroke_width_context =
      MakeLayerContext(LayerPropertyType::kStrokeWidth,
                       std::make_unique<AnimaXValueParam>(
                           static_cast<double>(kUpdatedStrokeWidth), 3));
  EXPECT_EQ(element.UpdateLayerProperty(stroke_width_context),
            PropertyUpdateResult::kSuccess);

  auto tracking_context =
      MakeLayerContext(LayerPropertyType::kTextTracking,
                       std::make_unique<AnimaXValueParam>(
                           static_cast<double>(kUpdatedTracking), 3));
  EXPECT_EQ(element.UpdateLayerProperty(tracking_context),
            PropertyUpdateResult::kSuccess);

  auto* size_animation =
      element.GetAnimationForProperty(LayerPropertyType::kTextSize);
  ASSERT_NE(size_animation, nullptr);
  EXPECT_EQ(size_animation->Type(), ValueType::kFloat);
  EXPECT_EQ(size_animation->GetSize(), 4);
  ASSERT_NE(GetStartValue<Float>(*size_animation, 3), nullptr);
  ASSERT_NE(GetEndValue<Float>(*size_animation, 2), nullptr);
  EXPECT_FLOAT_EQ(GetStartValue<Float>(*size_animation, 3)->Get(),
                  kUpdatedSize);
  EXPECT_FLOAT_EQ(GetEndValue<Float>(*size_animation, 2)->Get(), kUpdatedSize);
  EXPECT_FLOAT_EQ(GetStartValue<Float>(*size_animation, 0)->Get(), 10.f);

  auto* stroke_color_animation =
      element.GetAnimationForProperty(LayerPropertyType::kStrokeColor);
  ASSERT_NE(stroke_color_animation, nullptr);
  EXPECT_EQ(stroke_color_animation->Type(), ValueType::kColor);
  EXPECT_EQ(stroke_color_animation->GetSize(), 4);
  EXPECT_EQ(GetStartValue<Color>(*stroke_color_animation, 3)->GetInt(),
            kUpdatedStrokeColor);
  EXPECT_EQ(GetEndValue<Color>(*stroke_color_animation, 2)->GetInt(),
            kUpdatedStrokeColor);
  EXPECT_EQ(GetStartValue<Color>(*stroke_color_animation, 0)->GetInt(),
            kDocumentColor1);

  auto* stroke_width_animation =
      element.GetAnimationForProperty(LayerPropertyType::kStrokeWidth);
  ASSERT_NE(stroke_width_animation, nullptr);
  EXPECT_EQ(stroke_width_animation->Type(), ValueType::kFloat);
  EXPECT_EQ(stroke_width_animation->GetSize(), 4);
  ASSERT_NE(GetStartValue<Float>(*stroke_width_animation, 3), nullptr);
  ASSERT_NE(GetEndValue<Float>(*stroke_width_animation, 2), nullptr);
  EXPECT_FLOAT_EQ(GetStartValue<Float>(*stroke_width_animation, 3)->Get(),
                  kUpdatedStrokeWidth);
  EXPECT_FLOAT_EQ(GetEndValue<Float>(*stroke_width_animation, 2)->Get(),
                  kUpdatedStrokeWidth);
  EXPECT_FLOAT_EQ(GetStartValue<Float>(*stroke_width_animation, 0)->Get(), 1.f);

  auto* tracking_animation =
      element.GetAnimationForProperty(LayerPropertyType::kTextTracking);
  ASSERT_NE(tracking_animation, nullptr);
  EXPECT_EQ(tracking_animation->Type(), ValueType::kFloat);
  EXPECT_EQ(tracking_animation->GetSize(), 4);
  ASSERT_NE(GetStartValue<Float>(*tracking_animation, 3), nullptr);
  ASSERT_NE(GetEndValue<Float>(*tracking_animation, 2), nullptr);
  EXPECT_FLOAT_EQ(GetStartValue<Float>(*tracking_animation, 3)->Get(),
                  kUpdatedTracking);
  EXPECT_FLOAT_EQ(GetEndValue<Float>(*tracking_animation, 2)->Get(),
                  kUpdatedTracking);
  EXPECT_FLOAT_EQ(GetStartValue<Float>(*tracking_animation, 0)->Get(), 0.f);
}

TEST(PropertyUpdateElementTest,
     TextColorCallbackCreatesDocumentDerivedKeyframes) {
  TestPropertyUpdateElement element(MakeDocumentFrames());
  auto context = MakeLayerCallbackContext(LayerPropertyType::kTextColor);

  EXPECT_EQ(element.AddLayerPropertyCallback(context),
            PropertyUpdateResult::kSuccess);
  auto* color_animation =
      element.GetAnimationForProperty(LayerPropertyType::kTextColor);
  ASSERT_NE(color_animation, nullptr);
  EXPECT_EQ(color_animation->Type(), ValueType::kColor);
  EXPECT_EQ(color_animation->GetSize(), 4);
  EXPECT_TRUE(color_animation->HasValueCallback());
}

TEST(PropertyUpdateElementTest,
     DocumentDerivedColorUsesHoldRenderingSemantics) {
  CompositionModel composition(1.0f);
  composition.Init(std::make_unique<RectF>(0.f, 0.f, 100.f, 100.f), 0.f, 100.f,
                   10.f, false);
  TestPropertyUpdateElement element(MakeAnimatedDocumentFrames(composition));

  auto* color_animation =
      element.GetAnimationForProperty(LayerPropertyType::kTextColor);
  ASSERT_NE(color_animation, nullptr);
  EXPECT_EQ(color_animation->GetSize(), 2);

  color_animation->SetProgress(0.05f);

  auto& color = static_cast<Color&>(color_animation->GetValue());
  EXPECT_EQ(color.GetInt(), kDocumentColor0);
}

TEST(PropertyUpdateElementTest,
     ParserShapedDocumentLastFrameUpdateCreatesSeekableTerminalKeyframe) {
  CompositionModel composition(1.0f);
  composition.Init(std::make_unique<RectF>(0.f, 0.f, 100.f, 100.f), 0.f, 30.f,
                   30.f, false);
  TestPropertyUpdateElement element(MakeSegmentedDocumentFrames(composition));
  auto context =
      MakeLayerContext(LayerPropertyType::kTextColor,
                       std::make_unique<AnimaXValueParam>(kUpdatedColor, 3));

  EXPECT_EQ(element.UpdateLayerProperty(context),
            PropertyUpdateResult::kSuccess);

  auto* color_animation =
      element.GetAnimationForProperty(LayerPropertyType::kTextColor);
  ASSERT_NE(color_animation, nullptr);
  EXPECT_EQ(color_animation->Type(), ValueType::kColor);
  EXPECT_EQ(color_animation->GetSize(), 4);
  ASSERT_NE(GetEndValue<Color>(*color_animation, 2), nullptr);
  ASSERT_NE(GetStartValue<Color>(*color_animation, 3), nullptr);
  EXPECT_EQ(GetEndValue<Color>(*color_animation, 2)->GetInt(), kUpdatedColor);
  EXPECT_EQ(GetStartValue<Color>(*color_animation, 3)->GetInt(), kUpdatedColor);

  color_animation->SetProgress(25.f / composition.GetDurationFrames());

  auto& color = static_cast<Color&>(color_animation->GetValue());
  EXPECT_EQ(color.GetInt(), kUpdatedColor);
}

TEST(PropertyUpdateElementTest, ExistingAnimatorColorPathIsReused) {
  TestPropertyUpdateElement element(
      MakeDocumentFrames(),
      std::make_unique<ColorKeyframeAnimation>(MakeAnimatorColorFrames()));
  auto context =
      MakeLayerContext(LayerPropertyType::kTextColor,
                       std::make_unique<AnimaXValueParam>(kUpdatedColor, 3));

  EXPECT_EQ(element.AnimationCount(), 2);
  EXPECT_EQ(element.UpdateLayerProperty(context),
            PropertyUpdateResult::kSuccess);
  EXPECT_EQ(element.AnimationCount(), 2);

  auto* color_animation =
      element.GetAnimationForProperty(LayerPropertyType::kTextColor);
  ASSERT_NE(color_animation, nullptr);
  ASSERT_NE(GetStartValue<Color>(*color_animation, 3), nullptr);
  ASSERT_NE(GetEndValue<Color>(*color_animation, 2), nullptr);
  EXPECT_EQ(GetStartValue<Color>(*color_animation, 3)->GetInt(), kUpdatedColor);
  EXPECT_EQ(GetEndValue<Color>(*color_animation, 2)->GetInt(), kUpdatedColor);
  EXPECT_EQ(GetStartValue<Color>(*color_animation, 0)->GetInt(),
            kAnimatorColor0);
  EXPECT_EQ(GetStartValue<Color>(*color_animation, 1)->GetInt(),
            kAnimatorColor1);
  EXPECT_EQ(GetStartValue<Color>(*color_animation, 2)->GetInt(),
            kAnimatorColor2);
  EXPECT_EQ(GetEndValue<Color>(*color_animation, 3)->GetInt(), kAnimatorColor3);

  auto* text_animation =
      element.GetAnimationForProperty(LayerPropertyType::kTextValue);
  ASSERT_NE(text_animation, nullptr);
  EXPECT_EQ(GetStartValue<DocumentData>(*text_animation, 3)->GetColor(),
            kDocumentColor3);
  EXPECT_EQ(GetEndValue<DocumentData>(*text_animation, 2)->GetColor(),
            kDocumentColor2);
}
