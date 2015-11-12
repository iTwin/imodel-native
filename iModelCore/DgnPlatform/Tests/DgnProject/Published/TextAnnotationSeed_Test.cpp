//-------------------------------------------------------------------------------------- 
//     $Source: Tests/DgnProject/Published/TextAnnotationSeed_Test.cpp $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 

#include "AnnotationTestFixture.h"

// Republish API:           bb re DgnPlatform:PublishedApi
// Rebuild API:             bb re DgnPlatformDll
// Republish seed files:    bb re UnitTests_Documents
// Rebuild test:            bb re DgnProjectUnitTests BeGTestExe
// All code:                bb re DgnPlatform:PublishedApi DgnPlatformDll DgnProjectUnitTests BeGTestExe
// Run test:                %SrcRoot%BeGTest\RunTests.py -ax64 --gtest_filter="BasicTextAnnotationSeedTest.*:TextAnnotationSeedTest.*"

//=======================================================================================
// @bsiclass                                                   Umar.Hayat     07/15
//=======================================================================================
struct TextAnnotationSeedTest : public AnnotationTestFixture
{
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Umar.Hayat     07/15
    //---------------------------------------------------------------------------------------
public: TextAnnotationSeedTest() :
        AnnotationTestFixture (__FILE__, false /*2D*/, false /*needsBriefcase*/)
        {
        }

}; // TextAnnotationSeedTest

//---------------------------------------------------------------------------------------
// Verifies the mapping between TextAnnotationSeedProperty and data type in TextAnnotationSeedPropertyBag by ensuring it does not assert.
// Higher level TextAnnotationSeed tests provide otherwise good coverage for TextAnnotationSeedPropertyBag because styles uses a bag for their underlying storage.
// @bsimethod                                                   Umar.Hayat     07/15
//---------------------------------------------------------------------------------------
TEST(BasicTextAnnotationSeedTest, PropertyBagTypes)
    {
    TextAnnotationSeedPropertyBagPtr data = TextAnnotationSeedPropertyBag::Create();

    data->SetIntegerProperty(TextAnnotationSeedProperty::FrameStyleId, 2);
    data->SetIntegerProperty(TextAnnotationSeedProperty::LeaderStyleId, 1);
    data->SetIntegerProperty(TextAnnotationSeedProperty::TextStyleId, 1);
}

#define DECLARE_AND_SET_DATA_1(STYLE_PTR)\
    Utf8String name = "MySeed";                                                                STYLE_PTR->SetName(name.c_str());\
    Utf8String description = "MySeedDescription";                                                   STYLE_PTR->SetDescription(description.c_str());\
    AnnotationFrameStylePtr frameStyle = createAnnotationFrameStyle(project , "TestFrameStyle" );         STYLE_PTR->SetFrameStyleId(frameStyle->GetStyleId());\
    AnnotationLeaderStylePtr leaderStyle = createAnnotationLeaderStyle(project, "TestLeaderStyle");       STYLE_PTR->SetLeaderStyleId(leaderStyle->GetStyleId());\
    AnnotationTextStylePtr textStyle = createAnnotationTextStyle(project, "TestTextStyle");         STYLE_PTR->SetTextStyleId(textStyle->GetStyleId());

#define VERIFY_DATA_1(STYLE_PTR)\
    EXPECT_TRUE(name.Equals(STYLE_PTR->GetName()));\
    EXPECT_TRUE(description.Equals(STYLE_PTR->GetDescription()));\
    EXPECT_TRUE(frameStyle->GetStyleId() == STYLE_PTR->GetFrameStyleId());\
    EXPECT_TRUE(leaderStyle->GetStyleId() == STYLE_PTR->GetLeaderStyleId());\
    EXPECT_TRUE(textStyle->GetStyleId() == STYLE_PTR->GetTextStyleId());

