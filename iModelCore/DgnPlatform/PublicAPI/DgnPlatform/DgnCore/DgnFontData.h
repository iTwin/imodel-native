/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DgnFontData.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "../DgnPlatform.h"
#include "DgnDbTables.h"

typedef struct FT_FaceRec_* FT_Face; // Shield users from freetype.h because they have a bad include scheme.

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     03/2015
//=======================================================================================
struct IDgnFontData
{
    virtual ~IDgnFontData() {}
    virtual IDgnFontDataP _CloneWithoutData() = 0;
    virtual BentleyStatus _Embed(DgnFonts::DbFaceDataDirect&) = 0;
    virtual BentleyStatus _AddDataRef() = 0;
    virtual void _ReleaseDataRef() = 0;
};

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     03/2015
//=======================================================================================
struct IDgnTrueTypeFontData : IDgnFontData
{
    static BentleyStatus GetFamilyName(Utf8StringR familyName, FT_Face);
    virtual FT_Face _GetFaceP(DgnFontStyle) = 0;
};

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     03/2015
//=======================================================================================
struct IDgnRscFontData : IDgnFontData
{
    virtual BentleyStatus _ReadFontHeader(bvector<Byte>&) = 0;
    virtual BentleyStatus _ReadFractionMap(bvector<Byte>&) = 0;
    virtual BentleyStatus _ReadGlyphData(bvector<Byte>&, size_t offset, size_t size) = 0;
    virtual BentleyStatus _ReadGlyphDataOffsets(bvector<Byte>&) = 0;
    virtual BentleyStatus _ReadGlyphHeaders(bvector<Byte>&) = 0;
};

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     03/2015
//=======================================================================================
struct IDgnShxFontData : IDgnFontData
{
private:
    // This information is here for SHX (vs. DgnShxFont itself) because glyphs need access to sub-shape information, and I'd rather keep this low-level data here instead of expose it through the font.
    typedef bmap<DgnGlyph::T_Id, DgnShxFont::GlyphFPos> T_GlyphFPosCache;
    T_GlyphFPosCache m_glyphFPosCache;
    bool m_hasLoadedGlyphFPosCacheAndMetrics;
    Byte m_ascender;
    Byte m_descender;

    void LoadGlyphFPosCacheAndMetrics();
    void LoadNonUnicodeGlyphFPosCacheAndMetrics();
    void LoadUnicodeGlyphFPosCacheAndMetrics();

public:
    IDgnShxFontData() : m_hasLoadedGlyphFPosCacheAndMetrics(false), m_ascender(0), m_descender(0) {}
    virtual size_t _Read(void* buffer, size_t size, size_t count) = 0;
    virtual BentleyStatus _Seek(int64_t offset, BeFileSeekOrigin origin) = 0;
    virtual uint64_t _Tell() = 0;

    char GetNextChar() { char next; _Read(&next, sizeof(next), 1); return next; }
    uint16_t GetNextUInt16() { uint16_t next; _Read(&next, sizeof(next), 1); return next; }
    DgnShxFont::ShxType GetShxType();
    DgnShxFont::GlyphFPos const* GetGlyphFPos(DgnGlyph::T_Id);
    Byte GetAscender() { LoadGlyphFPosCacheAndMetrics(); return m_ascender; }   
    Byte GetDescender() { LoadGlyphFPosCacheAndMetrics(); return m_descender; }   
};

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     03/2015
//=======================================================================================
struct DgnFontPersistence : NonCopyableClass
{
    struct Db
    {
    private:
        static DgnFontPtr DgnTrueTypeFontFromDb(struct DgnFonts&, DgnFontId, Utf8CP name, Byte const* metadata, size_t metadataSize);
        static DgnFontPtr DgnRscFontFromDb(struct DgnFonts&, DgnFontId, Utf8CP name, Byte const* metadata, size_t metadataSize);
        static DgnFontPtr DgnShxFontFromDb(struct DgnFonts&, DgnFontId, Utf8CP name, Byte const* metadata, size_t metadataSize);
        static BentleyStatus DgnRscFontMetadataToDb(bvector<Byte>&, DgnRscFontCR);
        static BentleyStatus DgnShxFontMetadataToDb(bvector<Byte>&, DgnShxFontCR);

    public:
        DGNPLATFORM_EXPORT static DgnFontPtr FromDb(struct DgnFonts&, DgnFontId, DgnFontType, Utf8CP name, Byte const* metadata, size_t metadataSize);
        DGNPLATFORM_EXPORT static BentleyStatus MetadataToDb(bvector<Byte>&, DgnFontCR);
        DGNPLATFORM_EXPORT static BentleyStatus Embed(DgnFonts::DbFaceDataDirect&, DgnFontCR);
    };

    struct File
    {
        DGNPLATFORM_EXPORT static T_DgnFontPtrs FromTrueTypeFiles(bvector<BeFileName> const&, Utf8CP nameFilter);
        DGNPLATFORM_EXPORT static DgnFontPtr FromShxFile(BeFileNameCR);
    };

    struct OS
    {
        DGNPLATFORM_EXPORT static DgnFontPtr FromGlobalTrueTypeRegistry(Utf8CP name);
    };
    
    struct Missing
    {
        DGNPLATFORM_EXPORT static DgnFontPtr CreateMissingFont(DgnFontType, Utf8CP name);
    };
};

END_BENTLEY_DGN_NAMESPACE
