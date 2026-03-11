// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/model/value/document_data.h"

#include "gtest/gtest.h"

using namespace lynx::animax;

TEST(DocumentDataTest, DefaultConstructor) {
  auto doc = ValueFactory::Make<DocumentData>();
  EXPECT_TRUE(doc->IsEmpty());
  EXPECT_EQ(doc->GetText(), "");
  EXPECT_EQ(doc->GetFontName(), "");
  EXPECT_FLOAT_EQ(doc->GetSize(), 0.0f);
  EXPECT_EQ(doc->GetJustification(), DocumentJustification::kLeftAlign);
  EXPECT_EQ(doc->GetTracking(), 0);
  EXPECT_FLOAT_EQ(doc->GetLineHeight(), 0.0f);
  EXPECT_FLOAT_EQ(doc->GetBaselineShift(), 0.0f);
  EXPECT_EQ(doc->GetColor(), 0);
  EXPECT_EQ(doc->GetStrokeColor(), 0);
  EXPECT_FLOAT_EQ(doc->GetStrokeWidth(), 0.0f);
  EXPECT_FALSE(doc->GetStrokeOverfill());
  EXPECT_EQ(doc->GetBoxPosition(), nullptr);
  EXPECT_EQ(doc->GetBoxSize(), nullptr);
}

TEST(DocumentDataTest, ConstructorWithValues) {
  auto box_pos = ValueFactory::Make<PointF>(10.0f, 20.0f);
  auto box_size = ValueFactory::Make<PointF>(100.0f, 200.0f);
  auto doc = ValueFactory::Make<DocumentData>(
      "Test Text", "Arial", 16.0f, DocumentJustification::kCenter, 2, 1.5f,
      0.5f, 0xFF0000, 0x0000FF, 1.0f, true, std::move(box_pos),
      std::move(box_size));

  EXPECT_FALSE(doc->IsEmpty());
  EXPECT_EQ(doc->GetText(), "Test Text");
  EXPECT_EQ(doc->GetFontName(), "Arial");
  EXPECT_FLOAT_EQ(doc->GetSize(), 16.0f);
  EXPECT_EQ(doc->GetJustification(), DocumentJustification::kCenter);
  EXPECT_EQ(doc->GetTracking(), 2);
  EXPECT_FLOAT_EQ(doc->GetLineHeight(), 1.5f);
  EXPECT_FLOAT_EQ(doc->GetBaselineShift(), 0.5f);
  EXPECT_EQ(doc->GetColor(), 0xFF0000);
  EXPECT_EQ(doc->GetStrokeColor(), 0x0000FF);
  EXPECT_FLOAT_EQ(doc->GetStrokeWidth(), 1.0f);
  EXPECT_TRUE(doc->GetStrokeOverfill());

  auto* pos = doc->GetBoxPosition();
  EXPECT_NE(pos, nullptr);
  EXPECT_FLOAT_EQ(pos->GetX(), 10.0f);
  EXPECT_FLOAT_EQ(pos->GetY(), 20.0f);

  auto* size = doc->GetBoxSize();
  EXPECT_NE(size, nullptr);
  EXPECT_FLOAT_EQ(size->GetX(), 100.0f);
  EXPECT_FLOAT_EQ(size->GetY(), 200.0f);
}

TEST(DocumentDataTest, SetText) {
  auto doc = ValueFactory::Make<DocumentData>();
  doc->SetText("New Text");
  EXPECT_EQ(doc->GetText(), "New Text");
}

TEST(DocumentDataTest, SetColor) {
  auto doc = ValueFactory::Make<DocumentData>();
  doc->SetColor(0x00FF00);
  EXPECT_EQ(doc->GetColor(), 0x00FF00);
}

TEST(DocumentDataTest, SetSize) {
  auto doc = ValueFactory::Make<DocumentData>();
  doc->SetSize(24.0f);
  EXPECT_FLOAT_EQ(doc->GetSize(), 24.0f);
  EXPECT_FALSE(doc->IsEmpty());
}

TEST(DocumentDataTest, Copy) {
  auto box_pos = ValueFactory::Make<PointF>(10.0f, 20.0f);
  auto box_size = ValueFactory::Make<PointF>(100.0f, 200.0f);
  auto original = ValueFactory::Make<DocumentData>(
      "Test Text", "Arial", 16.0f, DocumentJustification::kCenter, 2, 1.5f,
      0.5f, 0xFF0000, 0x0000FF, 1.0f, true, std::move(box_pos),
      std::move(box_size));

  auto copy = original->Copy();
  auto* doc_copy = static_cast<DocumentData*>(copy.get());

  EXPECT_NE(doc_copy, nullptr);
  EXPECT_FALSE(doc_copy->IsEmpty());
  EXPECT_EQ(doc_copy->GetText(), "Test Text");
  EXPECT_EQ(doc_copy->GetFontName(), "Arial");
  EXPECT_FLOAT_EQ(doc_copy->GetSize(), 16.0f);
  EXPECT_EQ(doc_copy->GetJustification(), DocumentJustification::kCenter);
  EXPECT_EQ(doc_copy->GetTracking(), 2);
  EXPECT_FLOAT_EQ(doc_copy->GetLineHeight(), 1.5f);
  EXPECT_FLOAT_EQ(doc_copy->GetBaselineShift(), 0.5f);
  EXPECT_EQ(doc_copy->GetColor(), 0xFF0000);
  EXPECT_EQ(doc_copy->GetStrokeColor(), 0x0000FF);
  EXPECT_FLOAT_EQ(doc_copy->GetStrokeWidth(), 1.0f);
  EXPECT_TRUE(doc_copy->GetStrokeOverfill());

  auto* pos = doc_copy->GetBoxPosition();
  EXPECT_NE(pos, nullptr);
  EXPECT_FLOAT_EQ(pos->GetX(), 10.0f);
  EXPECT_FLOAT_EQ(pos->GetY(), 20.0f);

  auto* size = doc_copy->GetBoxSize();
  EXPECT_NE(size, nullptr);
  EXPECT_FLOAT_EQ(size->GetX(), 100.0f);
  EXPECT_FLOAT_EQ(size->GetY(), 200.0f);
}

TEST(DocumentDataTest, CopyEmpty) {
  auto original = ValueFactory::Make<DocumentData>();
  auto copy = original->Copy();
  auto* doc_copy = static_cast<DocumentData*>(copy.get());

  EXPECT_NE(doc_copy, nullptr);
  EXPECT_TRUE(doc_copy->IsEmpty());
  EXPECT_EQ(doc_copy->GetText(), "");
  EXPECT_EQ(doc_copy->GetFontName(), "");
  EXPECT_FLOAT_EQ(doc_copy->GetSize(), 0.0f);
  EXPECT_EQ(doc_copy->GetJustification(), DocumentJustification::kLeftAlign);
  EXPECT_EQ(doc_copy->GetTracking(), 0);
  EXPECT_FLOAT_EQ(doc_copy->GetLineHeight(), 0.0f);
  EXPECT_FLOAT_EQ(doc_copy->GetBaselineShift(), 0.0f);
  EXPECT_EQ(doc_copy->GetColor(), 0);
  EXPECT_EQ(doc_copy->GetStrokeColor(), 0);
  EXPECT_FLOAT_EQ(doc_copy->GetStrokeWidth(), 0.0f);
  EXPECT_FALSE(doc_copy->GetStrokeOverfill());
  EXPECT_EQ(doc_copy->GetBoxPosition(), nullptr);
  EXPECT_EQ(doc_copy->GetBoxSize(), nullptr);
}
