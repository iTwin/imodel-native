/*--------------------------------------------------------------------------------------+
|
|     $Source: TilePublisher/exe/main.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatform/DesktopTools/KnownDesktopLocationsAdmin.h>
#include <ThreeMx/ThreeMxApi.h>
#include <PointCloud/PointCloudApi.h>
#include <TilePublisher/CesiumPublisher.h>
#include <Raster/RasterApi.h>
#include <ScalableMeshSchema/ScalableMeshSchemaApi.h>
#include <ScalableMesh/ScalableMeshLib.h>
#include <RealityPlatform/RealityDataService.h>
#include <ConnectClientWrapperNative/ConnectClientWrapper.h>
#include <DgnPlatform/DesktopTools/ConfigurationManager.h>


#if defined(TILE_PUBLISHER_PROFILE)
#include <conio.h>
#endif

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_REALITYPLATFORM
USING_NAMESPACE_BENTLEY_RENDER
USING_NAMESPACE_BENTLEY_TILEPUBLISHER
USING_NAMESPACE_BENTLEY_TILEPUBLISHER_CESIUM

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

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   08/16
//=======================================================================================
struct Params : PublisherParams
{
    bool ParseArgs(int ac, wchar_t const** av);
    DgnDbPtr OpenDgnDb() const;
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr Params::OpenDgnDb() const
    {
    DgnDb::OpenParams openParams(DgnDb::OpenMode::Readonly);
    DgnDbPtr db = DgnDb::OpenDgnDb(nullptr, m_inputFileName, openParams);
    if (db.IsNull())
        printf("Failed to open file %ls\n", m_inputFileName.c_str());

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
                    printf ("Unrecognized ground point: %ls\n", av[i]);
                    return false;
                    }
                m_groundMode = GroundMode::FixedPoint;
                break;

            case ParamId::GroundHeight:
                if (1 != swscanf (arg.m_value.c_str(), L"%lf", &m_groundHeight))
                    {
                    printf ("Unrecognized ground height: %ls\n", av[i]);
                    return false;
                    }
                m_groundMode = GroundMode::FixedHeight;
                break;
            case ParamId::Depth:
                if (1 != swscanf (arg.m_value.c_str(), L"%u", &m_depth))
                    {
                    printf ("Expected unsigned integer for depth parameter\n");
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
                    printf ("Unrecognized geographic location: %ls\n", av[i]);
                    return false;
                    }
                m_geoLocated = true;
                break;
            case ParamId::GlobeImagery:
                m_imageryProvider = Utf8String(arg.m_value.c_str());
                break;
            case ParamId::GlobeTerrain:
                m_terrainProvider = Utf8String(arg.m_value.c_str());
                break;
            case ParamId::Tolerance:
                {
                static const double     s_minTolerance = 1.0E-5, s_maxTolerance = 100.0;

                if (1 != swscanf (arg.m_value.c_str(), L"%lf", &m_tolerance))
                    {
                    printf ("Unrecognized tolerance value: %ls\n", av[i]);
                    return false;
                    }
                if (m_tolerance < s_minTolerance || s_maxTolerance > s_maxTolerance)
                    {
                    printf ("Invalid tolerance: %lf (must be between %lf and %lf)\n", m_tolerance, s_minTolerance, s_maxTolerance);
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
                    printf ("Unrecognized texture mode: %ls\n", arg.m_value.c_str());
                    return false;
                    }
                    
                break;
                }

            default:
                printf("Unrecognized command option %ls\n", av[i]);
                return false;
            }
        }

    if (!haveInput)
        {
        printf("Input filename is required\n");
        return false;
        }

    if (m_outputDir.empty())
        m_outputDir = m_inputFileName.GetDirectoryName();

    if (m_tilesetName.empty())
        m_tilesetName = m_inputFileName.GetFileNameWithoutExtension().c_str();

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
static void printUsage(WCharCP exePath)
    {
    WString exeName = BeFileName::GetFileNameAndExtension(exePath);

    printf("Publish the contents of a DgnDb view as a Cesium tileset viewable in a web browser.\n\n");
    printf("Usage: %ls -i|--input= [OPTIONS...]\n\n", exeName.c_str());
    
    for (auto const& cmdArg : s_paramTable)
        printf("  --%ls=|-%ls=\t(%ls)\t%ls\n", cmdArg.m_verbose, cmdArg.m_abbrev, cmdArg.m_required ? L"required" : L"optional", cmdArg.m_descr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
static void printStatus(PublisherContext::Status status)
    {
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
        printf("Assertion Failure: %ls (%ls:%d)\n", msg, file, line);
        }
public:
    Host() { BeAssertFunctions::SetBeAssertHandler(&Host::OnAssert); }
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
            };
        return *new CsScalableMeshAdmin;
        };


    ScalableMesh::WsgTokenAdmin& _SupplyWsgTokenAdmin()
        {

        auto getTokenFunction = []() -> Utf8String
            {
            Utf8String emptyToken;
            return emptyToken;
            };
        return *new ScalableMesh::WsgTokenAdmin(getTokenFunction);
        }


    ScalableMesh::SASTokenAdmin& _SupplySASTokenAdmin()
        {
        auto getTokenFunction = [this](const Utf8String& realityDataGuid) -> Utf8String
            {
            SMHost::initializeRealityDataService();
            if (m_sasConnections.find(realityDataGuid) == m_sasConnections.end())
                {
                SASConnection newConnection;
                newConnection.m_handshake = new AzureHandshake(realityDataGuid, false /*writeable*/);
                m_sasConnections[realityDataGuid] = newConnection;
                }
            assert(m_sasConnections.find(realityDataGuid) != m_sasConnections.end());
            auto& sasConnection = m_sasConnections[realityDataGuid];
            assert(sasConnection.m_handshake != nullptr);
            int64_t currentTime;
            DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(currentTime);
            if ((sasConnection.m_azureTokenTimer - currentTime) < 0)
                {
                // Request Azure URL of the reality data
                RawServerResponse rawResponse = RealityDataService::BasicRequest((RealityDataUrl*)sasConnection.m_handshake);
                if (rawResponse.status != RequestStatus::BADREQ)
                    {
                    // The handshake status with Azure need not be checked, if the request fails the current token will be used until it expires (or return an empty token)
                    /*BentleyStatus handshakeStatus = */
                    sasConnection.m_handshake->ParseResponse(rawResponse.body, sasConnection.m_azureServer, sasConnection.m_azureToken, sasConnection.m_azureTokenTimer);
                    }
                else
                    {
                    // Try again after 50 minutes...
                    sasConnection.m_azureTokenTimer = currentTime + 1000 * 60 * 50;
                    assert(!"Problem with the handshake");
                    }
                }
            return sasConnection.m_azureToken;
            };
        return *new ScalableMesh::SASTokenAdmin(getTokenFunction);
        }


    ScalableMesh::SSLCertificateAdmin& _SupplySSLCertificateAdmin()
        {
        auto getSSLCertificatePath = []() -> Utf8String
            {
            Utf8String certificatePath;
            return certificatePath;
            };

        return *new ScalableMesh::SSLCertificateAdmin(getSSLCertificatePath);
        }

    // &&RB TODO: the following function can be removed when the url to RDS repository
    //            is being transferred in the ibim file.
    virtual Utf8String GetProjectWiseContextShareLink(const WString& path) override
        {
        if (m_smPaths->count(path) > 0 && path.substr(0, 8) == L"https://")
            {
            auto guidPos = path.find(L"/", 8) + 1;
            auto guidLength = path.find(L"/", guidPos) - guidPos;
            auto guid = Utf8String(path.substr(guidPos, guidLength));

            SMHost::initializeRealityDataService();

            Utf8String rdsUrl = "https://" + RealityDataService::GetServerName() + "v" + RealityDataService::GetWSGProtocol() + "/Repositories/" + RealityDataService::GetRepoName() + "/" + RealityDataService::GetSchemaName() + "/RealityData/" + guid;

            return rdsUrl;
            }
        return Utf8String();
        }

