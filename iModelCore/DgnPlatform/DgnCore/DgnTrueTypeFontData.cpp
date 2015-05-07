/*--------------------------------------------------------------------------------------+
|     $Source: DgnCore/DgnTrueTypeFontData.cpp $
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
+--------------------------------------------------------------------------------------*/

#if defined (BENTLEY_WIN32)
    #define UNICODE
    #include <Windows.h>
#endif

#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnCore/DgnFontData.h>
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
static DgnFontStyle ftStyleIndexToDgnFontStyle(FT_Long styleIndex)
    {
    DgnFontStyle style = DgnFontStyle::Regular;
    switch (styleIndex)
        {
        case FT_STYLE_FLAG_BOLD: style = DgnFontStyle::Bold; break;
        case FT_STYLE_FLAG_ITALIC: style = DgnFontStyle::Italic; break;
        case (FT_STYLE_FLAG_BOLD | FT_STYLE_FLAG_ITALIC) : style = DgnFontStyle::BoldItalic; break;
        default:
            BeAssert(0 == styleIndex);
            break;
        }
    
    return style;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2015
//---------------------------------------------------------------------------------------
static Utf8String dgnFontStyleToStyleName(DgnFontStyle style)
    {
    Utf8String styleName = DgnFonts::DbFaceDataDirect::FaceKey::FACE_NAME_Regular;
    switch (style)
        {
        case DgnFontStyle::Bold: styleName = DgnFonts::DbFaceDataDirect::FaceKey::FACE_NAME_Bold; break;
        case DgnFontStyle::Italic: styleName = DgnFonts::DbFaceDataDirect::FaceKey::FACE_NAME_Italic; break;
        case DgnFontStyle::BoldItalic: styleName = DgnFonts::DbFaceDataDirect::FaceKey::FACE_NAME_BoldItalic; break;
        default:
            BeAssert(DgnFontStyle::Regular == style);
            break;
        }

    return styleName;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2015
//---------------------------------------------------------------------------------------
static bool haveEmbeddingRights(FT_Face face)
    {
    if (NULL == face)
        return false;

    TT_OS2 const* os2Table = static_cast<TT_OS2 const*>(FT_Get_Sfnt_Table(face, ft_sfnt_os2));
    if (NULL == os2Table)
        return false;

    return (0 == (0x1 & os2Table->fsType));
    }

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     04/2015
//=======================================================================================
struct DgnTrueTypeFileFontData : IDgnTrueTypeFontData
{
private:
    typedef bset<FT_Long> T_FaceIndices;
    typedef bmap<BeFileName, bset<FT_Long>> T_FileMap;
    typedef bmap<DgnFontStyle, FT_Face> T_FaceMap;
    
    size_t m_openCount;
    T_FileMap m_fileMap;
    T_FaceMap m_faceMap;
    Utf8String m_familyName;

    void FreeAllFaces();

public:
    DgnTrueTypeFileFontData(Utf8CP familyName) : m_openCount(0), m_familyName(familyName) {}
    void RegisterFace(BeFileNameCR, FT_Long faceIndex);
    virtual IDgnFontData* _CloneWithoutData() override;
    virtual BentleyStatus _Embed(DgnFonts::DbFaceDataDirect&) override;
    virtual BentleyStatus _AddDataRef() override;
    virtual void _ReleaseDataRef() override;
    virtual FT_Face _GetFaceP(DgnFontStyle) override;
};

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2015
//---------------------------------------------------------------------------------------
void DgnTrueTypeFileFontData::FreeAllFaces()
    {
    for (T_FaceMap::const_reference faceMapEntry : m_faceMap)
        {
        FT_Error ftStatus = FT_Done_Face(faceMapEntry.second);
        if (FT_Err_Ok != ftStatus)
            FONT_LOG.warningv("Could not close face from family '%s' with DGN style %i (error code %i). This may be the source of a leak.", m_familyName.c_str(), (int)faceMapEntry.first, (int)ftStatus);
        }
    
    m_faceMap.clear();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2015
//---------------------------------------------------------------------------------------
void DgnTrueTypeFileFontData::RegisterFace(BeFileNameCR path, FT_Long faceIndex)
    {
    T_FileMap::iterator foundPathEntry = m_fileMap.find(path);
    T_FaceIndices* faceIndices = nullptr;
    
    if (m_fileMap.end() != foundPathEntry)
        faceIndices = &foundPathEntry->second;
    else
        faceIndices = &(m_fileMap[path] = T_FaceIndices());
    
    faceIndices->insert(faceIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2015
//---------------------------------------------------------------------------------------
IDgnFontData* DgnTrueTypeFileFontData::_CloneWithoutData()
    {
    DgnTrueTypeFileFontData* rhs = new DgnTrueTypeFileFontData(m_familyName.c_str());
    rhs->m_fileMap = m_fileMap;
    
    return rhs;
    }

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     04/2015
//=======================================================================================
struct FTFaceAutoFree
{
    FT_Face m_face;
    FTFaceAutoFree(FT_Face face) : m_face(face) {}
    ~FTFaceAutoFree() { if (nullptr != m_face) { FT_Done_Face(m_face); m_face = nullptr; } }
};

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnTrueTypeFileFontData::_Embed(DgnFonts::DbFaceDataDirect& faceData)
    {
    AutoDgnFontDataSession session(*this);
    if (!session.IsValid())
        return ERROR;
    
    FT_Library ftLib = T_HOST.GetFontAdmin().GetFreeTypeLibrary();
    if (nullptr == ftLib)
        return ERROR;
    
    // TrueType files can contain multiple families, so we must re-iterate the file, not just this family's faces. Otherwise, we may embed the same faces twice when embedding the next family.
    for (T_FileMap::const_reference file : m_fileMap)
        {
        BeFile fontDataFile;
        if (BeFileStatus::Success != fontDataFile.Open(file.first, BeFileAccess::Read))
            {
            FONT_LOG.errorv("Could not open TrueType file '%s' for read access to embed font data.", file.first.c_str());
            continue;
            }
        
        bvector<Byte> fontData;
        if (BeFileStatus::Success != fontDataFile.ReadEntireFile(fontData))
            {
            FONT_LOG.warningv("Could not read TrueType file data for embedding from file '%s'.", file.first.c_str());
            continue;
            }
        
        if (BeFileStatus::Success != fontDataFile.Close())
            {
            FONT_LOG.warningv("Could not close TrueType file '%s' after extracting font data for embedding.", file.first.c_str());
            continue;
            }

        DgnFonts::DbFaceDataDirect::T_FaceMap faceMap;
        FT_Long numFaces = 1;
        for (FT_Long iFace = 0; iFace < numFaces; ++iFace)
            {
            FT_Face face;
            FT_Error ftStatus = FT_New_Face(ftLib, Utf8String(file.first).c_str(), iFace, &face);
            if ((FT_Err_Ok != ftStatus) || (nullptr == face))
                {
                FONT_LOG.warningv("Could not open face %i from path '%s' (error code %i).", (int)iFace, Utf8String(file.first).c_str(), (int)ftStatus);
                return ERROR;
                }

            FTFaceAutoFree faceAutoFree(face);

            if (0 == iFace)
                numFaces = face->num_faces;

            if (!haveEmbeddingRights(face))
                {
                FONT_LOG.warningv("Could not embed face %i from path '%s'; it does not grant the right to embed.", (int)iFace, Utf8String(file.first).c_str());
                continue;
                }
            
            Utf8String familyName;
            IDgnTrueTypeFontData::GetFamilyName(familyName, face);
            
            DgnFontStyle style = ftStyleIndexToDgnFontStyle(face->style_flags);
            Utf8String styleName = dgnFontStyleToStyleName(style);
            DgnFonts::DbFaceDataDirect::FaceKey key(DgnFontType::TrueType, familyName.c_str(), styleName.c_str());
            
            if (faceData.Exists(key))
                {
                FONT_LOG.infov("Not attributing family/DGN face style '%s'/'%s' to new embedding data because it already exists in the database.", familyName.c_str(), styleName.c_str());
                continue;
                }
            
            faceMap[face->face_index] = key;
            }

        if (faceMap.empty())
            {
            FONT_LOG.infov("No new faces found in '%s' for embedding.", Utf8String(file.first).c_str());
            continue;
            }
        
        faceData.Insert(&fontData[0], fontData.size(), faceMap);
        }
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnTrueTypeFileFontData::_AddDataRef()
    {
    if (m_openCount > 0)
        {
        ++m_openCount;
        return SUCCESS;
        }

    FT_Library ftLib = T_HOST.GetFontAdmin().GetFreeTypeLibrary();
    if (nullptr == ftLib)
        return ERROR;
    
    for (T_FileMap::const_reference fileMapEntry : m_fileMap)
        {
        for (FT_Long const& faceIndex : fileMapEntry.second)
            {
            FT_Face face = nullptr;
            FT_Error ftStatus = FT_New_Face(ftLib, Utf8String(fileMapEntry.first).c_str(), faceIndex, &face);
            if ((FT_Err_Ok != ftStatus) || (nullptr == face))
                {
                FONT_LOG.warningv("Could not open face %i from path '%s' (error code %i).", (int)faceIndex, Utf8String(fileMapEntry.first).c_str(), (int)ftStatus);
                continue;
                }
            
            DgnFontStyle style = ftStyleIndexToDgnFontStyle(face->style_flags);
            if (m_faceMap.end() != m_faceMap.find(style))
                FONT_LOG.warningv("Face %i from path '%s' has DGN style %i, but a face with that style already exists; subsequent ones will be ignored.", (int)faceIndex, Utf8String(fileMapEntry.first).c_str(), (int)style);
            else
                m_faceMap[style] = face;
            }
        }

    // Since at an API level we try to disguise the concept of faces, I'm not sure what to do in the case of no regular face.
    // Instead of giving up, I figure it's better to come up with some remapping precedence.
    // For example, in Word, you'd get a Bold face in place of a Regular face if only Bold and BoldItalic faces existed.
    if (m_faceMap.end() == m_faceMap.find(DgnFontStyle::Regular))
        {
        T_FaceMap::iterator foundFace = m_faceMap.find(DgnFontStyle::Bold);
        if (m_faceMap.end() == foundFace)
            foundFace = m_faceMap.find(DgnFontStyle::Italic);
        if (m_faceMap.end() == foundFace)
            foundFace = m_faceMap.find(DgnFontStyle::BoldItalic);

        if (m_faceMap.end() == foundFace)
            {
            FONT_LOG.errorv("Could not load any face as a 'regular' face for family '%s'. This font will be considered missing, and may be substituted.", m_familyName.c_str());
            FreeAllFaces();
            return ERROR;
            }
        
        FT_Face effectiveFace = foundFace->second;
        m_faceMap.erase(foundFace);
        m_faceMap[DgnFontStyle::Regular] = effectiveFace;
        }
    
    ++m_openCount;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2015
//---------------------------------------------------------------------------------------
void DgnTrueTypeFileFontData::_ReleaseDataRef()
    {
    if (0 == m_openCount)
        {
        BeAssert(false);
        return;
        }

    if (--m_openCount > 0)
        return;

    FreeAllFaces();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2015
//---------------------------------------------------------------------------------------
FT_Face DgnTrueTypeFileFontData::_GetFaceP(DgnFontStyle style)
    {
    T_FaceMap::iterator foundFace = m_faceMap.find(style);
    if (m_faceMap.end() == foundFace)
        return nullptr;
    
    return foundFace->second;
    }

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     04/2015
//=======================================================================================
struct DgnTrueTypeDbFontData : IDgnTrueTypeFontData
{
private:
    struct FTFaceAndData
    {
        bvector<Byte> m_data;
        FT_Face m_face;
        FTFaceAndData() : m_face(nullptr) {}
    };
    
    typedef bmap<DgnFontStyle, FTFaceAndData*> T_FaceMap;

    DgnFonts::DbFaceDataDirect& m_dbFaceData;
    Utf8String m_familyName;
    size_t m_openCount;
    T_FaceMap m_faceMap;

    BentleyStatus FreeAllFaces();
    BentleyStatus LoadFace(DgnFontStyle);

public:
    DgnTrueTypeDbFontData(DgnFonts::DbFaceDataDirect& dbFaceData, Utf8CP familyName) : m_dbFaceData(dbFaceData), m_familyName(familyName), m_openCount(0) {}
    virtual IDgnFontData* _CloneWithoutData() override { return new DgnTrueTypeDbFontData(m_dbFaceData, m_familyName.c_str()); }
    virtual BentleyStatus _Embed(DgnFonts::DbFaceDataDirect&) override;
    virtual BentleyStatus _AddDataRef() override;
    virtual void _ReleaseDataRef() override;
    virtual FT_Face _GetFaceP(DgnFontStyle) override;
};

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnTrueTypeDbFontData::FreeAllFaces()
    {
    bool didAnyFail = false;
    
    for (T_FaceMap::reference faceEntry : m_faceMap)
        {
        FT_Error ftStatus = FT_Done_Face(faceEntry.second->m_face);
        if (FT_Err_Ok != ftStatus)
            {
            didAnyFail = true;
            FONT_LOG.warningv("Failed to free font face from family/DGN face style '%s'/%i from database. This may be a source of a leak.", faceEntry.second->m_face->family_name, (int)faceEntry.second->m_face->face_index);
            }
        
        delete faceEntry.second;
        }

    m_faceMap.clear();

    return (didAnyFail ? ERROR : SUCCESS);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2015
//---------------------------------------------------------------------------------------
static bool faceMapContainsFamily(DgnFonts::DbFaceDataDirect::T_FaceMapCR faces, Utf8CP familyName)
    {
    for (DgnFonts::DbFaceDataDirect::T_FaceMap::const_reference faceEntry : faces)
        {
        if (faceEntry.second.m_familyName.EqualsI(familyName))
            return true;
        }
    
    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnTrueTypeDbFontData::_Embed(DgnFonts::DbFaceDataDirect& faceData)
    {
    // TrueType font-to-blob is many-to-many, so we cannot directly go by face... need to just copy any overlapping embedding records.
    bool didAnyFail = false;
    DgnFonts::DbFaceDataDirect::Iterator allEmbeddedData = m_dbFaceData.MakeIterator();
    for (DgnFonts::DbFaceDataDirect::Iterator::Entry const& embeddedDataEntry : allEmbeddedData)
        {
        DgnFonts::DbFaceDataDirect::T_FaceMap faces = embeddedDataEntry.GenerateFaceMap();
        if (!faceMapContainsFamily(faces, m_familyName.c_str()))
            continue;
        
        DgnFonts::DbFaceDataDirect::T_FaceMap facesToTransfer;
        for (DgnFonts::DbFaceDataDirect::T_FaceMap::const_reference allFacesEntry : faces)
            {
            if (faceData.Exists(allFacesEntry.second))
                continue;
            
            facesToTransfer.insert(allFacesEntry);
            }
        
        if (facesToTransfer.empty())
            {
            FONT_LOG.infov("Embedded data entry %i from source has no new faces for destination when attempting to copy embedded data for family '%s'.", (int)embeddedDataEntry.GetId(), m_familyName.c_str());
            continue;
            }
        
        bvector<Byte> fontData;
        if (SUCCESS != m_dbFaceData.QueryById(fontData, embeddedDataEntry.GetId()))
            {
            didAnyFail |= true;
            FONT_LOG.errorv("Could not read font data to copy embedded data for family '%s'.", m_familyName.c_str());
            continue;
            }

        // Assume that if it was already embedded in one database, we have the right to embed it in another.
        
        if (SUCCESS != faceData.Insert(&fontData[0], fontData.size(), facesToTransfer))
            {
            didAnyFail |= true;
            FONT_LOG.warningv("Could not write font data to copy embedded data for family '%s'.", m_familyName.c_str());
            continue;
            }
        }
    
    return (didAnyFail ? ERROR : SUCCESS);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnTrueTypeDbFontData::LoadFace(DgnFontStyle style)
    {
    DgnFonts::DbFaceDataDirect::FaceKey key(DgnFontType::TrueType, m_familyName.c_str(), dgnFontStyleToStyleName(style).c_str());
    unique_ptr<FTFaceAndData> faceAndData(new FTFaceAndData());
    
    DgnFonts::DbFaceDataDirect::FaceSubId faceSubId;
    if (SUCCESS != m_dbFaceData.QueryByFace(faceAndData->m_data, faceSubId, key))
        return ERROR;

    FT_Library ftLib = T_HOST.GetFontAdmin().GetFreeTypeLibrary();
    if (nullptr == ftLib)
        return ERROR;

    FT_Error ftStatus = FT_New_Memory_Face(ftLib, &faceAndData->m_data[0], (FT_Long)faceAndData->m_data.size(), (FT_Long)faceSubId, &faceAndData->m_face);
    if ((FT_Err_Ok != ftStatus) || (nullptr == faceAndData->m_face))
        {
        FONT_LOG.errorv("Failed to load font face %i for family '%s' from database. This font will be considered unresolved and may be subject to substitution.", (int)faceSubId, m_familyName.c_str());
        return ERROR;
        }
        
    m_faceMap[style] = faceAndData.release();
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnTrueTypeDbFontData::_AddDataRef()
    {
    if (m_openCount > 0)
        {
        ++m_openCount;
        return SUCCESS;
        }

    LoadFace(DgnFontStyle::Regular);
    LoadFace(DgnFontStyle::Bold);
    LoadFace(DgnFontStyle::Italic);
    LoadFace(DgnFontStyle::BoldItalic);
    
    if (0 == m_faceMap.size())
        return ERROR;

    // Since at an API level we try to disguise the concept of faces, I'm not sure what to do in the case of no regular face.
    // Instead of giving up, I figure it's better to come up with some remapping precedence.
    // For example, in Word, you'd get a Bold face in place of a Regular face if only Bold and BoldItalic faces existed.
    if (m_faceMap.end() == m_faceMap.find(DgnFontStyle::Regular))
        {
        T_FaceMap::iterator foundFace = m_faceMap.find(DgnFontStyle::Bold);
        if (m_faceMap.end() == foundFace)
            foundFace = m_faceMap.find(DgnFontStyle::Italic);
        if (m_faceMap.end() == foundFace)
            foundFace = m_faceMap.find(DgnFontStyle::BoldItalic);

        if (m_faceMap.end() == foundFace)
            {
            FONT_LOG.errorv("Could not load any face as a 'regular' face for family '%s'. This font will be considered missing, and may be substituted.", m_familyName.c_str());
            FreeAllFaces();
            return ERROR;
            }

        FTFaceAndData* effectiveFace = foundFace->second;
        m_faceMap.erase(foundFace);
        m_faceMap[DgnFontStyle::Regular] = effectiveFace;
        }

    ++m_openCount;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2015
//---------------------------------------------------------------------------------------
void DgnTrueTypeDbFontData::_ReleaseDataRef()
    {
    if (0 == m_openCount)
        {
        BeAssert(false);
        return;
        }

    if (--m_openCount > 0)
        return;

    FreeAllFaces();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
FT_Face DgnTrueTypeDbFontData::_GetFaceP(DgnFontStyle style)
    {
    T_FaceMap::iterator foundFace = m_faceMap.find(style);
    if (m_faceMap.end() == foundFace)
        return nullptr;

    return foundFace->second->m_face;
    }

#if defined (BENTLEY_WIN32) // Windows Desktop-only

static const int FONT_LOGICAL_UNITS = 2048;

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     04/2015
//=======================================================================================
struct Win32TrueTypeContext : public DgnHost::IHostObject
{
    mutable HDC m_dc;

protected:
    virtual void _OnHostTermination(bool isProgramExit) override;

public:
    Win32TrueTypeContext() : m_dc(nullptr) {}
    static Win32TrueTypeContext& GetInstance();
    HDC GetDC() const;
};

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2015
//---------------------------------------------------------------------------------------
HDC Win32TrueTypeContext::GetDC() const
    {
    if (nullptr == m_dc)
        m_dc = ::CreateDCW(L"DISPLAY", NULL, NULL, NULL);

    return m_dc;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2015
//---------------------------------------------------------------------------------------
void Win32TrueTypeContext::_OnHostTermination(bool isProgramExit)
    {
    if (nullptr != m_dc)
        {
        ::DeleteDC(m_dc);
        m_dc = nullptr;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2015
//---------------------------------------------------------------------------------------
Win32TrueTypeContext& Win32TrueTypeContext::GetInstance()
    {
    static DgnHost::Key s_key;
    Win32TrueTypeContext* context = reinterpret_cast<Win32TrueTypeContext*>(T_HOST.GetHostObject(s_key));
    if (nullptr == context)
        {
        context = new Win32TrueTypeContext();
        T_HOST.SetHostObject(s_key, context);
        }

    return *context;
    }

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     04/2015
//=======================================================================================
struct Win32TrueTypeFontData : IDgnTrueTypeFontData
{
    struct FTFaceAndData
    {
        bvector<Byte> m_data;
        FT_Face m_face;
        FTFaceAndData() : m_face(nullptr) {}
    };
    
    typedef bmap<DgnFontStyle, FTFaceAndData*> T_FaceMap;

    Utf8String m_familyName;
    size_t m_openCount;
    T_FaceMap m_faceMap;

    BentleyStatus FreeAllFaces();
    BentleyStatus LoadFace(DgnFontStyle);
    BentleyStatus EmbedFace(DgnFonts::DbFaceDataDirect&, DgnFontStyle);
    BentleyStatus GetFontData(bvector<Byte>&, DgnFontStyle);

public:
    Win32TrueTypeFontData(Utf8CP familyName) : m_familyName(familyName), m_openCount(0) {}
    virtual IDgnFontDataP _CloneWithoutData() override { return new Win32TrueTypeFontData(m_familyName.c_str()); }
    virtual BentleyStatus _Embed(DgnFonts::DbFaceDataDirect&) override;
    virtual BentleyStatus _AddDataRef() override;
    virtual void _ReleaseDataRef() override;
    virtual FT_Face _GetFaceP(DgnFontStyle) override;
};

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2015
//---------------------------------------------------------------------------------------
BentleyStatus Win32TrueTypeFontData::FreeAllFaces()
    {
    bool didAnyFail = false;

    for (T_FaceMap::reference faceEntry : m_faceMap)
        {
        FT_Error ftStatus = FT_Done_Face(faceEntry.second->m_face);
        if (FT_Err_Ok != ftStatus)
            {
            didAnyFail = true;
            FONT_LOG.warningv("Failed to free font face from family/DGN face style '%s'/%i from database. This may be a source of a leak.", faceEntry.second->m_face->family_name, (int)faceEntry.second->m_face->face_index);
            }

        delete faceEntry.second;
        }

    m_faceMap.clear();

    return (didAnyFail ? ERROR : SUCCESS);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2015
//---------------------------------------------------------------------------------------
BentleyStatus Win32TrueTypeFontData::GetFontData(bvector<Byte>& data, DgnFontStyle style)
    {
    HDC fontDC = Win32TrueTypeContext::GetInstance().GetDC();
    if (nullptr == fontDC)
        return ERROR;

    // Select the font into the global context.
    bool isBold = ((DgnFontStyle::Bold == style) || (DgnFontStyle::BoldItalic == style));
    bool isItalic = ((DgnFontStyle::Italic == style) || (DgnFontStyle::BoldItalic == style));
    WString fontName(m_familyName.c_str(), BentleyCharEncoding::Utf8);

    HFONT newFont = ::CreateFontW(FONT_LOGICAL_UNITS, 0, 0, 0, isBold ? FW_BOLD : FW_LIGHT, isItalic, false, false,
        DEFAULT_CHARSET, OUT_TT_ONLY_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, DEFAULT_PITCH, fontName.c_str());

    if (nullptr == newFont)
        {
        FONT_LOG.errorv("Could not find DgnFontStyle %i for family '%s' from Windows. This font will be considered unresolved and may be subject to substitution.", (int)style, m_familyName.c_str());
        return ERROR;
        }

    HFONT previousFont = (HFONT)::SelectObject(fontDC, newFont);
    if (NULL != previousFont)
        ::DeleteObject(previousFont);

    // There is no reliable way to go from HFONT to file, but we can use GetFontData to read the raw bytes from the font.
    // The dwTable parameter to GetFontData allows as follows: If this parameter is zero, the information is retrieved starting at the beginning of the file for TrueType font files or from the beginning of the data for the currently selected font for TrueType Collection files. To retrieve the data from the beginning of the file for TrueType Collection files specify 'ttcf' (0x66637474).
    DWORD dwTable = 0;
    DWORD fontDataSize = ::GetFontData(fontDC, dwTable, 0, NULL, 0);
    if ((GDI_ERROR == fontDataSize) || (0 == fontDataSize))
        {
        FONT_LOG.errorv("Could not get data size for DgnFontStyle %i for family '%s' from Windows. This font will be considered unresolved and may be subject to substitution.", (int)style, m_familyName.c_str());
        return ERROR;
        }

    data.resize((size_t)fontDataSize);
    DWORD bytesRetrieved = ::GetFontData(fontDC, dwTable, 0, &data[0], fontDataSize);
    if ((GDI_ERROR == bytesRetrieved) || (0 == bytesRetrieved))
        {
        FONT_LOG.errorv("Could not get data for DgnFontStyle %i for family '%s' from Windows. This font will be considered unresolved and may be subject to substitution.", (int)style, m_familyName.c_str());
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2015
//---------------------------------------------------------------------------------------
BentleyStatus Win32TrueTypeFontData::LoadFace(DgnFontStyle style)
    {
    FT_Library ftLib = T_HOST.GetFontAdmin().GetFreeTypeLibrary();
    if (nullptr == ftLib)
        return ERROR;

    unique_ptr<FTFaceAndData> faceAndData(new FTFaceAndData());
    if (SUCCESS != GetFontData(faceAndData->m_data, style))
        return ERROR;
    
    FT_Error ftStatus = FT_New_Memory_Face(ftLib, &faceAndData->m_data[0], (FT_Long)faceAndData->m_data.size(), 0, &faceAndData->m_face);
    if ((FT_Err_Ok != ftStatus) || (nullptr == faceAndData->m_face))
        {
        FONT_LOG.errorv("Failed to load font face for DgnFontStyle %i for family '%s' from Windows via FreeType. This font will be considered unresolved and may be subject to substitution.", (int)style, m_familyName.c_str());
        return ERROR;
        }

    m_faceMap[style] = faceAndData.release();

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2015
//---------------------------------------------------------------------------------------
BentleyStatus Win32TrueTypeFontData::EmbedFace(DgnFonts::DbFaceDataDirect& faceData, DgnFontStyle style)
    {
    FT_Library ftLib = T_HOST.GetFontAdmin().GetFreeTypeLibrary();
    if (nullptr == ftLib)
        return ERROR;

    bvector<Byte> data;
    if (SUCCESS != GetFontData(data, style))
        return ERROR;

    DgnFonts::DbFaceDataDirect::T_FaceMap faceMap;
    FT_Long numFaces = 1;
    for (FT_Long iFace = 0; iFace < numFaces; ++iFace)
        {
        FT_Face face;
        FT_Error ftStatus = FT_New_Memory_Face(ftLib, &data[0], (FT_Long)data.size(), iFace, &face);
        if ((FT_Err_Ok != ftStatus) || (nullptr == face))
            {
            FONT_LOG.warningv("Could not open face %i from Windows for DgnFontStyle %i for family '%s' (error code %i).", (int)iFace, (int)style, m_familyName.c_str(), (int)ftStatus);
            return ERROR;
            }

        FTFaceAutoFree faceAutoFree(face);

        if (0 == iFace)
            numFaces = face->num_faces;

        if (!haveEmbeddingRights(face))
            {
            FONT_LOG.warningv("Could not embed face %i from Windows for DgnFontStyle %i for family '%s'; it does not grant the right to embed.", (int)iFace, (int)style, m_familyName.c_str());
            continue;
            }

        Utf8String familyName;
        IDgnTrueTypeFontData::GetFamilyName(familyName, face);

        DgnFontStyle style = ftStyleIndexToDgnFontStyle(face->style_flags);
        Utf8String styleName = dgnFontStyleToStyleName(style);
        DgnFonts::DbFaceDataDirect::FaceKey key(DgnFontType::TrueType, familyName.c_str(), styleName.c_str());

        if (faceData.Exists(key))
            {
            FONT_LOG.infov("Not attributing family/DGN face style '%s'/'%s' to new embedding data because it already exists in the database.", familyName.c_str(), styleName.c_str());
            continue;
            }

        faceMap[face->face_index] = key;
        }

    if (faceMap.empty())
        {
        FONT_LOG.infov("No new faces found in Windows for DgnFontStyle %i for family '%s'.",(int)style, m_familyName.c_str());
        return ERROR;
        }

    faceData.Insert(&data[0], data.size(), faceMap);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2015
//---------------------------------------------------------------------------------------
BentleyStatus Win32TrueTypeFontData::_Embed(DgnFonts::DbFaceDataDirect& faceData)
    {
    AutoDgnFontDataSession session(*this);
    if (!session.IsValid())
        return ERROR;
    
    // Font data is many-to-many... select each face into memory, get to the TrueType collection if present, and populate.
    bool didAllFail = true;
    didAllFail &= (SUCCESS != EmbedFace(faceData, DgnFontStyle::Regular));
    didAllFail &= (SUCCESS != EmbedFace(faceData, DgnFontStyle::Bold));
    didAllFail &= (SUCCESS != EmbedFace(faceData, DgnFontStyle::Italic));
    didAllFail &= (SUCCESS != EmbedFace(faceData, DgnFontStyle::BoldItalic));
    
    return (didAllFail ? ERROR : SUCCESS);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2015
//---------------------------------------------------------------------------------------
BentleyStatus Win32TrueTypeFontData::_AddDataRef()
    {
    if (m_openCount > 0)
        {
        ++m_openCount;
        return SUCCESS;
        }

    LoadFace(DgnFontStyle::Regular);
    LoadFace(DgnFontStyle::Bold);
    LoadFace(DgnFontStyle::Italic);
    LoadFace(DgnFontStyle::BoldItalic);

    if (0 == m_faceMap.size())
        return ERROR;

    // Since at an API level we try to disguise the concept of faces, I'm not sure what to do in the case of no regular face.
    // Instead of giving up, I figure it's better to come up with some remapping precedence.
    // For example, in Word, you'd get a Bold face in place of a Regular face if only Bold and BoldItalic faces existed.
    if (m_faceMap.end() == m_faceMap.find(DgnFontStyle::Regular))
        {
        T_FaceMap::iterator foundFace = m_faceMap.find(DgnFontStyle::Bold);
        if (m_faceMap.end() == foundFace)
            foundFace = m_faceMap.find(DgnFontStyle::Italic);
        if (m_faceMap.end() == foundFace)
            foundFace = m_faceMap.find(DgnFontStyle::BoldItalic);

        if (m_faceMap.end() == foundFace)
            {
            FONT_LOG.errorv("Could not load any face as a 'regular' face for family '%s'. This font will be considered missing, and may be substituted.", m_familyName.c_str());
            FreeAllFaces();
            return ERROR;
            }

        FTFaceAndData* effectiveFace = foundFace->second;
        m_faceMap.erase(foundFace);
        m_faceMap[DgnFontStyle::Regular] = effectiveFace;
        }

    ++m_openCount;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2015
//---------------------------------------------------------------------------------------
void Win32TrueTypeFontData::_ReleaseDataRef()
    {
    if (0 == m_openCount)
        {
        BeAssert(false);
        return;
        }

    if (--m_openCount > 0)
        return;

    FreeAllFaces();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2015
//---------------------------------------------------------------------------------------
FT_Face Win32TrueTypeFontData::_GetFaceP(DgnFontStyle style)
    {
    T_FaceMap::iterator foundFace = m_faceMap.find(style);
    if (m_faceMap.end() == foundFace)
        return nullptr;

    return foundFace->second->m_face;
    }

#endif // Windows Desktop-only

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
DgnFontPtr DgnFontPersistence::Db::DgnTrueTypeFontFromDb(DgnFonts& dbFonts, DgnFontId id, Utf8CP name, ByteCP metadata, size_t metadataSize)
    {
    return new DgnTrueTypeFont(name, new DgnTrueTypeDbFontData(dbFonts.DbFaceData(), name));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
static BentleyStatus convertTTNameString(Utf8StringR value, FT_SfntName const& name)
    {
    // There are over 50 ways that strings can be encoded within a TrueType file, many of which are hard to decode nowadays.
    // Of the 1000+ TrueType files on my system, supporting the various Unicodes plus macroman achieves full coverage, so I am focusing on that.
    
    Utf8String srcEncoding;
    switch (name.platform_id)
        {
        case TT_PLATFORM_APPLE_UNICODE:
            {
            switch (name.encoding_id)
                {
                // Some fonts use "Unicode 1.* semantics". I cannot find an easy way to go back to this old Unicode, so I'm hoping for our intents and purposes it is the same as UTF-16BE.
                case TT_APPLE_ID_DEFAULT: // fall through
                case TT_APPLE_ID_UNICODE_1_1: // fall through
                case TT_APPLE_ID_ISO_10646: // fall through
                case TT_APPLE_ID_UNICODE_2_0: srcEncoding = "UTF-16BE"; break;
                case TT_APPLE_ID_UNICODE_32: srcEncoding = "UTF-32BE"; break;
                default:
                    return ERROR;
                }
            
            break;
            }

        case TT_PLATFORM_MACINTOSH:
            {
            switch (name.encoding_id)
                {
                case TT_MAC_ID_ROMAN: srcEncoding = "macroman"; break;
                default:
                    return ERROR;
                }
            
            break;
            }
        
        case TT_PLATFORM_MICROSOFT:
            {
            switch (name.encoding_id)
                {
                case TT_MS_ID_UNICODE_CS: srcEncoding = "UTF-16BE"; break;
                default:
                    return ERROR;
                }

            break;
            }
        
        default:
            return ERROR;
        }
    
    bvector<Byte> outBuff;
    if (SUCCESS != BeStringUtilities::TranscodeStringDirect(outBuff, "UTF-8", (ByteCP)name.string, name.string_len, srcEncoding.c_str()))
        return ERROR;
    
    value.assign((CharCP)&outBuff[0], outBuff.size());
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
static bool isNameUnicode(FT_SfntName const& name) { return (TT_PLATFORM_APPLE_UNICODE == name.platform_id || (TT_PLATFORM_MICROSOFT == name.platform_id && TT_MS_ID_UNICODE_CS == name.encoding_id)); };

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
BentleyStatus IDgnTrueTypeFontData::GetFamilyName(Utf8StringR familyNameStr, FT_Face face)
    {
    // In order to determine what the base/regular face of a font is, we need more information than FT_Face::family_name/style_flags/style_name.
    // For example:
    //
    //  File        family_name             style_name
    //  arial.ttf   'Arial'                 'Regular'
    //  ariali.ttf  'Arial'                 'Italic'
    //  arialn.ttf  'Arial'                 'Narrow'
    //  arialni.ttf 'Arial'                 'Narrow Italic'
    //  vera.ttf    'Bitstream Vera Sans'   'Roman'
    //
    // Since every font in a family reports the same broad family_name, and style_name is not standardized, there's no way to know across all families what the base font is.
    // We must therefore dig into the name table for its actual family and style name. For example:
    //
    //  File        Font Family name(1)     Font Subfamily name(2)
    //  arial.ttf   'Arial'                 'Regular'
    //  ariali.ttf  'Arial'                 'Italic'
    //  arialn.ttf  'Arial Narrow'          'Regular'
    //  arialni.ttf 'Arial Narrow'          'Italic'
    //  vera.ttf    'Bitstream Vera Sans'   'Roman'
    //
    // Thus, we can use name ID 1 for family to properly group fonts together.

    if (nullptr == face)
        return ERROR;

    bool hasFamilyName = false;
    FT_SfntName familyName = {0};
    FT_UInt numStrings = FT_Get_Sfnt_Name_Count(face);
    for (FT_UInt iString = 0; iString < numStrings; ++iString)
        {
        FT_SfntName thisName;
        FT_Get_Sfnt_Name(face, iString, &thisName);

        // The values in the name table can have multiple definition with various encodings, targeting various languages.
        // We do not intend to support all 50+ possible ways to encode strings. I have sampled over 1000 fonts, and all can be decoded with merely a handful of schemes.
        // However, this also means we want to use a precedence so that we can use the most generous encoding possible.
        // The precedence rules below are seemingly arbitrary because I cannot find a part of the standard on how to prefer one name over another.
        // The name entries also have a language identifier that is also platform specific... I would like to be language-agnostic, so arbitrarily always taking the first one encountered... I don't know how to otherwise determine precedence for language.

        if (1 != thisName.name_id)
            continue;
        
        if (!hasFamilyName)
            {
            hasFamilyName = true;
            familyName = thisName;
            continue;
            }

        bool isThisNameUnicode = isNameUnicode(thisName);
        bool isTargetNameUnicode = isNameUnicode(familyName);

        // Unicode is always better.
        if (isThisNameUnicode && !isTargetNameUnicode)
            { familyName = thisName; continue; }

        // One locale is never better than another.
        if (!isThisNameUnicode)
            continue;

        // Newer Unicode is better. Arbitrarily treat Microsoft's Unicode as new, though not any better than the Unicode platform.
        if (TT_PLATFORM_APPLE_UNICODE == thisName.platform_id && TT_PLATFORM_APPLE_UNICODE == familyName.platform_id && thisName.encoding_id > familyName.encoding_id)
            { familyName = thisName; continue; }

        if (TT_PLATFORM_MICROSOFT == thisName.platform_id && TT_MS_ID_UNICODE_CS == thisName.encoding_id && TT_PLATFORM_APPLE_UNICODE == familyName.platform_id && familyName.encoding_id < TT_APPLE_ID_UNICODE_2_0)
            { familyName = thisName; continue; }
        }

    if (!hasFamilyName)
        {
        FONT_LOG.warningv("Could not determine the real family name of font face %i; using '%s' instead.", (int)face->face_index, face->family_name);
        familyNameStr = face->family_name;
        return ERROR;
        }

    if (SUCCESS != convertTTNameString(familyNameStr, familyName))
        {
        FONT_LOG.warningv("Could not decode the real family name of font face %i; using '%s' instead.", (int)face->face_index, face->family_name);
        familyNameStr = face->family_name;
        return ERROR;
        }
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
T_DgnFontPtrs DgnFontPersistence::File::FromTrueTypeFiles(bvector<BeFileName> const& paths, Utf8CP nameFilter)
    {
    T_DgnFontPtrs fonts;
    PRECONDITION(!paths.empty(), fonts);

    FT_Library ftLib = T_HOST.GetFontAdmin().GetFreeTypeLibrary();
    if (nullptr == ftLib)
        return fonts;

    // It is perhaps wasteful to open the faces below to inspect and store off their index, only to be re-opened again later by the data object, however this allows the data object to be simpler and have fewer code paths.
    // I do not expect this to be a common-enough operation to complicate code to optimize.
    
    struct FaceIndexAndPath
    {
        BeFileName m_path;
        FT_Long m_faceIndex;
        FaceIndexAndPath(BeFileNameCR path, FT_Long faceIndex) : m_path(path), m_faceIndex(faceIndex) {}
    };
    
    typedef bmap<Utf8String, bvector<FaceIndexAndPath>> T_FamilyFaceMap;
    T_FamilyFaceMap familyFaceMap;

    for (BeFileNameCR path : paths)
        {
        FT_Long numFaces = 1;
        for (FT_Long iFace = 0; iFace < numFaces; ++iFace)
            {
            FT_Face face = nullptr;
            FT_Error ftStatus = FT_New_Face(ftLib, Utf8String(path).c_str(), iFace, &face);
            if ((FT_Err_Ok != ftStatus) || (nullptr == face))
                {
                FONT_LOG.warningv("Could not open face %i from path '%s' (error code %i).", (int)iFace, Utf8String(path).c_str(), (int)ftStatus);
                continue;
                }

            if (0 == iFace)
                numFaces = face->num_faces;

            Utf8String familyName;
            if (SUCCESS != IDgnTrueTypeFontData::GetFamilyName(familyName, face))
                continue;
            
            T_FamilyFaceMap::iterator foundFamilyEntry = familyFaceMap.find(familyName);
            if (familyFaceMap.end() != foundFamilyEntry)
                foundFamilyEntry->second.push_back(FaceIndexAndPath(path, face->face_index));
            else
                familyFaceMap[familyName] = { FaceIndexAndPath(path, face->face_index) };
            
            ftStatus = FT_Done_Face(face);
            if (FT_Err_Ok != ftStatus)
                {
                FONT_LOG.warningv("Could not close face %i from path '%s' (error code %i). This may be the source of a leak.", (int)iFace, Utf8String(path).c_str(), (int)ftStatus);
                continue;
                }
            }
        }
    
    for (T_FamilyFaceMap::const_reference familyFaceEntry : familyFaceMap)
        {
        if (!Utf8String::IsNullOrEmpty(nameFilter) && !std::regex_match(familyFaceEntry.first.c_str(), std::regex(nameFilter)))
            continue;
        
        DgnTrueTypeFileFontData* data = new DgnTrueTypeFileFontData(familyFaceEntry.first.c_str());
        for (FaceIndexAndPath const& faceEntry : familyFaceEntry.second)
            data->RegisterFace(faceEntry.m_path, faceEntry.m_faceIndex);
        
        fonts.push_back(new DgnTrueTypeFont(familyFaceEntry.first.c_str(), data));
        }

    return fonts;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
DgnFontPtr DgnFontPersistence::OS::FromGlobalTrueTypeRegistry(Utf8CP name)
    {
#if !defined (BENTLEY_WIN32) // Windows Desktop-only
    return nullptr;
#else
    return new DgnTrueTypeFont(name, new Win32TrueTypeFontData(name));
#endif
    }