//---------------------------------------------------------------------------------------
// Creates a style and tests accessors.
// @bsimethod                                                   Umar.Hayat     07/15
//---------------------------------------------------------------------------------------
TEST_F(TextAnnotationSeedTest, DefaultsAndAccessors)
    {
    //.............................................................................................
    ASSERT_TRUE(NULL != m_testDgnManager.GetDgnProjectP());
    DgnDbR project = *m_testDgnManager.GetDgnProjectP();

    //.............................................................................................
    TextAnnotationSeedPtr style = TextAnnotationSeed::Create(project);
    ASSERT_TRUE(style.IsValid());

    // Basics
    EXPECT_TRUE(&project == &style->GetDbR());
    EXPECT_TRUE(!style->GetSeedId().IsValid()); // Cannot call SetId directly from published API.

    // Defaults
    EXPECT_TRUE(style->GetName().empty());
    EXPECT_TRUE(style->GetDescription().empty());
    EXPECT_FALSE(style->GetFrameStyleId().IsValid());
    EXPECT_FALSE(style->GetLeaderStyleId().IsValid());
    EXPECT_FALSE(style->GetTextStyleId().IsValid());

    // Set/Get round-trip
    DECLARE_AND_SET_DATA_1(style);
    VERIFY_DATA_1(style);
    }

//---------------------------------------------------------------------------------------
// Creates and clones a style.
// @bsimethod                                                   Umar.Hayat     07/15
//---------------------------------------------------------------------------------------
TEST_F(TextAnnotationSeedTest, DeepCopy)
    {
    //.............................................................................................
    ASSERT_TRUE(NULL != m_testDgnManager.GetDgnProjectP());
    DgnDbR project = *m_testDgnManager.GetDgnProjectP();

    //.............................................................................................
    TextAnnotationSeedPtr style = TextAnnotationSeed::Create(project);
    ASSERT_TRUE(style.IsValid());

    DECLARE_AND_SET_DATA_1(style);

    //.............................................................................................
    TextAnnotationSeedPtr clonedStyle = style->Clone();
    ASSERT_TRUE(clonedStyle.IsValid());
    ASSERT_TRUE(style.get() != clonedStyle.get());
    
    EXPECT_TRUE(&project == &clonedStyle->GetDbR());
    EXPECT_TRUE(style->GetSeedId() == clonedStyle->GetSeedId());
    VERIFY_DATA_1(clonedStyle);
    }

