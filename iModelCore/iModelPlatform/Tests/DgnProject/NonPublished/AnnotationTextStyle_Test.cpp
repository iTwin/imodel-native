//---------------------------------------------------------------------------------------------
//  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//  See LICENSE.md in the repository root for full copyright notice.
//---------------------------------------------------------------------------------------------

#include "AnnotationTestFixture.h"
#include <DgnPlatform/Annotations/Annotations.h>
#include "../../../PrivateApi/DgnPlatformInternal/DgnCore/Annotations/Annotations.fb.h"

//=======================================================================================
// @bsiclass
//=======================================================================================
struct AnnotationTextStyleTest : public AnnotationTestFixture
{

}; // AnnotationTextStyleTest


//---------------------------------------------------------------------------------------
// Verifies the mapping between AnnotationTextStyleProperty and data type in AnnotationTextStylePropertyBag by ensuring it does not assert.
// Higher level AnnotationTextStyle tests provide otherwise good coverage for AnnotationTextStylePropertyBag because styles uses a bag for their underlying storage.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST(BasicAnnotationTextStyleTest, PropertyBagTypes)
    {
    AnnotationTextStylePropertyBagPtr data = AnnotationTextStylePropertyBag::Create();

    data->SetIntegerProperty(AnnotationTextStyleProperty::ColorType, (uint32_t)AnnotationColorType::RGBA);
    data->SetIntegerProperty(AnnotationTextStyleProperty::ColorValue, 2);
    data->SetIntegerProperty(AnnotationTextStyleProperty::FontId, 2);
    data->SetRealProperty(AnnotationTextStyleProperty::Height, 2.0);
    data->SetRealProperty(AnnotationTextStyleProperty::LineSpacingFactor, 2.0);
    data->SetIntegerProperty(AnnotationTextStyleProperty::IsBold, 1);
    data->SetIntegerProperty(AnnotationTextStyleProperty::IsItalic, 1);
    data->SetIntegerProperty(AnnotationTextStyleProperty::IsUnderlined, 1);
    data->SetRealProperty(AnnotationTextStyleProperty::StackedFractionScale, 1.5);
    data->SetIntegerProperty(AnnotationTextStyleProperty::StackedFractionType, (uint32_t)AnnotationStackedFractionType::DiagonalBar);
    data->SetRealProperty(AnnotationTextStyleProperty::SubScriptOffsetFactor, 1.5);
    data->SetRealProperty(AnnotationTextStyleProperty::SubScriptScale, 1.5);
    data->SetRealProperty(AnnotationTextStyleProperty::SuperScriptScale, 1.5);
    data->SetRealProperty(AnnotationTextStyleProperty::WidthFactor, 2.0);
}

#define DECLARE_AND_SET_DATA_1(STYLE_PTR)\
    Utf8String name = "MyStyle";                                                                STYLE_PTR->SetName(name.c_str());\
    Utf8String description = "MyDescription";                                                   STYLE_PTR->SetDescription(description.c_str());\
    AnnotationColorType colorType = AnnotationColorType::RGBA;                                  STYLE_PTR->SetColorType(colorType);\
    ColorDef color(0xff, 0x00, 0x00);                                                           STYLE_PTR->SetColorValue(color);\
    FontId fontId((uint64_t)21);                                                             STYLE_PTR->SetFontId(fontId);\
    double height = 31.31;                                                                      STYLE_PTR->SetHeight(height);\
    bool isBold = true;                                                                         STYLE_PTR->SetIsBold(isBold);\
    bool isItalic = true;                                                                       STYLE_PTR->SetIsItalic(isItalic);\
    bool isUnderlined = true;                                                                   STYLE_PTR->SetIsUnderlined(isUnderlined);\
    AnnotationStackedFractionType fractionType = AnnotationStackedFractionType::DiagonalBar;    STYLE_PTR->SetStackedFractionType(fractionType);\
    double widthFactor = 41.41;                                                                 STYLE_PTR->SetWidthFactor(widthFactor);\
    double lineSpaceFactor = 1.5;                                                               STYLE_PTR->SetLineSpacingFactor(lineSpaceFactor);\
    double stackedFractionScale = 1.5;                                                          STYLE_PTR->SetStackedFractionScale(stackedFractionScale);\
    double subScriptOffsetFactor = 1.5;                                                         STYLE_PTR->SetSubScriptOffsetFactor(subScriptOffsetFactor);\
    double subScriptScale = 1.5;                                                                STYLE_PTR->SetSubScriptScale(subScriptScale);\
    double superScriptOffsetFactor = 1.5;                                                       STYLE_PTR->SetSuperScriptOffsetFactor(superScriptOffsetFactor);\
    double superScriptScale = 1.5;                                                              STYLE_PTR->SetSuperScriptScale(superScriptScale);

