/*--------------------------------------------------------------------------------------+
|     $Source: DgnCore/DgnRscFontData.cpp $
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnCore/DgnFontData.h>
#include <DgnPlatform/DgnCore/DgnRscFontStructures.h>
#include <DgnPlatform/DgnCore/DgnFont.fb.h>

using namespace flatbuffers;

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     03/2015
//=======================================================================================
struct DgnRscDbFontData : IDgnRscFontData
{
private:
    DgnFonts::DbFaceDataDirect& m_dbFaceData;
    Utf8String m_familyName;
    size_t m_openCount;
    bvector<Byte> m_fontData;

public:
    DgnRscDbFontData(DgnFonts::DbFaceDataDirect& dbFaceData, Utf8CP familyName) : m_dbFaceData(dbFaceData), m_familyName(familyName), m_openCount(0) {}

    virtual IDgnFontData* _CloneWithoutData() override { return new DgnRscDbFontData(m_dbFaceData, m_familyName.c_str()); }
    virtual BentleyStatus _Embed(DgnFonts::DbFaceDataDirect&) override;
    virtual BentleyStatus _AddDataRef() override;
    virtual void _ReleaseDataRef() override;
    virtual BentleyStatus _ReadFontHeader(bvector<Byte>&) override;
    virtual BentleyStatus _ReadFractionMap(bvector<Byte>&) override;
    virtual BentleyStatus _ReadGlyphData(bvector<Byte>&, size_t offset, size_t size) override;
    virtual BentleyStatus _ReadGlyphDataOffsets(bvector<Byte>&) override;
    virtual BentleyStatus _ReadGlyphHeaders(bvector<Byte>&) override;

    FB::DgnRscFont const* GetFB() const { PRECONDITION(m_openCount > 0, nullptr); return GetRoot<FB::DgnRscFont>(&m_fontData[0]); }
};

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnRscDbFontData::_Embed(DgnFonts::DbFaceDataDirect& faceData)
    {
    AutoDgnFontDataSession session(*this);
    if (!session.IsValid())
        return ERROR;

    DgnFonts::DbFaceDataDirect::FaceKey regularFace(DgnFontType::Rsc, m_familyName.c_str(), DgnFonts::DbFaceDataDirect::FaceKey::FACE_NAME_Regular);
    DgnFonts::DbFaceDataDirect::T_FaceMap faces;
    faces[0] = regularFace;

    if (faceData.Exists(regularFace))
        return ERROR;

    if (SUCCESS != faceData.Insert(&m_fontData[0], m_fontData.size(), faces))
        return ERROR;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnRscDbFontData::_AddDataRef()
    {
    if (m_openCount > 0)
        {
        ++m_openCount;
        return SUCCESS;
        }

    DgnFonts::DbFaceDataDirect::FaceSubId unused;
    if (SUCCESS != m_dbFaceData.QueryByFace(m_fontData, unused, DgnFonts::DbFaceDataDirect::FaceKey(DgnFontType::Rsc, m_familyName.c_str(), DgnFonts::DbFaceDataDirect::FaceKey::FACE_NAME_Regular)))
        return ERROR;
    
    ++m_openCount;

    if (1 != GetFB()->majorVersion())
        {
        _ReleaseDataRef();
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
void DgnRscDbFontData::_ReleaseDataRef()
    {
    if (0 == m_openCount)
        {
        BeAssert(false);
        return;
        }

    if (--m_openCount > 0)
        return;

    // Force the implementation to free its memory.
    decltype(m_fontData) empty;
    m_fontData.swap(empty);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnRscDbFontData::_ReadFontHeader(bvector<Byte>& buffer)
    {
    FB::DgnRscFont const* rscFontData = GetFB();
    if (nullptr == rscFontData)
        return ERROR;

    buffer.clear();
    ByteCP start = rscFontData->header()->Data();
    ByteCP end = start + rscFontData->header()->size();
    std::copy(start, end, std::back_inserter(buffer));

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnRscDbFontData::_ReadFractionMap(bvector<Byte>& buffer)
    {
    FB::DgnRscFont const* rscFontData = GetFB();
    if (nullptr == rscFontData)
        return ERROR;

    buffer.clear();
    ByteCP start = rscFontData->fractionMap()->Data();
    ByteCP end = start + rscFontData->fractionMap()->size();
    std::copy(start, end, std::back_inserter(buffer));

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnRscDbFontData::_ReadGlyphData(bvector<Byte>& buffer, size_t offset, size_t size)
    {
    FB::DgnRscFont const* rscFontData = GetFB();
    if (nullptr == rscFontData)
        return ERROR;

    buffer.clear();
    ByteCP start = rscFontData->glyphData()->Data() + offset;
    ByteCP end = start + size;
    std::copy(start, end, std::back_inserter(buffer));

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnRscDbFontData::_ReadGlyphDataOffsets(bvector<Byte>& buffer)
    {
    FB::DgnRscFont const* rscFontData = GetFB();
    if (nullptr == rscFontData)
        return ERROR;

    buffer.clear();
    ByteCP start = rscFontData->glyphDataOffsets()->Data();
    ByteCP end = start + rscFontData->glyphDataOffsets()->size();
    std::copy(start, end, std::back_inserter(buffer));

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnRscDbFontData::_ReadGlyphHeaders(bvector<Byte>& buffer)
    {
    FB::DgnRscFont const* rscFontData = GetFB();
    if (nullptr == rscFontData)
        return ERROR;

    buffer.clear();
    ByteCP start = rscFontData->glyphHeaders()->Data();
    ByteCP end = start + rscFontData->glyphHeaders()->size();
    std::copy(start, end, std::back_inserter(buffer));

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
static BentleyStatus metadataToJsonBlob(bvector<Byte>& metadataBlob, DgnRscFont::Metadata const& metadata)
    {
    Json::Value json(Json::ValueType::objectValue);

    json["codePage"] = Json::Value((int)metadata.m_codePage);
    json["degree"] = Json::Value((int)metadata.m_degreeCode);
    json["diameter"] = Json::Value((int)metadata.m_diameterCode);
    json["plusMinus"] = Json::Value((int)metadata.m_plusMinusCode);

    Utf8String metadataString = Json::FastWriter::ToString(json);
    metadataBlob.clear();
    std::copy(metadataString.begin(), metadataString.end(), std::back_inserter(metadataBlob));

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
static BentleyStatus metadataFromJsonBlob(DgnRscFont::Metadata& metadata, ByteCP metadataBlob, size_t metadataBlobSize)
    {
    Json::Reader reader;
    Json::Value json;
    if (!reader.parse((CharCP)metadataBlob, (CharCP)(metadataBlob + metadataBlobSize), json, false))
        return ERROR;

    Json::Value subVal = json["codePage"];
    if (subVal.isIntegral())
        metadata.m_codePage = (LangCodePage)subVal.asInt();

    subVal = json["degree"];
    if (subVal.isIntegral())
        metadata.m_degreeCode = (DgnGlyph::T_Id)subVal.asInt();

    subVal = json["diameter"];
    if (subVal.isIntegral())
        metadata.m_diameterCode = (DgnGlyph::T_Id)subVal.asInt();

    subVal = json["plusMinus"];
    if (subVal.isIntegral())
        metadata.m_plusMinusCode = (DgnGlyph::T_Id)subVal.asInt();

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
DgnFontPtr DgnFontPersistence::Db::DgnRscFontFromDb(DgnFonts& dbFonts, DgnFontId id, Utf8CP name, ByteCP metadata, size_t metadataSize)
    {
    DgnRscFontP font = new DgnRscFont(name, new DgnRscDbFontData(dbFonts.DbFaceData(), name));

    metadataFromJsonBlob(font->GetMetadataR(), metadata, metadataSize);

    return font;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnFontPersistence::Db::DgnRscFontMetadataToDb(bvector<Byte>& metadata, DgnRscFontCR font)
    {
    return metadataToJsonBlob(metadata, font.GetMetadata());
    }
