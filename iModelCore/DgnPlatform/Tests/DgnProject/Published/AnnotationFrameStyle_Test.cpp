//-------------------------------------------------------------------------------------- 
//     $Source: Tests/DgnProject/Published/AnnotationFrameStyle_Test.cpp $
//  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 

#include "DgnHandlersTests.h"
#include <DgnPlatform/Annotations/Annotations.h>

//=======================================================================================
// @bsiclass                                                   Umar.Hayat     07/15
//=======================================================================================
struct AnnotationFrameStyleTest : public GenericDgnModelTestFixture
{

}; // AnnotationFrameStyleTest

//---------------------------------------------------------------------------------------
// Verifies the mapping between AnnotationFrameStyleProperty and data type in AnnotationFrameStylePropertyBag by ensuring it does not assert.
// Higher level AnnotationFrameStyle tests provide otherwise good coverage for AnnotationFrameStylePropertyBag because styles uses a bag for their underlying storage.
// @bsimethod                                                   Umar.Hayat     07/15
//---------------------------------------------------------------------------------------
TEST_F(AnnotationFrameStyleTest, PropertyBagTypes)
    {
    AnnotationFrameStylePropertyBagPtr data = AnnotationFrameStylePropertyBag::Create();

    data->SetRealProperty(AnnotationFrameStyleProperty::CloudBulgeFactor, 2.0);
    data->SetRealProperty(AnnotationFrameStyleProperty::CloudDiameterFactor, 2.0);
    data->SetIntegerProperty(AnnotationFrameStyleProperty::FillColorValue, 2);
    data->SetRealProperty(AnnotationFrameStyleProperty::FillTransparency, 2.0);
    data->SetRealProperty(AnnotationFrameStyleProperty::HorizontalPadding, 2.0);
    data->SetIntegerProperty(AnnotationFrameStyleProperty::IsFillEnabled, 1);
    data->SetIntegerProperty(AnnotationFrameStyleProperty::IsStrokeCloud, 1);
    data->SetIntegerProperty(AnnotationFrameStyleProperty::IsStrokeEnabled, 0);
    data->SetIntegerProperty(AnnotationFrameStyleProperty::StrokeColorValue, 3);
    data->SetIntegerProperty(AnnotationFrameStyleProperty::StrokeWeight, 2);
    data->SetIntegerProperty(AnnotationFrameStyleProperty::Type, (int64_t)AnnotationFrameType::Box);
    data->SetRealProperty(AnnotationFrameStyleProperty::VerticalPadding, 2.0);
    }

#define DECLARE_AND_SET_DATA_1(STYLE_PTR)\
    Utf8String name = "MyStyle";                                                                STYLE_PTR->SetName(name.c_str());\
    Utf8String description = "MyDescription";                                                   STYLE_PTR->SetDescription(description.c_str());\
    double cloudBulgeFactor = 2.1;                                                              STYLE_PTR->SetCloudBulgeFactor(cloudBulgeFactor);\
    double cloudDiameterFactor = 2.1;                                                           STYLE_PTR->SetCloudDiameterFactor(cloudDiameterFactor);\
    ColorDef fillColor(0xff, 0x00, 0x00);                                                       STYLE_PTR->SetFillColorValue(fillColor);\
    double fillTransparency = 2.1;                                                              STYLE_PTR->SetFillTransparency(fillTransparency);\
    double horizontalPadding = 2.1;                                                             STYLE_PTR->SetHorizontalPadding(horizontalPadding);\
    bool isFillEnabled =true;                                                                   STYLE_PTR->SetIsFillEnabled(isFillEnabled);\
    bool isStrokeCloud =false;                                                                  STYLE_PTR->SetIsStrokeCloud(isStrokeCloud);\
    bool isStrokeEnabled =true;                                                                 STYLE_PTR->SetIsStrokeEnabled(isStrokeEnabled);\
    ColorDef strokeColor(0xff, 0x00, 0x00);                                                     STYLE_PTR->SetStrokeColorValue(strokeColor);\
    uint32_t strokeWeight=3;                                                                    STYLE_PTR->SetStrokeWeight(strokeWeight); \
    AnnotationFrameType type = AnnotationFrameType::Circle;                                     STYLE_PTR->SetType(type); \
    double verticalPadding = 2.1;                                                               STYLE_PTR->SetVerticalPadding(verticalPadding);\
    AnnotationColorType fillcolortype = AnnotationColorType::RGBA;                              STYLE_PTR->SetFillColorType(fillcolortype);\
    AnnotationColorType strokecolortype = AnnotationColorType::RGBA;                            STYLE_PTR->SetStrokeColorType(strokecolortype);