//---------------------------------------------------------------------------------------
// Verifies persistence in the style table.
// @bsimethod                                                   Umar.Hayat     07/15
//---------------------------------------------------------------------------------------
TEST_F(TextAnnotationSeedTest, TableReadWrite)
    {
    //.............................................................................................
    ASSERT_TRUE(NULL != m_testDgnManager.GetDgnProjectP());
    DgnDbR project = *m_testDgnManager.GetDgnProjectP();

    //.............................................................................................
    // Verify initial state. This is expected to be blank, but update this and other count checks if the seed files changes.
    EXPECT_EQ(0, TextAnnotationSeed::QueryCount(project));

    //.............................................................................................
    // Insert
    TextAnnotationSeedPtr testStyle = TextAnnotationSeed::Create(project);
    DECLARE_AND_SET_DATA_1(testStyle);

    ASSERT_TRUE(testStyle->Insert().IsValid());
    ASSERT_TRUE(testStyle->GetSeedId().IsValid());

    EXPECT_EQ(1, TextAnnotationSeed::QueryCount(project));

    //.............................................................................................
    // Query
    EXPECT_TRUE(TextAnnotationSeed::ExistsById(testStyle->GetSeedId(), project));
    EXPECT_TRUE(TextAnnotationSeed::ExistsByName(name, project));

    auto fileStyle = TextAnnotationSeed::QuerySeed(testStyle->GetSeedId(), project);
    ASSERT_TRUE(fileStyle.IsValid());
    
    EXPECT_TRUE(&project == &fileStyle->GetDbR());
    EXPECT_TRUE(testStyle->GetSeedId() == fileStyle->GetSeedId());
    VERIFY_DATA_1(fileStyle);

    fileStyle = TextAnnotationSeed::QuerySeed(name, project);
    EXPECT_TRUE(fileStyle.IsValid());
    
    EXPECT_TRUE(&project == &fileStyle->GetDbR());
    EXPECT_TRUE(testStyle->GetSeedId() == fileStyle->GetSeedId());
    VERIFY_DATA_1(fileStyle);
    
    //.............................................................................................
    // Update
    TextAnnotationSeedPtr mutatedStyle = fileStyle->Clone();
    ASSERT_TRUE(mutatedStyle.IsValid());

    name = "DifferentName"; mutatedStyle->SetName(name.c_str());

    ASSERT_TRUE(mutatedStyle->Update().IsValid());

    EXPECT_EQ(1, TextAnnotationSeed::QueryCount(project));

    fileStyle = TextAnnotationSeed::QuerySeed(name, project);
    EXPECT_TRUE(fileStyle.IsValid());
    
    EXPECT_TRUE(&project == &fileStyle->GetDbR());
    EXPECT_TRUE(mutatedStyle->GetSeedId() == fileStyle->GetSeedId());
    VERIFY_DATA_1(fileStyle);
    
    //.............................................................................................
    // Iterate
    auto iter1 = TextAnnotationSeed::MakeIterator(project);
    size_t numStyles = 0;
    for (auto const& style : iter1)
        {
        EXPECT_TRUE(!Utf8String::IsNullOrEmpty(style.GetName()));
        ASSERT_TRUE(style.GetId().IsValid());
        
        auto iterStyle = TextAnnotationSeed::QuerySeed(style.GetId(), project);
        ASSERT_TRUE(iterStyle.IsValid());

        EXPECT_TRUE(0 == name.compare(iterStyle->GetName()));

        ++numStyles;
        }

    EXPECT_TRUE(1 == numStyles);

    auto iter2 = TextAnnotationSeed::MakeOrderedIterator(project);
    numStyles = 0;

    for (auto const& style : iter2)
        {
        EXPECT_TRUE(!Utf8String::IsNullOrEmpty(style.GetName()));
        ASSERT_TRUE(style.GetId().IsValid());

        auto iterStyle = TextAnnotationSeed::QuerySeed(style.GetId(), project);
        ASSERT_TRUE(iterStyle.IsValid());

        EXPECT_TRUE(0 == name.compare(iterStyle->GetName()));

        ++numStyles;
        }

    EXPECT_TRUE(1 == numStyles);

    //.............................................................................................
    // Delete
    auto styleToDelete = TextAnnotationSeed::QuerySeed(mutatedStyle->GetSeedId(), project);
    ASSERT_TRUE(styleToDelete.IsValid());
    EXPECT_EQ(DgnDbStatus::DeletionProhibited, styleToDelete->Delete());
    }

