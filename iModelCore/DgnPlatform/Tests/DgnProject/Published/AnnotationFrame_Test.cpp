//-------------------------------------------------------------------------------------- 
//     $Source: Tests/DgnProject/Published/AnnotationFrame_Test.cpp $
//  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 

#include "AnnotationTestFixture.h"

//=======================================================================================
// @bsiclass                                                   Umar.Hayat     01/16
//=======================================================================================
struct AnnotationFrameTest : public AnnotationTestFixture
{

}; // AnnotationFrameTest

//---------------------------------------------------------------------------------------
// Creates a style and tests accessors.
// @bsimethod                                                   Umar.Hayat     01/16
//---------------------------------------------------------------------------------------
TEST_F(AnnotationFrameTest, DefaultsAndAccessors)
    {
    //.............................................................................................
    DgnDbR project = *GetDgnDb();

    //.............................................................................................
    AnnotationFrameStylePtr style = createAnnotationFrameStyle(project, "TestFrameStyle");
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
    
    project.SaveChanges();
    }

//---------------------------------------------------------------------------------------
// Creates and clones a style.
// @bsimethod                                                   Umar.Hayat     01/16
//---------------------------------------------------------------------------------------
TEST_F(AnnotationFrameTest, DeepCopy)
    {
    //.............................................................................................
    DgnDbR project = *GetDgnDb();

    //.............................................................................................
    AnnotationFramePtr frame = AnnotationFrame::Create(project);
    ASSERT_TRUE(frame.IsValid());

    //.............................................................................................
    AnnotationFramePtr clonedframe = frame->Clone();
    ASSERT_TRUE(clonedframe.IsValid());
    ASSERT_TRUE(frame.get() != clonedframe.get());
    
    EXPECT_TRUE(&project == &clonedframe->GetDbR());
    EXPECT_TRUE(frame->GetStyleId() == clonedframe->GetStyleId());

    project.SaveChanges();
    }

