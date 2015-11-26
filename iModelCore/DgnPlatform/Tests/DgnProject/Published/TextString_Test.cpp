//-------------------------------------------------------------------------------------- 
//     $Source: Tests/DgnProject/Published/TextString_Test.cpp $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 

#include "DgnHandlersTests.h"

// Rebuild API:             bb re DgnPlatform:PublicAPI DgnPlatform:PublishedApi
// Rebuild code:            bb re DgnPlatformDLL
// Rebuild test & harness:  bb re DgnProjectUnitTests BeGTestExe
// Rebuild everything:      bb re DgnPlatform:PublicAPI DgnPlatform:PublishedApi DgnPlatformDLL DgnProjectUnitTests BeGTestExe

USING_NAMESPACE_BENTLEY_DGNPLATFORM

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     07/2014
//=======================================================================================
struct TextStringTest : public GenericDgnModelTestFixture
{
    TextStringTest() : GenericDgnModelTestFixture (__FILE__, true /*3D*/, false /*needsBriefcase*/) { }
    DgnDbR GetProjectR() { return *m_testDgnManager.GetDgnProjectP(); }
    DgnModelR GetModelR() { return *GetDgnModelP(); }
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
