/*--------------------------------------------------------------------------------------+
|     $Source: DgnCore/DgnShxFont.cpp $
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnCore/DgnFontData.h>

#define LOG (*LoggingManager::GetLogger(L"DgnFont"))

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
DgnFontPtr DgnShxFont::Create(Utf8CP name, IDgnFontDataP data) { return new DgnShxFont(name, data); }
DgnFontPtr DgnShxFont::_Clone() const { return new DgnShxFont(*this); }
DgnShxFont::ShxType DgnShxFont::GetShxType() const { return const_cast<DgnShxFontP>(this)->GetDataP()->GetShxType(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnShxFont::_LayoutGlyphs(DgnGlyphLayoutResultR result, DgnGlyphLayoutContextCR context) const
    {
    BeAssert(false); // not implemented
    return ERROR;
    }