//---------------------------------------------------------------------------------------
// Verifies persistence in the style table.
// @bsimethod                                                   Umar.Hayat     07/15
//---------------------------------------------------------------------------------------
TEST_F(TextAnnotationSeedTest, InvalidOperations)
    {
        //.............................................................................................
        ASSERT_TRUE(NULL != m_testDgnManager.GetDgnProjectP());
        DgnDbR project = *m_testDgnManager.GetDgnProjectP();

        //.............................................................................................
        // Verify initial state. This is expected to be blank, but update this and other count checks if the seed files changes.
        EXPECT_EQ(0, TextAnnotationSeed::QueryCount(project));

        //.............................................................................................
        // Insert
        TextAnnotationSeedPtr testStyle = TextAnnotationSeed::Create(project);
        DECLARE_AND_SET_DATA_1(testStyle); 

        //.............................................................................................
        // Shoudl not exist before insert
        ASSERT_FALSE(TextAnnotationSeed::ExistsByName(testStyle->GetName(), project));

        ASSERT_TRUE(testStyle->Insert().IsValid());
        ASSERT_TRUE(testStyle->GetSeedId().IsValid());

        //.............................................................................................
        // Insert Redundant style
        TextAnnotationSeedPtr secondTestStyle = testStyle->Clone();
        EXPECT_EQ(1, TextAnnotationSeed::QueryCount(project));

        //.............................................................................................
        // Query
        EXPECT_TRUE(TextAnnotationSeed::ExistsById(testStyle->GetSeedId(), project));
        EXPECT_TRUE(TextAnnotationSeed::ExistsByName(name, project));

        auto fileStyle = TextAnnotationSeed::QuerySeed(testStyle->GetSeedId(), project);
        ASSERT_TRUE(fileStyle.IsValid());

        EXPECT_TRUE(&project == &fileStyle->GetDbR());
        EXPECT_TRUE(testStyle->GetSeedId() == fileStyle->GetSeedId());
        VERIFY_DATA_1(fileStyle);

        fileStyle = TextAnnotationSeed::QuerySeed(name, project);
        EXPECT_TRUE(fileStyle.IsValid());

        EXPECT_TRUE(&project == &fileStyle->GetDbR());
        EXPECT_TRUE(testStyle->GetSeedId() == fileStyle->GetSeedId());
        VERIFY_DATA_1(fileStyle);


        //.............................................................................................
        // Query Invalid
        EXPECT_FALSE(TextAnnotationSeed::ExistsByName("InvalidName", project));

        //.............................................................................................
        // Update
        TextAnnotationSeedPtr mutatedStyle = fileStyle->Clone();
        ASSERT_TRUE(mutatedStyle.IsValid());

        name = "DifferentName"; mutatedStyle->SetName(name.c_str());

        ASSERT_TRUE(mutatedStyle->Update().IsValid());

        EXPECT_EQ(1, TextAnnotationSeed::QueryCount(project));

        fileStyle = TextAnnotationSeed::QuerySeed(name, project);
        EXPECT_TRUE(fileStyle.IsValid());

        EXPECT_TRUE(&project == &fileStyle->GetDbR());
        EXPECT_TRUE(mutatedStyle->GetSeedId() == fileStyle->GetSeedId());
        VERIFY_DATA_1(fileStyle);

        //.............................................................................................
        // Update
        TextAnnotationSeedPtr secondMutatedStyle = fileStyle->Clone();
        secondMutatedStyle->SetName("DifferentName");

        //.............................................................................................
        // Iterate
        auto iter1 = TextAnnotationSeed::MakeIterator(project);
        size_t numStyles = 0;
        for (auto const& style : iter1)
        {
            EXPECT_TRUE(!Utf8String::IsNullOrEmpty(style.GetName()));
            ASSERT_TRUE(style.GetId().IsValid());

            auto iterStyle = TextAnnotationSeed::QuerySeed(style.GetId(), project);
            ASSERT_TRUE(iterStyle.IsValid());

            EXPECT_TRUE(0 == name.compare(iterStyle->GetName()));

            ++numStyles;
        }

        EXPECT_TRUE(1 == numStyles);

        auto iter2 = TextAnnotationSeed::MakeOrderedIterator(project);
        numStyles = 0;

        for (auto const& style : iter2)
        {
            EXPECT_TRUE(!Utf8String::IsNullOrEmpty(style.GetName()));
            ASSERT_TRUE(style.GetId().IsValid());

            auto iterStyle = TextAnnotationSeed::QuerySeed(style.GetId(), project);
            ASSERT_TRUE(iterStyle.IsValid());

            EXPECT_TRUE(0 == name.compare(iterStyle->GetName()));

            ++numStyles;
        }

        EXPECT_TRUE(1 == numStyles);


        //.............................................................................................
        // Delete
        auto styleToDelete = TextAnnotationSeed::QuerySeed(mutatedStyle->GetSeedId(), project);
        ASSERT_TRUE(styleToDelete.IsValid());
        EXPECT_EQ(DgnDbStatus::DeletionProhibited, styleToDelete->Delete());
        EXPECT_EQ(1, TextAnnotationSeed::QueryCount(project));
    }

