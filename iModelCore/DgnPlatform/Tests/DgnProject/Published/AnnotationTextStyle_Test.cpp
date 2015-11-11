//-------------------------------------------------------------------------------------- 
//     $Source: Tests/DgnProject/Published/AnnotationTextStyle_Test.cpp $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 

#include "DgnHandlersTests.h"
#include <DgnPlatform/Annotations/Annotations.h>

// Republish API:           bb re DgnPlatform:PublishedApi
// Rebuild API:             bb re DgnPlatformDll
// Republish seed files:    bb re UnitTests_Documents
// Rebuild test:            bb re DgnProjectUnitTests BeGTestExe
// All code:                bb re DgnPlatform:PublishedApi DgnPlatformDll DgnProjectUnitTests BeGTestExe
// Run test:                %SrcRoot%BeGTest\RunTests.py -ax64 --gtest_filter="BasicAnnotationTextStyleTest.*:AnnotationTextStyleTest.*"

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
class AnnotationTextStyleTest : public GenericDgnModelTestFixture
{
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     05/2014
    //---------------------------------------------------------------------------------------
    public: AnnotationTextStyleTest () :
        GenericDgnModelTestFixture (__FILE__, false /*2D*/, false /*needBriefcase*/)
        {
        }

}; // AnnotationTextStyleTest

//---------------------------------------------------------------------------------------
// Verifies the mapping between AnnotationTextStyleProperty and data type in AnnotationTextStylePropertyBag by ensuring it does not assert.
// Higher level AnnotationTextStyle tests provide otherwise good coverage for AnnotationTextStylePropertyBag because styles uses a bag for their underlying storage.
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
TEST(BasicAnnotationTextStyleTest, PropertyBagTypes)
    {
    AnnotationTextStylePropertyBagPtr data = AnnotationTextStylePropertyBag::Create();

    data->SetIntegerProperty(AnnotationTextStyleProperty::ColorValue, 2);
    data->SetIntegerProperty(AnnotationTextStyleProperty::FontId, 2);
    data->SetRealProperty(AnnotationTextStyleProperty::Height, 2.0);
    data->SetIntegerProperty(AnnotationTextStyleProperty::IsBold, 1);
    data->SetIntegerProperty(AnnotationTextStyleProperty::IsBold, 1);
    data->SetIntegerProperty(AnnotationTextStyleProperty::IsItalic, 2);
    data->SetIntegerProperty(AnnotationTextStyleProperty::IsItalic, 1);
    data->SetIntegerProperty(AnnotationTextStyleProperty::IsUnderlined, 1);
    data->SetIntegerProperty(AnnotationTextStyleProperty::IsUnderlined, 1);
    data->SetIntegerProperty(AnnotationTextStyleProperty::StackedFractionType, (uint32_t)AnnotationStackedFractionType::DiagonalBar);
    data->SetRealProperty(AnnotationTextStyleProperty::WidthFactor, 2.0);
    }

#define DECLARE_AND_SET_DATA_1(STYLE_PTR)\
    Utf8String name = "MyStyle";                                                                STYLE_PTR->SetName(name.c_str());\
    Utf8String description = "MyDescription";                                                   STYLE_PTR->SetDescription(description.c_str());\
    ColorDef color(0xff, 0x00, 0x00);                                                           STYLE_PTR->SetColorValue(color);\
    DgnFontId fontId((uint64_t)21);                                                              STYLE_PTR->SetFontId(fontId);\
    double height = 31.31;                                                                      STYLE_PTR->SetHeight(height);\
    bool isBold = true;                                                                         STYLE_PTR->SetIsBold(isBold);\
    bool isItalic = true;                                                                       STYLE_PTR->SetIsItalic(isItalic);\
    bool isUnderlined = true;                                                                   STYLE_PTR->SetIsUnderlined(isUnderlined);\
    AnnotationStackedFractionType fractionType = AnnotationStackedFractionType::DiagonalBar;    STYLE_PTR->SetStackedFractionType(fractionType);\
    double widthFactor = 41.41;                                                                 STYLE_PTR->SetWidthFactor(widthFactor);

#define VERIFY_DATA_1(STYLE_PTR)\
    EXPECT_TRUE(name.Equals(STYLE_PTR->GetName()));\
    EXPECT_TRUE(description.Equals(STYLE_PTR->GetDescription()));\
    EXPECT_TRUE(color == STYLE_PTR->GetColorValue());\
    EXPECT_TRUE(fontId == STYLE_PTR->GetFontId());\
    EXPECT_TRUE(height == STYLE_PTR->GetHeight());\
    EXPECT_TRUE(isBold == STYLE_PTR->IsBold());\
    EXPECT_TRUE(isItalic == STYLE_PTR->IsItalic());\
    EXPECT_TRUE(isUnderlined == STYLE_PTR->IsUnderlined());\
    EXPECT_TRUE(fractionType == STYLE_PTR->GetStackedFractionType());\
    EXPECT_TRUE(widthFactor == STYLE_PTR->GetWidthFactor());

