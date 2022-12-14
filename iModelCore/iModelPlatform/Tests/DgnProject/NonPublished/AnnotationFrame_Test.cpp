//---------------------------------------------------------------------------------------------
//  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//  See LICENSE.md in the repository root for full copyright notice.
//---------------------------------------------------------------------------------------------

#include "AnnotationTestFixture.h"

//=======================================================================================
// @bsiclass
//=======================================================================================
struct AnnotationFrameTest : public AnnotationTestFixture
{

}; // AnnotationFrameTest

//---------------------------------------------------------------------------------------
// Creates a style and tests accessors.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(AnnotationFrameTest, DefaultsAndAccessors)
    {
    //.............................................................................................
    DgnDbR project = *GetDgnDb(L"DefaultsAndAccessors");

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
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(AnnotationFrameTest, DeepCopy)
    {
    //.............................................................................................
    DgnDbR project = *GetDgnDb(L"DeepCopy");

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

