/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/Converters/FontConverterApp.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#if defined (BENTLEY_WIN32)
    #define UNICODE
    #include <Windows.h>
#endif

#include <Logging/bentleylogging.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/DgnFontData.h>
#include <DgnPlatform/DesktopTools/KnownDesktopLocationsAdmin.h>
#include <Bentley/BeFileListIterator.h>
#include <BeXml/BeXml.h>
#include <DgnDbSync/DgnV8/DgnV8Font.h>
#include <RmgrTools/Tools/RscFileManager.h>
#include <regex>

USING_NAMESPACE_BENTLEY_SQLITE;
USING_NAMESPACE_BENTLEY_DGN;

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
static AString utf8ToLocale(Utf8CP str) { return AString(WString(str, BentleyCharEncoding::Utf8).c_str()); }
static AString utf8ToLocale(Utf8StringCR str) { return utf8ToLocale(str.c_str()); }

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     03/2015
//=======================================================================================
struct FontConverterApp : DgnPlatformLib::Host
{
    virtual IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override { return *new KnownDesktopLocationsAdmin(); }
    virtual void _SupplyProductName(Utf8StringR name) override { name.assign("FontConverter"); }
    virtual L10N::SqlangFiles _SupplySqlangFiles() override;
};

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
L10N::SqlangFiles FontConverterApp::_SupplySqlangFiles()
    {
    BeFileName sqlangFile(GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
    sqlangFile.AppendToPath(L"sqlang");
    sqlangFile.AppendToPath(L"FontConverter_en-US.sqlang.db3");

    return L10N::SqlangFiles(sqlangFile);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
static void initLogging(BeFileNameCR programPath, BeFileNameCR userPath)
    {
    BeFileName configPath;
    if (userPath.IsAbsolutePath())
        {
        configPath = userPath;
        }
    else
        {
        configPath = BeFileName(BeFileName::DevAndDir, programPath);
        configPath.AppendToPath(userPath);
        }

    configPath.BeGetFullPathName();

    if (!configPath.DoesPathExist() || configPath.IsDirectory())
        {
        NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::CONSOLE_LOGGING_PROVIDER);
        }
    else
        {
        NativeLogging::LoggingConfig::SetOption(CONFIG_OPTION_CONFIG_FILE, configPath);
        NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::LOG4CXX_LOGGING_PROVIDER);
        }
    
    NativeLogging::LoggingConfig::SetSeverity(L"DgnFont", NativeLogging::LOG_INFO);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
static void printHelp()
    {
    printf("\
FontConverter - Converts/imports fonts into a BeSQLite (including DgnDb) database.\n\
    -o  Required        Path to output database; if the file does not exist, a new BeSQLite database will be created\n\
    -f  Required or -s  Path(s) to source font file(s); supports *.ttf, *.rsc, *.shx; delimit with semi-colons; supports wildcards\n\
    -n  Optional        If RSC or TrueType file(s) were specified, a font name filter regular expression (default='.*')\n\
    -s  Required or -f  Toggle to query the global OS registry of fonts by-name, instead of searching for file names\n\
    -t  Optional        Font table name (default='dgn_Font')\n\
    -l  Optional        Logging configuration file (default='./FontConverter.logging.config.xml')\n\
    -c  Optional        Path to an MstnFontConfig.xml file (default='<GetDgnPlatformAssetsDirectory>fonts/MstnFontConfig.xml')\n\
    ");
    }

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     03/2015
//=======================================================================================
struct AppArgs
{
    BeFileName m_programPath;
    BeFileName m_outputPath;
    BeFileName m_fontPath;
    Utf8String m_nameFilter;
    bool m_searchOSFonts;
    Utf8String m_tableName;
    BeFileName m_logConfigPath;
    BeFileName m_fontConfigPath;

    AppArgs()
        {
        m_nameFilter = ".*";
        m_searchOSFonts = false;
        m_tableName = DGN_TABLE_Font;
        m_logConfigPath.assign(L"FontConverter.logging.config.xml");
        }
};

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
static BentleyStatus parseArgs(AppArgs& appArgs, int argc, WCharCP argv[])
    {
    if (argc < 1)
        {
        printf("ERROR - could not get program path from argc/argv.\n");
        return ERROR;
        }

    appArgs.m_programPath.AssignOrClear(argv[0]);
    
    for (size_t iArg = 1; iArg < argc; ++iArg)
        {
        WCharCP currArg = argv[iArg];
        WCharCP argValue = (((iArg + 1) < argc) ? argv[iArg + 1] : nullptr);
        
        if (0 == wcscmp(L"-o", currArg)) { appArgs.m_outputPath.AssignOrClear(argValue); ++iArg; continue; }
        if (0 == wcscmp(L"-f", currArg)) { appArgs.m_fontPath.AssignOrClear(argValue); ++iArg; continue; }
        if (0 == wcscmp(L"-n", currArg)) { appArgs.m_nameFilter.Assign(argValue); ++iArg; continue; }
        if (0 == wcscmp(L"-s", currArg)) { appArgs.m_searchOSFonts = true; continue; }
        if (0 == wcscmp(L"-t", currArg)) { appArgs.m_tableName.Assign(argValue); ++iArg; continue; }
        if (0 == wcscmp(L"-l", currArg)) { appArgs.m_logConfigPath.AssignOrClear(argValue); ++iArg; continue; }
        if (0 == wcscmp(L"-c", currArg)) { appArgs.m_fontConfigPath.AssignOrClear(argValue); ++iArg; continue; }

        printf("ERROR - unknown argument '%s'.\n", AString(currArg).c_str());
        return ERROR;
        }
    
    if (appArgs.m_outputPath.empty() || (appArgs.m_fontPath.empty() && !appArgs.m_searchOSFonts))
        {
        printf("ERROR - missing or invalid required arguments.\n");
        return ERROR;
        }
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
static BentleyStatus embedFont(DgnFonts& dbFonts, DgnFontCR font)
    {
    DgnFontPtr existingFont = dbFonts.DbFontMap().QueryByTypeAndName(font.GetType(), font.GetName().c_str());
    if (existingFont.IsValid())
        {
        DgnFontId existingId = dbFonts.DbFontMap().QueryIdByTypeAndName(font.GetType(), font.GetName().c_str());
        if (!existingId.IsValid())
            {
            printf("ERROR - could not get ID for existing font with type/name %i/'%s'.\n", (int)font.GetType(), utf8ToLocale(font.GetName()).c_str());
            return ERROR;
            }
        
        if (SUCCESS != dbFonts.DbFontMap().Update(font, existingId))
            {
            printf("ERROR - could not update font with id/type/name %i/%i/'%s'.\n", (int)existingId.GetValue(), (int)font.GetType(), utf8ToLocale(font.GetName()).c_str());
            return ERROR;
            }
        
        printf("INFO - Updated font with id/type/name %i/%i/'%s'.\n", (int)existingId.GetValue(), (int)font.GetType(), utf8ToLocale(font.GetName()).c_str());
        }
    else
        {
        DgnFontId newId;
        if (SUCCESS != dbFonts.DbFontMap().Insert(font, newId))
            {
            printf("ERROR - could not insert font with type/name %i/'%s'.\n", (int)font.GetType(), utf8ToLocale(font.GetName()).c_str());
            return ERROR;
            }

        printf("INFO - Inserted font with id/type/name %i/%i/'%s'.\n", (int)newId.GetValue(), (int)font.GetType(), utf8ToLocale(font.GetName()).c_str());
        }
    
    if (SUCCESS != DgnFontPersistence::Db::Embed(dbFonts.DbFaceData(), font))
        {
        printf("ERROR - could not embed font data for font with type/name %i/'%s'.\n", (int)font.GetType(), utf8ToLocale(font.GetName()).c_str());
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
static BeXmlNodeP findFontConfigNode(Utf8CP elementName, Utf8StringCR fontName, BeXmlDomR fontConfig)
    {
    BeXmlNodeP effectiveNode = nullptr;

    // First try for a config node by-name... since we use a delimited string in the <Name> element, I don't know how to use XPath directly, hence the complexity.
    BeXmlDom::IterableNodeSet fontNodes;
    fontConfig.SelectNodes(fontNodes, Utf8PrintfString("/FontConfig/Fonts/%s", elementName).c_str(), nullptr);
    for (BeXmlNodeP configNode : fontNodes)
        {
        xmlXPathContextPtr configNodeContext = fontConfig.AcquireXPathContext(configNode);
        Utf8String namesString;
        if (BEXML_Success != fontConfig.SelectNodeContent(namesString, "./Name", configNodeContext, BeXmlDom::NODE_BIAS_Last))
            continue;

        bvector<Utf8String> names;
        BeStringUtilities::Split(namesString.c_str(), ",;", names);
        for (Utf8StringCR name : names)
            {
            if (fontName.EqualsI(name))
                {
                effectiveNode = configNode;
                break;
                }
            }

        if (nullptr != effectiveNode)
            break;
        }

    // If we didn't find one by-name, try to find the wild card.
    if (nullptr == effectiveNode)
        {
        if (BEXML_Success != fontConfig.SelectNode(effectiveNode, Utf8PrintfString("/FontConfig/Fonts/%s[Name=\"*\"]", elementName).c_str(), nullptr, BeXmlDom::NODE_BIAS_First))
            return nullptr;
        }

    return effectiveNode;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
static BentleyStatus embedTrueTypeFontFiles(DgnFonts& dbFonts, bvector<BeFileName> const& paths, BeXmlDomP fontConfig, Utf8StringCR nameFilter)
    {
    T_DgnFontPtrs fonts = DgnFontPersistence::File::FromTrueTypeFiles(paths, nameFilter.c_str());
    bool didAnyFail = false;

    for (DgnFontPtr& font : fonts)
        {
        if (SUCCESS != embedFont(dbFonts, *font))
            didAnyFail |= true;
        
        // A process can only have a limited number of files open at once; therefore, discard the font when done to free its resouces (including file handle(s)).
        font = nullptr;
        }

    return (didAnyFail ? ERROR : SUCCESS);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
static void convertRscMetadata(DgnRscFontR font, BeXmlDomP fontConfig)
    {
    if (nullptr == fontConfig)
        return;

    BeXmlNodeP configNode = findFontConfigNode("RscFontInfo", font.GetName(), *fontConfig);
    if (nullptr == configNode)
        return;

    xmlXPathContextPtr effectiveNodeContext = fontConfig->AcquireXPathContext(configNode);
    uint16_t u16;
    uint32_t u32;

    if (BEXML_Success == fontConfig->SelectNodeContentAsUInt32(u32, "./CodePage", effectiveNodeContext, BeXmlDom::NODE_BIAS_Last))
        font.GetMetadataR().m_codePage = (LangCodePage)u32;

    if (BEXML_Success == fontConfig->SelectNodeContentAsUInt16(u16, "./DegreeChar", effectiveNodeContext, BeXmlDom::NODE_BIAS_Last))
        font.GetMetadataR().m_degreeCode = (DgnGlyph::T_Id)u16;

    if (BEXML_Success == fontConfig->SelectNodeContentAsUInt16(u16, "./DiameterChar", effectiveNodeContext, BeXmlDom::NODE_BIAS_Last))
        font.GetMetadataR().m_diameterCode = (DgnGlyph::T_Id)u16;

    if (BEXML_Success == fontConfig->SelectNodeContentAsUInt16(u16, "./PlusMinusChar", effectiveNodeContext, BeXmlDom::NODE_BIAS_Last))
        font.GetMetadataR().m_plusMinusCode = (DgnGlyph::T_Id)u16;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
static BentleyStatus embedRscFontFile(DgnFonts& dbFonts, BeFileNameCR path, BeXmlDomP fontConfig, Utf8StringCR nameFilter)
    {
    T_DgnFontPtrs fonts = DgnV8FontPersistence::File::FromRscFile(path, nameFilter.c_str());
    bool didAnyFail = false;

    for (DgnFontPtr const& font : fonts)
        {
        if (SUCCESS != embedFont(dbFonts, *font))
            {
            didAnyFail |= true;
            continue;
            }
        
        convertRscMetadata((DgnRscFontR)*font, fontConfig);
        }
    
    return (didAnyFail ? ERROR : SUCCESS);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
static void convertShxMetadata(DgnShxFontR font, BeXmlDomP fontConfig)
    {
    if (nullptr == fontConfig)
        return;

    BeXmlNodeP configNode = findFontConfigNode("ShxFontInfo", font.GetName(), *fontConfig);
    if (nullptr == configNode)
        return;
    
    xmlXPathContextPtr effectiveNodeContext = fontConfig->AcquireXPathContext(configNode);
    uint16_t u16;
    uint32_t u32;
    
    if (BEXML_Success == fontConfig->SelectNodeContentAsUInt32(u32, "./CodePage", effectiveNodeContext, BeXmlDom::NODE_BIAS_Last))
        font.GetMetadataR().m_codePage = (LangCodePage)u32;

    if (BEXML_Success == fontConfig->SelectNodeContentAsUInt16(u16, "./DegreeChar", effectiveNodeContext, BeXmlDom::NODE_BIAS_Last))
        font.GetMetadataR().m_degreeCode = (DgnGlyph::T_Id)u16;
    
    if (BEXML_Success == fontConfig->SelectNodeContentAsUInt16(u16, "./DiameterChar", effectiveNodeContext, BeXmlDom::NODE_BIAS_Last))
        font.GetMetadataR().m_diameterCode = (DgnGlyph::T_Id)u16;
    
    if (BEXML_Success == fontConfig->SelectNodeContentAsUInt16(u16, "./PlusMinusChar", effectiveNodeContext, BeXmlDom::NODE_BIAS_Last))
        font.GetMetadataR().m_plusMinusCode = (DgnGlyph::T_Id)u16;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
static BentleyStatus embedShxFontFile(DgnFonts& dbFonts, BeFileNameCR path, BeXmlDomP fontConfig)
    {
    DgnFontPtr font = DgnFontPersistence::File::FromShxFile(path);
    if (!font.IsValid())
        {
        printf("ERROR - could not create an SHX font from file '%s'.\n", AString(path).c_str());
        return ERROR;
        }
    
    convertShxMetadata((DgnShxFontR)*font, fontConfig);

    return embedFont(dbFonts, *font);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
static BentleyStatus embedFontFile(DgnFonts& dbFonts, BeFileNameCR path, BeXmlDomP fontConfig, Utf8StringCR nameFilter)
    {
    WString extension = path.GetExtension();
    extension.ToLower();

    if (0 == wcscmp(L"ttf", extension.c_str())) return embedTrueTypeFontFiles(dbFonts, { path }, fontConfig, nameFilter);
    if (0 == wcscmp(L"ttc", extension.c_str())) return embedTrueTypeFontFiles(dbFonts, { path }, fontConfig, nameFilter);
    if (0 == wcscmp(L"otf", extension.c_str())) return embedTrueTypeFontFiles(dbFonts, { path }, fontConfig, nameFilter);
    if (0 == wcscmp(L"otc", extension.c_str())) return embedTrueTypeFontFiles(dbFonts, { path }, fontConfig, nameFilter);
    if (0 == wcscmp(L"rsc", extension.c_str())) return embedRscFontFile(dbFonts, path, fontConfig, nameFilter);
    if (0 == wcscmp(L"shx", extension.c_str())) return embedShxFontFile(dbFonts, path, fontConfig);

    printf("ERROR - file '%s' has unsupported extension '%s', it will be skipped.\n", AString(path).c_str(), AString(extension.c_str()).c_str());
    return ERROR;
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
static BentleyStatus embedOSFonts(DgnFonts& dbFonts, AppArgs const& appArgs, BeXmlDomP fontConfig)
    {
    LOGFONT lf;
    memset(&lf, 0, sizeof(lf));
    lf.lfCharSet = DEFAULT_CHARSET;

    HDC fontDC = ::CreateDCW(L"DISPLAY", NULL, NULL, NULL);
    bset<Utf8String> fontNames;
    ::EnumFontFamiliesEx(fontDC, &lf, enumerateSystemFontsCallback, reinterpret_cast<LPARAM>(&fontNames), 0);
    ::DeleteObject(fontDC);
    
    bool didAnyFail = false;

    for (Utf8StringCR fontName : fontNames)
        {
        if (!Utf8String::IsNullOrEmpty(appArgs.m_nameFilter.c_str()) && !std::regex_match(fontName.c_str(), std::regex(appArgs.m_nameFilter.c_str(), std::regex_constants::ECMAScript | std::regex_constants::icase)))
            continue;
        
        DgnFontPtr font = DgnFontPersistence::OS::FromGlobalTrueTypeRegistry(fontName.c_str());
        if (SUCCESS != embedFont(dbFonts, *font))
            didAnyFail |= true;

        // For large imports, free up memory/system resouces.
        font = nullptr;
        }

    return (didAnyFail ? ERROR : SUCCESS);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
static BentleyStatus embedFonts(DgnFonts& dbFonts, AppArgs const& appArgs, BeXmlDomP fontConfig)
    {
    if (appArgs.m_searchOSFonts)
        return embedOSFonts(dbFonts, appArgs, fontConfig);
    
    BeFileListIterator pathIter(appArgs.m_fontPath, false);
    BeFileName nextPath;
    bool didAnyFail = false;

    // Since multiple TrueType files may be required for a single font, separate them and embed them as one unit.
    bvector<BeFileName> trueTypePaths;

    while (SUCCESS == pathIter.GetNextFileName(nextPath))
        {
        WString extension = nextPath.GetExtension();
        extension.ToLower();
        if (extension.Equals(L"ttf") || extension.Equals(L"ttc") || extension.Equals(L"otf") || extension.Equals(L"otc"))
            {
            trueTypePaths.push_back(nextPath);
            continue;
            }
        
        didAnyFail |= (SUCCESS != embedFontFile(dbFonts, nextPath, fontConfig, appArgs.m_nameFilter));
        }
    
    if (!trueTypePaths.empty())
        didAnyFail |= (SUCCESS != embedTrueTypeFontFiles(dbFonts, trueTypePaths, fontConfig, appArgs.m_nameFilter));

    return (didAnyFail ? ERROR : SUCCESS);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2015
//---------------------------------------------------------------------------------------
int wmain(int argc, wchar_t const* argv[])
    {
    AppArgs appArgs;
    if (SUCCESS != parseArgs(appArgs, argc, argv))
        {
        printHelp();
        return 1;
        }
    
    initLogging(appArgs.m_programPath, appArgs.m_logConfigPath);
    
    RscFileManager::StaticInitialize (L"not-used");

    FontConverterApp app;
    DgnPlatformLib::Initialize(app, true);

    if (appArgs.m_fontConfigPath.empty())
        {
        appArgs.m_fontConfigPath = app.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory();
        appArgs.m_fontConfigPath.AppendToPath(L"fonts");
        appArgs.m_fontConfigPath.AppendToPath(L"MstnFontConfig.xml");
        }

    BeXmlDomPtr dom;
    BeXmlStatus xmlStatus;
    dom = BeXmlDom::CreateAndReadFromFile(xmlStatus, appArgs.m_fontConfigPath);
    if ((BEXML_Success != xmlStatus) || !dom.IsValid())
        printf("WARNING - font config file '%s' could not be opened, or is not valid XML; defaults will be used instead.\n", AString(appArgs.m_fontConfigPath).c_str());

    Db outputDb;
    if (appArgs.m_outputPath.DoesPathExist())
        {
        if (BE_SQLITE_OK != outputDb.OpenBeSQLiteDb(appArgs.m_outputPath, Db::OpenParams(Db::OpenMode::ReadWrite)))
            {
            printf("ERROR - could not open the existing output database '%s'\n", AString(appArgs.m_outputPath).c_str());
            return 1;
            }
        }
    else
        {
        if (BE_SQLITE_OK != outputDb.CreateNewDb(appArgs.m_outputPath))
            {
            printf("ERROR - could not create the output database '%s'\n", AString(appArgs.m_outputPath).c_str());
            return 1;
            }
        }
    
    DgnFonts dbFonts(outputDb, appArgs.m_tableName.c_str());
    if (!dbFonts.DbFontMap().DoesFontTableExist())
        dbFonts.DbFontMap().CreateFontTable();

    if (SUCCESS != embedFonts(dbFonts, appArgs, dom.get()))
        return 1;

    outputDb.SaveChanges();

#ifdef VERIFY
    DgnFonts::DbFontMapDirect::Iterator allFonts = dbFonts.DbFontMap().MakeIterator();
    for (DgnFonts::DbFontMapDirect::Iterator::const_iterator fontEntry : allFonts)
        {
        DgnFontCP font = dbFonts.FindFontById(fontEntry.GetId());
        if (nullptr == font)
            {
            printf("WARNING - Could not load font %i ('%s') after query.\n", (int)fontEntry.GetId().GetValue(), utf8ToLocale(fontEntry.GetName()).c_str());
            continue;
            }
        
        if (!font->IsResolved())
            {
            printf("WARNING - Could not resolve font %i ('%s') after query.\n", (int)fontEntry.GetId().GetValue(), utf8ToLocale(fontEntry.GetName()).c_str());
            continue;
            }
        }
#endif

    return 0;
    }
