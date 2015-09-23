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
        AnnotationTestFixture (__FILE__, false /*2D*/)
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
    AnnotationFrameStylePtr frameStyle = createAnnotationFrameStyle(project , "TestFrameStyle" );         STYLE_PTR->SetFrameStyleId(frameStyle->GetId());\
    AnnotationLeaderStylePtr leaderStyle = createAnnotationLeaderStyle(project, "TestLeaderStyle");       STYLE_PTR->SetLeaderStyleId(leaderStyle->GetId());\
    AnnotationTextStylePtr textStyle = createAnnotationTextStyle(project, "TestTextStyle");         STYLE_PTR->SetTextStyleId(textStyle->GetId());

#define VERIFY_DATA_1(STYLE_PTR)\
    EXPECT_TRUE(name.Equals(STYLE_PTR->GetName()));\
    EXPECT_TRUE(description.Equals(STYLE_PTR->GetDescription()));\
    EXPECT_TRUE(frameStyle->GetId() == STYLE_PTR->GetFrameStyleId());\
    EXPECT_TRUE(leaderStyle->GetId() == STYLE_PTR->GetLeaderStyleId());\
    EXPECT_TRUE(textStyle->GetId() == STYLE_PTR->GetTextStyleId());

#define INVALIDSTYLEID DgnStyleId((uint64_t)-1)

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
    EXPECT_TRUE(!style->GetId().IsValid()); // Cannot call SetId directly from published API.

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
    EXPECT_TRUE(style->GetId() == clonedStyle->GetId());
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
    EXPECT_TRUE(0 == project.Styles().TextAnnotationSeeds().MakeIterator().QueryCount());

    //.............................................................................................
    // Insert
    TextAnnotationSeedPtr testStyle = TextAnnotationSeed::Create(project);
    DECLARE_AND_SET_DATA_1(testStyle);

    ASSERT_TRUE(SUCCESS == project.Styles().TextAnnotationSeeds().Insert(*testStyle));
    ASSERT_TRUE(testStyle->GetId().IsValid());

    EXPECT_TRUE(1 == project.Styles().TextAnnotationSeeds().MakeIterator().QueryCount());

    //.............................................................................................
    // Query
    EXPECT_TRUE(project.Styles().TextAnnotationSeeds().ExistsById(testStyle->GetId()));
    EXPECT_TRUE(project.Styles().TextAnnotationSeeds().ExistsByName(name.c_str()));

    TextAnnotationSeedPtr fileStyle = project.Styles().TextAnnotationSeeds().QueryById(testStyle->GetId());
    ASSERT_TRUE(fileStyle.IsValid());
    
    EXPECT_TRUE(&project == &fileStyle->GetDbR());
    EXPECT_TRUE(testStyle->GetId() == fileStyle->GetId());
    VERIFY_DATA_1(fileStyle);

    fileStyle = project.Styles().TextAnnotationSeeds().QueryByName(name.c_str());
    EXPECT_TRUE(fileStyle.IsValid());
    
    EXPECT_TRUE(&project == &fileStyle->GetDbR());
    EXPECT_TRUE(testStyle->GetId() == fileStyle->GetId());
    VERIFY_DATA_1(fileStyle);
    
    //.............................................................................................
    // Update
    TextAnnotationSeedPtr mutatedStyle = fileStyle->Clone();
    ASSERT_TRUE(mutatedStyle.IsValid());

    name = "DifferentName"; mutatedStyle->SetName(name.c_str());

    ASSERT_TRUE(SUCCESS == project.Styles().TextAnnotationSeeds().Update(*mutatedStyle));

    EXPECT_TRUE(1 == project.Styles().TextAnnotationSeeds().MakeIterator().QueryCount());

    fileStyle = project.Styles().TextAnnotationSeeds().QueryByName(name.c_str());
    EXPECT_TRUE(fileStyle.IsValid());
    
    EXPECT_TRUE(&project == &fileStyle->GetDbR());
    EXPECT_TRUE(mutatedStyle->GetId() == fileStyle->GetId());
    VERIFY_DATA_1(fileStyle);
    
    //.............................................................................................
    // Iterate
    DgnTextAnnotationSeeds::Iterator iter1 = project.Styles().TextAnnotationSeeds().MakeIterator();
    size_t numStyles = 0;
    for (auto const& style : iter1)
        {
        EXPECT_TRUE(!Utf8String::IsNullOrEmpty(style.GetName()));
        ASSERT_TRUE(style.GetId().IsValid());
        
        TextAnnotationSeedPtr iterStyle = project.Styles().TextAnnotationSeeds().QueryById(style.GetId());
        ASSERT_TRUE(iterStyle.IsValid());

        EXPECT_TRUE(0 == name.compare(iterStyle->GetName()));

        ++numStyles;
        }

    EXPECT_TRUE(1 == numStyles);

    DgnTextAnnotationSeeds::Iterator iter2 = project.Styles().TextAnnotationSeeds().MakeIterator();
    iter2.Params().SetWhere("ORDER BY Name");
    numStyles = 0;

    for (auto const& style : iter2)
        {
        EXPECT_TRUE(!Utf8String::IsNullOrEmpty(style.GetName()));
        ASSERT_TRUE(style.GetId().IsValid());

        TextAnnotationSeedPtr iterStyle = project.Styles().TextAnnotationSeeds().QueryById(style.GetId());
        ASSERT_TRUE(iterStyle.IsValid());

        EXPECT_TRUE(0 == name.compare(iterStyle->GetName()));

        ++numStyles;
        }

    EXPECT_TRUE(1 == numStyles);

    //.............................................................................................
    // Delete
    ASSERT_TRUE(SUCCESS == project.Styles().TextAnnotationSeeds().Delete(mutatedStyle->GetId()));

    EXPECT_TRUE(0 == project.Styles().TextAnnotationSeeds().MakeIterator().QueryCount());
    EXPECT_TRUE(!project.Styles().TextAnnotationSeeds().ExistsById(testStyle->GetId()));
    EXPECT_TRUE(!project.Styles().TextAnnotationSeeds().ExistsByName(name.c_str()));
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
        EXPECT_TRUE(0 == project.Styles().TextAnnotationSeeds().MakeIterator().QueryCount());

        //.............................................................................................
        // Insert
        TextAnnotationSeedPtr testStyle = TextAnnotationSeed::Create(project);
        DECLARE_AND_SET_DATA_1(testStyle); 

        //.............................................................................................
        // Shoudl not exist before insert
        // You cannot pass an invalid ID to ExistsById... ASSERT_TRUE(false == project.Styles().TextAnnotationSeeds().ExistsById(testStyle->GetId()));
        ASSERT_TRUE(false == project.Styles().TextAnnotationSeeds().ExistsByName(testStyle->GetName().c_str()));
        // You cannot pass an invalid ID to QueryById... ASSERT_TRUE(false == project.Styles().TextAnnotationSeeds().QueryById(testStyle->GetId()).IsValid());
        ASSERT_TRUE(false == project.Styles().TextAnnotationSeeds().QueryByName(testStyle->GetName().c_str()).IsValid());

        ASSERT_TRUE(SUCCESS == project.Styles().TextAnnotationSeeds().Insert(*testStyle));
        ASSERT_TRUE(testStyle->GetId().IsValid());

        //.............................................................................................
        // Insert Redundant style
        TextAnnotationSeedPtr secondTestStyle = testStyle->Clone();
        EXPECT_TRUE(1 == project.Styles().TextAnnotationSeeds().MakeIterator().QueryCount());

        //.............................................................................................
        // Query
        EXPECT_TRUE(project.Styles().TextAnnotationSeeds().ExistsById(testStyle->GetId()));
        EXPECT_TRUE(project.Styles().TextAnnotationSeeds().ExistsByName(name.c_str()));

        TextAnnotationSeedPtr fileStyle = project.Styles().TextAnnotationSeeds().QueryById(testStyle->GetId());
        ASSERT_TRUE(fileStyle.IsValid());

        EXPECT_TRUE(&project == &fileStyle->GetDbR());
        EXPECT_TRUE(testStyle->GetId() == fileStyle->GetId());
        VERIFY_DATA_1(fileStyle);

        fileStyle = project.Styles().TextAnnotationSeeds().QueryByName(name.c_str());
        EXPECT_TRUE(fileStyle.IsValid());

        EXPECT_TRUE(&project == &fileStyle->GetDbR());
        EXPECT_TRUE(testStyle->GetId() == fileStyle->GetId());
        VERIFY_DATA_1(fileStyle);


        //.............................................................................................
        // Query Invalid
        // You cannot pass an invalid ID to ExistsById... EXPECT_FALSE(project.Styles().TextAnnotationSeeds().ExistsById(INVALIDSTYLEID));
        EXPECT_FALSE(project.Styles().TextAnnotationSeeds().ExistsByName("InvalidName"));
        // You cannot pass an invalid ID to QueryById... EXPECT_FALSE(project.Styles().TextAnnotationSeeds().QueryById(INVALIDSTYLEID).IsValid());
        EXPECT_FALSE(project.Styles().TextAnnotationSeeds().QueryByName("InvalideName").IsValid());

        //.............................................................................................
        // Update
        TextAnnotationSeedPtr mutatedStyle = fileStyle->Clone();
        ASSERT_TRUE(mutatedStyle.IsValid());

        name = "DifferentName"; mutatedStyle->SetName(name.c_str());

        ASSERT_TRUE(SUCCESS == project.Styles().TextAnnotationSeeds().Update(*mutatedStyle));

        EXPECT_TRUE(1 == project.Styles().TextAnnotationSeeds().MakeIterator().QueryCount());

        fileStyle = project.Styles().TextAnnotationSeeds().QueryByName(name.c_str());
        EXPECT_TRUE(fileStyle.IsValid());

        EXPECT_TRUE(&project == &fileStyle->GetDbR());
        EXPECT_TRUE(mutatedStyle->GetId() == fileStyle->GetId());
        VERIFY_DATA_1(fileStyle);

        //.............................................................................................
        // Update
        TextAnnotationSeedPtr secondMutatedStyle = fileStyle->Clone();
        secondMutatedStyle->SetName("DifferentName");
        secondMutatedStyle->SetId(INVALIDSTYLEID);
        // Cannot pass an invalid ID to Update... ASSERT_TRUE(ERROR == project.Styles().TextAnnotationSeeds().Update(*secondMutatedStyle));

        //.............................................................................................
        // Iterate
        DgnTextAnnotationSeeds::Iterator iter1 = project.Styles().TextAnnotationSeeds().MakeIterator();
        size_t numStyles = 0;
        for (auto const& style : iter1)
        {
            EXPECT_TRUE(!Utf8String::IsNullOrEmpty(style.GetName()));
            ASSERT_TRUE(style.GetId().IsValid());

            TextAnnotationSeedPtr iterStyle = project.Styles().TextAnnotationSeeds().QueryById(style.GetId());
            ASSERT_TRUE(iterStyle.IsValid());

            EXPECT_TRUE(0 == name.compare(iterStyle->GetName()));

            ++numStyles;
        }

        EXPECT_TRUE(1 == numStyles);

        DgnTextAnnotationSeeds::Iterator iter2 = project.Styles().TextAnnotationSeeds().MakeIterator();
        iter2.Params().SetWhere("ORDER BY Name");
        numStyles = 0;

        for (auto const& style : iter2)
        {
            EXPECT_TRUE(!Utf8String::IsNullOrEmpty(style.GetName()));
            ASSERT_TRUE(style.GetId().IsValid());

            TextAnnotationSeedPtr iterStyle = project.Styles().TextAnnotationSeeds().QueryById(style.GetId());
            ASSERT_TRUE(iterStyle.IsValid());

            EXPECT_TRUE(0 == name.compare(iterStyle->GetName()));

            ++numStyles;
        }

        EXPECT_TRUE(1 == numStyles);


        //.............................................................................................
        // Delete by Invalid id 
        // Cannot pass an invalid ID to Delete... ASSERT_TRUE(ERROR == project.Styles().TextAnnotationSeeds().Delete(INVALIDSTYLEID));

        //.............................................................................................
        // Delete
        ASSERT_TRUE(SUCCESS == project.Styles().TextAnnotationSeeds().Delete(mutatedStyle->GetId()));

        EXPECT_TRUE(0 == project.Styles().TextAnnotationSeeds().MakeIterator().QueryCount());
        EXPECT_TRUE(!project.Styles().TextAnnotationSeeds().ExistsById(testStyle->GetId()));
        EXPECT_TRUE(!project.Styles().TextAnnotationSeeds().ExistsByName(name.c_str()));

    }
