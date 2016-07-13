//-------------------------------------------------------------------------------------- 
//     $Source: Tests/DgnProject/Published/TextString_Test.cpp $
//  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 

#include "DgnHandlersTests.h"
#include <DgnPlatform/TextStyleInterop.h>

// Rebuild API:             bb re DgnPlatform:PublicAPI DgnPlatform:PublishedApi
// Rebuild code:            bb re DgnPlatformDLL
// Rebuild test & harness:  bb re DgnProjectUnitTests BeGTestExe
// Rebuild everything:      bb re DgnPlatform:PublicAPI DgnPlatform:PublishedApi DgnPlatformDLL DgnProjectUnitTests BeGTestExe

USING_NAMESPACE_BENTLEY_DGN

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     07/2014
//=======================================================================================
struct TextStringTest : public GenericDgnModelTestFixture
{
    TextStringTest()  { }
    DgnDbR GetProjectR() { return *GetDgnDb(); }
};

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2014
//---------------------------------------------------------------------------------------
TEST_F(TextStringTest, SanityMeasure)
    {
    // Purpose: Create a TextString and ensure it was able to layout its glyph.
    // A non-blank range implies that the font was found, loaded from the DB/system, LayoutGlyphs succeeded, and TextString processed the results.
    // Exact floating point range has proven fuzzy in the past due to variations in font versions and operating systems.
    // It is sufficient for this sanity test to ensure a non-null/empty range was computed, not exactly what the range was. Writing more advanced range tests will be an art.

    TextStringStylePtr style = TextStringStyle::Create();
    style->SetFont(DgnFontManager::GetLastResortTrueTypeFont());
    style->SetSize(DPoint2d::From(1000.0, 1000.0));
    
    TextStringPtr text = TextString::Create();
    text->SetText("lorem ipsum");
    text->SetStyle(*style);

    DRange2d range = text->GetRange();
    EXPECT_TRUE(!range.IsNull());
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                            Umar.Hayat                 02/16
//---------------------------------------------------------------------------------------
TEST_F(TextStringTest, BoundingShape)
    {
    TextStringStylePtr style = TextStringStyle::Create();
    style->SetFont(DgnFontManager::GetLastResortTrueTypeFont());
    style->SetSize(DPoint2d::From(1000.0, 1000.0));

        {
        TextStringPtr text = TextString::Create();
        text->SetText("lorem ipsum");
        text->SetStyle(*style);
        DPoint3d point[4];
        text->ComputeBoundingShape(point, 0, 0);
        printf("%f , %f , %f \n", point[1].x, point[1].y, point[1].z);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                            Umar.Hayat                 02/16
//---------------------------------------------------------------------------------------
TEST_F(TextStringTest, TextStringToAnnotation)
    {
    TextStringStylePtr tss = TextStringStyle::Create();
    tss->SetFont(DgnFontManager::GetLastResortTrueTypeFont());
    tss->SetSize(DPoint2d::From(1000.0, 1000.0));
    AnnotationTextStyle ats(GetProjectR());
    ASSERT_TRUE(SUCCESS == TextStyleInterop::TextStringToAnnotation(GetProjectR(), ats, *tss));
    EXPECT_TRUE(GetProjectR().Fonts().FindId(tss->GetFont()) == ats.GetFontId());
    
    }
