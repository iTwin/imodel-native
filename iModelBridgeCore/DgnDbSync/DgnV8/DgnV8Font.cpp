/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/DgnV8Font.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#define NOMINMAX
#include <windows.h>

#include "ConverterInternal.h"
#include <DgnDbSync/DgnV8/DgnV8Font.h>
#include <DgnPlatform/DgnRscFontStructures.h>
#include <DgnPlatform/DgnFont.fb.h>
#include <regex>
#include <DgnPlatform/DgnFontData.h>
#include <VersionedDgnV8Api/RmgrTools/Tools/mdlResource.h>
#include <VersionedDgnV8Api/RmgrTools/Tools/rmgrstrl.h>

// Via RequiredRepository entry in the DgnV8ConverterDLL Part. The way this piece was designed was that is was so small, every library gets and builds as source.
#include "../../../../V8IModelExtraFiles/V8IModelExtraFiles.h"

#undef LOG
#define LOG (*LoggingManager::GetLogger(L"DgnFont"))

USING_NAMESPACE_BENTLEY_DGN
using namespace BentleyApi::flatbuffers;

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE
//=======================================================================================
// @bsiclass                                                    Jeff.Marker     03/2015
//=======================================================================================
struct DgnRscFileFontData : IDgnRscFontData
{
private:
    BeFileName m_path;
    Utf8String m_familyName;
    uint32_t m_rscId;
    size_t m_openCount;
    RscFileHandle m_fileHandle;
    
    BentleyStatus ReadGlyphData(bvector<Byte>&);
    static BentleyStatus ReadResource(bvector<Byte>& buffer, RscFileHandle, RscType type, RscId id);
    static BentleyStatus ReadResource(bvector<Byte>& buffer, RscFileHandle, RscType type, RscId id, size_t size, size_t offset);

public:
    DgnRscFileFontData(BeFileName path, Utf8CP familyName, uint32_t rscId) : m_path(path), m_familyName(familyName), m_rscId(rscId), m_openCount(0), m_fileHandle(0) {}