#define VERIFY_DATA_1(STYLE_PTR)\
    EXPECT_TRUE(name.Equals(STYLE_PTR->GetName()));\
    EXPECT_TRUE(description.Equals(STYLE_PTR->GetDescription()));\
    EXPECT_TRUE(cloudBulgeFactor == STYLE_PTR->GetCloudBulgeFactor());\
    EXPECT_TRUE(cloudDiameterFactor == STYLE_PTR->GetCloudDiameterFactor());\
    EXPECT_TRUE(fillcolortype == STYLE_PTR->GetFillColorType());\
    EXPECT_TRUE(fillColor == STYLE_PTR->GetFillColorValue());\
    EXPECT_TRUE(fillTransparency == STYLE_PTR->GetFillTransparency());\
    EXPECT_TRUE(horizontalPadding == STYLE_PTR->GetHorizontalPadding());\
    EXPECT_TRUE(isFillEnabled == STYLE_PTR->IsFillEnabled());\
    EXPECT_TRUE(isStrokeCloud == STYLE_PTR->IsStrokeCloud());\
    EXPECT_TRUE(isStrokeEnabled == STYLE_PTR->IsStrokeEnabled());\
    EXPECT_TRUE(strokecolortype == STYLE_PTR->GetStrokeColorType());\
    EXPECT_TRUE(strokeColor == STYLE_PTR->GetStrokeColorValue());\
    EXPECT_TRUE(strokeWeight == STYLE_PTR->GetStrokeWeight());\
    EXPECT_TRUE(type == STYLE_PTR->GetType());\
    EXPECT_TRUE(verticalPadding == STYLE_PTR->GetVerticalPadding());

//---------------------------------------------------------------------------------------
// Creates a style and tests accessors.
// @bsimethod                                                   Umar.Hayat     07/15
//---------------------------------------------------------------------------------------
TEST_F(AnnotationFrameStyleTest, DefaultsAndAccessors)
    {
    //.............................................................................................
    DgnDbR project = *GetDgnDb(L"DefaultsAndAccessors");

    //.............................................................................................
    AnnotationFrameStylePtr style = AnnotationFrameStyle::Create(project);
    ASSERT_TRUE(style.IsValid());

    // Basics
    EXPECT_TRUE(&project == &style->GetDgnDb());
    EXPECT_TRUE(!style->GetElementId().IsValid()); // Cannot call SetId directly from published API.

    // Defaults
    EXPECT_TRUE(style->GetName().empty());
    EXPECT_TRUE(style->GetDescription().empty());
    EXPECT_TRUE(0 == style->GetFillColorValue().GetValue());
    EXPECT_TRUE(AnnotationFrameType::InvisibleBox == style->GetType());
    EXPECT_TRUE(0 == style->GetStrokeWeight());

    // Set/Get round-trip
    DECLARE_AND_SET_DATA_1(style);
    VERIFY_DATA_1(style);
    }

