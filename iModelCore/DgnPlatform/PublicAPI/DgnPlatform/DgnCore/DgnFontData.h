/*--------------------------------------------------------------------------------------+
|     $Source: PublicAPI/DgnPlatform/DgnCore/DgnFontData.h $
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "../DgnPlatform.h"
#include "DgnDbTables.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

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
    static BentleyStatus GetFamilyName(Utf8StringR familyName, struct FT_FaceRec_&);
    virtual struct FT_FaceRec_* _GetFaceP(DgnFontStyle) = 0;
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
    virtual size_t _Read(void* buffer, size_t size, size_t count) = 0;
    virtual BentleyStatus _Seek(int64_t offset, BeFileSeekOrigin origin) = 0;
    virtual uint64_t _Tell() = 0;

    char GetNextChar() { char next; _Read(&next, sizeof(next), 1); return next; }
    uint16_t GetNextUInt16() { uint16_t next; _Read(&next, sizeof(next), 1); return next; }

    DgnShxFont::ShxType GetShxType();
};

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     03/2015
//=======================================================================================
struct DgnFontPersistence : NonCopyableClass
{
    struct Db
    {
    private:
        static DgnFontPtr DgnTrueTypeFontFromDb(struct DgnFonts&, DgnFontId, Utf8CP name, ByteCP metadata, size_t metadataSize);
        static DgnFontPtr DgnRscFontFromDb(struct DgnFonts&, DgnFontId, Utf8CP name, ByteCP metadata, size_t metadataSize);
        static DgnFontPtr DgnShxFontFromDb(struct DgnFonts&, DgnFontId, Utf8CP name, ByteCP metadata, size_t metadataSize);
        static BentleyStatus DgnRscFontMetadataToDb(bvector<Byte>&, DgnRscFontCR);
        static BentleyStatus DgnShxFontMetadataToDb(bvector<Byte>&, DgnShxFontCR);

    public:
        DGNPLATFORM_EXPORT static DgnFontPtr FromDb(struct DgnFonts&, DgnFontId, DgnFontType, Utf8CP name, ByteCP metadata, size_t metadataSize);
        DGNPLATFORM_EXPORT static BentleyStatus MetadataToDb(bvector<Byte>&, DgnFontCR);
        DGNPLATFORM_EXPORT static BentleyStatus Embed(DgnFonts::DbFaceDataDirect&, DgnFontCR);

        DGNPLATFORM_EXPORT static IDgnTrueTypeFontData* CreateIDgnTrueTypeFontData(DgnFonts&, Utf8CP familyName);
        DGNPLATFORM_EXPORT static IDgnRscFontData* CreateIDgnRscFontData(DgnFonts&, Utf8CP familyName);
        DGNPLATFORM_EXPORT static IDgnShxFontData* CreateIDgnShxFontData(DgnFonts&, Utf8CP familyName);
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

    struct RawData
    {
        
    };
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