//---------------------------------------------------------------------------------------
// Creates a style and tests accessors.
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
TEST_F(AnnotationTextStyleTest, DefaultsAndAccessors)
    {
    //.............................................................................................
    ASSERT_TRUE(NULL != m_testDgnManager.GetDgnProjectP());
    DgnDbR project = *m_testDgnManager.GetDgnProjectP();

    //.............................................................................................
    AnnotationTextStylePtr style = AnnotationTextStyle::Create(project);
    ASSERT_TRUE(style.IsValid());

    // Basics
    EXPECT_TRUE(&project == &style->GetDgnDb());
    EXPECT_TRUE(!style->GetStyleId().IsValid()); // Cannot call SetId directly from published API.

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
    ASSERT_TRUE(NULL != m_testDgnManager.GetDgnProjectP());
    DgnDbR project = *m_testDgnManager.GetDgnProjectP();

    //.............................................................................................
    AnnotationTextStylePtr style = AnnotationTextStyle::Create(project);
    ASSERT_TRUE(style.IsValid());

    DECLARE_AND_SET_DATA_1(style);

    //.............................................................................................
    AnnotationTextStylePtr clonedStyle = style->Clone();
    ASSERT_TRUE(clonedStyle.IsValid());
    ASSERT_TRUE(style.get() != clonedStyle.get());
    
    EXPECT_TRUE(&project == &clonedStyle->GetDgnDb());
    EXPECT_TRUE(style->GetStyleId() == clonedStyle->GetStyleId());
    VERIFY_DATA_1(clonedStyle);
    }

//---------------------------------------------------------------------------------------
// Verifies persistence in the style table.
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
TEST_F(AnnotationTextStyleTest, TableReadWrite)
    {
    //.............................................................................................
    ASSERT_TRUE(NULL != m_testDgnManager.GetDgnProjectP());
    DgnDbR project = *m_testDgnManager.GetDgnProjectP();

    //.............................................................................................
    // Verify initial state.
	// The GenericDgnModelTestFixture will open 2dMetricGeneral, which contains a text element with style (none), thus there will be 1 style (the equivalent of style (none)).
	static const size_t SEED_STYLE_COUNT = 1;
        EXPECT_EQ((0 + SEED_STYLE_COUNT), AnnotationTextStyle::QueryCount(project));

    //.............................................................................................
    // Insert
    AnnotationTextStylePtr testStyle = AnnotationTextStyle::Create(project);
    DECLARE_AND_SET_DATA_1(testStyle);

    ASSERT_TRUE(testStyle->Insert().IsValid());
    ASSERT_TRUE(testStyle->GetStyleId().IsValid());

	EXPECT_EQ((1 + SEED_STYLE_COUNT), AnnotationTextStyle::QueryCount(project));

    //.............................................................................................
    // Query
    EXPECT_TRUE(AnnotationTextStyle::ExistsById(testStyle->GetStyleId(), project));
    EXPECT_TRUE(AnnotationTextStyle::ExistsByName(name.c_str(), project));

    AnnotationTextStyleCPtr fileStyle = AnnotationTextStyle::QueryStyle(testStyle->GetStyleId(), project);
    ASSERT_TRUE(fileStyle.IsValid());
    
    EXPECT_TRUE(&project == &fileStyle->GetDgnDb());
    EXPECT_TRUE(testStyle->GetStyleId() == fileStyle->GetStyleId());
    VERIFY_DATA_1(fileStyle);

    fileStyle = AnnotationTextStyle::QueryStyle(name, project);
    EXPECT_TRUE(fileStyle.IsValid());
    
    EXPECT_TRUE(&project == &fileStyle->GetDgnDb());
    EXPECT_TRUE(testStyle->GetStyleId() == fileStyle->GetStyleId());
    VERIFY_DATA_1(fileStyle);
    
    //.............................................................................................
    // Update
    AnnotationTextStylePtr mutatedStyle = fileStyle->Clone();
    ASSERT_TRUE(mutatedStyle.IsValid());

    name = "DifferentName"; mutatedStyle->SetName(name.c_str());
    color = ColorDef(0x00, 0xff, 0x00); mutatedStyle->SetColorValue(color);
    height *= 100.0; mutatedStyle->SetHeight(height);
    isBold = false; mutatedStyle->SetIsBold(isBold);

    ASSERT_TRUE(mutatedStyle->Update().IsValid());

	EXPECT_EQ((1 + SEED_STYLE_COUNT), AnnotationTextStyle::QueryCount(project));

    fileStyle = AnnotationTextStyle::QueryStyle(name, project);
    EXPECT_TRUE(fileStyle.IsValid());
    
    EXPECT_TRUE(&project == &fileStyle->GetDgnDb());
    EXPECT_TRUE(mutatedStyle->GetStyleId() == fileStyle->GetStyleId());
    VERIFY_DATA_1(fileStyle);
    
    //.............................................................................................
    // Iterate
    auto iter1 = AnnotationTextStyle::MakeIterator(project);
    size_t numStyles = 0;
	bool foundOurStyle = false;

    for (auto const& style : iter1)
        {
        EXPECT_TRUE(!Utf8String::IsNullOrEmpty(style.GetName()));
        ASSERT_TRUE(style.GetId().IsValid());
        
        AnnotationTextStyleCPtr iterStyle = AnnotationTextStyle::QueryStyle(style.GetId(), project);
        ASSERT_TRUE(iterStyle.IsValid());

		if (!foundOurStyle)
			foundOurStyle = (0 == name.compare(iterStyle->GetName()));

        ++numStyles;
        }

	EXPECT_EQ((1 + SEED_STYLE_COUNT), numStyles);
	EXPECT_TRUE(foundOurStyle);

    auto iter2 = AnnotationTextStyle::MakeOrderedIterator(project);
    numStyles = 0;
	foundOurStyle = false;
    
	for (auto const& style : iter2)
        {
        EXPECT_TRUE(!Utf8String::IsNullOrEmpty(style.GetName()));
        ASSERT_TRUE(style.GetId().IsValid());

        auto iterStyle = AnnotationTextStyle::QueryStyle(style.GetId(), project);
        ASSERT_TRUE(iterStyle.IsValid());

		if (!foundOurStyle)
			foundOurStyle = (0 == name.compare(iterStyle->GetName()));

        ++numStyles;
        }

	EXPECT_EQ((1 + SEED_STYLE_COUNT), numStyles);
	EXPECT_TRUE(foundOurStyle);
    }

