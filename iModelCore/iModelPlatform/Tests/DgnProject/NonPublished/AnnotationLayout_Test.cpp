﻿//---------------------------------------------------------------------------------------------
//  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//  See LICENSE.md in the repository root for full copyright notice.
//---------------------------------------------------------------------------------------------

#include "AnnotationTestFixture.h"

//=======================================================================================
// @bsiclass
//=======================================================================================
struct AnnotationLayoutTest : public GenericDgnModelTestFixture
{

}; 
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(AnnotationLayoutTest, AnnotationFrameLayout)
    {
    DgnDbR db = *GetDgnDb(L"AnnotationFrameLayout");
    AnnotationFrameStylePtr frameStyle = AnnotationTestFixture::createAnnotationFrameStyle(db, "TestFrameStyle");
    AnnotationFramePtr frame = AnnotationFrame::Create(db, frameStyle->GetElementId());
    ASSERT_TRUE(frame.IsValid());

    static Utf8CP ANNOTATION_TEXT_1 = "Hello world.";
    AnnotationTextStylePtr   textStyle = AnnotationTestFixture::createAnnotationTextStyle(db, "TesttextStyle");
    AnnotationTextBlockPtr textBlock = AnnotationTextBlock::Create(db, textStyle->GetElementId(), ANNOTATION_TEXT_1);
    AnnotationTextBlockLayoutPtr docLayout = AnnotationTextBlockLayout::Create(*textBlock);
    AnnotationFrameLayoutPtr frameLayout = AnnotationFrameLayout::Create(*frame,*docLayout);

    size_t count = frameLayout->GetAttachmentIdCount();
    EXPECT_EQ(8, count);
    }
