/*--------------------------------------------------------------------------------------+
|     $Source: DgnCore/DgnShxFontData.cpp $
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnCore/DgnFontData.h>

#define FONT_LOG (*LoggingManager::GetLogger(L"DgnFont"))

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
DgnShxFont::ShxType IDgnShxFontData::GetShxType()
    {
    static const CharCP UNIFONT_HEADER = "AutoCAD-86 unifont 1.0";
    static const CharCP SHAPES1_0_HEADER = "AutoCAD-86 shapes 1.0";
    static const CharCP SHAPES1_1_HEADER = "AutoCAD-86 shapes 1.1";
    static const size_t MAX_HEADER = 40; // arbitrary to compensate for bad data

    AutoDgnFontDataSession session(*this);
    if (!session.IsValid())
        return DgnShxFont::ShxType::Invalid;

    _Seek(0, BeFileSeekOrigin::Begin);
    
    char fileHeader[MAX_HEADER];
    char nextChar = 0;

    for (size_t iChar = 0; 0x1a != nextChar; ++iChar)
        {
        // Normal data terminates header with 0x1a; bail if we exceed the buffer looking.
        if (iChar >= _countof(fileHeader))
            return DgnShxFont::ShxType::Invalid;
        
        fileHeader[iChar] = nextChar = GetNextChar();
        }

    if (nullptr != strstr(fileHeader, UNIFONT_HEADER))
        return DgnShxFont::ShxType::Unicode;
    
    if ((nullptr == strstr(fileHeader, SHAPES1_0_HEADER)) && (nullptr == strstr(fileHeader, SHAPES1_1_HEADER)))
        return DgnShxFont::ShxType::Invalid;

    // Font (vs. shape) files immediately follow the header with glyph code 0.
    if (0 == GetNextUInt16())
        return DgnShxFont::ShxType::Locale;

    return DgnShxFont::ShxType::Invalid;
    }

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     03/2015
//=======================================================================================
struct DgnShxDbFontData : IDgnShxFontData
{
private:
    DgnFonts::DbFaceDataDirect& m_dbFaceData;
    Utf8String m_familyName;
    size_t m_openCount;
    bvector<Byte> m_fontData;
    uint64_t m_dataPosition;

public:
    DgnShxDbFontData(DgnFonts::DbFaceDataDirect& dbFaceData, Utf8CP familyName) : m_dbFaceData(dbFaceData), m_familyName(familyName), m_openCount(0) {}

    virtual IDgnFontData* _CloneWithoutData() override { return new DgnShxDbFontData(m_dbFaceData, m_familyName.c_str()); }
    virtual BentleyStatus _Embed(DgnFonts::DbFaceDataDirect&) override;
    virtual BentleyStatus _AddDataRef() override;
    virtual void _ReleaseDataRef() override;
    virtual size_t _Read(void* buffer, size_t size, size_t count) override;
    virtual BentleyStatus _Seek(int64_t offset, BeFileSeekOrigin origin) override;
    virtual uint64_t _Tell() override;
};

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnShxDbFontData::_Embed(DgnFonts::DbFaceDataDirect& faceData)
    {
    AutoDgnFontDataSession session(*this);
    if (!session.IsValid())
        return ERROR;
    
    DgnFonts::DbFaceDataDirect::FaceKey regularFace(DgnFontType::Shx, m_familyName.c_str(), DgnFonts::DbFaceDataDirect::FaceKey::FACE_NAME_Regular);    
    if (faceData.Exists(regularFace))
        {
        FONT_LOG.infov("Embedded data for SHX family '%s' already exists; it will not be automatically replaced.", regularFace.m_familyName.c_str());
        return ERROR;
        }
    
    DgnFonts::DbFaceDataDirect::T_FaceMap faces;
    faces[0] = regularFace;
    if (SUCCESS != faceData.Insert(&m_fontData[0], m_fontData.size(), faces))
        {
        FONT_LOG.errorv("Unable to write embedded font data for SHX family '%s'.", regularFace.m_familyName.c_str());
        return ERROR;
        }
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnShxDbFontData::_AddDataRef()
    {
    if (m_openCount > 0)
        {
        ++m_openCount;
        return SUCCESS;
        }
    
    DgnFonts::DbFaceDataDirect::FaceSubId ignored;
    if (SUCCESS != m_dbFaceData.QueryByFace(m_fontData, ignored, DgnFonts::DbFaceDataDirect::FaceKey(DgnFontType::Shx, m_familyName.c_str(), DgnFonts::DbFaceDataDirect::FaceKey::FACE_NAME_Regular)))
        {
        FONT_LOG.warningv("Could not load SHX font data from database with name '%s'. This font will be considered unresolved.", m_familyName.c_str());
        return ERROR;
        }

    ++m_openCount;
    m_dataPosition = 0;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
void DgnShxDbFontData::_ReleaseDataRef()
    {
    if (0 == m_openCount)
        {
        BeAssert(false);
        return;
        }

    if (--m_openCount > 0)
        return;
    
    m_dataPosition = 0;
    
    // Force the implementation to free its memory.
    decltype(m_fontData) empty;
    m_fontData.swap(empty);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
size_t DgnShxDbFontData::_Read(void* buffer, size_t size, size_t count)
    {
    PRECONDITION(m_openCount > 0, ERROR);
    
    size_t remainingBytes = (m_fontData.size() - (size_t)m_dataPosition);
    size_t requestedBytes = (size * count);
    size_t actuallyRead = std::min(remainingBytes, requestedBytes);

    memcpy(buffer, &m_fontData[m_dataPosition], actuallyRead);
    m_dataPosition += actuallyRead;

    return actuallyRead;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnShxDbFontData::_Seek(int64_t offset, BeFileSeekOrigin origin)
    {
    PRECONDITION(m_openCount > 0, ERROR);

    int64_t newPosition;
    switch (origin)
        {
        case BeFileSeekOrigin::Current: newPosition = ((int64_t)m_dataPosition + offset); break;
        case BeFileSeekOrigin::End: newPosition = ((int64_t)m_fontData.size() + offset); break;
        case BeFileSeekOrigin::Begin: newPosition = (int64_t)offset; break;

        default:
            BeAssert(false);
            return ERROR;
        }

    if ((newPosition < 0) || ((size_t)newPosition >= m_fontData.size()))
        return ERROR;

    m_dataPosition = (uint64_t)newPosition;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
uint64_t DgnShxDbFontData::_Tell()
    {
    PRECONDITION(m_openCount > 0, 0);

    return m_dataPosition;
    }

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     03/2015
//=======================================================================================
struct DgnShxFileFontData : IDgnShxFontData
{
private:
    BeFileName m_path;
    BeFile m_file;
    size_t m_openCount;

public:
    DgnShxFileFontData(BeFileName path) : m_path(path), m_openCount(0) {}

    virtual IDgnFontData* _CloneWithoutData() override { return new DgnShxFileFontData(m_path); }
    virtual BentleyStatus _Embed(DgnFonts::DbFaceDataDirect&) override;
    virtual BentleyStatus _AddDataRef() override;
    virtual void _ReleaseDataRef() override;
    virtual size_t _Read(void* buffer, size_t size, size_t count) override;
    virtual BentleyStatus _Seek(int64_t offset, BeFileSeekOrigin origin) override;
    virtual uint64_t _Tell() override;
};

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnShxFileFontData::_Embed(DgnFonts::DbFaceDataDirect& faceData)
    {
    AutoDgnFontDataSession session(*this);
    if (!session.IsValid())
        return ERROR;

    bvector<Byte> fontData;
    if (BeFileStatus::Success != m_file.ReadEntireFile(fontData))
        {
        FONT_LOG.warningv("Could not read SHX file data for embedding from file '%s'.", Utf8String(m_path).c_str());
        return ERROR;
        }

    DgnFonts::DbFaceDataDirect::FaceKey regularFace(DgnFontType::Shx, Utf8String(m_path.GetFileNameWithoutExtension().c_str()).c_str(), DgnFonts::DbFaceDataDirect::FaceKey::FACE_NAME_Regular);
    if (faceData.Exists(regularFace))
        {
        FONT_LOG.infov("Embedded data for SHX family '%s' already exists; it will not be automatically replaced.", regularFace.m_familyName.c_str());
        return ERROR;
        }

    DgnFonts::DbFaceDataDirect::T_FaceMap faces;
    faces[0] = regularFace;
    if (SUCCESS != faceData.Insert(&fontData[0], fontData.size(), faces))
        {
        FONT_LOG.errorv("Unable to write embedded font data for SHX family '%s'.", regularFace.m_familyName.c_str());
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnShxFileFontData::_AddDataRef()
    {
    if (m_openCount > 0)
        {
        ++m_openCount;
        return SUCCESS;
        }

    if (BeFileStatus::Success != m_file.Open(m_path, BeFileAccess::Read))
        {
        FONT_LOG.warningv("Could not open SHX font file '%s'. This font will be considered unresolved.", Utf8String(m_path).c_str());
        return ERROR;
        }

    ++m_openCount;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
void DgnShxFileFontData::_ReleaseDataRef()
    {
    if (0 == m_openCount)
        {
        BeAssert(false);
        return;
        }

    if (--m_openCount > 0)
        return;

    if (BeFileStatus::Success != m_file.Close())
        FONT_LOG.warningv("Could not close SHX font file '%s'.", Utf8String(m_path).c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
size_t DgnShxFileFontData::_Read(void* buffer, size_t size, size_t count)
    {
    PRECONDITION(m_openCount > 0, ERROR);

    uint32_t bytesRead;
    if (BeFileStatus::Success != m_file.Read(buffer, &bytesRead, (uint32_t)(size * count)))
        return 0;

    return (size_t)bytesRead;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnShxFileFontData::_Seek(int64_t offset, BeFileSeekOrigin origin)
    {
    PRECONDITION(m_openCount > 0, ERROR);

    if (BeFileStatus::Success != m_file.SetPointer((uint64_t)offset, origin))
        return ERROR;
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
uint64_t DgnShxFileFontData::_Tell()
    {
    PRECONDITION(m_openCount > 0, 0);

    uint64_t position;
    if (BeFileStatus::Success != m_file.GetPointer(position))
        return 0;
    
    return position;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
static BentleyStatus metadataToJsonBlob(bvector<Byte>& metadataBlob, DgnShxFont::Metadata const& metadata)
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
static BentleyStatus metadataFromJsonBlob(DgnShxFont::Metadata& metadata, ByteCP metadataBlob, size_t metadataBlobSize)
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
DgnFontPtr DgnFontPersistence::Db::DgnShxFontFromDb(DgnFonts& dbFonts, DgnFontId id, Utf8CP name, ByteCP metadata, size_t metadataSize)
    {
    DgnShxFontP font = new DgnShxFont(name, new DgnShxDbFontData(dbFonts.DbFaceData(), name));
    
    if (SUCCESS != metadataFromJsonBlob(font->GetMetadataR(), metadata, metadataSize))
        FONT_LOG.warningv("Could not parse metadata for font of id/type/name %i/%i/'%s'.", (int)id.GetValue(), (int)font->GetType(), name);

    return font;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnFontPersistence::Db::DgnShxFontMetadataToDb(bvector<Byte>& metadata, DgnShxFontCR font)
    {
    return metadataToJsonBlob(metadata, font.GetMetadata());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
static void setMetadataToDefault(DgnShxFont::Metadata& metadata, DgnShxFont::ShxType shxType)
    {
    switch (shxType)
        {
        case DgnShxFont::ShxType::Invalid:
            metadata.m_codePage = LangCodePage::Unknown;
            metadata.m_degreeCode = 0;
            metadata.m_diameterCode = 0;
            metadata.m_plusMinusCode = 0;
            break;

        case DgnShxFont::ShxType::Unicode:
            metadata.m_codePage = LangCodePage::Unicode;
            metadata.m_degreeCode = 176;
            metadata.m_diameterCode = 8709;
            metadata.m_plusMinusCode = 177;
            break;

        case DgnShxFont::ShxType::Locale:
            BeStringUtilities::GetCurrentCodePage(metadata.m_codePage);
            metadata.m_degreeCode = 256;
            metadata.m_diameterCode = 258;
            metadata.m_plusMinusCode = 257;
            break;

        default:
            BeAssert(false);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
DgnFontPtr DgnFontPersistence::File::FromShxFile(BeFileNameCR path)
    {
    Utf8String pathBaseName(path.GetFileNameWithoutExtension());
    
    DgnShxFontP font = new DgnShxFont(pathBaseName.c_str(), new DgnShxFileFontData(path));
    DgnShxFont::ShxType shxType = font->GetDataP()->GetShxType();
    if (DgnShxFont::ShxType::Invalid != shxType)
        setMetadataToDefault(font->GetMetadataR(), shxType);
    
    return font;
    }
