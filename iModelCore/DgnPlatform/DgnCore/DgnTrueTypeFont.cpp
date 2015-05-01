/*--------------------------------------------------------------------------------------+
|     $Source: DgnCore/DgnTrueTypeFont.cpp $
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <regex>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TRUETYPE_TABLES_H
#include FT_OUTLINE_H
#include FT_SFNT_NAMES_H
#include FT_TRUETYPE_IDS_H

#define FONT_LOG (*LoggingManager::GetLogger(L"DgnFont"))

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2015
//---------------------------------------------------------------------------------------
FT_Library DgnPlatformLib::Host::FontAdmin::_GetFreeTypeLibrary()
    {
    if (!m_triedToLoadFTLibrary)
        {
        m_triedToLoadFTLibrary = true;

        FT_Error loadStatus = FT_Init_FreeType(&m_ftLibrary);
        if ((nullptr == m_ftLibrary) || (FT_Err_Ok != loadStatus))
            {
            FONT_LOG.errorv("Failed to load the freetype2 library (error code %i). TrueType fonts may be unusable.", (int)loadStatus);
            BeAssert(false);
            }
        }

    return m_ftLibrary;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
DgnFontPtr DgnTrueTypeFont::Create(Utf8CP name, IDgnFontDataP data) { return new DgnTrueTypeFont(name, data); }
DgnFontPtr DgnTrueTypeFont::_Clone() const { return new DgnTrueTypeFont(*this); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnTrueTypeFont::_LayoutGlyphs(DgnGlyphLayoutResultR result, DgnGlyphLayoutContextCR context) const
    {
    BeAssert(false); // not implemented
    return ERROR;
    }
