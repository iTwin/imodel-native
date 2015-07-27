//-------------------------------------------------------------------------------------- 
//     $Source: Tests/DgnProject/Published/AnnotationLeader_Test.cpp $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 

#include "DgnHandlersTests.h"
#include <DgnPlatform/DgnCore/Annotations/Annotations.h>

// Republish API:           bb re DgnPlatform:PublishedApi
// Rebuild API:             bb re DgnPlatformDll
// Republish seed files:    bb re UnitTests_Documents
// Rebuild test:            bb re DgnProjectUnitTests BeGTestExe
// All code:                bb re DgnPlatform:PublishedApi DgnPlatformDll DgnProjectUnitTests BeGTestExe
// Run test:                %SrcRoot%BeGTest\RunTests.py -ax64 --gtest_filter="BasicAnnotationLeaderTest.*:AnnotationLeaderTest.*"

//=======================================================================================
// @bsiclass                                                   Umar.Hayat     07/15
//=======================================================================================
struct AnnotationLeaderTest : public AnnotationTestFixture
{
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Umar.Hayat     07/15
    //---------------------------------------------------------------------------------------
public: AnnotationLeaderTest() :
        GenericDgnModelTestFixture (__FILE__, false /*2D*/)
        {
        }

}; // AnnotationLeaderTest

#define DECLARE_AND_SET_DATA_1(STYLE_PTR)\
    Utf8String name = "MyStyle";                                                                STYLE_PTR->SetName(name.c_str());\
    Utf8String description = "MyDescription";                                                   STYLE_PTR->SetDescription(description.c_str());\
    ElementColor lineColor(ColorDef(0xff, 0x00, 0x00));                                         STYLE_PTR->SetLineColor(lineColor);\
    ElementColor terminatorColor(ColorDef(0xff, 0xff, 0x00));                                   STYLE_PTR->SetTerminatorColor(terminatorColor);\
    AnnotationLeaderLineType lineType = AnnotationLeaderLineType::Curved;                       STYLE_PTR->SetLineType(lineType);\
    uint32_t lineWeight=3;                                                                      STYLE_PTR->SetLineWeight(lineWeight); \
    double terminatorScaleFactor = 2.1;                                                         STYLE_PTR->SetTerminatorScaleFactor(terminatorScaleFactor);\
    AnnotationLeaderTerminatorType terminatorType = AnnotationLeaderTerminatorType::OpenArrow;  STYLE_PTR->SetTerminatorType(terminatorType); \
    uint32_t terminatorWeight = 3;                                                              STYLE_PTR->SetTerminatorWeight(terminatorWeight); 

#define VERIFY_DATA_1(STYLE_PTR)\
    EXPECT_TRUE(name.Equals(STYLE_PTR->GetName()));\
    EXPECT_TRUE(description.Equals(STYLE_PTR->GetDescription()));\
    EXPECT_TRUE(lineColor == STYLE_PTR->GetLineColor());\
    EXPECT_TRUE(terminatorColor == STYLE_PTR->GetTerminatorColor());\
    EXPECT_TRUE(lineType == STYLE_PTR->GetLineType());\
    EXPECT_TRUE(lineWeight == STYLE_PTR->GetLineWeight());\
    EXPECT_TRUE(terminatorScaleFactor == STYLE_PTR->GetTerminatorScaleFactor());\
    EXPECT_TRUE(terminatorType == STYLE_PTR->GetTerminatorType());\
    EXPECT_TRUE(terminatorWeight == STYLE_PTR->GetTerminatorWeight());

//---------------------------------------------------------------------------------------
// Creates a style and tests accessors.
// @bsimethod                                                   Umar.Hayat     07/15
//---------------------------------------------------------------------------------------
TEST_F(AnnotationLeaderTest, DefaultsAndAccessors)
    {
    //.............................................................................................
    ASSERT_TRUE(NULL != m_testDgnManager.GetDgnProjectP());
    DgnDbR project = *m_testDgnManager.GetDgnProjectP();

    //.............................................................................................
    AnnotationLeaderStylePtr style = createAnnotationLeaderStyle(project, "TestLeaderStyle");
    AnnotationLeaderPtr leader = AnnotationLeader::Create(project,style->GetId());
    ASSERT_TRUE(leader.IsValid());

    // Basics
    EXPECT_TRUE(&project == &leader->GetDbR());
    EXPECT_TRUE(style->GetId() == leader->GetStyleId());

    AnnotationLeaderPtr leader2 = AnnotationLeader::Create(project, style->GetId());
    ASSERT_TRUE(leader2.IsValid());

    // Basics
    EXPECT_TRUE(&project == &leader->GetDbR());
    leader->SetStyleId(style->GetId(),SetAnnotationLeaderStyleOptions::Default);
    leader->get
    leader->SetTargetAttachmentType(AnnotationLeaderTargetAttachmentType::PhysicalPoint);
    leader

    // Defaults
    EXPECT_TRUE(style->GetName().empty());

    }