#define VERIFY_DATA_1(STYLE_PTR)\
    EXPECT_TRUE(name.Equals(STYLE_PTR->GetName()));\
    EXPECT_TRUE(description.Equals(STYLE_PTR->GetDescription()));\
    EXPECT_TRUE(colorType == STYLE_PTR->GetColorType());\
    EXPECT_TRUE(color == STYLE_PTR->GetColorValue());\
    EXPECT_TRUE(fontId == STYLE_PTR->GetFontId());\
    EXPECT_TRUE(height == STYLE_PTR->GetHeight());\
    EXPECT_TRUE(isBold == STYLE_PTR->IsBold());\
    EXPECT_TRUE(isItalic == STYLE_PTR->IsItalic());\
    EXPECT_TRUE(isUnderlined == STYLE_PTR->IsUnderlined());\
    EXPECT_TRUE(fractionType == STYLE_PTR->GetStackedFractionType());\
    EXPECT_TRUE(widthFactor == STYLE_PTR->GetWidthFactor());\
    EXPECT_TRUE(lineSpaceFactor == STYLE_PTR->GetLineSpacingFactor());\
    EXPECT_TRUE(stackedFractionScale == STYLE_PTR->GetStackedFractionScale());\
    EXPECT_TRUE(subScriptOffsetFactor == STYLE_PTR->GetSubScriptOffsetFactor());\
    EXPECT_TRUE(subScriptScale == STYLE_PTR->GetSubScriptScale());\
    EXPECT_TRUE(superScriptOffsetFactor == STYLE_PTR->GetSuperScriptOffsetFactor());\
    EXPECT_TRUE(superScriptScale == STYLE_PTR->GetSuperScriptScale());

//---------------------------------------------------------------------------------------
// Creates a style and tests accessors.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(AnnotationTextStyleTest, DefaultsAndAccessors)
    {
    //.............................................................................................
    DgnDbR project = *GetDgnDb(L"DefaultsAndAccessors");

    //.............................................................................................
    AnnotationTextStylePtr style = AnnotationTextStyle::Create(project.GetDictionaryModel());
    ASSERT_TRUE(style.IsValid());

    // Basics
    EXPECT_TRUE(&project == &style->GetDgnDb());
    EXPECT_TRUE(!style->GetElementId().IsValid()); // Cannot call SetId directly from published API.

    // Defaults
    EXPECT_TRUE(style->GetName().empty());
    EXPECT_TRUE(style->GetDescription().empty());
    EXPECT_TRUE(0 == style->GetColorValue().GetValue());
    EXPECT_TRUE(!style->GetFontId().IsValid());
    EXPECT_TRUE(1.0 == style->GetHeight());
    EXPECT_TRUE(!style->IsBold());
    EXPECT_TRUE(!style->IsItalic());
    EXPECT_TRUE(!style->IsUnderlined());
    EXPECT_TRUE(AnnotationStackedFractionType::HorizontalBar == style->GetStackedFractionType());
    EXPECT_TRUE(1.0 == style->GetWidthFactor());

    // Set/Get round-trip
    DECLARE_AND_SET_DATA_1(style);
    VERIFY_DATA_1(style);
    }

//---------------------------------------------------------------------------------------
// Creates and clones a style.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(AnnotationTextStyleTest, DeepCopy)
    {
    //.............................................................................................
    DgnDbR project = *GetDgnDb();

    //.............................................................................................
    AnnotationTextStylePtr style = AnnotationTextStyle::Create(project.GetDictionaryModel());
    ASSERT_TRUE(style.IsValid());

    DECLARE_AND_SET_DATA_1(style);

    //.............................................................................................
    AnnotationTextStylePtr clonedStyle = style->CreateCopy();
    ASSERT_TRUE(clonedStyle.IsValid());
    ASSERT_TRUE(style.get() != clonedStyle.get());

    EXPECT_TRUE(&project == &clonedStyle->GetDgnDb());
    EXPECT_TRUE(style->GetElementId() == clonedStyle->GetElementId());
    VERIFY_DATA_1(clonedStyle);
    }

