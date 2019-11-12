/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include    "DwgImportInternal.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_DWGDB
USING_NAMESPACE_DWG

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
AnnotationTextStyleCPtr DwgImporter::_ImportTextStyle (DwgDbTextStyleTableRecordCR dwgStyle)
    {
    Utf8String          name;
    DwgHelper::ValidateStyleName (name, dwgStyle.GetName());

    DwgFontInfo fontInfo;
    if (DwgDbStatus::Success != dwgStyle.GetFontInfo(fontInfo) && dwgStyle.IsShapeFile())
        {
        this->ReportIssue (IssueSeverity::Info, IssueCategory::MissingData(), Issue::Message(), Utf8PrintfString("Skipping text style %s", name.c_str()).c_str());
        return  nullptr;
        }

    DgnFontCP   dgnFont = this->GetDgnFontFor (fontInfo);
    if (nullptr == dgnFont)
        {
        this->ReportError (IssueCategory::VisualFidelity(), Issue::Message(), Utf8PrintfString("Text style %s has no font", name.c_str()).c_str());
        return  nullptr;
        }

    auto model = this->GetOptions().GetMergeDefinitions() ? &this->GetDgnDb().GetDictionaryModel() : this->GetOrCreateJobDefinitionModel().get();
    AnnotationTextStylePtr  dgnStyle = AnnotationTextStyle::GetForEdit (*model, name.c_str());

    bool updating = dgnStyle.IsValid();
    if (!updating)
        dgnStyle = AnnotationTextStyle::Create (*model);

    if (!dgnStyle.IsValid())
        return  nullptr;

    dgnStyle->SetName (name.c_str());
    
    DgnFontId   fontId = m_dgndb->Fonts().AcquireId (*dgnFont);
    dgnStyle->SetFontId (fontId);
    dgnStyle->SetIsBold (fontInfo.IsBold());

    if (dgnFont->GetType() == DgnFontType::Shx)
        dgnStyle->SetIsItalic (dwgStyle.GetObliqueAngle() != 0);
    else
        dgnStyle->SetIsItalic (fontInfo.IsItalic());

    double      widthFactor = dwgStyle.GetWidthFactor ();
    double      height = dwgStyle.GetTextSize ();

    if (fabs(height) > 1.e-4)
        {
        double      toMeters = 1.0;
        this->GetRootTransform().IsRigidScale (toMeters);

        height *= toMeters;
        }
    else
        {
        // 0 means no height for the style and text to get size from active height!
        height = 0.0;
        }

    dgnStyle->SetHeight (height);
    dgnStyle->SetWidthFactor (widthFactor);

    // WIP - set linespacing
    // dgnStyle->SetLineSpacingFactor (1.0);

    AnnotationTextStyleCPtr     newStyle = updating ? dgnStyle->Update() : dgnStyle->Insert ();

    return  newStyle.IsValid() ? newStyle : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::_ImportTextStyleSection ()
    {
    DwgDbTextStyleTablePtr  textstyleTable(m_dwgdb->GetTextStyleTableId(), DwgDbOpenMode::ForRead);
    if (textstyleTable.IsNull())
        return  BSIERROR;

    DwgDbSymbolTableIteratorPtr iter = textstyleTable->NewIterator ();
    if (!iter.IsValid() || !iter->IsValid())
        return  BSIERROR;

    this->SetStepName (ProgressMessage::STEP_IMPORTING_TEXTSTYLES());

    uint32_t    count = 0;
    for (iter->Start(); !iter->Done(); iter->Step())
        {
        DwgDbTextStyleTableRecordPtr    textstyle(iter->GetRecordId(), DwgDbOpenMode::ForRead);
        if (textstyle.IsNull())
            {
            this->ReportIssue (DwgImporter::IssueSeverity::Warning, IssueCategory::MissingData(), Issue::CantOpenObject(), "Text style record");
            continue;
            }

        if ((count++ % 100) == 0)
            this->Progress ();

        DwgString       name = textstyle->GetName ();
        if (name.IsEmpty())
            {
            if (textstyle->UpgradeOpen() == DwgDbStatus::Success)
                {
                name.Assign (WPrintfString(L"Unnamed-%d", count).c_str());
                textstyle->SetName (name);
                this->ReportIssue (IssueSeverity::Info, IssueCategory::UnexpectedData(), Issue::Message(), Utf8PrintfString("Unamed text style is assigned a name %ls", name.c_str()).c_str());
                }
            else
                {
                this->ReportIssue (IssueSeverity::Info, IssueCategory::UnexpectedData(), Issue::Message(), "Unamed text styles exist");
                }
            }
        else
            {
            LOG_TEXTSTYLE.tracev("Processinging DWG textstyle %ls", name.c_str());
            }

        auto dgnStyle = this->_ImportTextStyle (*textstyle.get());

        if (!dgnStyle.IsValid())
            {
            // set the active style as the default style, but if no active style exists, use the first style:
            if (textstyle->IsActiveTextStyle() || !m_defaultTextstyleId.IsValid())
                {
                m_defaultTextstyleId = dgnStyle->GetElementId ();
                m_defaultFont = dgnStyle->ResolveFont().Clone ();
                }
            m_importedTextstyles.insert (T_DwgDgnTextStyleId(textstyle->GetObjectId(), dgnStyle->GetElementId()));
            }
        }
    
    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId    DwgImporter::GetDgnTextStyleFor (DwgDbObjectIdCR tstyleId)
    {
    auto        found = m_importedTextstyles.find (tstyleId);
    if (found != m_importedTextstyles.end())
        return  found->second;

    return  DgnElementId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/19
+---------------+---------------+---------------+---------------+---------------+------*/
DgnFontCP       DwgImporter::ResolveFont (DgnFontCP font)
    {
    /*-----------------------------------------------------------------------------------
    Above call m_dgndb->Fonts().AcquireId() for textstyle effectively inserts a font into db,
    but also turns the font's data type from DgnShxFileFontData to DgnShxDbFontData without
    the face data embedded. Later on when DgnFont::IsResolved is called the first time, 
    DgnShxDbFontData tries to retrieve face data from embedded list which does not exist yet 
    as out fonts embedding takes place the end of an import job.  An unresolved font may 
    result in text or textstyle to be replaced by LastResortShxFont, as a case shown in 
    VSTS119701.

    This method serves as a callback from font admin (via iModelBridge). It resolves the
    input font as a db font, but only if it has been embedded (i.e. IsResolved returns true).
    Otherwise it resolves to loaded file font, which is of DgnShxFileFontData which loads
    face data from file the first time IsResolved if called.
    -----------------------------------------------------------------------------------*/
    if (font == nullptr)
        return  nullptr;

    // treat loaded fonts as resolved - we will embed fonts at the end of the import job
    auto type = font->GetType();
    auto name = font->GetName();
    auto found = m_dgndb->Fonts().FindFontByTypeAndName(type, name.c_str());
    // don't check IsResolved - at this time LastResortTrueTypeFont returns false!
    if (nullptr == found)
        found = m_loadedFonts.FindDgnFont(type, name);
    return  found;    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnFontCP       DwgImporter::GetDgnFontFor (DwgFontInfoCR dwgFont)
    {
    WString     typeface = dwgFont.GetTypeFace ();
    DgnFontType fontType = DwgHelper::GetFontType (dwgFont);
    Utf8String  fontName (DgnFontType::TrueType == fontType ? typeface.c_str() : dwgFont.GetShxFontName().c_str());

    // opt for an existing mebdded db font over our loaded file font
    DgnFontCP    dgnFont = m_dgndb->Fonts().FindFontByTypeAndName (fontType, fontName.c_str());
    if (nullptr == dgnFont || !dgnFont->IsResolved())
        dgnFont = m_loadedFonts.FindDgnFont (fontType, fontName);

    if (nullptr == dgnFont)
        dgnFont = DgnFontType::TrueType == fontType ? &DgnFontManager::GetLastResortTrueTypeFont() : &DgnFontManager::GetLastResortShxFont();

    if (nullptr == dgnFont)
        {
        BeAssert (false && L"Unexpected null DgnFontCP!");
        return  nullptr;
        }

    return  dgnFont;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnFontCP       DwgImporter::WorkingFonts::FindDgnFont (DgnFontType type, Utf8StringCR name, bool warning)
    {
    DgnFontCP   dgnFont = nullptr;
    if (name.empty() || (DgnFontType::Shx != type && DgnFontType::TrueType != type))
        return  dgnFont;

    this->LoadFonts ();

    T_FontMap&  loadedFonts = DgnFontType::TrueType == type ? m_truetypeFonts : m_shxFonts;

    Utf8String  fontName = name;
    fontName.ToLower ();

    // remove file extension from input font name:
    if (DgnFontType::Shx == type)
        fontName = fontName.substr (0, fontName.rfind(".shx"));
    else
        fontName = fontName.substr (0, fontName.rfind(".ttf"));

    auto        found = loadedFonts.find (fontName);
    if (found != loadedFonts.end())
        {
        dgnFont = found->second.m_font.get();
        }
    else if (warning && m_missingFonts.find(fontName) == m_missingFonts.end())
        {
        Utf8CP typestr = DgnFontType::Shx == type ? "SHX" : "TTF";
        m_dwgImporter.ReportIssueV (IssueSeverity::Warning, IssueCategory::MissingData(), Issue::FontMissing(), nullptr, typestr, name.c_str());
        m_missingFonts.insert (fontName);
        }

#ifdef DEBUGLOADEDFONTS
    for (auto font : loadedFonts)
        LOG.debug ("Loaded font map <%s = %ls>\n", font.first.c_str(), font.second.m_path.c_str());
#endif

    return  dgnFont;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DwgImporter::WorkingFonts::FindFontPath (BeFileNameR path, DgnFontType type, Utf8StringCR name) const
    {
    T_FontMap const&    loadedFonts = DgnFontType::TrueType == type ? m_truetypeFonts : m_shxFonts;

    // remove file extension name:
    Utf8String  fontName = DgnFontType::Shx == type ? name.substr(0, name.rfind(".shx")) : name.substr(0, name.rfind(".ttf"));
    fontName.ToLower ();

    auto        found = loadedFonts.find (fontName);
    if (found != loadedFonts.end())
        {
        path = found->second.m_path;
        return  true;
        }
    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
size_t          DwgImporter::WorkingFonts::LoadFonts ()
    {
    if (m_loaded)
        return  m_truetypeFonts.size() + m_shxFonts.size();

    m_loaded = true;

    m_dwgImporter.SetTaskName (ProgressMessage::TASK_LOADING_FONTS(), "Workspace");

    static BeFileName   s_cwd;
    if (s_cwd.empty())
        {
#ifdef BENTLEY_WIN32
        WChar    moduleDir[MAX_PATH] = { 0 };
        if (::GetModuleFileNameW(nullptr, moduleDir, _countof(moduleDir)) > 0)
            {
            s_cwd = BeFileName (BeFileName::DevAndDir, moduleDir);
            s_cwd.AppendSeparator ();
            }
#endif
        }

    BeFileName  searchPaths (m_dwgImporter._GetFontSearchPaths().c_str(), BentleyCharEncoding::Utf8);
    searchPaths.ReplaceAll (L"$(AppRoot)", s_cwd.c_str());

    bvector<WString>    pathList;
    BeStringUtilities::Split(searchPaths.c_str(), L";", nullptr, pathList);
    
    for (auto& entry : pathList)
        {
        BeFileName  path(entry);
        if (path.IsDirectory())
            path.AppendToPath(L"*.*");

        BeFileName  file;
        for (BeFileListIterator iter(path, false); SUCCESS == iter.GetNextFileName(file); )
            {
            WString extension = file.GetExtension();
            extension.ToLower ();
            if (extension.Equals(L"ttf") || extension.Equals(L"ttc") || extension.Equals(L"otf") || extension.Equals(L"otc"))
                this->LoadTrueTypeFont (file);
            else if (extension.Equals(L"shx") || extension.Equals(L"shp"))
                this->LoadShxFont (file);

            m_dwgImporter.Progress ();
            }
        }

    m_dwgImporter.SetTaskName (ProgressMessage::TASK_LOADING_FONTS(), "System");

    this->LoadOSFonts ();

    // set fallback fonts
    auto    fallbackShx = m_shxFonts.find ("simplex");
    if (fallbackShx == m_shxFonts.end())
        fallbackShx = m_shxFonts.begin ();
    if (fallbackShx != m_shxFonts.end())
        m_dwgImporter.SetFallbackFontPathForText (fallbackShx->second.m_path, DgnFontType::Shx);

    auto    fallbackTtf = m_truetypeFonts.find ("arial");
    if (fallbackTtf == m_truetypeFonts.end())
        fallbackTtf = m_truetypeFonts.begin ();
    if (fallbackTtf != m_truetypeFonts.end())
        {
        auto pathOrName = fallbackTtf->second.m_path;
        if (pathOrName.empty() && fallbackTtf->second.m_font.IsValid())
            pathOrName.SetNameUtf8(fallbackTtf->second.m_font->GetName());
        m_dwgImporter.SetFallbackFontPathForText (pathOrName, DgnFontType::TrueType);
        }

    fallbackShx = m_shxFonts.find ("ltypeshp");
    if (fallbackShx != m_shxFonts.end())
        m_dwgImporter.SetFallbackFontPathForShape (fallbackShx->second.m_path);

    return  m_truetypeFonts.size() + m_shxFonts.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::WorkingFonts::LoadTrueTypeFont (BeFileNameCR path)
    {
    T_DgnFontPtrs   fonts = DgnFontPersistence::File::FromTrueTypeFiles({path}, nullptr);
    if (fonts.size() == 0)
        return  BSIERROR;

    for (auto& ttfont : fonts)
        {
        WorkingFont workingfont(path, ttfont.get());
        Utf8String  fontName = ttfont->GetName ();
        fontName.ToLower ();

        m_truetypeFonts.insert (T_FontEntry(fontName, workingfont));
        }

    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::WorkingFonts::LoadShxFont (BeFileNameCR path)
    {
    DgnFontPtr  shxfont = DgnFontPersistence::File::FromShxFile (path);
    if (!shxfont.IsValid())
        return  BSIERROR;

    WorkingFont workingfont(path, shxfont.get());
    Utf8String  fontName = shxfont->GetName ();
    fontName.ToLower ();

    m_shxFonts.insert (T_FontEntry(fontName, workingfont));

    // WIP - SHX metadata

    return  BSISUCCESS;
    }

#if defined (BENTLEY_WIN32)
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
#endif

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2015
//---------------------------------------------------------------------------------------
BentleyStatus   DwgImporter::WorkingFonts::LoadOSFonts ()
    {
#if defined (BENTLEY_WIN32)
    LOGFONTW lf;
    memset (&lf, 0, sizeof(lf));
    lf.lfCharSet = DEFAULT_CHARSET;

    HDC fontDC = ::CreateDCW(L"DISPLAY", NULL, NULL, NULL);
    bset<Utf8String> fontNames;
    ::EnumFontFamiliesExW (fontDC, &lf, enumerateSystemFontsCallback, reinterpret_cast<LPARAM>(&fontNames), 0);
    ::DeleteObject (fontDC);
    
    for (Utf8StringCR fontName : fontNames)
        {
        DgnFontPtr  font = DgnFontPersistence::OS::FromGlobalTrueTypeRegistry(fontName.c_str());
        WorkingFont workingfont(BeFileName(), font.get());
        Utf8String  name = font->GetName ();
        name.ToLower ();

        m_truetypeFonts.insert (T_FontEntry(name, workingfont));
        m_dwgImporter.Progress ();
        }
    return  BSISUCCESS;
#else
    return  BSIERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DwgImporter::GetFallbackFontPathForShape (BeFileNameR outName) const
    {
    outName = m_fallbackFonts.m_shxForShape;
    return  outName.DoesPathExist();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImporter::SetFallbackFontPathForShape (BeFileNameCR filename)
    {
    m_fallbackFonts.m_shxForShape = filename;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DwgImporter::GetFallbackFontPathForText (BeFileNameR outName, DgnFontType fontType) const
    {
    if (DgnFontType::TrueType == fontType)
        {
        outName = m_fallbackFonts.m_truetype;
        return  !outName.empty();
        }
    else
        {
        outName = m_fallbackFonts.m_shxForText;
        return  outName.DoesPathExist();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImporter::SetFallbackFontPathForText (BeFileNameCR inName, DgnFontType fontType)
    {
    if (DgnFontType::TrueType == fontType)
        m_fallbackFonts.m_truetype = inName;
    else
        m_fallbackFonts.m_shxForText = inName;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2015
//---------------------------------------------------------------------------------------
static bool shouldEmbedUsedFont(DwgImporter::Config const& importConfig, DgnFontType type, Utf8CP name)
    {
    Utf8String fontTypeString;
    switch (type)
        {
        case DgnFontType::Shx: fontTypeString = "SHX"; break;
        case DgnFontType::TrueType: fontTypeString = "TrueType"; break;
        default:
            BeAssert(false);
            return false;
        }
    
    Utf8String embedAction;
    
    // Node matching explicit type and name takes precedence.
    importConfig.EvaluateXPath(embedAction, Utf8PrintfString("/ConvertConfig/Fonts/Font[@type='%s' and @name='%s']/EmbedAction", fontTypeString.c_str(), name).c_str());
    
    // Check for name wild card.
    if (embedAction.empty())
        importConfig.EvaluateXPath(embedAction, Utf8PrintfString("/ConvertConfig/Fonts/Font[@type='*' and @name='%s']/EmbedAction", name).c_str());

    // Check for type wild card.
    if (embedAction.empty())
        importConfig.EvaluateXPath(embedAction, Utf8PrintfString("/ConvertConfig/Fonts/Font[@type='%s' and @name='*']/EmbedAction", fontTypeString.c_str()).c_str());
    
    // Check for full wild card.
    if (embedAction.empty())
        importConfig.EvaluateXPath(embedAction, "/ConvertConfig/Fonts/Font[@type='*' and @name='*']/EmbedAction");
    
    if (embedAction.empty())
        return true; // Otherwise, documented default is to embed all used fonts.

    return (embedAction.Equals("IfUsed") || embedAction.Equals("Always"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2015
//---------------------------------------------------------------------------------------
void    DwgImporter::_EmbedFonts ()
    {
    SetStepName(ProgressMessage::STEP_EMBED_FONTS());
    
    // By default, embed any used fonts.
    // The converter can only reasonably care about embedding workspace fonts... otherwise anything in the DB is outside its scope.
    DgnFonts::DbFontMapDirect::Iterator allFonts = m_dgndb->Fonts().DbFontMap().MakeIterator();
    for (DgnFonts::DbFontMapDirect::Iterator::Entry const& fontEntry : allFonts)
        {
        // Do NOT use a DgnFont object from the DB... the DB is self-contained, and if asked for a font pre-embedding, it will give you a DgnFont object with unresolved data.
        // Use the type and name to look up a workspace font and embed it. When a DB-based font attempts to resolve later, it will find this embedded data by type and name.
        DgnFontCP   workingFont = m_loadedFonts.FindDgnFont (fontEntry.GetType(), fontEntry.GetName(), false);
        if (nullptr == workingFont)
            continue;

        auto fontType = fontEntry.GetType ();
        auto fontName = fontEntry.GetName ();
        if (!shouldEmbedUsedFont(m_config, fontType, fontName))
            continue;

        // if the font has been embedded, don't bother again - VSTS 131026.
        auto& fontData = m_dgndb->Fonts().DbFaceData ();
        if (fontData.Exists(DgnFonts::DbFaceDataDirect::FaceKey(fontType, fontName, DgnFonts::DbFaceDataDirect::FaceKey::FACE_NAME_Regular)))
            continue;
        
        if (BSISUCCESS != DgnFontPersistence::Db::Embed(fontData, *workingFont))
            ReportIssueV(IssueSeverity::Warning, IssueCategory::MissingData(), Issue::CannotEmbedFont(), nullptr, (int)fontType, fontName);
        
        Progress();
        }
    
    // Check the config for any forced embedding we can find in the workspace.
    if (nullptr != m_config.GetDom())
        {
        BeXmlDom::IterableNodeSet fontsToAlwaysEmbed;
        m_config.GetDom()->SelectNodes(fontsToAlwaysEmbed, "/ConvertConfig/Fonts/Font[EmbedAction='Always']", nullptr);
        for (BeXmlNodeP node : fontsToAlwaysEmbed)
            {
            Utf8String fontTypeName;
            DgnFontType fontType;
            node->GetAttributeStringValue(fontTypeName, "type");

            if (fontTypeName.Equals("SHX"))
                fontType = DgnFontType::Shx;
            else if (fontTypeName.Equals("TrueType"))
                fontType = DgnFontType::TrueType;
            else
                continue;
            
            Utf8String fontName;
            node->GetAttributeStringValue(fontName, "name");

            DgnFontCP   workingFont = m_loadedFonts.FindDgnFont (fontType, fontName);
            if (nullptr == workingFont)
                continue;

            if (BSISUCCESS != DgnFontPersistence::Db::Embed(m_dgndb->Fonts().DbFaceData(), *workingFont))
                ReportIssueV(IssueSeverity::Warning, IssueCategory::MissingData(), Issue::CannotEmbedFont(), nullptr, (int)fontType, fontName.c_str());

            Progress();
            }
        }
    }

