/*--------------------------------------------------------------------------------------+
|     $Source: DgnCore/DgnRscFont.cpp $
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnCore/DgnRscFontStructures.h>

#define FONT_LOG (*LoggingManager::GetLogger(L"DgnFont"))

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
DgnFontPtr DgnRscFont::Create(Utf8CP name, IDgnFontDataP data) { return new DgnRscFont(name, data); }
DgnFontPtr DgnRscFont::_Clone() const { return new DgnRscFont(*this); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnRscFont::_LayoutGlyphs(DgnGlyphLayoutResultR result, DgnGlyphLayoutContextCR context) const
    {
    BeAssert(false); // not implemented
    return ERROR;
    }
