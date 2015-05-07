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
    TextStringTest() : GenericDgnModelTestFixture (__FILE__, true /*3D*/) { }
    DgnDbR GetProjectR() { return *m_testDgnManager.GetDgnProjectP(); }
    DgnModelR GetModelR() { return *GetDgnModelP(); }
};

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2014
//---------------------------------------------------------------------------------------
TEST_F(TextStringTest, SanityMeasure)
    {
    TextStringStylePtr style = TextStringStyle::Create();
    style->SetFont(DgnFontManager::GetLastResortTrueTypeFont());
    style->SetSize(DPoint2d::From(1000.0, 1000.0));
    
    TextStringPtr text = TextString::Create();
    text->SetText("lorem ipsum");
    text->SetStyle(*style);

    DRange2d range = text->GetRange();
    EXPECT_TRUE(!range.IsNull());
    }
