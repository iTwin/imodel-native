//-------------------------------------------------------------------------------------- 
//     $Source: Tests/DgnProject/Published/AnnotationFrame_Test.cpp $
//  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 

#include "AnnotationTestFixture.h"

// Republish API:           bb re DgnPlatform:PublishedApi
// Rebuild API:             bb re DgnPlatformDll
// Republish seed files:    bb re UnitTests_Documents
// Rebuild test:            bb re DgnPlatform:UnitTests-Published
// All code:                bb re DgnPlatform:PublishedApi DgnPlatformDll DgnPlatform:UnitTests-Published
// Run test:                %SrcRoot%BeGTest\RunTests.py -ax64 --gtest_filter="AnnotationFrameTest.*"

//=======================================================================================
// @bsiclass                                                   Umar.Hayat     01/16
//=======================================================================================
struct AnnotationFrameTest : public AnnotationTestFixture
{
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Umar.Hayat     01/16
    //---------------------------------------------------------------------------------------
public: AnnotationFrameTest() :
    AnnotationTestFixture(__FILE__, false /*2D*/, false /*needBriefcase*/)
        {
        }

}; // AnnotationFrameTest

//---------------------------------------------------------------------------------------
// Creates a style and tests accessors.
// @bsimethod                                                   Umar.Hayat     01/16
//---------------------------------------------------------------------------------------
TEST_F(AnnotationFrameTest, DefaultsAndAccessors)
    {
    //.............................................................................................
    ASSERT_TRUE(NULL != m_testDgnManager.GetDgnProjectP());
    DgnDbR project = *m_testDgnManager.GetDgnProjectP();

    //.............................................................................................
    AnnotationFrameStylePtr style = AnnotationTestFixture::createAnnotationFrameStyle(project, "TestFrameStyle");
    AnnotationFramePtr frame = AnnotationFrame::Create(project,style->GetElementId());
    ASSERT_TRUE(frame.IsValid());

    EXPECT_TRUE(&project == &frame->GetDbR());
    EXPECT_TRUE(style->GetElementId() == frame->GetStyleId());
    
    EXPECT_TRUE(&project == &frame->GetDbR());
    frame->SetStyleId(style->GetElementId(),SetAnnotationFrameStyleOptions::Default);
    EXPECT_TRUE(style->GetElementId() == frame->GetStyleId());

    AnnotationFramePtr frame2 = AnnotationFrame::Create(project);
    ASSERT_TRUE(frame2.IsValid());
    frame2->SetStyleId(style->GetElementId(), SetAnnotationFrameStyleOptions::Direct);
    EXPECT_TRUE(style->GetElementId() == frame->GetStyleId());
    
    }

//---------------------------------------------------------------------------------------
// Creates and clones a style.
// @bsimethod                                                   Umar.Hayat     01/16
//---------------------------------------------------------------------------------------
TEST_F(AnnotationFrameTest, DeepCopy)
    {
    //.............................................................................................
    ASSERT_TRUE(NULL != m_testDgnManager.GetDgnProjectP());
    DgnDbR project = *m_testDgnManager.GetDgnProjectP();

    //.............................................................................................
    AnnotationFramePtr frame = AnnotationFrame::Create(project);
    ASSERT_TRUE(frame.IsValid());

    //.............................................................................................
    AnnotationFramePtr clonedframe = frame->Clone();
    ASSERT_TRUE(clonedframe.IsValid());
    ASSERT_TRUE(frame.get() != clonedframe.get());
    
    EXPECT_TRUE(&project == &clonedframe->GetDbR());
    EXPECT_TRUE(frame->GetStyleId() == clonedframe->GetStyleId());
    
    }

