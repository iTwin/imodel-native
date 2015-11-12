/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/DgnProject/Published/AnnotationLeaderStyle_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <DgnPlatform/Annotations/Annotations.h>

// Republish API:           bb re DgnPlatform:PublishedApi
// Rebuild API:             bb re DgnPlatformDll
// Republish seed files:    bb re UnitTests_Documents
// Rebuild test:            bb re DgnProjectUnitTests BeGTestExe
// All code:                bb re DgnPlatform:PublishedApi DgnPlatformDll DgnProjectUnitTests BeGTestExe
// Run test:                %SrcRoot%BeGTest\RunTests.py -ax64 --gtest_filter="BasicAnnotationLeaderStyleTest.*:AnnotationLeaderStyleTest.*"

//=======================================================================================
// @bsiclass                                                   Umar.Hayat     07/15
//=======================================================================================
struct AnnotationLeaderStyleTest : public GenericDgnModelTestFixture
{
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Umar.Hayat     07/15
    //---------------------------------------------------------------------------------------
public: AnnotationLeaderStyleTest() :
        GenericDgnModelTestFixture (__FILE__, false /*2D*/, false /*needBriefcase*/)
        {
        }

}; // AnnotationLeaderStyleTest

//---------------------------------------------------------------------------------------
// Verifies the mapping between AnnotationLeaderStyleProperty and data type in AnnotationLeaderStylePropertyBag by ensuring it does not assert.
// Higher level AnnotationLeaderStyle tests provide otherwise good coverage for AnnotationLeaderStylePropertyBag because styles uses a bag for their underlying storage.
// @bsimethod                                                   Umar.Hayat     07/15
//---------------------------------------------------------------------------------------
TEST(BasicAnnotationLeaderStyleTest, PropertyBagTypes)
    {
    AnnotationLeaderStylePropertyBagPtr data = AnnotationLeaderStylePropertyBag::Create();

    data->SetIntegerProperty(AnnotationLeaderStyleProperty::LineColorValue, 2);
    data->SetIntegerProperty(AnnotationLeaderStyleProperty::LineType, (int64_t)AnnotationLeaderLineType::Straight);
    data->SetIntegerProperty(AnnotationLeaderStyleProperty::LineWeight, 2);
    data->SetIntegerProperty(AnnotationLeaderStyleProperty::TerminatorColorValue, 1);
    data->SetRealProperty(AnnotationLeaderStyleProperty::TerminatorScaleFactor, 2.0);
    data->SetIntegerProperty(AnnotationLeaderStyleProperty::TerminatorType, (int64_t)AnnotationLeaderTerminatorType::OpenArrow);
    data->SetIntegerProperty(AnnotationLeaderStyleProperty::TerminatorWeight, 1);
}

#define DECLARE_AND_SET_DATA_1(STYLE_PTR)\
    Utf8String name = "MyStyle";                                                                STYLE_PTR->SetName(name.c_str());\
    Utf8String description = "MyDescription";                                                   STYLE_PTR->SetDescription(description.c_str());\
    ColorDef lineColor(0xff, 0x00, 0x00);                                                       STYLE_PTR->SetLineColorValue(lineColor);\
    ColorDef terminatorColor(0xff, 0xff, 0x00);                                                 STYLE_PTR->SetTerminatorColorValue(terminatorColor);\
    AnnotationLeaderLineType lineType = AnnotationLeaderLineType::Curved;                       STYLE_PTR->SetLineType(lineType);\
    uint32_t lineWeight=3;                                                                      STYLE_PTR->SetLineWeight(lineWeight); \
    double terminatorScaleFactor = 2.1;                                                         STYLE_PTR->SetTerminatorScaleFactor(terminatorScaleFactor);\
    AnnotationLeaderTerminatorType terminatorType = AnnotationLeaderTerminatorType::OpenArrow;  STYLE_PTR->SetTerminatorType(terminatorType); \
    uint32_t terminatorWeight = 3;                                                              STYLE_PTR->SetTerminatorWeight(terminatorWeight); 

