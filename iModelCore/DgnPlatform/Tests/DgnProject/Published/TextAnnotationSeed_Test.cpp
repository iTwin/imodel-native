//-------------------------------------------------------------------------------------- 
//     $Source: Tests/DgnProject/Published/TextAnnotationSeed_Test.cpp $
//  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 

#include "AnnotationTestFixture.h"

//=======================================================================================
// @bsiclass                                                   Umar.Hayat     07/15
//=======================================================================================
struct TextAnnotationSeedTest : public AnnotationTestFixture
{

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
    Utf8String name = "MySeed";                                                                             STYLE_PTR->SetName(name.c_str());\
    Utf8String description = "MySeedDescription";                                                           STYLE_PTR->SetDescription(description.c_str());\
    AnnotationFrameStylePtr frameStyle = createAnnotationFrameStyle(project , "TestFrameStyle" );           STYLE_PTR->SetFrameStyleId(frameStyle->GetElementId());\
    AnnotationLeaderStylePtr leaderStyle = createAnnotationLeaderStyle(project, "TestLeaderStyle");         STYLE_PTR->SetLeaderStyleId(leaderStyle->GetElementId());\
    AnnotationTextStylePtr textStyle = createAnnotationTextStyle(project, "TestTextStyle");                 STYLE_PTR->SetTextStyleId(textStyle->GetElementId());

#define VERIFY_DATA_1(STYLE_PTR)\
    EXPECT_TRUE(name.Equals(STYLE_PTR->GetName()));\
    EXPECT_TRUE(description.Equals(STYLE_PTR->GetDescription()));\
    EXPECT_TRUE(frameStyle->GetElementId() == STYLE_PTR->GetFrameStyleId());\
    EXPECT_TRUE(leaderStyle->GetElementId() == STYLE_PTR->GetLeaderStyleId());\
    EXPECT_TRUE(textStyle->GetElementId() == STYLE_PTR->GetTextStyleId());

//---------------------------------------------------------------------------------------
// Creates a style and tests accessors.
// @bsimethod                                                   Umar.Hayat     07/15
//---------------------------------------------------------------------------------------
TEST_F(TextAnnotationSeedTest, DefaultsAndAccessors)
    {
    //.............................................................................................
    DgnDbR project = *GetDgnDb(L"DefaultsAndAccessors");

    //.............................................................................................
    TextAnnotationSeedPtr style = TextAnnotationSeed::Create(project);
    ASSERT_TRUE(style.IsValid());

    // Basics
    EXPECT_TRUE(&project == &style->GetDgnDb());
    EXPECT_TRUE(!style->GetElementId().IsValid()); // Cannot call SetId directly from published API.

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
// Creates and CreateCopys a style.
// @bsimethod                                                   Umar.Hayat     07/15
//---------------------------------------------------------------------------------------
TEST_F(TextAnnotationSeedTest, DeepCopy)
    {
    //.............................................................................................
    DgnDbR project = *GetDgnDb(L"DeepCopy");

    //.............................................................................................
    TextAnnotationSeedPtr style = TextAnnotationSeed::Create(project);
    ASSERT_TRUE(style.IsValid());

    DECLARE_AND_SET_DATA_1(style);

    //.............................................................................................
    TextAnnotationSeedPtr CreateCopydStyle = style->CreateCopy();
    ASSERT_TRUE(CreateCopydStyle.IsValid());
    ASSERT_TRUE(style.get() != CreateCopydStyle.get());
    
    EXPECT_TRUE(&project == &CreateCopydStyle->GetDgnDb());
    EXPECT_TRUE(style->GetElementId() == CreateCopydStyle->GetElementId());
    VERIFY_DATA_1(CreateCopydStyle);
    }

//---------------------------------------------------------------------------------------
// Verifies persistence in the style table.
// @bsimethod                                                   Umar.Hayat     07/15
//---------------------------------------------------------------------------------------
TEST_F(TextAnnotationSeedTest, TableReadWrite)
    {
    //.............................................................................................
    DgnDbR project = *GetDgnDb();

    //.............................................................................................
    // Verify initial state. This is expected to be blank, but update this and other count checks if the seed files changes.
    EXPECT_EQ(0, TextAnnotationSeed::QueryCount(project));

    //.............................................................................................
    // Insert
    TextAnnotationSeedPtr testStyle = TextAnnotationSeed::Create(project);
    DECLARE_AND_SET_DATA_1(testStyle);

    ASSERT_TRUE(testStyle->Insert().IsValid());
    ASSERT_TRUE(testStyle->GetElementId().IsValid());

    EXPECT_EQ(1, TextAnnotationSeed::QueryCount(project));

    //.............................................................................................
    // Query
    EXPECT_TRUE(TextAnnotationSeed::Get(project, testStyle->GetElementId()).IsValid());
    EXPECT_TRUE(TextAnnotationSeed::Get(project, name.c_str()).IsValid());

    auto fileStyle = TextAnnotationSeed::Get(project, testStyle->GetElementId());
    ASSERT_TRUE(fileStyle.IsValid());
    
    EXPECT_TRUE(&project == &fileStyle->GetDgnDb());
    EXPECT_TRUE(testStyle->GetElementId() == fileStyle->GetElementId());
    VERIFY_DATA_1(fileStyle);

    fileStyle = TextAnnotationSeed::Get(project, name.c_str());
    EXPECT_TRUE(fileStyle.IsValid());
    
    EXPECT_TRUE(&project == &fileStyle->GetDgnDb());
    EXPECT_TRUE(testStyle->GetElementId() == fileStyle->GetElementId());
    VERIFY_DATA_1(fileStyle);
    
    //.............................................................................................
    // Update
    TextAnnotationSeedPtr mutatedStyle = fileStyle->CreateCopy();
    ASSERT_TRUE(mutatedStyle.IsValid());

    name = "DifferentName"; mutatedStyle->SetName(name.c_str());

    ASSERT_TRUE(mutatedStyle->Update().IsValid());

    EXPECT_EQ(1, TextAnnotationSeed::QueryCount(project));

    fileStyle = TextAnnotationSeed::Get(project, name.c_str());
    EXPECT_TRUE(fileStyle.IsValid());
    
    EXPECT_TRUE(&project == &fileStyle->GetDgnDb());
    EXPECT_TRUE(mutatedStyle->GetElementId() == fileStyle->GetElementId());
    VERIFY_DATA_1(fileStyle);
    
    //.............................................................................................
    // Iterate
    auto iter1 = TextAnnotationSeed::MakeIterator(project);
    size_t numStyles = 0;
    for (auto const& style : iter1)
        {
        EXPECT_TRUE(!Utf8String::IsNullOrEmpty(style.GetName()));
        ASSERT_TRUE(style.GetElementId().IsValid());
        
        auto iterStyle = TextAnnotationSeed::Get(project, style.GetElementId());
        ASSERT_TRUE(iterStyle.IsValid());

        EXPECT_TRUE(0 == name.compare(iterStyle->GetName()));

        ++numStyles;
        }

    EXPECT_TRUE(1 == numStyles);

    auto iter2 = TextAnnotationSeed::MakeIterator(project);
    numStyles = 0;

    for (auto const& style : iter2)
        {
        EXPECT_TRUE(!Utf8String::IsNullOrEmpty(style.GetName()));
        ASSERT_TRUE(style.GetElementId().IsValid());

        auto iterStyle = TextAnnotationSeed::Get(project, style.GetElementId());
        ASSERT_TRUE(iterStyle.IsValid());

        EXPECT_TRUE(0 == name.compare(iterStyle->GetName()));

        ++numStyles;
        }

    EXPECT_TRUE(1 == numStyles);

    //.............................................................................................
    // Delete
    auto styleToDelete = TextAnnotationSeed::Get(project, mutatedStyle->GetElementId());
    ASSERT_TRUE(styleToDelete.IsValid());
    EXPECT_EQ(DgnDbStatus::DeletionProhibited, project.Elements().Delete(*styleToDelete));
    }

//---------------------------------------------------------------------------------------
// Verifies persistence in the style table.
// @bsimethod                                                   Umar.Hayat     07/15
//---------------------------------------------------------------------------------------
TEST_F(TextAnnotationSeedTest, InvalidOperations)
    {
        //.............................................................................................
        DgnDbR project = *GetDgnDb();

        //.............................................................................................
        // Verify initial state. This is expected to be blank, but update this and other count checks if the seed files changes.
        EXPECT_EQ(0, TextAnnotationSeed::QueryCount(project));

        //.............................................................................................
        // Insert
        TextAnnotationSeedPtr testStyle = TextAnnotationSeed::Create(project);
        DECLARE_AND_SET_DATA_1(testStyle); 

        //.............................................................................................
        // Shoudl not exist before insert
        ASSERT_FALSE(TextAnnotationSeed::Get(project, testStyle->GetName().c_str()).IsValid());

        ASSERT_TRUE(testStyle->Insert().IsValid());
        ASSERT_TRUE(testStyle->GetElementId().IsValid());

        //.............................................................................................
        // Insert Redundant style
        TextAnnotationSeedPtr secondTestStyle = testStyle->CreateCopy();
        EXPECT_EQ(1, TextAnnotationSeed::QueryCount(project));

        //.............................................................................................
        // Query
        EXPECT_TRUE(TextAnnotationSeed::Get(project, testStyle->GetElementId()).IsValid());
        EXPECT_TRUE(TextAnnotationSeed::Get(project, name.c_str()).IsValid());

        auto fileStyle = TextAnnotationSeed::Get(project, testStyle->GetElementId());
        ASSERT_TRUE(fileStyle.IsValid());

        EXPECT_TRUE(&project == &fileStyle->GetDgnDb());
        EXPECT_TRUE(testStyle->GetElementId() == fileStyle->GetElementId());
        VERIFY_DATA_1(fileStyle);

        fileStyle = TextAnnotationSeed::Get(project, name.c_str());
        EXPECT_TRUE(fileStyle.IsValid());

        EXPECT_TRUE(&project == &fileStyle->GetDgnDb());
        EXPECT_TRUE(testStyle->GetElementId() == fileStyle->GetElementId());
        VERIFY_DATA_1(fileStyle);


        //.............................................................................................
        // Query Invalid
        EXPECT_FALSE(TextAnnotationSeed::Get(project, "InvalidName").IsValid());

        //.............................................................................................
        // Update
        TextAnnotationSeedPtr mutatedStyle = fileStyle->CreateCopy();
        ASSERT_TRUE(mutatedStyle.IsValid());

        name = "DifferentName"; mutatedStyle->SetName(name.c_str());

        ASSERT_TRUE(mutatedStyle->Update().IsValid());

        EXPECT_EQ(1, TextAnnotationSeed::QueryCount(project));

        fileStyle = TextAnnotationSeed::Get(project, name.c_str());
        EXPECT_TRUE(fileStyle.IsValid());

        EXPECT_TRUE(&project == &fileStyle->GetDgnDb());
        EXPECT_TRUE(mutatedStyle->GetElementId() == fileStyle->GetElementId());
        VERIFY_DATA_1(fileStyle);

        //.............................................................................................
        // Update
        TextAnnotationSeedPtr secondMutatedStyle = fileStyle->CreateCopy();
        secondMutatedStyle->SetName("DifferentName");

        //.............................................................................................
        // Iterate
        auto iter1 = TextAnnotationSeed::MakeIterator(project);
        size_t numStyles = 0;
        for (auto const& style : iter1)
        {
            EXPECT_TRUE(!Utf8String::IsNullOrEmpty(style.GetName()));
            ASSERT_TRUE(style.GetElementId().IsValid());

            auto iterStyle = TextAnnotationSeed::Get(project, style.GetElementId());
            ASSERT_TRUE(iterStyle.IsValid());

            EXPECT_TRUE(0 == name.compare(iterStyle->GetName()));

            ++numStyles;
        }

        EXPECT_TRUE(1 == numStyles);

        auto iter2 = TextAnnotationSeed::MakeIterator(project);
        numStyles = 0;

        for (auto const& style : iter2)
        {
            EXPECT_TRUE(!Utf8String::IsNullOrEmpty(style.GetName()));
            ASSERT_TRUE(style.GetElementId().IsValid());

            auto iterStyle = TextAnnotationSeed::Get(project, style.GetElementId());
            ASSERT_TRUE(iterStyle.IsValid());

            EXPECT_TRUE(0 == name.compare(iterStyle->GetName()));

            ++numStyles;
        }

        EXPECT_TRUE(1 == numStyles);


        //.............................................................................................
        // Delete
        auto styleToDelete = TextAnnotationSeed::Get(project, mutatedStyle->GetElementId());
        ASSERT_TRUE(styleToDelete.IsValid());
        EXPECT_EQ(DgnDbStatus::DeletionProhibited, project.Elements().Delete(*styleToDelete));
        EXPECT_EQ(1, TextAnnotationSeed::QueryCount(project));
    }


