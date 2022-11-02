//---------------------------------------------------------------------------------------------
//  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//  See LICENSE.md in the repository root for full copyright notice.
//---------------------------------------------------------------------------------------------

#include "AnnotationTestFixture.h"

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
AnnotationTextStylePtr AnnotationTestFixture::createAnnotationTextStyle(DgnDbR project , const char* styleName)
    {
    FontId ttFontId = project.Fonts().GetId(FontType::TrueType, "arial");
    EXPECT_TRUE(ttFontId.IsValid());

    AnnotationTextStylePtr docStyle = AnnotationTextStyle::Create(project.GetDictionaryModel());
    docStyle->SetName(styleName);
    docStyle->SetFontId(ttFontId);
    docStyle->SetHeight(11.0);

    EXPECT_TRUE(docStyle->Insert().IsValid());
    EXPECT_TRUE(docStyle->GetElementId().IsValid());

    return docStyle;
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
AnnotationTextRunPtr AnnotationTestFixture::createAnnotationTextRun(DgnDbR project , AnnotationTextStylePtr runStyle)
    {
        AnnotationTextRunPtr textRun = AnnotationTextRun::Create(project);
    textRun->SetStyleId(runStyle->GetElementId(), SetAnnotationTextStyleOptions::Default);
    return textRun;
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
AnnotationParagraphPtr AnnotationTestFixture::createAnnotationParagraph(DgnDbR project , AnnotationTextStylePtr parStyle, AnnotationTextRunPtr textRun)
    {
    AnnotationParagraphPtr par1 = AnnotationParagraph::Create(project);
    par1->SetStyleId(parStyle->GetElementId(), SetAnnotationTextStyleOptions::Default);
    par1->GetRunsR().push_back(textRun);
    return par1;
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
AnnotationTextBlockPtr AnnotationTestFixture::createAnnotationTextBlock(DgnDbR project , AnnotationTextStylePtr docStyle, AnnotationParagraphPtr par1)
    {
    AnnotationTextBlockPtr doc = AnnotationTextBlock::Create(project);
    doc->SetStyleId(docStyle->GetElementId(), SetAnnotationTextStyleOptions::Default);
    doc->GetParagraphsR().push_back(par1);
    return doc;
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
AnnotationLeaderStylePtr AnnotationTestFixture::createAnnotationLeaderStyle(DgnDbR project, const char* styleName)
    {
    //.............................................................................................
    AnnotationLeaderStylePtr style = AnnotationLeaderStyle::Create(project.GetDictionaryModel());
    EXPECT_TRUE(style.IsValid());
    style->SetName(styleName);

    EXPECT_TRUE(style->Insert().IsValid());
    EXPECT_TRUE(style->GetElementId().IsValid());
    return style;
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
AnnotationFrameStylePtr AnnotationTestFixture::createAnnotationFrameStyle(DgnDbR project, const char* styleName)
    {
    AnnotationFrameStylePtr style = AnnotationFrameStyle::Create(project.GetDictionaryModel());
    EXPECT_TRUE(style.IsValid());

    style->SetName(styleName);
    EXPECT_TRUE(style->Insert().IsValid());
    EXPECT_TRUE(style->GetElementId().IsValid());
    return style;
    }