#define VERIFY_DATA_1(STYLE_PTR)\
    EXPECT_TRUE(name.Equals(STYLE_PTR->GetName()));\
    EXPECT_TRUE(description.Equals(STYLE_PTR->GetDescription()));\
    EXPECT_TRUE(lineColor == STYLE_PTR->GetLineColorValue());\
    EXPECT_TRUE(terminatorColor == STYLE_PTR->GetTerminatorColorValue());\
    EXPECT_TRUE(lineType == STYLE_PTR->GetLineType());\
    EXPECT_TRUE(lineWeight == STYLE_PTR->GetLineWeight());\
    EXPECT_TRUE(terminatorScaleFactor == STYLE_PTR->GetTerminatorScaleFactor());\
    EXPECT_TRUE(terminatorType == STYLE_PTR->GetTerminatorType());\
    EXPECT_TRUE(terminatorWeight == STYLE_PTR->GetTerminatorWeight());

//---------------------------------------------------------------------------------------
// Creates a style and tests accessors.
// @bsimethod                                                   Umar.Hayat     07/15
//---------------------------------------------------------------------------------------
TEST_F(AnnotationLeaderStyleTest, DefaultsAndAccessors)
    {
    //.............................................................................................
    ASSERT_TRUE(NULL != m_testDgnManager.GetDgnProjectP());
    DgnDbR project = *m_testDgnManager.GetDgnProjectP();

    //.............................................................................................
    AnnotationLeaderStylePtr style = AnnotationLeaderStyle::Create(project);
    ASSERT_TRUE(style.IsValid());

    // Basics
    EXPECT_TRUE(&project == &style->GetDbR());
    EXPECT_TRUE(!style->GetElementId().IsValid()); // Cannot call SetId directly from published API.

    // Defaults
    EXPECT_TRUE(style->GetName().empty());
    EXPECT_TRUE(style->GetDescription().empty());
    EXPECT_TRUE(0 == style->GetLineColorValue().GetValue());
    EXPECT_TRUE(AnnotationLeaderLineType::None == style->GetLineType());
    EXPECT_TRUE(0 == style->GetLineWeight());
    EXPECT_TRUE(0 == style->GetTerminatorColorValue().GetValue());
    EXPECT_TRUE(1.0 == style->GetTerminatorScaleFactor());
    EXPECT_TRUE(AnnotationLeaderTerminatorType::None == style->GetTerminatorType());
    EXPECT_TRUE(0 == style->GetTerminatorWeight());

    // Set/Get round-trip
    DECLARE_AND_SET_DATA_1(style);
    VERIFY_DATA_1(style);
    }

//---------------------------------------------------------------------------------------
// Creates and clones a style.
// @bsimethod                                                   Umar.Hayat     07/15
//---------------------------------------------------------------------------------------
TEST_F(AnnotationLeaderStyleTest, DeepCopy)
    {
    //.............................................................................................
    ASSERT_TRUE(NULL != m_testDgnManager.GetDgnProjectP());
    DgnDbR project = *m_testDgnManager.GetDgnProjectP();

    //.............................................................................................
    AnnotationLeaderStylePtr style = AnnotationLeaderStyle::Create(project);
    ASSERT_TRUE(style.IsValid());

    DECLARE_AND_SET_DATA_1(style);

    //.............................................................................................
    AnnotationLeaderStylePtr clonedStyle = style->Clone();
    ASSERT_TRUE(clonedStyle.IsValid());
    ASSERT_TRUE(style.get() != clonedStyle.get());
    
    EXPECT_TRUE(&project == &clonedStyle->GetDbR());
    EXPECT_TRUE(style->GetElementId() == clonedStyle->GetElementId());
    VERIFY_DATA_1(clonedStyle);
    }