private:

    struct SASConnection
        {
        int64_t m_azureTokenTimer = 0;
        Utf8String m_azureToken;
        Utf8String m_azureServer;
        AzureHandshake* m_handshake;
        };

    bmap<Utf8String, SASConnection> m_sasConnections;

    static StatusInt initializeRealityDataService()
        {
        if (RealityDataService::AreParametersSet()) return SUCCESS;

        WString serverUrl;

#if 0 
        //WString serverUrl = L"connect-realitydataservices.bentley.com"; //this probably should be a function in the RDS API.
        serverUrl = L"qa-connect-realitydataservices.bentley.com"; //this probably should be a function in the RDS API.
#endif

        try     {
            Bentley::Connect::Wrapper::Native::ConnectClientWrapper connectClient;
            std::wstring buddiUrl;
            connectClient.GetBuddiUrl(L"RealityDataServices", buddiUrl);

            serverUrl.assign(buddiUrl.c_str());
            }
        catch (...)
            {
            }

        serverUrl.ReplaceI(L"https://", L"");  // remove scheme prefix 

        if (0 == serverUrl.size())
            return ERROR;


        RealityDataService::SetServerComponents(Utf8String(serverUrl.c_str()),
            RealityDataService::GetWSGProtocol(),
            RealityDataService::GetRepoName(),
            RealityDataService::GetSchemaName());

        //hardcoded the one for RM Internal for now. Need a UI of some sort to pick the project if
        //user has multiples, etc
        //RealityDataService::SetProjectId(Utf8String("75c7d1d7-1e32-4c4f-842d-ea6bade38638"));
        //RealityDataService::SetProjectId(Utf8String("75c7d1d7-1e32-4c4f-842d-ea6bade38638"));
        //RealityDataService::SetProjectId(Utf8String("4b8643d2-c6b0-4d77-b491-61408fe03b79"));

        Utf8String projectGUID("95b8160c-8df9-437b-a9bf-22ad01fecc6b");

        WString projectGUIDw;

        if (BSISUCCESS == ConfigurationManager::GetVariable(projectGUIDw, L"SM_PROJECT_GUID"))
            {
            projectGUID.Assign(projectGUIDw.c_str());
            }

        RealityDataService::SetProjectId(projectGUID);

        return SUCCESS;
        }

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

    Params createParams;
    if (!createParams.ParseArgs(ac, av))
        {
        printUsage(av[0]);
        return 1;
        }

    Host host;
    DgnPlatformLib::Initialize(host, false);

    DgnDomains::RegisterDomain(ThreeMx::ThreeMxDomain::GetDomain(), DgnDomain::Required::No, DgnDomain::Readonly::Yes);
    DgnDomains::RegisterDomain(PointCloud::PointCloudDomain::GetDomain(), DgnDomain::Required::No, DgnDomain::Readonly::Yes);
    DgnDomains::RegisterDomain(Raster::RasterDomain::GetDomain(), DgnDomain::Required::No, DgnDomain::Readonly::Yes);
    DgnDomains::RegisterDomain(ScalableMeshSchema::ScalableMeshDomain::GetDomain(), DgnDomain::Required::No, DgnDomain::Readonly::Yes);
    ScalableMesh::ScalableMeshLib::Initialize(*new SMHost());
                                                                                  
    DgnDbPtr db = createParams.OpenDgnDb();
    if (db.IsNull())
        return 1;

    DgnViewIdSet viewsToPublish;
    DgnViewId defaultView = createParams.GetViewIds(viewsToPublish, *db);
    if (!defaultView.IsValid())
        return 1;

    static size_t       s_maxTilesetDepth = 5;          // Limit depth of tileset to avoid lag on initial load (or browser crash) on large tilesets.

    TilesetPublisher publisher(*db, createParams, viewsToPublish, defaultView, s_maxTilesetDepth);

    if (!createParams.GetOverwriteExistingOutputFile())
        {
        BeFileName  outputFile (nullptr, createParams.GetOutputDirectory().c_str(), publisher.GetRootName().c_str(), L".html");
        if (outputFile.DoesPathExist())
            {
            printf ("Output file: %ls aready exists and \"No Replace\" option is specified\n", outputFile.c_str());
            return 1;
            }
        }

    printf("Publishing:\n"
           "\tInput: View %s from %ls\n"
           "\tOutput: %ls%ls.html\n"
           "\tData: %ls\n",
            createParams.GetViewName().c_str(), createParams.GetInputFileName().c_str(), publisher.GetOutputDirectory().c_str(), publisher.GetRootName().c_str(), publisher.GetDataDirectory().c_str());
            
    auto status = publisher.Publish (createParams);
    printStatus(status);

    return static_cast<int>(status);
    }