//---------------------------------------------------------------------------------------
// Verifies persistence in the style table.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(AnnotationTextStyleTest, TableReadWrite)
    {
    //.............................................................................................
    DgnDbR project = *GetDgnDb();

    //.............................................................................................
    // Verify initial state.
    EXPECT_EQ((0 ), AnnotationTextStyle::QueryCount(project));

    //.............................................................................................
    // Insert
    AnnotationTextStylePtr testStyle = AnnotationTextStyle::Create(project.GetDictionaryModel());
    DECLARE_AND_SET_DATA_1(testStyle);

    ASSERT_TRUE(testStyle->Insert().IsValid());
    ASSERT_TRUE(testStyle->GetElementId().IsValid());

    EXPECT_EQ((1), AnnotationTextStyle::QueryCount(project));

    //.............................................................................................
    // Query
    EXPECT_TRUE(AnnotationTextStyle::Get(project, testStyle->GetElementId()).IsValid());
    EXPECT_TRUE(AnnotationTextStyle::Get(project.GetDictionaryModel(), name.c_str()).IsValid());

    AnnotationTextStyleCPtr fileStyle = AnnotationTextStyle::Get(project, testStyle->GetElementId());
    ASSERT_TRUE(fileStyle.IsValid());

    EXPECT_TRUE(&project == &fileStyle->GetDgnDb());
    EXPECT_TRUE(testStyle->GetElementId() == fileStyle->GetElementId());
    VERIFY_DATA_1(fileStyle);

    fileStyle = AnnotationTextStyle::Get(project.GetDictionaryModel(), name.c_str());
    EXPECT_TRUE(fileStyle.IsValid());

    EXPECT_TRUE(&project == &fileStyle->GetDgnDb());
    EXPECT_TRUE(testStyle->GetElementId() == fileStyle->GetElementId());
    VERIFY_DATA_1(fileStyle);

    //.............................................................................................
    // Update
    AnnotationTextStylePtr mutatedStyle = fileStyle->CreateCopy();
    ASSERT_TRUE(mutatedStyle.IsValid());

    name = "DifferentName"; mutatedStyle->SetName(name.c_str());
    color = ColorDef(0x00, 0xff, 0x00); mutatedStyle->SetColorValue(color);
    height *= 100.0; mutatedStyle->SetHeight(height);
    isBold = false; mutatedStyle->SetIsBold(isBold);

    EXPECT_EQ(DgnDbStatus::Success, mutatedStyle->Update());

    EXPECT_EQ((1), AnnotationTextStyle::QueryCount(project));

    fileStyle = AnnotationTextStyle::Get(project.GetDictionaryModel(), name.c_str());
    EXPECT_TRUE(fileStyle.IsValid());

    EXPECT_TRUE(&project == &fileStyle->GetDgnDb());
    EXPECT_TRUE(mutatedStyle->GetElementId() == fileStyle->GetElementId());
    VERIFY_DATA_1(fileStyle);

    //.............................................................................................
    // Iterate
    auto iter1 = AnnotationTextStyle::MakeIterator(project);
    size_t numStyles = 0;
    bool foundOurStyle = false;

    for (auto const& style : iter1)
        {
        EXPECT_TRUE(!Utf8String::IsNullOrEmpty(style.GetName()));
        ASSERT_TRUE(style.GetElementId().IsValid());

        AnnotationTextStyleCPtr iterStyle = AnnotationTextStyle::Get(project, style.GetElementId());
        ASSERT_TRUE(iterStyle.IsValid());

        if (!foundOurStyle)
            foundOurStyle = (0 == name.compare(iterStyle->GetName()));

        ++numStyles;
        }

    EXPECT_EQ((1), numStyles);
    EXPECT_TRUE(foundOurStyle);

    auto iter2 = AnnotationTextStyle::MakeIterator(project);
    numStyles = 0;
    foundOurStyle = false;

    for (auto const& style : iter2)
        {
        EXPECT_TRUE(!Utf8String::IsNullOrEmpty(style.GetName()));
        ASSERT_TRUE(style.GetElementId().IsValid());

        auto iterStyle = AnnotationTextStyle::Get(project, style.GetElementId());
        ASSERT_TRUE(iterStyle.IsValid());

        if (!foundOurStyle)
            foundOurStyle = (0 == name.compare(iterStyle->GetName()));

        ++numStyles;
        }

    EXPECT_EQ((1), numStyles);
    EXPECT_TRUE(foundOurStyle);
    project.SaveChanges();
    }

