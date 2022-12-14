//---------------------------------------------------------------------------------------------
//  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//  See LICENSE.md in the repository root for full copyright notice.
//---------------------------------------------------------------------------------------------

#include "DgnHandlersTests.h"
#include <DgnPlatform/TextStyleInterop.h>

USING_NAMESPACE_BENTLEY_DGN

//=======================================================================================
// @bsiclass
//=======================================================================================
struct TextStringTest : public DgnDbTestFixture
{
    TextStringTest()  { }
};

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(TextStringTest, SanityMeasure)
    {
    // Purpose: Create a TextString and ensure it was able to layout its glyph.
    // A non-blank range implies that the font was found, loaded from the DB/system, LayoutGlyphs succeeded, and TextString processed the results.
    // Exact floating point range has proven fuzzy in the past due to variations in font versions and operating systems.
    // It is sufficient for this sanity test to ensure a non-null/empty range was computed, not exactly what the range was. Writing more advanced range tests will be an art.
    SetupSeedProject();
    DgnDbR db = GetDgnDb();

    TextStringStylePtr style = TextStringStyle::Create();
    style->SetSize(DPoint2d::From(1000.0, 1000.0));

    TextStringPtr text = TextString::Create(db);
    text->SetText("lorem ipsum");
    text->SetStyle(*style);

    DRange2d range = text->GetRange();
    EXPECT_TRUE(!range.IsNull());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(TextStringTest, TextStringToAnnotation)
    {
    SetupSeedProject();
    DgnDbR db = GetDgnDb();
    TextStringStylePtr tss = TextStringStyle::Create();
    tss->SetFont(FontId((uint64_t) 1));
    tss->SetSize(DPoint2d::From(1000.0, 1000.0));
    AnnotationTextStyle ats(db.GetDictionaryModel());
    ASSERT_TRUE(SUCCESS == TextStyleInterop::TextStringToAnnotation(db, ats, *tss));
    EXPECT_TRUE(tss->GetFont() == ats.GetFontId());
    }
