//-------------------------------------------------------------------------------------- 
//     $Source: Tests/DgnProject/Published/AnnotationTextStyle_Test.cpp $
//  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 

#include "AnnotationTestFixture.h"
#include <DgnPlatform/Annotations/Annotations.h>

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
struct AnnotationTextStyleTest : public AnnotationTestFixture
{

}; // AnnotationTextStyleTest


//---------------------------------------------------------------------------------------
// Verifies the mapping between AnnotationTextStyleProperty and data type in AnnotationTextStylePropertyBag by ensuring it does not assert.
// Higher level AnnotationTextStyle tests provide otherwise good coverage for AnnotationTextStylePropertyBag because styles uses a bag for their underlying storage.
// @bsimethod                                                   Jeff.Marker     05/2014
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
    DgnFontId fontId((uint64_t)21);                                                             STYLE_PTR->SetFontId(fontId);\
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
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
TEST_F(AnnotationTextStyleTest, DefaultsAndAccessors)
    {
    //.............................................................................................
    DgnDbR project = *GetDgnDb(L"DefaultsAndAccessors");

    //.............................................................................................
    AnnotationTextStylePtr style = AnnotationTextStyle::Create(project);
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
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
TEST_F(AnnotationTextStyleTest, DeepCopy)
    {
    //.............................................................................................
    DgnDbR project = *GetDgnDb();

    //.............................................................................................
    AnnotationTextStylePtr style = AnnotationTextStyle::Create(project);
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
// @bsimethod                                                   Jeff.Marker     05/2014
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
    AnnotationTextStylePtr testStyle = AnnotationTextStyle::Create(project);
    DECLARE_AND_SET_DATA_1(testStyle);

    ASSERT_TRUE(testStyle->Insert().IsValid());
    ASSERT_TRUE(testStyle->GetElementId().IsValid());

    EXPECT_EQ((1), AnnotationTextStyle::QueryCount(project));

    //.............................................................................................
    // Query
    EXPECT_TRUE(AnnotationTextStyle::Get(project, testStyle->GetElementId()).IsValid());
    EXPECT_TRUE(AnnotationTextStyle::Get(project, name.c_str()).IsValid());

    AnnotationTextStyleCPtr fileStyle = AnnotationTextStyle::Get(project, testStyle->GetElementId());
    ASSERT_TRUE(fileStyle.IsValid());
    
    EXPECT_TRUE(&project == &fileStyle->GetDgnDb());
    EXPECT_TRUE(testStyle->GetElementId() == fileStyle->GetElementId());
    VERIFY_DATA_1(fileStyle);

    fileStyle = AnnotationTextStyle::Get(project, name.c_str());
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

    ASSERT_TRUE(mutatedStyle->Update().IsValid());

    EXPECT_EQ((1), AnnotationTextStyle::QueryCount(project));

    fileStyle = AnnotationTextStyle::Get(project, name.c_str());
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