//---------------------------------------------------------------------------------------
// Creates and clones a style.
// @bsimethod                                                   Umar.Hayat     07/15
//---------------------------------------------------------------------------------------
TEST_F(AnnotationLeaderTest, DeepCopy)
    {
    //.............................................................................................
    ASSERT_TRUE(NULL != m_testDgnManager.GetDgnProjectP());
    DgnDbR project = *m_testDgnManager.GetDgnProjectP();

    //.............................................................................................
    AnnotationLeaderPtr style = AnnotationLeader::Create(project);
    ASSERT_TRUE(style.IsValid());

    DECLARE_AND_SET_DATA_1(style);

    //.............................................................................................
    AnnotationLeaderPtr clonedStyle = style->Clone();
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
TEST_F(AnnotationLeaderTest, TableReadWrite)
    {
    //.............................................................................................
    ASSERT_TRUE(NULL != m_testDgnManager.GetDgnProjectP());
    DgnDbR project = *m_testDgnManager.GetDgnProjectP();

    //.............................................................................................
    // Verify initial state. This is expected to be blank, but update this and other count checks if the seed files changes.
    EXPECT_TRUE(0 == project.Styles().AnnotationLeaders().MakeIterator().QueryCount());

    //.............................................................................................
    // Insert
    AnnotationLeaderPtr testStyle = AnnotationLeader::Create(project);
    DECLARE_AND_SET_DATA_1(testStyle);

    ASSERT_TRUE(SUCCESS == project.Styles().AnnotationLeaders().Insert(*testStyle));
    ASSERT_TRUE(testStyle->GetId().IsValid());

    EXPECT_TRUE(1 == project.Styles().AnnotationLeaders().MakeIterator().QueryCount());

    //.............................................................................................
    // Query
    EXPECT_TRUE(project.Styles().AnnotationLeaders().ExistsById(testStyle->GetId()));
    EXPECT_TRUE(project.Styles().AnnotationLeaders().ExistsByName(name.c_str()));

    AnnotationLeaderPtr fileStyle = project.Styles().AnnotationLeaders().QueryById(testStyle->GetId());
    ASSERT_TRUE(fileStyle.IsValid());
    
    EXPECT_TRUE(&project == &fileStyle->GetDbR());
    EXPECT_TRUE(testStyle->GetId() == fileStyle->GetId());
    VERIFY_DATA_1(fileStyle);

    fileStyle = project.Styles().AnnotationLeaders().QueryByName(name.c_str());
    EXPECT_TRUE(fileStyle.IsValid());
    
    EXPECT_TRUE(&project == &fileStyle->GetDbR());
    EXPECT_TRUE(testStyle->GetId() == fileStyle->GetId());
    VERIFY_DATA_1(fileStyle);
    
    //.............................................................................................
    // Update
    AnnotationLeaderPtr mutatedStyle = fileStyle->Clone();
    ASSERT_TRUE(mutatedStyle.IsValid());

    name = "DifferentName"; mutatedStyle->SetName(name.c_str());
    lineColor = ElementColor(ColorDef(0x00, 0xff, 0x00)); mutatedStyle->SetLineColor(lineColor);
    lineWeight = 5; mutatedStyle->SetLineWeight(lineWeight);
    terminatorColor = ElementColor(ColorDef(0xff, 0xff, 0x00)); mutatedStyle->SetTerminatorColor(terminatorColor);

    ASSERT_TRUE(SUCCESS == project.Styles().AnnotationLeaders().Update(*mutatedStyle));

    EXPECT_TRUE(1 == project.Styles().AnnotationLeaders().MakeIterator().QueryCount());

    fileStyle = project.Styles().AnnotationLeaders().QueryByName(name.c_str());
    EXPECT_TRUE(fileStyle.IsValid());
    
    EXPECT_TRUE(&project == &fileStyle->GetDbR());
    EXPECT_TRUE(mutatedStyle->GetId() == fileStyle->GetId());
    VERIFY_DATA_1(fileStyle);
    
    //.............................................................................................
    // Iterate
    DgnAnnotationLeaders::Iterator iter1 = project.Styles().AnnotationLeaders().MakeIterator();
    size_t numStyles = 0;
    for (auto const& style : iter1)
        {
        EXPECT_TRUE(!Utf8String::IsNullOrEmpty(style.GetName()));
        ASSERT_TRUE(style.GetId().IsValid());
        
        AnnotationLeaderPtr iterStyle = project.Styles().AnnotationLeaders().QueryById(style.GetId());
        ASSERT_TRUE(iterStyle.IsValid());

        EXPECT_TRUE(0 == name.compare(iterStyle->GetName()));

        ++numStyles;
        }

    EXPECT_TRUE(1 == numStyles);

    DgnAnnotationLeaders::Iterator iter2 = project.Styles().AnnotationLeaders().MakeIterator();
    iter2.Params().SetWhere("ORDER BY Name");
    numStyles = 0;

    for (auto const& style : iter2)
        {
        EXPECT_TRUE(!Utf8String::IsNullOrEmpty(style.GetName()));
        ASSERT_TRUE(style.GetId().IsValid());

        AnnotationLeaderPtr iterStyle = project.Styles().AnnotationLeaders().QueryById(style.GetId());
        ASSERT_TRUE(iterStyle.IsValid());

        EXPECT_TRUE(0 == name.compare(iterStyle->GetName()));

        ++numStyles;
        }

    EXPECT_TRUE(1 == numStyles);

    //.............................................................................................
    // Delete
    ASSERT_TRUE(SUCCESS == project.Styles().AnnotationLeaders().Delete(mutatedStyle->GetId()));

    EXPECT_TRUE(0 == project.Styles().AnnotationLeaders().MakeIterator().QueryCount());
    EXPECT_TRUE(!project.Styles().AnnotationLeaders().ExistsById(testStyle->GetId()));
    EXPECT_TRUE(!project.Styles().AnnotationLeaders().ExistsByName(name.c_str()));
    }