    virtual IDgnFontData* _CloneWithoutData() override { return new DgnRscFileFontData(m_path, m_familyName.c_str(), m_rscId); }
    virtual BentleyStatus _Embed(DgnFonts::DbFaceDataDirect&) override;
    virtual BentleyStatus _AddDataRef() override;
    virtual void _ReleaseDataRef() override;
    virtual BentleyStatus _ReadFontHeader(bvector<Byte>&) override;
    virtual BentleyStatus _ReadFractionMap(bvector<Byte>&) override;
    virtual BentleyStatus _ReadGlyphData(bvector<Byte>&, size_t offset, size_t size) override;
    virtual BentleyStatus _ReadGlyphDataOffsets(bvector<Byte>&) override;
    virtual BentleyStatus _ReadGlyphHeaders(bvector<Byte>&) override;
};

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnRscFileFontData::_Embed(DgnFonts::DbFaceDataDirect& faceData)
    {
    AutoDgnFontDataSession session(*this);
    if (!session.IsValid())
        return ERROR;

    FlatBufferBuilder encoder;
    bvector<Byte> buffer;

    Offset<Vector<Byte>> fontHeaderOffset;
    if (SUCCESS == _ReadFontHeader(buffer))
        fontHeaderOffset = encoder.CreateVector(buffer);

    Offset<Vector<Byte>> fractionMapOffset;
    if (SUCCESS == _ReadFractionMap(buffer))
        fractionMapOffset = encoder.CreateVector(buffer);

    Offset<Vector<Byte>> glyphDataOffset;
    if (SUCCESS == ReadGlyphData(buffer))
        glyphDataOffset = encoder.CreateVector(buffer);

    Offset<Vector<Byte>> glyphDataOffsetsOffset;
    if (SUCCESS == _ReadGlyphDataOffsets(buffer))
        glyphDataOffsetsOffset = encoder.CreateVector(buffer);

    Offset<Vector<Byte>> glyphHeadersOffset;
    if (SUCCESS == _ReadGlyphHeaders(buffer))
        glyphHeadersOffset = encoder.CreateVector(buffer);

    FB::DgnRscFontBuilder builder(encoder);
    builder.add_majorVersion(1);
    builder.add_majorVersion(0);
    builder.add_header(fontHeaderOffset);
    builder.add_glyphData(glyphDataOffset);
    builder.add_glyphDataOffsets(glyphDataOffsetsOffset);
    builder.add_glyphHeaders(glyphHeadersOffset);

    if (0 != fractionMapOffset.o)
        builder.add_fractionMap(fractionMapOffset);

    encoder.Finish(builder.Finish());

    bvector<Byte> fontData;
    fontData.resize(encoder.GetSize());
    memcpy(&fontData[0], encoder.GetBufferPointer(), encoder.GetSize());

    DgnFonts::DbFaceDataDirect::FaceKey regularFace(DgnFontType::Rsc, m_familyName.c_str(), DgnFonts::DbFaceDataDirect::FaceKey::FACE_NAME_Regular);
    if (faceData.Exists(regularFace))
        {
        LOG.infov("Embedded data for RSC family '%s' already exists; it will not be automatically replaced.", regularFace.m_familyName.c_str());
        return ERROR;
        }

    DgnFonts::DbFaceDataDirect::T_FaceMap faces;
    faces[0] = regularFace;
    if (SUCCESS != faceData.Insert(&fontData[0], fontData.size(), faces))
        {
        LOG.errorv("Unable to write embedded font data for RSC family '%s'.", regularFace.m_familyName.c_str());
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnRscFileFontData::_AddDataRef()
    {
    if (m_openCount > 0)
        {
        ++m_openCount;
        return SUCCESS;
        }

    if (SUCCESS != RmgrResource::OpenFile(&m_fileHandle, m_path, RSC_READONLY))
        {
        m_fileHandle = 0;
        LOG.warningv("Could not open RSC font file '%s'. This font will be considered unresolved.", Utf8String(m_path).c_str());
        return ERROR;
        }

    ++m_openCount;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
void DgnRscFileFontData::_ReleaseDataRef()
    {
    if (0 == m_openCount)
        {
        BeAssert(false);
        return;
        }

    if (--m_openCount > 0)
        return;
    
    RmgrResource::CloseFile(m_fileHandle);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnRscFileFontData::ReadResource(bvector<Byte>& buffer, RscFileHandle file, RscType type, RscId id) { return ReadResource(buffer, file, type, id, 0, 0); }
BentleyStatus DgnRscFileFontData::ReadResource(bvector<Byte>& buffer, RscFileHandle file, RscType type, RscId id, size_t size, size_t offset)
    {
    void* resource = RmgrResourceMT::Load(file, type, id);
    if (nullptr == resource)
        return ERROR;

    UInt32 realSize;
    if (SUCCESS != RmgrResourceMT::Query(&realSize, resource, RSC_QRY_SIZE))
        return ERROR;

    if (offset >= realSize)
        return ERROR;

    if (0 == size)
        size = realSize;
    else
        size = std::min(size, (size_t)realSize - offset);

    if (0 == size)
        return ERROR;
    
    buffer.resize(size);
    memcpy(&buffer[0], (Byte const*)resource + offset, size);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnRscFileFontData::_ReadFontHeader(bvector<Byte>& buffer)
    {
    PRECONDITION(m_openCount > 0, ERROR);
    
    return ReadResource(buffer, m_fileHandle, RTYPE_FontHeader, m_rscId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnRscFileFontData::_ReadFractionMap(bvector<Byte>& buffer)
    {
    PRECONDITION(m_openCount > 0, ERROR);
    
    bvector<Byte> fontHeaderBuffer;
    if (SUCCESS != _ReadFontHeader(fontHeaderBuffer))
        return ERROR;

    RscFontHeader* fontHeader = reinterpret_cast<RscFontHeader*>(&fontHeaderBuffer[0]);

    return ReadResource(buffer, m_fileHandle, RTYPE_FontFractions, fontHeader->fractMap);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnRscFileFontData::ReadGlyphData(bvector<Byte>& buffer)
    {
    PRECONDITION(m_openCount > 0, ERROR);

    return ReadResource(buffer, m_fileHandle, RTYPE_FontVectors, m_rscId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnRscFileFontData::_ReadGlyphData(bvector<Byte>& buffer, size_t offset, size_t size)
    {
    PRECONDITION(m_openCount > 0, ERROR);
    
    return ReadResource(buffer, m_fileHandle, RTYPE_FontVectors, m_rscId, size, offset);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnRscFileFontData::_ReadGlyphDataOffsets(bvector<Byte>& buffer)
    {
    PRECONDITION(m_openCount > 0, ERROR);

    return ReadResource(buffer, m_fileHandle, RTYPE_FontVectorOffsets, m_rscId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnRscFileFontData::_ReadGlyphHeaders(bvector<Byte>& buffer)
    {
    PRECONDITION(m_openCount > 0, ERROR);

    return ReadResource(buffer, m_fileHandle, RTYPE_FontCharacters, m_rscId);
    }

END_DGNDBSYNC_DGNV8_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
T_DgnFontPtrs DgnV8FontPersistence::File::FromRscFile(BentleyApi::BeFileNameCR path, BentleyApi::Utf8CP nameFilter)
    {
    T_DgnFontPtrs fonts;

    RscFileHandle fileHandle;
    if (SUCCESS != RmgrResource::OpenFile(&fileHandle, path, RSC_READONLY))
        {
        LOG.errorv("Could not open RSC file '%s' to scan for fonts.", Utf8String(path).c_str());
        return fonts;
        }

    // Scan for name/rscID mappings.
    int numFontNameLists;
    if (SUCCESS != RmgrResource::QueryClass(&numFontNameLists, fileHandle, RTYPE_FontName, RSC_QRY_COUNT, NULL))
        {
        LOG.infov("RSC file '%s' contains no fonts.", Utf8String(path).c_str());
        return fonts;
        }

    typedef Bentley::bmap<Bentley::WString, long> T_NameToRscIDMap;
    T_NameToRscIDMap nameToRscIDMap;

    for (int iFontNameList = 0; iFontNameList < numFontNameLists; ++iFontNameList)
        {
        long fontNameListID;
        if (SUCCESS != RmgrResource::QueryClass(&fontNameListID, fileHandle, RTYPE_FontName, RSC_QRY_ID, &iFontNameList))
            {
            BeDataAssert(false);
            continue;
            }

        Bentley::StringList* fontNameList = mdlStringList_loadResourceWithType(fileHandle, RTYPE_FontName, fontNameListID);
        if (NULL == fontNameList)
            {
            BeDataAssert(false);
            continue;
            }

        long numFontNameListEntries = mdlStringList_size(fontNameList);

        for (long iFontNameListEntry = 0; iFontNameListEntry < numFontNameListEntries; ++iFontNameListEntry)
            {
            Bentley::WString fontName;
            InfoField* infoFields;
            if (SUCCESS != mdlStringList_getMemberString(fontName, &infoFields, fontNameList, iFontNameListEntry))
                {
                BeDataAssert(false);
                continue;
                }

            nameToRscIDMap[fontName] = (long)infoFields[1];
            }

        mdlStringList_destroy(fontNameList);
        }

    // Scan for name/number mappings.
    int numFontNumberLists;
    if (SUCCESS != RmgrResource::QueryClass(&numFontNumberLists, fileHandle, RTYPE_FontLibrary, RSC_QRY_COUNT, NULL))
        {
        LOG.infov("RSC file '%s' contains no fonts.", Utf8String(path).c_str());
        return fonts;
        }

    for (int iFontNumberList = 0; iFontNumberList < numFontNumberLists; ++iFontNumberList)
        {
        long fontNumberListID;
        if (SUCCESS != RmgrResource::QueryClass(&fontNumberListID, fileHandle, RTYPE_FontLibrary, RSC_QRY_ID, &iFontNumberList))
            {
            BeDataAssert(false);
            continue;
            }

        Bentley::StringList* fontNumberList = mdlStringList_loadResourceWithType(fileHandle, RTYPE_FontLibrary, fontNumberListID);
        if (NULL == fontNumberList)
            {
            BeDataAssert(false);
            continue;
            }

        long numFontNumberListEntries = mdlStringList_size(fontNumberList);

        for (long iFontNumberListEntry = 0; iFontNumberListEntry < numFontNumberListEntries; ++iFontNumberListEntry)
            {
            Bentley::WString fontNameW;
            InfoField* infoFields;
            if (SUCCESS != mdlStringList_getMemberString(fontNameW, &infoFields, fontNumberList, iFontNumberListEntry))
                {
                BeDataAssert(false);
                continue;
                }

            T_NameToRscIDMap::const_iterator rscIDEntry = nameToRscIDMap.find(fontNameW);
            if (nameToRscIDMap.end() == rscIDEntry)
                {
                BeDataAssert(false);
                continue;
                }

            Utf8String fontName(fontNameW.c_str());
            
            if (!Utf8String::IsNullOrEmpty(nameFilter) && !std::regex_match(fontName.c_str(), std::regex(nameFilter)))
                continue;
            
            fonts.push_back(new DgnRscFont(fontName.c_str(), new DgnRscFileFontData(path, fontName.c_str(), rscIDEntry->second)));
            }

        mdlStringList_destroy(fontNumberList);
        }

    return fonts;
    }

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2015
//---------------------------------------------------------------------------------------
static DgnV8Api::DgnFontFilter dbFontTypeToV8Filter(DgnFontType dbType)
    {
    switch (dbType)
        {
        case DgnFontType::Rsc: return DgnV8Api::DgnFontFilter::Resource;
        case DgnFontType::Shx: return DgnV8Api::DgnFontFilter::Shx;
        case DgnFontType::TrueType: return DgnV8Api::DgnFontFilter::TrueType;
        }

    BeAssert(false);
    return DgnV8Api::DgnFontFilter::All;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
static void loadRscFontFile(Converter::T_WorkspaceFonts& workspaceFonts, BeFileNameCR path)
    {
    T_DgnFontPtrs fonts = DgnV8FontPersistence::File::FromRscFile(path, nullptr);
    for (DgnFontPtr const& font : fonts)
        {
        workspaceFonts[Converter::T_WorkspaceFontKey(font->GetType(), font->GetName())] = font;
        
        // Use the V8 font system to read the font configuration if available.
        WString v8FontName(font->GetName().c_str(), BentleyCharEncoding::Utf8);
        DgnV8Api::DgnFont const* v8Font = DgnV8Api::DgnFontManager::FindSystemFont(v8FontName.c_str(), dbFontTypeToV8Filter(font->GetType()));
        if (nullptr == v8Font)
            continue;

        DgnRscFont& dbRscFont = (DgnRscFont&)*font;

        dbRscFont.GetMetadataR().m_codePage = (BentleyApi::LangCodePage)v8Font->GetCodePage();
        dbRscFont.GetMetadataR().m_degreeCode = v8Font->GetDegreeCharCode();
        dbRscFont.GetMetadataR().m_diameterCode = v8Font->GetDiameterCharCode();
        dbRscFont.GetMetadataR().m_plusMinusCode = v8Font->GetPlusMinusCharCode();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
static void loadShxFontFile(Converter::T_WorkspaceFonts& workspaceFonts, BeFileNameCR path)
    {
    DgnFontPtr font = DgnFontPersistence::File::FromShxFile(path);
    if (!font.IsValid())
        return;

    workspaceFonts[Converter::T_WorkspaceFontKey(font->GetType(), font->GetName())] = font;

    // Use the V8 font system to read the font configuration if available.
    WString v8FontName(font->GetName().c_str(), BentleyCharEncoding::Utf8);
    DgnV8Api::DgnFont const* v8Font = DgnV8Api::DgnFontManager::FindSystemFont(v8FontName.c_str(), dbFontTypeToV8Filter(font->GetType()));
    if (nullptr == v8Font)
        return;

    DgnShxFont& dbShxFont = (DgnShxFont&)*font;

    dbShxFont.GetMetadataR().m_codePage = (BentleyApi::LangCodePage)v8Font->GetCodePage();
    dbShxFont.GetMetadataR().m_degreeCode = v8Font->GetDegreeCharCode();
    dbShxFont.GetMetadataR().m_diameterCode = v8Font->GetDiameterCharCode();
    dbShxFont.GetMetadataR().m_plusMinusCode = v8Font->GetPlusMinusCharCode();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
static void loadTrueTypeFontFiles(Converter::T_WorkspaceFonts& workspaceFonts, bvector<BeFileName> const& paths)
    {
    T_DgnFontPtrs fonts = DgnFontPersistence::File::FromTrueTypeFiles(paths, nullptr);
    for (DgnFontPtr const& font : fonts)
        {
        workspaceFonts[Converter::T_WorkspaceFontKey(font->GetType(), font->GetName())] = font;
        // TrueType doesn't have any metadata to import.
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
static void loadFontFile(Converter::T_WorkspaceFonts& workspaceFonts, BeFileNameCR path)
    {
    WString extension = path.GetExtension();
    extension.ToLower();

    if (0 == wcscmp(L"ttf", extension.c_str())) { loadTrueTypeFontFiles(workspaceFonts, { path }); return; }
    if (0 == wcscmp(L"ttc", extension.c_str())) { loadTrueTypeFontFiles(workspaceFonts, { path }); return; }
    if (0 == wcscmp(L"otf", extension.c_str())) { loadTrueTypeFontFiles(workspaceFonts, { path }); return; }
    if (0 == wcscmp(L"otc", extension.c_str())) { loadTrueTypeFontFiles(workspaceFonts, { path }); return; }
    if (0 == wcscmp(L"rsc", extension.c_str())) { loadRscFontFile(workspaceFonts, path); return; }
    if (0 == wcscmp(L"shx", extension.c_str())) { loadShxFontFile(workspaceFonts, path); return; }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2015
//---------------------------------------------------------------------------------------
static int CALLBACK enumerateSystemFontsCallback(LOGFONTW CONST* lpelfe, TEXTMETRICW CONST*, DWORD, LPARAM lParam)
    {
    bset<Utf8String>& fontNames = *reinterpret_cast<bset<Utf8String>*>(lParam);
    WString faceNameW = lpelfe->lfFaceName;

    // If the font name is ?????? or @????? then it is not usable.
    size_t iChar = 0;
    for (; 0 != faceNameW[iChar]; ++iChar)
        {
        if ((L'?' != faceNameW[iChar]) && (L'@' != faceNameW[iChar]))
            break;
        }

    if (0 != faceNameW[iChar])
        fontNames.insert(Utf8String(faceNameW.c_str()));

    return 1;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2015
//---------------------------------------------------------------------------------------
static void loadOSFonts(Converter::T_WorkspaceFonts& workspaceFonts)
    {
    LOGFONTW lf;
    memset(&lf, 0, sizeof(lf));
    lf.lfCharSet = DEFAULT_CHARSET;

    HDC fontDC = ::CreateDCW(L"DISPLAY", NULL, NULL, NULL);
    bset<Utf8String> fontNames;
    ::EnumFontFamiliesExW(fontDC, &lf, enumerateSystemFontsCallback, reinterpret_cast<LPARAM>(&fontNames), 0);
    ::DeleteObject(fontDC);
    
    for (Utf8StringCR fontName : fontNames)
        {
        DgnFontPtr font = DgnFontPersistence::OS::FromGlobalTrueTypeRegistry(fontName.c_str());
        workspaceFonts[Converter::T_WorkspaceFontKey(font->GetType(), font->GetName())] = font;
        }
    }

//=======================================================================================
// This is not intended to be a well-behaved data object that can freely release and re-acquire its resources as necessary; it will hold data until deleted.
// This is deemed sufficient for the converter's needs. Reading data back out of the structured store may not be possible due to the locks required, and is not particularly cheap anyway.
// This is strongly modeled after platform's DgnRscDbFontData.
// @bsiclass                                                    Jeff.Marker     10/2016
//=======================================================================================
struct DgnRscInMemoryData : IDgnRscFontData
{
private:
    Utf8String m_familyName;
    bvector<Byte> m_header;
    bvector<Byte> m_fractionMap;
    bvector<Byte> m_glyphData;
    bvector<Byte> m_glyphDataOffsets;
    bvector<Byte> m_glyphHeaders;

public:
    DgnRscInMemoryData(Utf8StringCR familyName) : m_familyName(familyName) { }

    void SetHeaderBuffer(ByteCP data, size_t dataSize) { m_header.clear(); std::copy(data, data + dataSize, std::back_inserter(m_header)); }
    void SetFractionMapBuffer(ByteCP data, size_t dataSize) { m_fractionMap.clear(); std::copy(data, data + dataSize, std::back_inserter(m_fractionMap)); }
    void SetGlyphDataBuffer(ByteCP data, size_t dataSize) { m_glyphData.clear(); std::copy(data, data + dataSize, std::back_inserter(m_glyphData)); }
    void SetGlyphDataOffsetsBuffer(ByteCP data, size_t dataSize) { m_glyphDataOffsets.clear(); std::copy(data, data + dataSize, std::back_inserter(m_glyphDataOffsets)); }
    void SetGlyphHeadersBuffer(ByteCP data, size_t dataSize) { m_glyphHeaders.clear(); std::copy(data, data + dataSize, std::back_inserter(m_glyphHeaders)); }

    bool IsValid() { return !m_familyName.empty() && !m_header.empty() && !m_glyphData.empty() && !m_glyphDataOffsets.empty() && !m_glyphHeaders.empty(); }

    virtual IDgnFontDataP _CloneWithoutData() override;
    virtual BentleyStatus _Embed(DgnFonts::DbFaceDataDirect&) override;
    virtual BentleyStatus _AddDataRef() override { return SUCCESS; }
    virtual void _ReleaseDataRef() override { }
    virtual BentleyStatus _ReadFontHeader(bvector<Byte>& buffer) override { std::copy(m_header.begin(), m_header.end(), std::back_inserter(buffer)); return SUCCESS; }
    virtual BentleyStatus _ReadFractionMap(bvector<Byte>& buffer) override { std::copy(m_fractionMap.begin(), m_fractionMap.end(), std::back_inserter(buffer)); return SUCCESS; }
    virtual BentleyStatus _ReadGlyphData(bvector<Byte>& buffer, size_t offset, size_t size) override { std::copy(m_glyphData.begin() + offset, m_glyphData.begin() + offset + size, std::back_inserter(buffer)); return SUCCESS; }
    virtual BentleyStatus _ReadGlyphDataOffsets(bvector<Byte>& buffer) override { std::copy(m_glyphDataOffsets.begin(), m_glyphDataOffsets.end(), std::back_inserter(buffer)); return SUCCESS; }
    virtual BentleyStatus _ReadGlyphHeaders(bvector<Byte>& buffer) override { std::copy(m_glyphHeaders.begin(), m_glyphHeaders.end(), std::back_inserter(buffer)); return SUCCESS; }
};

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2016
//---------------------------------------------------------------------------------------
IDgnFontDataP DgnRscInMemoryData::_CloneWithoutData()
    {
    DgnRscInMemoryData* clone = new DgnRscInMemoryData(m_familyName);
    clone->SetHeaderBuffer(&m_header[0], m_header.size());
    clone->SetFractionMapBuffer(&m_fractionMap[0], m_fractionMap.size());
    clone->SetGlyphDataBuffer(&m_glyphData[0], m_glyphData.size());
    clone->SetGlyphDataOffsetsBuffer(&m_glyphDataOffsets[0], m_glyphDataOffsets.size());
    clone->SetGlyphHeadersBuffer(&m_glyphHeaders[0], m_glyphHeaders.size());
    
    return clone;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2016
//---------------------------------------------------------------------------------------
BentleyStatus DgnRscInMemoryData::_Embed(DgnFonts::DbFaceDataDirect& faceData)
    {
    FlatBufferBuilder encoder;
    bvector<Byte> buffer;

    Offset<Vector<Byte>> fontHeaderOffset = encoder.CreateVector(m_header);
    Offset<Vector<Byte>> fractionMapOffset = encoder.CreateVector(m_fractionMap);
    Offset<Vector<Byte>> glyphDataOffset = encoder.CreateVector(m_glyphData);
    Offset<Vector<Byte>> glyphDataOffsetsOffset = encoder.CreateVector(m_glyphDataOffsets);
    Offset<Vector<Byte>> glyphHeadersOffset = encoder.CreateVector(m_glyphHeaders);
    
    FB::DgnRscFontBuilder builder(encoder);
    builder.add_majorVersion(1);
    builder.add_majorVersion(0);
    builder.add_header(fontHeaderOffset);
    builder.add_glyphData(glyphDataOffset);
    builder.add_glyphDataOffsets(glyphDataOffsetsOffset);
    builder.add_glyphHeaders(glyphHeadersOffset);

    if (0 != fractionMapOffset.o)
        builder.add_fractionMap(fractionMapOffset);

    encoder.Finish(builder.Finish());

    bvector<Byte> fontData;
    fontData.resize(encoder.GetSize());
    memcpy(&fontData[0], encoder.GetBufferPointer(), encoder.GetSize());

    DgnFonts::DbFaceDataDirect::FaceKey regularFace(DgnFontType::Rsc, m_familyName.c_str(), DgnFonts::DbFaceDataDirect::FaceKey::FACE_NAME_Regular);
    if (faceData.Exists(regularFace))
        {
        LOG.infov("Embedded data for RSC family '%s' already exists; it will not be automatically replaced.", regularFace.m_familyName.c_str());
        return ERROR;
        }

    DgnFonts::DbFaceDataDirect::T_FaceMap faces;
    faces[0] = regularFace;
    if (SUCCESS != faceData.Insert(&fontData[0], fontData.size(), faces))
        {
        LOG.errorv("Unable to write embedded font data for RSC family '%s'.", regularFace.m_familyName.c_str());
        return ERROR;
        }

    return SUCCESS;
    }

//=======================================================================================
// This is not intended to be a well-behaved data object that can freely release and re-acquire its resources as necessary; it will hold data until deleted.
// This is deemed sufficient for the converter's needs. Reading data back out of the structured store may not be possible due to the locks required, and is not particularly cheap anyway.
// This is strongly modeled after platform's DgnShxDbFontData.
// @bsiclass                                                    Jeff.Marker     10/2016
//=======================================================================================
struct DgnShxInMemoryData : IDgnShxFontData
{
private:
    Utf8String m_familyName;
    bvector<Byte> m_data;
    uint64_t m_dataPosition;

public:
    DgnShxInMemoryData(Utf8StringCR familyName) : m_familyName(familyName), m_dataPosition(0) { }

    void SetDataBuffer(ByteCP data, size_t dataSize) { m_data.clear(); std::copy(data, data + dataSize, std::back_inserter(m_data)); }

    virtual IDgnFontDataP _CloneWithoutData() override;
    virtual BentleyStatus _Embed(DgnFonts::DbFaceDataDirect&) override;
    virtual BentleyStatus _AddDataRef() override { return SUCCESS; }
    virtual void _ReleaseDataRef() override { }
    virtual size_t _Read(void* buffer, size_t size, size_t count) override;
    virtual BentleyStatus _Seek(int64_t offset, BeFileSeekOrigin origin) override;
    virtual uint64_t _Tell() override { return m_dataPosition; }
};

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2016
//---------------------------------------------------------------------------------------
IDgnFontDataP DgnShxInMemoryData::_CloneWithoutData()
    {
    DgnShxInMemoryData* clone = new DgnShxInMemoryData(m_familyName);
    clone->SetDataBuffer(&m_data[0], m_data.size());
    
    return clone;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2016
//---------------------------------------------------------------------------------------
BentleyStatus DgnShxInMemoryData::_Embed(DgnFonts::DbFaceDataDirect& faceData)
    {
    DgnFonts::DbFaceDataDirect::FaceKey regularFace(DgnFontType::Shx, m_familyName.c_str(), DgnFonts::DbFaceDataDirect::FaceKey::FACE_NAME_Regular);
    if (faceData.Exists(regularFace))
        return ERROR;

    DgnFonts::DbFaceDataDirect::T_FaceMap faces;
    faces[0] = regularFace;
    if (SUCCESS != faceData.Insert(&m_data[0], m_data.size(), faces))
        return ERROR;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2016
//---------------------------------------------------------------------------------------
size_t DgnShxInMemoryData::_Read(void* buffer, size_t size, size_t count)
    {
    size_t remainingBytes = (m_data.size() - (size_t)m_dataPosition);
    size_t requestedBytes = (size * count);
    size_t actuallyRead = std::min(remainingBytes, requestedBytes);

    memcpy(buffer, &m_data[(size_t)m_dataPosition], actuallyRead);
    m_dataPosition += actuallyRead;

    return actuallyRead;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2016
//---------------------------------------------------------------------------------------
BentleyStatus DgnShxInMemoryData::_Seek(int64_t offset, BeFileSeekOrigin origin)
    {
    int64_t newPosition;
    switch (origin)
        {
        case BeFileSeekOrigin::Current: newPosition = ((int64_t)m_dataPosition + offset); break;
        case BeFileSeekOrigin::End: newPosition = ((int64_t)m_data.size() + offset); break;
        case BeFileSeekOrigin::Begin: newPosition = (int64_t)offset; break;

        default:
            BeAssert(false);
            return ERROR;
        }

    if ((newPosition < 0) || ((size_t)newPosition >= m_data.size()))
        return ERROR;

    m_dataPosition = (uint64_t)newPosition;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
static void loadShxFontBuffer(Converter::T_WorkspaceFonts& workspaceFonts, Utf8String name, ByteCP data, size_t dataSize)
    {
    DgnShxInMemoryData* fontData = new DgnShxInMemoryData(name);
    fontData->SetDataBuffer(data, dataSize);

    DgnShxFontP font = new DgnShxFont(name.c_str(), fontData);
    
    workspaceFonts[Converter::T_WorkspaceFontKey(font->GetType(), font->GetName())] = font;

    // Use the V8 font system to read the font configuration if available.
    WString v8FontName(font->GetName().c_str(), BentleyCharEncoding::Utf8);
    DgnV8Api::DgnFont const* v8Font = DgnV8Api::DgnFontManager::FindSystemFont(v8FontName.c_str(), dbFontTypeToV8Filter(font->GetType()));
    if (nullptr != v8Font)
        {
        font->GetMetadataR().m_codePage = (BentleyApi::LangCodePage)v8Font->GetCodePage();
        font->GetMetadataR().m_degreeCode = v8Font->GetDegreeCharCode();
        font->GetMetadataR().m_diameterCode = v8Font->GetDiameterCharCode();
        font->GetMetadataR().m_plusMinusCode = v8Font->GetPlusMinusCharCode();
        
        return;
        }
    
    switch (font->GetShxType())
        {
        case DgnShxFont::ShxType::Unicode:
            font->GetMetadataR().m_codePage = LangCodePage::Unicode;
            font->GetMetadataR().m_degreeCode = 176;
            font->GetMetadataR().m_diameterCode = 8709;
            font->GetMetadataR().m_plusMinusCode = 177;
            break;

        case DgnShxFont::ShxType::Locale:
            BeStringUtilities::GetCurrentCodePage(font->GetMetadataR().m_codePage);
            font->GetMetadataR().m_degreeCode = 256;
            font->GetMetadataR().m_diameterCode = 258;
            font->GetMetadataR().m_plusMinusCode = 257;
            break;

        default:
            font->GetMetadataR().m_codePage = LangCodePage::Unknown;
            font->GetMetadataR().m_degreeCode = 0;
            font->GetMetadataR().m_diameterCode = 0;
            font->GetMetadataR().m_plusMinusCode = 0;
            break;
        }
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
void Converter::_LoadEmbeddedV8Fonts(DgnV8Api::DgnFile& v8File)
    {
    // Expect to be able to open the root store.
    IStoragePtr rootStg;
    HRESULT hr = ::StgOpenStorageEx(v8File.GetFileName().c_str(), (STGM_SHARE_DENY_WRITE | STGM_READ), STGFMT_ANY, 0, NULL, 0, IID_IStorage, (void**)&rootStg);
    
    // DgnV7 files can come in here, which are not structured stores.
    if (E_NOINTERFACE == hr)
        return;
    
    if (FAILED(hr))
        { BeAssert(false); return; }

    // Use Bern's API for decoding file name from stream name.
    static const WCharCP EMBEDDED_FONT_STORAGE_NAME = L".EmbeddedFonts";
    V8IModelExtraFiles::EmbeddedFileListPtr v8EmbeddedFiles = V8IModelExtraFiles::EmbeddedFileList::ReadFromPackagedIModel(rootStg, EMBEDDED_FONT_STORAGE_NAME);
    if (nullptr == v8EmbeddedFiles.get())
        return;

    // See if the storage exists. This must be done after the call to V8IModelExtraFiles::EmbeddedFileList::ReadFromPackagedIModel, because it requires an exclusive lock on this storage.
    IStoragePtr embeddedFilesStg;
    hr = rootStg->OpenStorage(EMBEDDED_FONT_STORAGE_NAME, NULL, (STGM_SHARE_EXCLUSIVE | STGM_READ), NULL, 0, &embeddedFilesStg);
    if (FAILED(hr))
        return;

    // RSC is *:1 between blob and font.
    // SHX just works (1:1 relationship between blob and font).
    // TrueType is *:* between blob and font(s).
    //  Further, due to the *:* nature, platform's TrueType-from-file logic is very complicated and I don't want to duplicate that here.
    //  The most expedient "good enough" approach for the converter will be to write temp files to re-use this logic. 

    typedef bmap<bpair<uint32_t,WString>,DgnRscInMemoryData*> T_RscFontDataMap;
    T_RscFontDataMap rscFontDatas; // to collect the various pieces in different streams to be combined into a single font later
    bvector<BeFileName> trueTypeFontPaths; // to aggregate all files to have platform process them in bulk at the end

    for (auto fileIter = v8EmbeddedFiles->cbegin(); v8EmbeddedFiles->cend() != fileIter; ++fileIter)
        {
        auto fileName = (*fileIter)->GetFileName();

        // File name is of the format FontNumber.FontType[-FileSubType].FontName
        bvector<WString> fileNameComponents;
        BeStringUtilities::Split(fileName, L".", NULL, fileNameComponents);
        if (3 != fileNameComponents.size())
            { BeAssert(false); continue; }

        uint32_t fontNumber;
        if (swscanf(fileNameComponents[0].c_str(), L"%u", &fontNumber) <= 0)
            { BeAssert(false); continue; }

        WString fontTypeComponent = fileNameComponents[1];
        auto fileSubTypeOffset = fontTypeComponent.find(L'-');
        WString fileSubTypeComponent;
        if (WString::npos != fileSubTypeOffset)
            {
            fileSubTypeComponent = fontTypeComponent.substr(fileSubTypeOffset + 1);
            fontTypeComponent.resize(fileSubTypeOffset);
            }

        WString fontNameComponent = fileNameComponents[2];
        if (fileNameComponents.size() > 3)
            std::for_each(fileNameComponents.begin() + 3, fileNameComponents.end(), [&](WStringCR item) { fontNameComponent += L"."; fontNameComponent += item; });

        int fontTypeNumber = 0;

        if (fontTypeComponent.EqualsI(L"rsc")) { fontTypeNumber = (int)DgnFontType::Rsc; }
        else if (fontTypeComponent.EqualsI(L"shx")) { fontTypeNumber = (int)DgnFontType::Shx; }
        else if (fontTypeComponent.EqualsI(L"tt")) { fontTypeNumber = (int)DgnFontType::TrueType; }
        else { BeAssert(false); continue; }

        // Get an IStream for this object.
        IStreamPtr fileStream;
        hr = embeddedFilesStg->OpenStream((*fileIter)->GetStreamName().c_str(), NULL, (STGM_SHARE_EXCLUSIVE | STGM_READ), 0, &fileStream);
        if (FAILED(hr))
            { BeAssert(false); continue; }

        // Determine stream size.
        STATSTG fileStreamStat;
        hr = fileStream->Stat(&fileStreamStat, STATFLAG_NONAME);
        if (FAILED(hr))
            { BeAssert(false); continue; }

        auto fileStreamSize = fileStreamStat.cbSize.QuadPart;
        if (0 == fileStreamStat.cbSize.QuadPart)
            { BeAssert(false); continue; }

        // Read the entire stream into memory so we can embed it.
        // We still support 32-bit importers, so be mindful (e.g. don't crash) with large files.
        try
            {
            ScopedArray<Byte> fileData(static_cast<size_t>(fileStreamSize));
            ULONG numBytesRead;
            hr = fileStream->Read(fileData.GetData(), (ULONG)fileStreamSize, &numBytesRead);
            if (FAILED(hr) || (numBytesRead != fileStreamSize))
                { BeAssert(false); continue; }

            if (fontTypeComponent.EqualsI(L"rsc"))
                {
                T_RscFontDataMap::iterator existingData = rscFontDatas.find(T_RscFontDataMap::key_type(fontNumber, fontNameComponent));
                DgnRscInMemoryData* data;
                if (rscFontDatas.end() != existingData)
                    {
                    data = existingData->second;
                    }
                else
                    {
                    data = new DgnRscInMemoryData(Utf8String(fontNameComponent.c_str()));
                    rscFontDatas[T_RscFontDataMap::key_type(fontNumber, fontNameComponent)] = data;
                    }
                
                if (fileSubTypeComponent.empty()) { data->SetHeaderBuffer(fileData.GetData(), (size_t)fileStreamSize); }
                else if (fileSubTypeComponent.EqualsI(L"chrhdrs")) { data->SetGlyphHeadersBuffer(fileData.GetData(), (size_t)fileStreamSize); }
                else if (fileSubTypeComponent.EqualsI(L"vecoffs")) { data->SetGlyphDataOffsetsBuffer(fileData.GetData(), (size_t)fileStreamSize); }
                else if (fileSubTypeComponent.EqualsI(L"vecs")) { data->SetGlyphDataBuffer(fileData.GetData(), (size_t)fileStreamSize); }
                else if (fileSubTypeComponent.EqualsI(L"fracs")) { data->SetFractionMapBuffer(fileData.GetData(), (size_t)fileStreamSize); }
                else { BeAssert(false); /* Unknown/unexpected file sub-type. */ }
                }
            else if (fontTypeComponent.EqualsI(L"shx"))
                {
                BeAssert(fileSubTypeComponent.empty()); // SHX doesn't support multiple files.
                loadShxFontBuffer(m_workspaceFonts, Utf8String(fontNameComponent), fileData.GetData(), (size_t)fileStreamSize);
                }
            else if (fontTypeComponent.EqualsI(L"tt"))
                {
                BeAssert(fileSubTypeComponent.empty()); // TrueType doesn't support multiple files.
                
                WCHAR tempPath[MAX_PATH];
                if (0 == ::GetTempPathW(MAX_PATH, tempPath))
                    { BeAssert(false); continue; }

                WCHAR tempName[MAX_PATH];
                if (0 == ::GetTempFileNameW(tempPath, L"fnt", 0, tempName))
                    { BeAssert(false); continue; }

                HANDLE tempFile = ::CreateFileW(tempName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                if (INVALID_HANDLE_VALUE == tempFile)
                    { BeAssert(false); continue; }
                
                struct AutoCloseFile {
                    private: HANDLE m_handle;
                    public: AutoCloseFile(HANDLE h) : m_handle(h) {}
                    ~AutoCloseFile() {::CloseHandle(m_handle); }
                    } autoClose(tempFile);
                
                BeFileName beTempName(tempName);
                m_tempFontFiles.insert(beTempName);
                
                DWORD bytesWritten = 0;
                if ((0 == ::WriteFile(tempFile, fileData.GetData(), fileStreamSize, &bytesWritten, nullptr)) || (bytesWritten != (DWORD)fileStreamSize))
                    { BeAssert(false); continue; }

                trueTypeFontPaths.push_back(beTempName);
                }
            else
                {
                BeAssert(false); // Unknown/unexpected font type.
                }
            }
        catch (std::bad_alloc)
            {
            BeAssert(false);
            }
        }
    
    for (T_RscFontDataMap::const_reference rscFontData : rscFontDatas)
        {
        // TFS#729926 and TFS#729931 have corrupt RSC fonts embedded in their pakcaged .i.dgn.
        // RSC fonts are required to have a font header, glyph headers, glyph data offesets, and glyph data; fractions are optional.
        // If any imported RSC fonts fail to have all required data, pretend they didn't exist because we can't use them later.
        DgnRscInMemoryData* data = rscFontData.second;
        if (nullptr == data || !data->IsValid())
            {
            //BeDataAssert(false);
            continue;
            }
        
        DgnRscFontP rscFont = new DgnRscFont(Utf8String(rscFontData.first.second).c_str(), data);
        m_workspaceFonts[T_WorkspaceFontKey(rscFont->GetType(), rscFont->GetName())] = rscFont;
        
        // Need to save off RSC number map because DgnV8 knows nothing about these embedded fonts.
        // And when _RemapV8Font is called later, we have to know to look at this map first to inject the embedded fonts since normal V8 logic will fail to know these exist.
        // Note that I believe we only need this interjection for RSC fonts because DgnV8 can't even help us look up RSC fonts by name; other font types exist in DgnV8's map, so we can at least find them through normal means via type/name.
        m_v8EmbeddedRscFontMap[rscFontData.first.first] = rscFont;
        }
    
    if (!trueTypeFontPaths.empty())
        {
        T_DgnFontPtrs ttFonts = DgnFontPersistence::File::FromTrueTypeFiles(trueTypeFontPaths, nullptr);
        for (T_DgnFontPtrs::const_reference ttFont : ttFonts)
            m_workspaceFonts[T_WorkspaceFontKey(ttFont->GetType(), ttFont->GetName())] = ttFont;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2015
//---------------------------------------------------------------------------------------
void Converter::_EnsureWorkspaceFontsAreLoaded()
    {
    if (m_hasLoadedWorkspaceFonts)
        return;

    m_hasLoadedWorkspaceFonts = true;

    // First try to load fonts embedded in the DgnV8 file.
    // True, these aren't strictly "workspace" fonts, but for purposes of a DgnV8 converter, this should be good enough.
    // Font embedding only happens for i-models. Further, during i-model publishing, embedded fonts are aggregated to the root file.
    _LoadEmbeddedV8Fonts(this->_GetFontRootV8File());

    Utf8String fontSearchPathStr = _GetWorkspaceFontSearchPaths();
    if (fontSearchPathStr.empty())
        return;

    static BeFileName s_appRoot;
    if (s_appRoot.empty())
        {
        WChar moduleFileName[MAX_PATH];
        ::GetModuleFileNameW(nullptr, moduleFileName, _countof(moduleFileName));
        s_appRoot = BeFileName(BeFileName::DevAndDir, moduleFileName);
        }

    BeFileName fontSearchPathStrW(fontSearchPathStr.c_str(), BentleyCharEncoding::Utf8);
    fontSearchPathStrW.ReplaceAll(L"$(AppRoot)", s_appRoot.c_str());
    fontSearchPathStrW.ReplaceAll(L"$(v8SdkDir)", GetParams().GetV8SdkRelativeDir().c_str());

    if (WString::npos != fontSearchPathStrW.find(L"$(V8WorkspaceFontPaths)"))
        {
        WString workspacePaths;
        Bentley::WString paths;
        
        DgnV8Api::DgnPlatformLib::GetHost().GetFontAdmin()._GetRscFontPaths(paths);
        if (!paths.empty())
            {
            if (!workspacePaths.empty())
                workspacePaths += L";";

            workspacePaths += paths.c_str();
            }

        DgnV8Api::DgnPlatformLib::GetHost().GetFontAdmin()._GetShxFontPaths(paths);
        if (!paths.empty())
            {
            if (!workspacePaths.empty())
                workspacePaths += L";";

            workspacePaths += paths.c_str();
            }

        DgnV8Api::DgnPlatformLib::GetHost().GetFontAdmin()._GetTrueTypeFontPaths(paths);
        if (!paths.empty())
            {
            if (!workspacePaths.empty())
                workspacePaths += L";";

            workspacePaths += paths.c_str();
            }
        
        fontSearchPathStrW.ReplaceAll(L"$(V8WorkspaceFontPaths)", workspacePaths.c_str());
        }

    bvector<WString> allFontSearchPaths;
    BeStringUtilities::Split(fontSearchPathStrW.c_str(), L";", nullptr, allFontSearchPaths);
    
    // Beneficial to de-dup, but order is important...
    bvector<WString> fontSearchPaths;
    for (WStringCR pathW : allFontSearchPaths)
        {
        if (fontSearchPaths.end() == std::find(fontSearchPaths.begin(), fontSearchPaths.end(), pathW))
            fontSearchPaths.push_back(pathW);
        }

    // Since multiple TrueType files may be required for a single font, separate them and embed them as one unit so multiple files can be considered at once.
    bvector<BeFileName> trueTypePaths;

    for (WStringCR pathW : fontSearchPaths)
        {
        BeFileName path(pathW);
        if (path.IsDirectory())
            path.AppendToPath(L"*.*");
    
        BeFileListIterator pathIter(path.c_str(), false);
        BeFileName nextPath;
        while (SUCCESS == pathIter.GetNextFileName(nextPath))
            {
            WString extension = nextPath.GetExtension();
            extension.ToLower();
            if (extension.Equals(L"ttf") || extension.Equals(L"ttc") || extension.Equals(L"otf") || extension.Equals(L"otc"))
                {
                trueTypePaths.push_back(nextPath);
                continue;
                }
        
            loadFontFile(m_workspaceFonts, nextPath);
            }
        }
    
    if (!trueTypePaths.empty())
        loadTrueTypeFontFiles(m_workspaceFonts, trueTypePaths);
    
    loadOSFonts(m_workspaceFonts);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2015
//---------------------------------------------------------------------------------------
static DgnFontType v8FontTypeToDb(DgnV8Api::DgnFontType v8Type)
    {
    switch (v8Type)
        {
        case DgnV8Api::DgnFontType::Rsc: return DgnFontType::Rsc;
        case DgnV8Api::DgnFontType::Shx: return DgnFontType::Shx;
        case DgnV8Api::DgnFontType::TrueType: return DgnFontType::TrueType;
        }

    BeAssert(false);
    return DgnFontType::TrueType;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2015
//---------------------------------------------------------------------------------------
DgnFont const* Converter::_ImportV8Font(DgnV8Api::DgnFont const& v8Font)
    {
    _EnsureWorkspaceFontsAreLoaded();

    T_WorkspaceFonts::const_iterator foundWorkspaceFont = m_workspaceFonts.find(T_WorkspaceFontKey(v8FontTypeToDb(v8Font.GetType()), Utf8String(v8Font.GetName().c_str())));
    if (m_workspaceFonts.end() != foundWorkspaceFont)
        return foundWorkspaceFont->second.get();

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2015
//---------------------------------------------------------------------------------------
DgnFont const& Converter::_RemapV8Font(DgnV8Api::DgnFile& v8File, uint32_t v8FontId)
    {
    // Did we already remap it?
    T_FontRemapKey remapKey(&v8File, v8FontId);
    T_FontRemap::const_iterator foundRemap = m_fontRemap.find(remapKey);
    if (m_fontRemap.end() != foundRemap)
        return *foundRemap->second;

    // Was it embedded in the DgnV8 file? Even though fonts can be embedded in the structure storage, DgnV8 itself knows nothing about them, hence we need to interject.
    // Note that I believe we only need this interjection for RSC fonts because DgnV8 can't even help us look up RSC fonts by name; other font types exist in DgnV8's map, so we can at least find them through normal means via type/name.
    _EnsureWorkspaceFontsAreLoaded();
    T_V8EmbeddedRscFontMap::const_iterator foundEmbeddedFont = m_v8EmbeddedRscFontMap.find(v8FontId);
    if (m_v8EmbeddedRscFontMap.end() != foundEmbeddedFont)
        return *(foundEmbeddedFont->second);
    
    // Otherwise need to create a mapping.
    // In both DgnV8 and DgnDb, fonts are unique by type and name. Note that multiple entries can exist in our remap for the same DgnDb font because it may have different IDs among the DgnV8 files.
    DgnV8Api::DgnFont const* v8Font = v8File.GetDgnFontMapP()->GetFontP(v8FontId);
    DgnFontCP dbFont = nullptr;

    // Corrupt DgnV8 font ID? Note that nullptr means no entry; not whether it's missing or not.
    if (nullptr == v8Font)
        {
        // DgnV8 reserves font ID ranges for types; remap to the nearest last resort font.
        if (v8FontId <= DgnV8Api::MAX_USTN_NUMBER)
            dbFont = &T_HOST.GetFontAdmin().GetLastResortRscFont();
        else if (v8FontId <= DgnV8Api::MAX_SHX_NUMBER)
            dbFont = &T_HOST.GetFontAdmin().GetLastResortShxFont();
        else
            dbFont = &T_HOST.GetFontAdmin().GetLastResortTrueTypeFont();
        }
    else
        {
        DgnFontType dbFontType = v8FontTypeToDb(v8Font->GetType());
        Utf8String dbFontName(v8Font->GetName().c_str());

        // See if our DB already knows of a font by that type and name.
        dbFont = m_dgndb->Fonts().FindFontByTypeAndName(dbFontType, dbFontName.c_str());

        // Otherwise need to find it in the search paths and import it.
        if (nullptr == dbFont)
            {
            dbFont = _ImportV8Font(*v8Font);
            
            // Converter wasn't configured to find the font? I suppose we should bring across the type and name and make a missing font...
            //  even though for all intents and purposes it will only ever be serviced by a last resort font.
            if (nullptr == dbFont)
                {
                DgnFontPtr missingFont = DgnFontPersistence::Missing::CreateMissingFont(dbFontType, dbFontName.c_str());
                m_workspaceFonts[T_WorkspaceFontKey(missingFont->GetType(), missingFont->GetName())] = missingFont;
                dbFont = missingFont.get();
                }
            }
        }

    m_fontRemap[remapKey] = const_cast<DgnFont*>(dbFont);
    
    BeAssert(nullptr != dbFont);
    return *dbFont;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2015
//---------------------------------------------------------------------------------------
static bool shouldEmbedUsedFont(Converter::Config const& importConfig, DgnFontType type, Utf8CP name)
    {
    Utf8String fontTypeString;
    switch (type)
        {
        case DgnFontType::Rsc: fontTypeString = "RSC"; break;
        case DgnFontType::Shx: fontTypeString = "SHX"; break;
        case DgnFontType::TrueType: fontTypeString = "TrueType"; break;
        default:
            BeAssert(false);
            return false;
        }
    
    Utf8String embedAction;
    
    // Node matching explicit type and name takes precedence.
    importConfig.EvaluateXPath(embedAction, Utf8PrintfString("/ImportConfig/Fonts/Font[@type='%s' and @name='%s']/EmbedAction", fontTypeString.c_str(), name).c_str());
    
    // Check for name wild card.
    if (embedAction.empty())
        importConfig.EvaluateXPath(embedAction, Utf8PrintfString("/ImportConfig/Fonts/Font[@type='*' and @name='%s']/EmbedAction", name).c_str());

    // Check for type wild card.
    if (embedAction.empty())
        importConfig.EvaluateXPath(embedAction, Utf8PrintfString("/ImportConfig/Fonts/Font[@type='%s' and @name='*']/EmbedAction", fontTypeString.c_str()).c_str());
    
    // Check for full wild card.
    if (embedAction.empty())
        importConfig.EvaluateXPath(embedAction, "/ImportConfig/Fonts/Font[@type='*' and @name='*']/EmbedAction");
    
    if (embedAction.empty())
        return true; // Otherwise, documented default is to embed all used fonts.

    return (embedAction.Equals("IfUsed") || embedAction.Equals("Always"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2015
//---------------------------------------------------------------------------------------
void Converter::_EmbedFonts()
    {
    SetStepName(Converter::ProgressMessage::STEP_EMBED_FONTS());
    
    // By default, embed any used fonts.
    // The converter can only reasonably care about embedding workspace fonts... otherwise anything in the DB is outside its scope.
    DgnFonts::DbFontMapDirect::Iterator allFonts = m_dgndb->Fonts().DbFontMap().MakeIterator();
    for (DgnFonts::DbFontMapDirect::Iterator::Entry const& fontEntry : allFonts)
        {
        // Do NOT use a DgnFont object from the DB... the DB is self-contained, and if asked for a font pre-embedding, it will give you a DgnFont object with unresolved data.
        // Use the type and name to look up a workspace font and embed it. When a DB-based font attempts to resolve later, it will find this embedded data by type and name.
        T_WorkspaceFonts::const_iterator foundWorkspaceFont = m_workspaceFonts.find(T_WorkspaceFontKey(fontEntry.GetType(), fontEntry.GetName()));
        if (m_workspaceFonts.end() == foundWorkspaceFont)
            continue;

        if (!shouldEmbedUsedFont(m_config, fontEntry.GetType(), fontEntry.GetName()))
            continue;
        
        if (SUCCESS != DgnFontPersistence::Db::Embed(m_dgndb->Fonts().DbFaceData(), *foundWorkspaceFont->second))
            ReportIssueV(Converter::IssueSeverity::Warning, IssueCategory::MissingData(), Issue::CannotEmbedFont(), nullptr, (int)fontEntry.GetType(), fontEntry.GetName());
        
        ReportProgress();
        }
    
    // Check the config for any forced embedding we can find in the workspace.
    if (nullptr != m_config.GetDom())
        {
        BeXmlDom::IterableNodeSet fontsToAlwaysEmbed;
        m_config.GetDom()->SelectNodes(fontsToAlwaysEmbed, "/ImportConfig/Fonts/Font[EmbedAction='Always']", nullptr);
        for (BeXmlNodeP node : fontsToAlwaysEmbed)
            {
            Utf8String fontTypeName;
            DgnFontType fontType;
            node->GetAttributeStringValue(fontTypeName, "type");
            if (fontTypeName.Equals("RSC")) fontType = DgnFontType::Rsc;
            else if (fontTypeName.Equals("SHX")) fontType = DgnFontType::Shx;
            else if (fontTypeName.Equals("TrueType")) fontType = DgnFontType::TrueType;
            else continue;
            
            Utf8String fontName;
            node->GetAttributeStringValue(fontName, "name");

            // Just in case the file's data used no fonts (in which case we would have never populated the "workspace").
            // This quick-returns, so not an issue to call in the loop.
            _EnsureWorkspaceFontsAreLoaded();

            T_WorkspaceFonts::const_iterator foundWorkspaceFont = m_workspaceFonts.find(T_WorkspaceFontKey(fontType, fontName));
            if (m_workspaceFonts.end() == foundWorkspaceFont)
                continue;

            if (SUCCESS != DgnFontPersistence::Db::Embed(m_dgndb->Fonts().DbFaceData(), *foundWorkspaceFont->second))
                ReportIssueV(Converter::IssueSeverity::Warning, IssueCategory::MissingData(), Issue::CannotEmbedFont(), nullptr, (int)fontType, fontName.c_str());

            ReportProgress();
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2015
//---------------------------------------------------------------------------------------
DgnFontCP Converter::_TryResolveFont(DgnFontCP requestedFont)
    {
    if (nullptr == requestedFont)
        return nullptr;
    
    T_WorkspaceFonts::const_iterator foundWorkspaceFont = m_workspaceFonts.find(T_WorkspaceFontKey(requestedFont->GetType(), requestedFont->GetName()));
    if (m_workspaceFonts.end() == foundWorkspaceFont)
        return nullptr;
    
    return foundWorkspaceFont->second.get();
    }











//---------------------------------------------------------------------------------------
// @bsimethod                                                   Vern.Francisco     01/2018
//!
//! This function is a clone of the Converter methods to support the LigthWeightConverter
//!
//---------------------------------------------------------------------------------------
void LightWeightConverter::_EnsureWorkspaceFontsAreLoaded()
    {
    if (m_hasLoadedWorkspaceFonts)
        return;

    m_hasLoadedWorkspaceFonts = true;

    Utf8String fontSearchPathStr; 

    static BeFileName s_appRoot;
    if (s_appRoot.empty())
        {
        WChar moduleFileName[MAX_PATH];
        ::GetModuleFileNameW(nullptr, moduleFileName, _countof(moduleFileName));
        s_appRoot = BeFileName(BeFileName::DevAndDir, moduleFileName);
        }

    WString workspacePaths;
    Bentley::WString paths;

    DgnV8Api::DgnPlatformLib::GetHost().GetFontAdmin()._GetRscFontPaths(paths);
    if (!paths.empty())
        {
        if (!workspacePaths.empty())
            workspacePaths += L";";

        workspacePaths += paths.c_str();
        }

    DgnV8Api::DgnPlatformLib::GetHost().GetFontAdmin()._GetShxFontPaths(paths);
    if (!paths.empty())
        {
        if (!workspacePaths.empty())
            workspacePaths += L";";

        workspacePaths += paths.c_str();
        }

    DgnV8Api::DgnPlatformLib::GetHost().GetFontAdmin()._GetTrueTypeFontPaths(paths);
    if (!paths.empty())
        {
        if (!workspacePaths.empty())
            workspacePaths += L";";

        workspacePaths += paths.c_str();
        }

    BeFileName fontSearchPathStrW(workspacePaths.c_str());

    bvector<WString> allFontSearchPaths;
    BeStringUtilities::Split(fontSearchPathStrW.c_str(), L";", nullptr, allFontSearchPaths);

    // Beneficial to de-dup, but order is important...
    bvector<WString> fontSearchPaths;
    for (WStringCR pathW : allFontSearchPaths)
        {
        if (fontSearchPaths.end() == std::find(fontSearchPaths.begin(), fontSearchPaths.end(), pathW))
            fontSearchPaths.push_back(pathW);
        }

    // Since multiple TrueType files may be required for a single font, separate them and embed them as one unit so multiple files can be considered at once.
    bvector<BeFileName> trueTypePaths;

    for (WStringCR pathW : fontSearchPaths)
        {
        BeFileName path(pathW);
        if (path.IsDirectory())
            path.AppendToPath(L"*.*");

        BeFileListIterator pathIter(path.c_str(), false);
        BeFileName nextPath;
        while (SUCCESS == pathIter.GetNextFileName(nextPath))
            {
            WString extension = nextPath.GetExtension();
            extension.ToLower();
            if (extension.Equals(L"ttf") || extension.Equals(L"ttc") || extension.Equals(L"otf") || extension.Equals(L"otc"))
                {
                trueTypePaths.push_back(nextPath);
                continue;
                }

            loadFontFile(m_workspaceFonts, nextPath);
            }
        }

    if (!trueTypePaths.empty())
        loadTrueTypeFontFiles(m_workspaceFonts, trueTypePaths);

    loadOSFonts(m_workspaceFonts);
    }
	
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Vern.Francisco     01/2018
//!
//! This function is a clone of the Converter methods to support the LigthWeightConverter
//!
//---------------------------------------------------------------------------------------
void LightWeightConverter::_EmbedFonts()
    {
    // By default, embed any used fonts.
    // The converter can only reasonably care about embedding workspace fonts... otherwise anything in the DB is outside its scope.
    DgnFonts::DbFontMapDirect::Iterator allFonts = m_dgndb->Fonts().DbFontMap().MakeIterator();
    for (DgnFonts::DbFontMapDirect::Iterator::Entry const& fontEntry : allFonts)
        {
        // Do NOT use a DgnFont object from the DB... the DB is self-contained, and if asked for a font pre-embedding, it will give you a DgnFont object with unresolved data.
        // Use the type and name to look up a workspace font and embed it. When a DB-based font attempts to resolve later, it will find this embedded data by type and name.
        T_WorkspaceFonts::const_iterator foundWorkspaceFont = m_workspaceFonts.find(T_WorkspaceFontKey(fontEntry.GetType(), fontEntry.GetName()));
        if (m_workspaceFonts.end() == foundWorkspaceFont)
            continue;

        if (SUCCESS != DgnFontPersistence::Db::Embed(m_dgndb->Fonts().DbFaceData(), *foundWorkspaceFont->second))
            {
//            ReportIssueV(Converter::IssueSeverity::Warning, IssueCategory::MissingData(), Issue::CannotEmbedFont(), nullptr, (int) fontEntry.GetType(), fontEntry.GetName());
            }
        }
    }

	
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Vern.Francisco     01/2018
//!
//! This function is a clone of the Converter methods to support the LigthWeightConverter
//!
//---------------------------------------------------------------------------------------
DgnFont const* LightWeightConverter::_ImportV8Font(DgnV8Api::DgnFont const& v8Font)
    {
    _EnsureWorkspaceFontsAreLoaded();

    T_WorkspaceFonts::const_iterator foundWorkspaceFont = m_workspaceFonts.find(T_WorkspaceFontKey(v8FontTypeToDb(v8Font.GetType()), Utf8String(v8Font.GetName().c_str())));
    if (m_workspaceFonts.end() != foundWorkspaceFont)
        return foundWorkspaceFont->second.get();

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Vern.Francisco     01/2018
//!
//! This function is a clone of the Converter methods to support the LigthWeightConverter
//!
//---------------------------------------------------------------------------------------
DgnFont const& LightWeightConverter::_RemapV8Font(DgnV8Api::DgnFile& v8File, uint32_t v8FontId)
    {
    // Did we already remap it?
    T_FontRemapKey remapKey(&v8File, v8FontId);
    T_FontRemap::const_iterator foundRemap = m_fontRemap.find(remapKey);
    if (m_fontRemap.end() != foundRemap)
        return *foundRemap->second;

    // Was it embedded in the DgnV8 file? Even though fonts can be embedded in the structure storage, DgnV8 itself knows nothing about them, hence we need to interject.
    // Note that I believe we only need this interjection for RSC fonts because DgnV8 can't even help us look up RSC fonts by name; other font types exist in DgnV8's map, so we can at least find them through normal means via type/name.
    _EnsureWorkspaceFontsAreLoaded();
    T_V8EmbeddedRscFontMap::const_iterator foundEmbeddedFont = m_v8EmbeddedRscFontMap.find(v8FontId);
    if (m_v8EmbeddedRscFontMap.end() != foundEmbeddedFont)
        return *(foundEmbeddedFont->second);

    // Otherwise need to create a mapping.
    // In both DgnV8 and DgnDb, fonts are unique by type and name. Note that multiple entries can exist in our remap for the same DgnDb font because it may have different IDs among the DgnV8 files.
    DgnV8Api::DgnFont const* v8Font = v8File.GetDgnFontMapP()->GetFontP(v8FontId);
    DgnFontCP dbFont = nullptr;

    // Corrupt DgnV8 font ID? Note that nullptr means no entry; not whether it's missing or not.
    if (nullptr == v8Font)
        {
        // DgnV8 reserves font ID ranges for types; remap to the nearest last resort font.
        if (v8FontId <= DgnV8Api::MAX_USTN_NUMBER)
            dbFont = &T_HOST.GetFontAdmin().GetLastResortRscFont();
        else if (v8FontId <= DgnV8Api::MAX_SHX_NUMBER)
            dbFont = &T_HOST.GetFontAdmin().GetLastResortShxFont();
        else
            dbFont = &T_HOST.GetFontAdmin().GetLastResortTrueTypeFont();
        }
    else
        {
        DgnFontType dbFontType = v8FontTypeToDb(v8Font->GetType());
        Utf8String dbFontName(v8Font->GetName().c_str());

        // See if our DB already knows of a font by that type and name.
        dbFont = m_dgndb->Fonts().FindFontByTypeAndName(dbFontType, dbFontName.c_str());

        // Otherwise need to find it in the search paths and import it.
        if (nullptr == dbFont)
            {
            dbFont = _ImportV8Font(*v8Font);

            // Converter wasn't configured to find the font? I suppose we should bring across the type and name and make a missing font...
            //  even though for all intents and purposes it will only ever be serviced by a last resort font.
            if (nullptr == dbFont)
                {
                DgnFontPtr missingFont = DgnFontPersistence::Missing::CreateMissingFont(dbFontType, dbFontName.c_str());
                m_workspaceFonts[T_WorkspaceFontKey(missingFont->GetType(), missingFont->GetName())] = missingFont;
                dbFont = missingFont.get();
                }
            }
        }

    m_fontRemap[remapKey] = const_cast<DgnFont*>(dbFont);

    BeAssert(nullptr != dbFont);
    return *dbFont;
    }

END_DGNDBSYNC_DGNV8_NAMESPACE