//---------------------------------------------------------------------------------------
// Verifies style persistence in FlatBuffers serialization.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(AnnotationTextStyleTest, FlatBuffersSerialization)
    {
    //.............................................................................................
    DgnDbR project = *GetDgnDb();

    //.............................................................................................
    // Initialize
    AnnotationTextStylePtr testStyle = AnnotationTextStyle::Create(project.GetDictionaryModel());
    DECLARE_AND_SET_DATA_1(testStyle);

    //.............................................................................................
    // Update expected props with the same values as in TableReadWrite test above
    name = "DifferentName"; testStyle->SetName(name.c_str());
    color = ColorDef(0x00, 0xff, 0x00); //testStyle->SetColorValue(color);
    height *= 100.0; //testStyle->SetHeight(height);
    // But don't reset isBold to false - default values are not serialized into FB by default:
    //isBold = false; //testStyle->SetIsBold(isBold);

    //.............................................................................................
    // Restore style props from FlatBuffers 1.0.0 dump to check compatibility with currently used FB version
    const Byte buffer[] = {
        0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x0c, 0x00, 0x0b, 0x00, 0x0a, 0x00, 0x04, 0x00,
        0x0a, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x0e, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x15, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x76, 0xa8, 0x40,
        0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x3f, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x3f,
        0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x3f,
        0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x3f, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x3f,
        0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x3f, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0xae, 0x47, 0xe1, 0x7a, 0xb4, 0x44, 0x40
        };
    // Cannot use AnnotationTextStylePersistence::DecodeFromFlatBuf() here and so do the same manually:
    auto fbStyle = flatbuffers::GetRoot<FB::AnnotationTextStyle>(buffer);
    ASSERT_NE(nullptr, fbStyle);
    EXPECT_TRUE(fbStyle->has_majorVersion());
    EXPECT_TRUE(fbStyle->has_minorVersion());
    ASSERT_TRUE(fbStyle->has_setters());
    int fbStyleSettersCount = 0;
    for (const auto& setter : *fbStyle->setters())
        {
        fbStyleSettersCount++;
        switch (setter->key())
            {
                case FB::AnnotationTextStyleProperty_ColorType: testStyle->SetColorType((AnnotationColorType) setter->integerValue()); break;
                case FB::AnnotationTextStyleProperty_ColorValue: testStyle->SetColorValue(ColorDef((int32_t) setter->integerValue())); break;
                case FB::AnnotationTextStyleProperty_FontId: testStyle->SetFontId(FontId((uint64_t) setter->integerValue())); break;
                case FB::AnnotationTextStyleProperty_Height: testStyle->SetHeight(setter->realValue()); break;
                case FB::AnnotationTextStyleProperty_LineSpacingFactor: testStyle->SetLineSpacingFactor(setter->realValue()); break;
                case FB::AnnotationTextStyleProperty_IsBold: testStyle->SetIsBold(0 != setter->integerValue()); break;
                case FB::AnnotationTextStyleProperty_IsItalic: testStyle->SetIsItalic(0 != setter->integerValue()); break;
                case FB::AnnotationTextStyleProperty_IsUnderlined: testStyle->SetIsUnderlined(0 != setter->integerValue()); break;
                case FB::AnnotationTextStyleProperty_StackedFractionScale: testStyle->SetStackedFractionScale(setter->realValue()); break;
                case FB::AnnotationTextStyleProperty_StackedFractionType: testStyle->SetStackedFractionType((AnnotationStackedFractionType) setter->integerValue()); break;
                case FB::AnnotationTextStyleProperty_SubScriptOffsetFactor: testStyle->SetSubScriptOffsetFactor(setter->realValue()); break;
                case FB::AnnotationTextStyleProperty_SubScriptScale: testStyle->SetSubScriptScale(setter->realValue()); break;
                case FB::AnnotationTextStyleProperty_SuperScriptOffsetFactor: testStyle->SetSuperScriptOffsetFactor(setter->realValue()); break;
                case FB::AnnotationTextStyleProperty_SuperScriptScale: testStyle->SetSuperScriptScale(setter->realValue()); break;
                case FB::AnnotationTextStyleProperty_WidthFactor: testStyle->SetWidthFactor(setter->realValue()); break;
            }
        }
    EXPECT_EQ(14, fbStyleSettersCount) << "14 of 15 AnnotationTextStyleProperty filled setters are expected";
    VERIFY_DATA_1(testStyle);
    }
