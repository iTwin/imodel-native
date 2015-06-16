/*--------------------------------------------------------------------------------------+
|     $Source: DgnCore/DgnShxFontData.cpp $
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnCore/DgnFontData.h>

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     07/2012
//=======================================================================================
struct AutoRestoreFPos
{
private:
    IDgnShxFontData* m_data;
    uint64_t m_positionToRestore;

public:
    AutoRestoreFPos(IDgnShxFontData& data) : m_data(&data), m_positionToRestore(data._Tell()) {}
    ~AutoRestoreFPos() { m_data->_Seek((int64_t)m_positionToRestore, BeFileSeekOrigin::Begin); }
};

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

    // Allow this to be used in the middle of other read operations.
    AutoRestoreFPos restoreFPos(*this);
    
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

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2015
//---------------------------------------------------------------------------------------
void IDgnShxFontData::LoadNonUnicodeGlyphFPosCacheAndMetrics()
    {
    _Seek(0, BeFileSeekOrigin::Begin);
    
    // Skip header...
    for (size_t iByte = 0; ((iByte < 40) && (0x1a != GetNextChar())); ++iByte)
        ;

    // Skip FirstCode and LastCode (both UInt16)...
    _Seek(4, BeFileSeekOrigin::Current);

    size_t numGlyphs = (size_t)GetNextUInt16();
    size_t dataStart = (size_t)(_Tell() + (4 * numGlyphs));
    size_t dataOffset = 0;
    DgnShxFont::GlyphFPos const* zeroGlyphFPos = nullptr;

    // Read glyph GlyphFPos data.
    for (size_t iGlyph = 0; iGlyph < numGlyphs; ++iGlyph)
        {
        DgnGlyph::T_Id glyphId = (DgnGlyph::T_Id)GetNextUInt16();
        DgnShxFont::GlyphFPos glyphFPos;
        glyphFPos.m_dataOffset = (dataStart + dataOffset);
        glyphFPos.m_dataSize = (size_t)GetNextUInt16();

        m_glyphFPosCache[glyphId] = glyphFPos;
        dataOffset += glyphFPos.m_dataSize;
        
        if (0 == glyphId)
            zeroGlyphFPos = &m_glyphFPosCache[glyphId];
        }
    
    //.............................................................................................
    // Read metric data. Char 0 is the font specifier...
    if (nullptr == zeroGlyphFPos)
        {
        m_ascender = 1;
        return;
        }

    if (0 != _Seek(zeroGlyphFPos->m_dataOffset, BeFileSeekOrigin::Begin))
        return;

    size_t pBufSize = (size_t)zeroGlyphFPos->m_dataSize;
    ScopedArray<Byte> pBuf(pBufSize);

    // Read code 0 which should be the font specifier.
    _Read(pBuf.GetData(), pBufSize, 1);

    // Look for a NULL terminator.
    size_t iLen = 0;
    for (; ((iLen < pBufSize) && (0 != pBuf.GetData()[iLen])); ++iLen)
        ;

    // No Terminator found?
    if (iLen == pBufSize)
        return;

    // Prehistoric legacy...
    if ((pBufSize - iLen) != 5)
        return;

    size_t iOffset = (iLen + 1);
    m_ascender = pBuf.GetData()[iOffset++];

    // This hack comes from the OpenDWG code. I don't understand it but see TR#182253 for font with this problem (gbcbig.shx).
    if (0 == m_ascender)
        m_ascender = 8;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2015
//---------------------------------------------------------------------------------------
void IDgnShxFontData::LoadUnicodeGlyphFPosCacheAndMetrics()
    {
    _Seek(0, BeFileSeekOrigin::Begin);
    
    // Skip header.
    for (size_t iByte = 0; ((iByte < 40) && (0x1a != GetNextChar())); ++iByte)
        ;

    size_t numGlyphs = (size_t)GetNextUInt16();

    // Not sure why we have to do this, but old code does, and most fonts indeed have less 1 glyph.
    --numGlyphs;

    // Skip 2 bytes...
    _Seek(2, BeFileSeekOrigin::Current);

    size_t fontInfoSize = (size_t)GetNextUInt16();
    ScopedArray<Byte> fontInfo(fontInfoSize);

    _Read(fontInfo.GetData(), fontInfoSize, 1);

    size_t firstNullOffset = strlen(reinterpret_cast<char*>(fontInfo.GetData()));
    size_t fontInfoDataOffset = (firstNullOffset + 1);

    // ascender
    m_ascender = fontInfo.GetData()[fontInfoDataOffset++];
    
    // skip descender
    ++fontInfoDataOffset;

    // Skip "modes".
    ++fontInfoDataOffset;

    // Special case when the "encoding" is "shape file".
    if (2 == fontInfo.GetData()[fontInfoDataOffset])
        m_ascender = 1;

    uint64_t nextAddress = _Tell();

    for (size_t iGlyph = 0; iGlyph < numGlyphs; ++iGlyph)
        {
        if (0 != _Seek((int64_t)nextAddress, BeFileSeekOrigin::Begin))
            break;

        DgnGlyph::T_Id glyphId = (DgnGlyph::T_Id)GetNextUInt16();

        DgnShxFont::GlyphFPos glyphFPos;
        glyphFPos.m_dataOffset = (nextAddress + 4);
        glyphFPos.m_dataSize = GetNextUInt16();

        nextAddress = (size_t)(glyphFPos.m_dataOffset + glyphFPos.m_dataSize);

        m_glyphFPosCache[glyphId] = glyphFPos;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2015
//---------------------------------------------------------------------------------------
void IDgnShxFontData::LoadGlyphFPosCacheAndMetrics()
    {
    // Because we have to load and O(n) iterate the header to even know what glyphs are in the file, do it once up-front instead of repeatedly for each glyph.
    // The glyphs will still load their glyph geometry data on-demand.
    if (m_hasLoadedGlyphFPosCacheAndMetrics)
        return;

    m_hasLoadedGlyphFPosCacheAndMetrics = true;

    // Allow this to be used in the middle of other read operations.
    AutoRestoreFPos restoreFPos(*this); 
    
    switch (const_cast<IDgnShxFontData*>(this)->GetShxType())
        {
        case DgnShxFont::ShxType::Locale: LoadNonUnicodeGlyphFPosCacheAndMetrics(); return;
        case DgnShxFont::ShxType::Unicode: LoadUnicodeGlyphFPosCacheAndMetrics(); return;
        default:
            return;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2015
//---------------------------------------------------------------------------------------
DgnShxFont::GlyphFPos const* IDgnShxFontData::GetGlyphFPos(DgnGlyph::T_Id glyphId)
    {
    LoadGlyphFPosCacheAndMetrics();

    T_GlyphFPosCache::const_iterator foundGlyphFPos = m_glyphFPosCache.find(glyphId);
    if (m_glyphFPosCache.end() != foundGlyphFPos)
        return &foundGlyphFPos->second;

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2015
//---------------------------------------------------------------------------------------
Byte IDgnShxFontData::GetAscender()
    {
    LoadGlyphFPosCacheAndMetrics();
    return m_ascender;
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
        return ERROR;
    
    DgnFonts::DbFaceDataDirect::T_FaceMap faces;
    faces[0] = regularFace;
    if (SUCCESS != faceData.Insert(&m_fontData[0], m_fontData.size(), faces))
        return ERROR;
    
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
        return ERROR;

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

    memcpy(buffer, &m_fontData[(size_t)m_dataPosition], actuallyRead);
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

    // Allow this to be used in the middle of other read operations.
    AutoRestoreFPos restoreFPos(*this);
    _Seek(0, BeFileSeekOrigin::Begin);
    
    bvector<Byte> fontData;
    if (BeFileStatus::Success != m_file.ReadEntireFile(fontData))
        return ERROR;

    DgnFonts::DbFaceDataDirect::FaceKey regularFace(DgnFontType::Shx, Utf8String(m_path.GetFileNameWithoutExtension().c_str()).c_str(), DgnFonts::DbFaceDataDirect::FaceKey::FACE_NAME_Regular);
    if (faceData.Exists(regularFace))
        return ERROR;

    DgnFonts::DbFaceDataDirect::T_FaceMap faces;
    faces[0] = regularFace;
    if (SUCCESS != faceData.Insert(&fontData[0], fontData.size(), faces))
        return ERROR;

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
        return ERROR;

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

    m_file.Close();
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
    
    metadataFromJsonBlob(font->GetMetadataR(), metadata, metadataSize);

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
    DgnShxFont::ShxType shxType = font->GetShxType();
    if (DgnShxFont::ShxType::Invalid != shxType)
        setMetadataToDefault(font->GetMetadataR(), shxType);
    
    return font;
    }