//---------------------------------------------------------------------------------------
// Verifies persistence in the style table.
// @bsimethod                                                   Umar.Hayat     07/15
//---------------------------------------------------------------------------------------
TEST_F(AnnotationLeaderStyleTest, TableReadWrite)
    {
    //.............................................................................................
    ASSERT_TRUE(NULL != m_testDgnManager.GetDgnProjectP());
    DgnDbR project = *m_testDgnManager.GetDgnProjectP();

    //.............................................................................................
    // Verify initial state. This is expected to be blank, but update this and other count checks if the seed files changes.
    EXPECT_EQ(0, AnnotationLeaderStyle::QueryCount(project));

    //.............................................................................................
    // Insert
    AnnotationLeaderStylePtr testStyle = AnnotationLeaderStyle::Create(project);
    DECLARE_AND_SET_DATA_1(testStyle);

    ASSERT_TRUE(testStyle->Insert().IsValid());
    ASSERT_TRUE(testStyle->GetElementId().IsValid());

    EXPECT_EQ(1, AnnotationLeaderStyle::QueryCount(project));

    //.............................................................................................
    // Query
    EXPECT_TRUE(AnnotationLeaderStyle::ExistsById(testStyle->GetElementId(), project));
    EXPECT_TRUE(AnnotationLeaderStyle::ExistsByName(name, project));

    auto fileStyle = AnnotationLeaderStyle::QueryStyle(testStyle->GetElementId(), project);
    ASSERT_TRUE(fileStyle.IsValid());
    
    EXPECT_TRUE(&project == &fileStyle->GetDbR());
    EXPECT_TRUE(testStyle->GetElementId() == fileStyle->GetElementId());
    VERIFY_DATA_1(fileStyle);

    fileStyle = AnnotationLeaderStyle::QueryStyle(name, project);
    EXPECT_TRUE(fileStyle.IsValid());
    
    EXPECT_TRUE(&project == &fileStyle->GetDbR());
    EXPECT_TRUE(testStyle->GetElementId() == fileStyle->GetElementId());
    VERIFY_DATA_1(fileStyle);
    
    //.............................................................................................
    // Update
    AnnotationLeaderStylePtr mutatedStyle = fileStyle->Clone();
    ASSERT_TRUE(mutatedStyle.IsValid());

    name = "DifferentName"; mutatedStyle->SetName(name.c_str());
    lineColor = ColorDef(0x00, 0xff, 0x00); mutatedStyle->SetLineColorValue(lineColor);
    lineWeight = 5; mutatedStyle->SetLineWeight(lineWeight);
    terminatorColor = ColorDef(0xff, 0xff, 0x00); mutatedStyle->SetTerminatorColorValue(terminatorColor);

    ASSERT_TRUE(mutatedStyle->Update().IsValid());

    EXPECT_EQ(1, AnnotationLeaderStyle::QueryCount(project));

    fileStyle = AnnotationLeaderStyle::QueryStyle(name, project);
    EXPECT_TRUE(fileStyle.IsValid());
    
    EXPECT_TRUE(&project == &fileStyle->GetDbR());
    EXPECT_TRUE(mutatedStyle->GetElementId() == fileStyle->GetElementId());
    VERIFY_DATA_1(fileStyle);
    
    //.............................................................................................
    // Iterate
    auto iter1 = AnnotationLeaderStyle::MakeIterator(project);
    size_t numStyles = 0;
    for (auto const& style : iter1)
        {
        EXPECT_TRUE(!Utf8String::IsNullOrEmpty(style.GetName()));
        ASSERT_TRUE(style.GetId().IsValid());
        
        auto iterStyle = AnnotationLeaderStyle::QueryStyle(style.GetId(), project);
        ASSERT_TRUE(iterStyle.IsValid());

        EXPECT_TRUE(0 == name.compare(iterStyle->GetName()));

        ++numStyles;
        }

    EXPECT_TRUE(1 == numStyles);

    auto iter2 = AnnotationLeaderStyle::MakeOrderedIterator(project);
    numStyles = 0;

    for (auto const& style : iter2)
        {
        EXPECT_TRUE(!Utf8String::IsNullOrEmpty(style.GetName()));
        ASSERT_TRUE(style.GetId().IsValid());

        auto iterStyle = AnnotationLeaderStyle::QueryStyle(style.GetId(), project);
        ASSERT_TRUE(iterStyle.IsValid());

        EXPECT_TRUE(0 == name.compare(iterStyle->GetName()));

        ++numStyles;
        }

    EXPECT_TRUE(1 == numStyles);

    //.............................................................................................
    // Delete
    auto styleToDelete = AnnotationLeaderStyle::QueryStyle(mutatedStyle->GetElementId(), project);
    EXPECT_EQ(DgnDbStatus::DeletionProhibited, styleToDelete->Delete());
    }