//---------------------------------------------------------------------------------------
// Creates and clones a style.
// @bsimethod                                                   Umar.Hayat     07/15
//---------------------------------------------------------------------------------------
TEST_F(AnnotationFrameStyleTest, DeepCopy)
    {
    //.............................................................................................
    DgnDbR project = *GetDgnDb(L"DeepCopy");

    //.............................................................................................
    AnnotationFrameStylePtr style = AnnotationFrameStyle::Create(project);
    ASSERT_TRUE(style.IsValid());

    DECLARE_AND_SET_DATA_1(style);

    //.............................................................................................
    AnnotationFrameStylePtr clonedStyle = style->CreateCopy();
    ASSERT_TRUE(clonedStyle.IsValid());
    ASSERT_TRUE(style.get() != clonedStyle.get());
    
    EXPECT_TRUE(&project == &clonedStyle->GetDgnDb());
    EXPECT_TRUE(style->GetElementId() == clonedStyle->GetElementId());
    VERIFY_DATA_1(clonedStyle);
    }

//---------------------------------------------------------------------------------------
// Verifies persistence in the style table.
// @bsimethod                                                   Umar.Hayat     07/15
//---------------------------------------------------------------------------------------
TEST_F(AnnotationFrameStyleTest, TableReadWrite)
    {
    //.............................................................................................
    DgnDbR project = *GetDgnDb(L"TableReadWrite");

    //.............................................................................................
    // Verify initial state. This is expected to be blank, but update this and other count checks if the seed files changes.
    EXPECT_EQ(0, AnnotationFrameStyle::QueryCount(project));

    //.............................................................................................
    // Insert
    AnnotationFrameStylePtr testStyle = AnnotationFrameStyle::Create(project);
    DECLARE_AND_SET_DATA_1(testStyle);

    ASSERT_TRUE(testStyle->Insert().IsValid());
    ASSERT_TRUE(testStyle->GetElementId().IsValid());

    EXPECT_EQ(1, AnnotationFrameStyle::QueryCount(project));

    //.............................................................................................
    // Query
    EXPECT_TRUE(AnnotationFrameStyle::Get(project, testStyle->GetElementId()).IsValid());
    EXPECT_TRUE(AnnotationFrameStyle::Get(project, name.c_str()).IsValid());

    AnnotationFrameStyleCPtr fileStyle = AnnotationFrameStyle::Get(project, testStyle->GetElementId());
    ASSERT_TRUE(fileStyle.IsValid());
    
    EXPECT_TRUE(&project == &fileStyle->GetDgnDb());
    EXPECT_TRUE(testStyle->GetElementId() == fileStyle->GetElementId());
    VERIFY_DATA_1(fileStyle);

    fileStyle = AnnotationFrameStyle::Get(project, name.c_str());
    EXPECT_TRUE(fileStyle.IsValid());
    
    EXPECT_TRUE(&project == &fileStyle->GetDgnDb());
    EXPECT_TRUE(testStyle->GetElementId() == fileStyle->GetElementId());
    VERIFY_DATA_1(fileStyle);
    
    //.............................................................................................
    // Update
    AnnotationFrameStylePtr mutatedStyle = fileStyle->CreateCopy();
    ASSERT_TRUE(mutatedStyle.IsValid());

    name = "DifferentName"; mutatedStyle->SetName(name.c_str());
    fillColor = ColorDef(0x00, 0x00, 0xff); mutatedStyle->SetFillColorValue(fillColor);
    strokeWeight = 5; mutatedStyle->SetStrokeWeight(strokeWeight);
    strokeColor = ColorDef(0xff, 0xff, 0xf0); mutatedStyle->SetStrokeColorValue(strokeColor);
    isStrokeEnabled = true; mutatedStyle->SetIsStrokeEnabled(isStrokeEnabled);
    isStrokeCloud = true; mutatedStyle->SetIsStrokeCloud(isStrokeCloud);

    ASSERT_TRUE(mutatedStyle->Update().IsValid());

    EXPECT_EQ(1, AnnotationFrameStyle::QueryCount(project));

    fileStyle = AnnotationFrameStyle::Get(project, name.c_str());
    EXPECT_TRUE(fileStyle.IsValid());
    
    EXPECT_TRUE(&project == &fileStyle->GetDgnDb());
    EXPECT_TRUE(mutatedStyle->GetElementId() == fileStyle->GetElementId());
    VERIFY_DATA_1(fileStyle);
    
    //.............................................................................................
    // Iterate
    auto iter1 = AnnotationFrameStyle::MakeIterator(project);
    size_t numStyles = 0;
    for (auto const& style : iter1)
        {
        EXPECT_TRUE(!Utf8String::IsNullOrEmpty(style.GetName()));
        ASSERT_TRUE(style.GetElementId().IsValid());
        
        AnnotationFrameStyleCPtr iterStyle = AnnotationFrameStyle::Get(project, style.GetElementId());
        ASSERT_TRUE(iterStyle.IsValid());

        EXPECT_TRUE(0 == name.compare(iterStyle->GetName()));

        ++numStyles;
        }

    EXPECT_TRUE(1 == numStyles);

    auto iter2 = AnnotationFrameStyle::MakeIterator(project);
    numStyles = 0;

    for (auto const& style : iter2)
        {
        EXPECT_TRUE(!Utf8String::IsNullOrEmpty(style.GetName()));
        ASSERT_TRUE(style.GetElementId().IsValid());

        AnnotationFrameStyleCPtr iterStyle = AnnotationFrameStyle::Get(project, style.GetElementId());
        ASSERT_TRUE(iterStyle.IsValid());

        EXPECT_TRUE(0 == name.compare(iterStyle->GetName()));

        ++numStyles;
        }

    EXPECT_TRUE(1 == numStyles);

    //.............................................................................................
    // Delete
    auto styleToDelete = AnnotationFrameStyle::Get(project, mutatedStyle->GetElementId());
    ASSERT_TRUE(styleToDelete.IsValid());
    EXPECT_EQ(DgnDbStatus::DeletionProhibited, project.Elements().Delete(*styleToDelete));
    }
