/*--------------------------------------------------------------------------------------+
|
|     $Source: TilePublisher/exe/main.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatform/DesktopTools/KnownDesktopLocationsAdmin.h>
#include <ThreeMx/ThreeMxApi.h>
#include <PointCloud/PointCloudApi.h>
#include <TilePublisher/CesiumPublisher.h>
#include <TilePublisher/CesiumTilePublisher.h>
#include <Raster/RasterApi.h>
#include <ScalableMeshSchema/ScalableMeshSchemaApi.h>
#include <ScalableMesh/ScalableMeshLib.h>
#include <RealityPlatformTools/RealityDataService.h>
#include <DgnPlatform/DesktopTools/ConfigurationManager.h>
#include <Logging/bentleylogging.h>
#include <WebServices/iModelHub/Client/ClientHelper.h>


#define LOG (*NativeLogging::LoggingManager::GetLogger(L"TilePublisher"))

#if defined(TILE_PUBLISHER_PROFILE)
#include <conio.h>
#endif

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_REALITYPLATFORM
USING_NAMESPACE_BENTLEY_RENDER
USING_NAMESPACE_BENTLEY_TILEPUBLISHER
USING_NAMESPACE_BENTLEY_TILEPUBLISHER_CESIUM
USING_NAMESPACE_BENTLEY_IMODELHUB

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   08/16
//=======================================================================================
enum class ParamId
{
    Input = 0,
    View,
    Output,
    Name,
    GroundHeight,
    GroundPoint,
    Tolerance,
    Depth,
    SurfacesOnly,
    GeographicLocation,
    GlobeImagery,
    GlobeTerrain,
    NoReplace,
    VerboseStatistics,
    TextureMode,
    UserName,
    Password,
    Environment,
    GlobeOn,
    GlobeOff,
    OnlyHistory,
    AddHistory,
    Invalid,
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   08/16
//=======================================================================================
struct CommandParam
{
    WCharCP     m_abbrev;
    WCharCP     m_verbose;
    WCharCP     m_descr;
    bool        m_required;
    bool        m_boolean;  // True => arg is true if param specified. False => expect 'arg=value'.
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   08/16
//=======================================================================================
static CommandParam s_paramTable[] =
    {
        { L"i",  L"input", L"Name of the .bim file to publish", true },
        { L"v",  L"view", L"Name of the view that will be initially opened in the viewer. If omitted, the default view is used", false },
        { L"o",  L"output", L"Directory in which to place the output .html file. If omitted, the output is placed in the .bim file's directory", false },
        { L"n",  L"name", L"Name of the .html file and root name of the tileset .json and .b3dm files. If omitted, uses the name of the .bim file", false },
        { L"gh", L"groundheight",L"Ground height (meters).", false},
        { L"gp", L"groundpoint",L"Ground Location in database coordinates (meters).", false},
        { L"t",  L"tolerance",L"Tolerance (meters).", false},
        { L"d",  L"depth",L"Publish tiles to specified depth. e.g. 0=publish only the root tile.", false},
        { L"su", L"surfaces", L"Publish only surfaces for 3D models. (no polylines, text etc.)", false, true },
        { L"l",  L"geographicLocation", L"Geographic location (longitude, latitude)", false },
        { L"ip", L"imageryProvider", L"Imagery Provider {BingMapRoads, BingMapsAerial, BingMapsAerialWithLabels, MapboxSatellite, MapboxTerrain, MapboxStreets, MapboxStreetsClassic, StamenWatercolor, StamenToner, ESRIWorldImagery, ", false, false },
        { L"tp", L"terrainProvider", L"Terrain Provider", false, false },
        { L"nr", L"noreplace", L"Do not replace existing files", false, true },
        { L"vs", L"verbose", L"Output verbose statistics during publishing", false, true },
        { L"tx", L"textureMode", L"Texture mode - \"Embedded (default)\", \"External\", or \"Compressed\"", false, false },
        { L"un", L"username", L"UserName for I-Model hub (History Publishing)", false, false },
        { L"pa", L"password", L"Password for I-Model hub (History Publishing)", false, false },
        { L"en", L"environment", L"Environment for I-Model hub (History Publishing)", false, false },
        { L"gl1", L"globeOn", L"Force globe on in all views", false, true },
        { L"gl0", L"globeOff", L"Force globe off in all views", false, true },
        { L"hi",  L"historyOnly", L"Publish only history", false, true },
        { L"h+",  L"history", L"Publish history and TIP", false, true },
    };

static const size_t s_paramTableSize = _countof(s_paramTable);

                                      
//=======================================================================================
// @bsistruct                                                   Paul.Connelly   08/16
//=======================================================================================
struct CommandArg
{
    ParamId     m_paramId;
    WString     m_value;

    explicit CommandArg(WCharCP raw) : m_paramId(ParamId::Invalid)
        {
        if (WString::IsNullOrEmpty(raw) || '-' != *raw)
            return;

        ++raw;
        bool verboseParamName = *raw == '-';
        if (verboseParamName)
            ++raw;

        WCharCP equalPos = wcschr(raw, '=');

        if (nullptr == equalPos)        // Batch files don't support=
            equalPos = wcschr(raw, ':');

        bool haveArgValue = nullptr != equalPos;
        WCharCP argValue = haveArgValue ? equalPos+1 : nullptr;
        auto paramNameLen = haveArgValue ? equalPos - raw : wcslen(raw);

        if (0 == paramNameLen)  
            return;

        for (size_t i = 0; i < s_paramTableSize; i++)
            {
            auto const& param = s_paramTable[i];
            WCharCP paramName = verboseParamName ? param.m_verbose : param.m_abbrev;
            if (0 != wcsncmp(raw, paramName, paramNameLen) || paramNameLen != wcslen(paramName))
                continue;

            if ((nullptr == equalPos) != (param.m_boolean))
                return;

            m_paramId = static_cast<ParamId>(i);

            if (!param.m_boolean)
                {
                m_value = argValue;
                m_value.Trim(L"\"");
                m_value.Trim();
                }

            break;
            }
        }
};


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
struct ServiceLocalState : public IJsonLocalState
    {
    private:
        Json::Value m_map;

    public:
        JsonValueR GetStubMap()
            {
            return m_map;
            }
        //! Saves the Utf8String value in the local state.
        //! @note The nameSpace and key pair must be unique.
        void _SaveValue(Utf8CP nameSpace, Utf8CP key, Utf8StringCR value) override
            {
            Utf8PrintfString identifier("%s/%s", nameSpace, key);

            if (value == "null")
                {
                m_map.removeMember(identifier);
                }
            else
                {
                m_map[identifier] = value;
                }
            };
        //! Returns a stored Utf8String from the local state.
        //! @note The nameSpace and key pair uniquely identifies the value.
        Utf8String _GetValue(Utf8CP nameSpace, Utf8CP key) const override
            {
            Utf8PrintfString identifier("%s/%s", nameSpace, key);
            return m_map.isMember(identifier) ? m_map[identifier].asCString() : "null";
            };
    };



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
static ServiceLocalState* getLocalState()
    {
    // MT Note: C++11 guarantees that the following line of code will be executed only once and in a thread-safe manner:
    ServiceLocalState* s_localState = new ServiceLocalState;
    return s_localState;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
static WebServices::ClientInfoPtr getClientInfo()
    {
    static Utf8CP s_productId = "1654"; // Navigator Desktop
    // MT Note: C++11 guarantees that the following line of code will be executed only once and in a thread-safe manner:
    static WebServices::ClientInfoPtr s_clientInfo = WebServices::ClientInfoPtr(
        new WebServices::ClientInfo("Bentley-Test", BeVersion(1, 0), "{41FE7A91-A984-432D-ABCF-9B860A8D5360}", "TestDeviceId", "TestSystem", s_productId));
    return s_clientInfo;
    }


//=======================================================================================
// @bsistruct                                                   Paul.Connelly   08/16
//=======================================================================================
struct Params : PublisherParams
{
    bool ParseArgs(int ac, wchar_t const** av);
    DgnDbPtr OpenDgnDb() const;
                                      
                                          // History (WIP) requires IModel Hub connection.
    Utf8String                      m_userName;
    Utf8String                      m_password;
    Utf8String                      m_environment;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DoSignInForHistory()
    {
    Credentials                 credentials;
    WebServices::UrlProvider::Environment   urlEnvironment = WebServices::UrlProvider::Environment::Qa;

    if (m_environment.StartsWithI("Dev"))
        urlEnvironment =  urlEnvironment = WebServices::UrlProvider::Environment::Dev;
    else if (0 == m_environment.CompareToI("Release"))
        urlEnvironment = WebServices::UrlProvider::Environment::Release;


    credentials.SetUsername(m_userName);
    credentials.SetPassword(m_password);

    Http::HttpClient::Initialize(T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
    iModel::Hub::ClientHelper::Initialize(getClientInfo(), getLocalState());
    UrlProvider::Initialize(urlEnvironment, UrlProvider::DefaultTimeout, getLocalState());

    Tasks::AsyncError error;
    m_client = iModel::Hub::ClientHelper::GetInstance()->SignInWithCredentials(&error, credentials);

    BeAssert (m_client.IsValid());

    return m_client.IsValid() ? SUCCESS : ERROR;
    }

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr Params::OpenDgnDb() const
    {
    DgnDb::OpenParams openParams(DgnDb::OpenMode::Readonly);
    DgnDbPtr db = DgnDb::OpenDgnDb(nullptr, m_inputFileName, openParams);
    if (db.IsNull())
        LOG.errorv("Failed to open file %ls\n", m_inputFileName.c_str());

    return db;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool Params::ParseArgs(int ac, wchar_t const** av)
    {
    if (ac < 2)
        return false;

    bool haveInput = false;
    for (int i = 1; i < ac; i++)
        {
        CommandArg arg(av[i]);
        switch (arg.m_paramId)
            {
            case ParamId::Input:
                haveInput = true;
                BeFileName::FixPathName(m_inputFileName, arg.m_value.c_str());
                break;
            case ParamId::View:
                m_viewName = Utf8String(arg.m_value.c_str());
                break;
            case ParamId::Output:
                BeFileName::FixPathName(m_outputDir, arg.m_value.c_str());
                break;
            case ParamId::Name:
                m_tilesetName = arg.m_value.c_str();
                break;
            case ParamId::GroundPoint:
               if (3 != swscanf (arg.m_value.c_str(), L"%lf,%lf,%lf", &m_groundPoint.x, &m_groundPoint.y, &m_groundPoint.z))
                    {
                    LOG.errorv ("Unrecognized ground point: %ls\n", av[i]);
                    return false;
                    }
                m_groundMode = GroundMode::FixedPoint;
                break;

            case ParamId::GroundHeight:
                if (1 != swscanf (arg.m_value.c_str(), L"%lf", &m_groundHeight))
                    {
                    LOG.errorv ("Unrecognized ground height: %ls\n", av[i]);
                    return false;
                    }
                m_groundMode = GroundMode::FixedHeight;
                break;
            case ParamId::Depth:
                if (1 != swscanf (arg.m_value.c_str(), L"%u", &m_depth))
                    {
                    LOG.error ("Expected unsigned integer for depth parameter\n");
                    return false;
                    }
                break;
            case ParamId::SurfacesOnly:
                m_surfacesOnly = true;
                break;
            case ParamId::VerboseStatistics:
                m_verbose = true;
                break;
            case ParamId::GeographicLocation:
                if (2 != swscanf (arg.m_value.c_str(), L"%lf,%lf", &m_geoLocation.longitude, &m_geoLocation.latitude))
                    {
                    LOG.errorv ("Unrecognized geographic location: %ls\n", av[i]);
                    return false;
                    }
                m_geoLocated = true;
                break;
            case ParamId::GlobeImagery:
                m_imageryProvider = Utf8String(arg.m_value.c_str());
                m_globeMode = PublisherContext::GlobeMode::On;
                break;
            case ParamId::GlobeTerrain:
                m_terrainProvider = Utf8String(arg.m_value.c_str());
                break;
            case ParamId::Tolerance:
                {
                static const double     s_minTolerance = 1.0E-5, s_maxTolerance = 100.0;

                if (1 != swscanf (arg.m_value.c_str(), L"%lf", &m_tolerance))
                    {
                    LOG.errorv ("Unrecognized tolerance value: %ls\n", av[i]);
                    return false;
                    }
                if (m_tolerance < s_minTolerance || s_maxTolerance > s_maxTolerance)
                    {
                    LOG.errorv ("Invalid tolerance: %lf (must be between %lf and %lf)\n", m_tolerance, s_minTolerance, s_maxTolerance);
                    return false;
                    }
                break;
                }
            case ParamId::NoReplace:
                m_overwriteExisting = false;
                break;
            case ParamId::TextureMode:
                {
                WString     textureModeString = arg.m_value;
                
                textureModeString.ToLower();
                if (textureModeString == L"embedded")
                    m_textureMode = PublisherContext::TextureMode::Embedded;
                else if (textureModeString == L"external")
                    m_textureMode = PublisherContext::TextureMode::External;
                else if (textureModeString == L"compressed")
                    m_textureMode = PublisherContext::TextureMode::Compressed;
                else
                    {
                    LOG.errorv ("Unrecognized texture mode: %ls\n", arg.m_value.c_str());
                    return false;
                    }
                    
                break;
                }

            case ParamId::UserName:
                m_userName = Utf8String(arg.m_value.c_str());
                break;


            case ParamId::Password:
                m_password = Utf8String(arg.m_value.c_str());
                break;

            case ParamId::Environment:
                m_environment = Utf8String(arg.m_value.c_str());
                break;

            case ParamId::GlobeOn:
                m_globeMode = PublisherContext::GlobeMode::On;
                break;

            case ParamId::GlobeOff:
                m_globeMode = PublisherContext::GlobeMode::Off;
                break;

            case ParamId::OnlyHistory:
                m_historyMode = HistoryMode::OnlyHistory;
                break;

            case ParamId::AddHistory:
                m_historyMode = HistoryMode::AddHistory;
                break;


            default:
                LOG.errorv("Unrecognized command option %ls\n", av[i]);
                return false;

            }
        }


    if (HistoryMode::OmitHistory != m_historyMode)
        {
        if (m_userName.empty() || m_password.empty())
            {
            LOG.error ("Username and password are reaquired for history publishing\n");
            return false;
            }
        }

    
    if (!haveInput)
        {
        LOG.error("Input filename is required\n");
        return false;
        }

    if (m_outputDir.empty())
        m_outputDir = m_inputFileName.GetDirectoryName();

    if (m_tilesetName.empty())                                                      
        {
        if (!m_inputFileName.empty())
            m_tilesetName = m_inputFileName.GetFileNameWithoutExtension().c_str();
        }


    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
static void printUsage(WCharCP exePath)
    {
    WString exeName = BeFileName::GetFileNameAndExtension(exePath);

    LOG.info("Publish the contents of a DgnDb view as a Cesium tileset viewable in a web browser.\n\n");
    LOG.infov("Usage: %ls -i|--input= [OPTIONS...]\n\n", exeName.c_str());
    
    for (auto const& cmdArg : s_paramTable)
        LOG.infov("  --%ls=|-%ls=\t(%ls)\t%ls\n", cmdArg.m_verbose, cmdArg.m_abbrev, cmdArg.m_required ? L"required" : L"optional", cmdArg.m_descr);
    }


//=======================================================================================
// @bsistruct                                                   Paul.Connelly   08/16
//=======================================================================================
struct Host : DgnPlatformLib::Host
{
private:
    void _SupplyProductName(Utf8StringR name) override { name.assign("TilePublisher"); }
    IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override { return *new KnownDesktopLocationsAdmin(); }
    BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() override
        {
        BeFileName sqlang(GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
        sqlang.AppendToPath(L"sqlang/DgnPlatform_en.sqlang.db3");
        return BeSQLite::L10N::SqlangFiles(sqlang);
        }

    static void OnAssert(WCharCP msg, WCharCP file, unsigned line, BeAssertFunctions::AssertType type)
        {
        LOG.errorv("Assertion Failure: %ls (%ls:%d)\n", msg, file, line);
        }
public:
    Host() { EnsureAssertHandler(); }

    static void EnsureAssertHandler()
        {
        // Stupid Raster creates its own Host for ImagePP, which replaces the stupid static BeAssertHandler, replacing our Host's _OnAssert()...
        BeAssertFunctions::SetBeAssertHandler(&Host::OnAssert);
        }
};


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.St-Pierre                  08/17
+---------------+---------------+---------------+---------------+---------------+------*/
struct  SMHost : ScalableMesh::ScalableMeshLib::Host
    {
    SMHost()
        {
        }

    ScalableMesh::ScalableMeshAdmin& _SupplyScalableMeshAdmin()
        {
        struct CsScalableMeshAdmin : public ScalableMesh::ScalableMeshAdmin
            {
            virtual IScalableMeshTextureGeneratorPtr _GetTextureGenerator() override
                {
                IScalableMeshTextureGeneratorPtr generator;
                return generator;
                }

            virtual bool _CanImportPODfile() const override
                {
                return false;
                }

            virtual Utf8String _GetProjectID() const override
                {
                Utf8String projectGUID("95b8160c-8df9-437b-a9bf-22ad01fecc6b");

                WString projectGUIDw;

                if (BSISUCCESS == ConfigurationManager::GetVariable(projectGUIDw, L"SM_PROJECT_GUID"))
                    {
                    projectGUID.Assign(projectGUIDw.c_str());
                    }
                return projectGUID;
                }
            };
        return *new CsScalableMeshAdmin;
        };

    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
int wmain(int ac, wchar_t const** av)
    {
#if defined(TILE_PUBLISHER_PROFILE)
    printf("Press a key to start...\n");
    _getch();
#endif

    NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::CONSOLE_LOGGING_PROVIDER);
    NativeLogging::LoggingConfig::SetSeverity(L"TilePublisher", NativeLogging::LOG_TRACE);

    Params createParams;
    if (!createParams.ParseArgs(ac, av))
        {
        printUsage(av[0]);
        return 1;
        }
    
    WString     bimiumVar;
    if (SUCCESS == ConfigurationManager::GetVariable(bimiumVar, L"BIMIUM_DIST_DIR"))
        createParams.SetBimiumDistDir(BeFileName(bimiumVar.c_str()));

    Host host;
    DgnPlatformLib::Initialize(host, false);

    DgnDomains::RegisterDomain(ThreeMx::ThreeMxDomain::GetDomain(), DgnDomain::Required::No, DgnDomain::Readonly::Yes);
    DgnDomains::RegisterDomain(PointCloud::PointCloudDomain::GetDomain(), DgnDomain::Required::No, DgnDomain::Readonly::Yes);
    DgnDomains::RegisterDomain(Raster::RasterDomain::GetDomain(), DgnDomain::Required::No, DgnDomain::Readonly::Yes);
    DgnDomains::RegisterDomain(ScalableMeshSchema::ScalableMeshDomain::GetDomain(), DgnDomain::Required::No, DgnDomain::Readonly::Yes);

    ScalableMesh::ScalableMeshLib::Initialize(*new SMHost());

    Host::EnsureAssertHandler();

    DgnDbPtr db = createParams.OpenDgnDb();
    if (db.IsNull())
        return 1;

    DgnViewIdSet viewsToPublish;
    DgnViewId defaultView = createParams.GetViewIds(viewsToPublish, *db);
    if (!defaultView.IsValid())
        return 1;

    if (HistoryMode::OmitHistory != createParams.GetHistoryMode() &&
        SUCCESS != createParams.DoSignInForHistory())
        {
        LOG.errorv ("Unable to sign in for history extraction\n");
        return 1;
        }

    static size_t       s_maxTilesetDepth = 5;          // Limit depth of tileset to avoid lag on initial load (or browser crash) on large tilesets.

#ifdef DIRECT_CESIUM_PUBLISH
    CesiumDirect::DirectPublisher publisher(*db, createParams, viewsToPublish, defaultView);
#else
    TilesetPublisher publisher(*db, db->GeoLocation().ComputeProjectExtents(), createParams, viewsToPublish, defaultView, s_maxTilesetDepth);
#endif

    if (!createParams.GetOverwriteExistingOutputFile())
        {
        BeFileName  outputFile (nullptr, createParams.GetOutputDirectory().c_str(), publisher.GetRootName().c_str(), L".html");
        if (outputFile.DoesPathExist())
            {
            LOG.errorv ("Output file: %ls aready exists and \"No Replace\" option is specified\n", outputFile.c_str());
            return 1;
            }
        }

    LOG.debugv("Publishing:\n"
           "\tInput: View %s from %ls\n"
           "\tOutput: %ls%ls.html\n"
           "\tData: %ls\n",
            createParams.GetViewName().c_str(), createParams.GetInputFileName().c_str(), publisher.GetOutputDirectory().c_str(), publisher.GetRootName().c_str(), publisher.GetDataDirectory().c_str());
            
    auto status = publisher.Publish (createParams);

    static const Utf8CP s_msg[] =
        {
        "Publishing succeeded",
        "No geometry to publish",
        "Publishing aborted",
        "Failed to write to base directory",
        "Failed to create subdirectory",
        "Failed to write scene",
        "Failed to write node"
        };

    auto index = static_cast<uint32_t>(status);
    Utf8CP msg = index < _countof(s_msg) ? s_msg[index] : "Unrecognized error";
    printf("Result: %hs.\n", msg);

    return static_cast<int>(status);
    }