//---------------------------------------------------------------------------------------
// Verifies persistence in the style table.
// @bsimethod                                                   Umar.Hayat     07/15
//---------------------------------------------------------------------------------------
TEST_F(AnnotationFrameStyleTest, InvalidOperations)
    {
        //.............................................................................................
        DgnDbR project = *GetDgnDb(L"InvalidOperations");

        //.............................................................................................
        // Verify initial state. This is expected to be blank, but update this and other count checks if the seed files changes.
        EXPECT_EQ(0, AnnotationFrameStyle::QueryCount(project));

        //.............................................................................................
        // Insert
        AnnotationFrameStylePtr testStyle = AnnotationFrameStyle::Create(project);
        DECLARE_AND_SET_DATA_1(testStyle); 

        //.............................................................................................
        // Shoudl not exist before insert
        // Cannot pass an invalid ID to ExistsById... ASSERT_TRUE(false == project.Styles().AnnotationFrameStyles().ExistsById(testStyle->GetElementId()));
        ASSERT_FALSE(AnnotationFrameStyle::Get(project, testStyle->GetName().c_str()).IsValid());

        ASSERT_TRUE(testStyle->Insert().IsValid());
        ASSERT_TRUE(testStyle->GetElementId().IsValid());

        //.............................................................................................
        // Insert Redundant style
        AnnotationFrameStylePtr secondTestStyle = testStyle->CreateCopy();
        EXPECT_EQ(1, AnnotationFrameStyle::QueryCount(project));

        //.............................................................................................
        // Query
        EXPECT_TRUE(AnnotationFrameStyle::Get(project, testStyle->GetElementId()).IsValid());
        EXPECT_TRUE(AnnotationFrameStyle::Get(project, name.c_str()).IsValid());

        AnnotationFrameStyleCPtr fileStyle = AnnotationFrameStyle::Get(project, testStyle->GetElementId());
        ASSERT_TRUE(fileStyle.IsValid());

        EXPECT_TRUE(&project == &fileStyle->GetDgnDb());
        EXPECT_TRUE(testStyle->GetElementId() == fileStyle->GetElementId());
        VERIFY_DATA_1(fileStyle);

        fileStyle = AnnotationFrameStyle::Get(project, name.c_str());
        EXPECT_TRUE(fileStyle.IsValid());

        EXPECT_TRUE(&project == &fileStyle->GetDgnDb());
        EXPECT_TRUE(testStyle->GetElementId() == fileStyle->GetElementId());
        VERIFY_DATA_1(fileStyle);

        //.............................................................................................
        // Query Invalid
        EXPECT_FALSE(AnnotationFrameStyle::Get(project, "InvalidName").IsValid());

        //.............................................................................................
        // Update
        AnnotationFrameStylePtr mutatedStyle = fileStyle->CreateCopy();
        ASSERT_TRUE(mutatedStyle.IsValid());

        name = "DifferentName"; mutatedStyle->SetName(name.c_str());
        fillColor = ColorDef(0x00, 0x00, 0xff); mutatedStyle->SetFillColorValue(fillColor);
        strokeWeight = 5; mutatedStyle->SetStrokeWeight(strokeWeight);
        strokeColor = ColorDef(0xff, 0xff, 0xf0); mutatedStyle->SetStrokeColorValue(strokeColor);
        isStrokeEnabled = true; mutatedStyle->SetIsStrokeEnabled(isStrokeEnabled);
        isStrokeCloud = true; mutatedStyle->SetIsStrokeCloud(isStrokeCloud);

        ASSERT_TRUE(mutatedStyle->Update().IsValid());

        EXPECT_EQ(1, AnnotationFrameStyle::QueryCount(project));

        fileStyle = AnnotationFrameStyle::Get(project, name.c_str());
        EXPECT_TRUE(fileStyle.IsValid());

        EXPECT_TRUE(&project == &fileStyle->GetDgnDb());
        EXPECT_TRUE(mutatedStyle->GetElementId() == fileStyle->GetElementId());
        VERIFY_DATA_1(fileStyle);

        //.............................................................................................
        // Update
        AnnotationFrameStylePtr secondMutatedStyle = fileStyle->CreateCopy();
        secondMutatedStyle->SetName("DifferentName");

        //.............................................................................................
        // Iterate
        auto iter1 = AnnotationFrameStyle::MakeIterator(project);
        size_t numStyles = 0;
        for (auto const& style : iter1)
        {
            EXPECT_TRUE(!Utf8String::IsNullOrEmpty(style.GetName()));
            ASSERT_TRUE(style.GetElementId().IsValid());

            auto iterStyle = AnnotationFrameStyle::Get(project, style.GetElementId());
            ASSERT_TRUE(iterStyle.IsValid());

            EXPECT_TRUE(0 == name.compare(iterStyle->GetName()));

            ++numStyles;
        }

        EXPECT_TRUE(1 == numStyles);

        auto iter2 = AnnotationFrameStyle::MakeIterator(project);
        numStyles = 0;

        for (auto const& style : iter2)
        {
            EXPECT_TRUE(!Utf8String::IsNullOrEmpty(style.GetName()));
            ASSERT_TRUE(style.GetElementId().IsValid());

            auto iterStyle = AnnotationFrameStyle::Get(project, style.GetElementId());
            ASSERT_TRUE(iterStyle.IsValid());

            EXPECT_TRUE(0 == name.compare(iterStyle->GetName()));

            ++numStyles;
        }

        EXPECT_TRUE(1 == numStyles);


        //.............................................................................................
        // Delete
        auto styleToDelete = AnnotationFrameStyle::Get(project, mutatedStyle->GetElementId());
        ASSERT_TRUE(styleToDelete.IsValid());
        EXPECT_EQ(DgnDbStatus::DeletionProhibited, project.Elements().Delete(*styleToDelete));
    }
