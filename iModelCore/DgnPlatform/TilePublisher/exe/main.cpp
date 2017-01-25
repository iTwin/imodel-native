/*--------------------------------------------------------------------------------------+

|
|     $Source: TilePublisher/exe/main.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatform/DesktopTools/WindowsKnownLocationsAdmin.h>
#include <ThreeMx/ThreeMxApi.h>
#include <PointCloud/PointCloudApi.h>
#include <DgnPlatform/TilePublisher/TilePublisher.h>
#include "Constants.h"

#if defined(TILE_PUBLISHER_PROFILE)
#include <conio.h>
#endif

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_RENDER
using namespace BentleyApi::Dgn::Render::Tile3d;

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   08/16
//=======================================================================================
enum class GroundMode
{
    Abosolute,
    FixedHeight,        // Point at center of range and fixed (zero default) height is located at ground level.
    FixedPoint,         // Specified point is located at ground level.
};

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
    DisplayGlobe,
    NoReplace,
    Incremental,
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
        { L"ip", L"imageryProvider", L"Imagery Provider", false, false },
        { L"tp", L"terrainProvider", L"Terrain Provider", false, false },
        { L"dg", L"displayGlobe", L"Display with globe, sky etc.)", false, true },
        { L"nr", L"noreplace", L"Do not replace existing files", false, true },
        { L"up", L"update", L"Update existing tileset from model changes", false, true },
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
struct PublisherParams
{
private:
    BeFileName                      m_inputFileName;    //!< Path to the .bim
    Utf8String                      m_viewName;         //!< Name of the view definition from which to publish
    BeFileName                      m_outputDir;        //!< Directory in which to place the output
    WString                         m_tilesetName;      //!< Root name of the output tileset files
    double                          m_groundHeight;     //!< Height of ground plane.
    DPoint3d                        m_groundPoint;      //!< Ground point. (if m_groundMode == GroundMode::FixedOrigin
    GroundMode                      m_groundMode;
    double                          m_tolerance;
    uint32_t                        m_depth = 0xffffffff;
    bool                            m_surfacesOnly = false;
    bool                            m_verbose = false;
    Utf8String                      m_imageryProvider;
    Utf8String                      m_terrainProvider;
    bool                            m_displayGlobe = false;
    GeoPoint                        m_geoLocation = {-75.686844444444444444444444444444, 40.065702777777777777777777777778, 0.0 };   // Bentley Exton flagpole...
    bool                            m_overwriteExisting = true;
    bool                            m_publish = false;
    PublisherContext::TextureMode   m_textureMode = PublisherContext::TextureMode::Embedded;

    DgnViewId GetDefaultViewId(DgnDbR db) const;
public:
    PublisherParams () : m_groundHeight(0.0), m_groundPoint(DPoint3d::FromZero()), m_groundMode(GroundMode::FixedHeight), m_tolerance (.001), m_displayGlobe(false) { }
    BeFileNameCR GetInputFileName() const { return m_inputFileName; }
    BeFileNameCR GetOutputDirectory() const { return m_outputDir; }
    WStringCR GetTilesetName() const { return m_tilesetName; }
    Utf8StringCR GetViewName() const { return m_viewName; }
    double  GetGroundHeight() const { return m_groundHeight; }
    GroundMode GetGroundMode() const { return m_groundMode; }
    DPoint3dCR GetGroundPoint() const { return  m_groundPoint; }
    double GetTolerance() const { return m_tolerance; }
    uint32_t GetDepth() const { return m_depth; }
    bool SurfacesOnly() const { return m_surfacesOnly; }
    bool WantVerboseStatistics() const { return m_verbose; }
    GeoPointCP GetGeoLocation() const { return m_displayGlobe ? &m_geoLocation : nullptr; }
    bool GetOverwriteExistingOutputFile() const { return m_overwriteExisting; }
    bool GetIncremental() const { return m_publish; }
    PublisherContext::TextureMode GetTextureMode() const { return m_textureMode; }

    Utf8StringCR GetImageryProvider() const { return m_imageryProvider; }
    Utf8StringCR GetTerrainProvider() const { return m_terrainProvider; }

    bool ParseArgs(int ac, wchar_t const** av);
    DgnDbPtr OpenDgnDb() const;
    DgnViewId GetViewIds(DgnViewIdSet& viewIds, DgnDbR db);
    Json::Value  GetViewerOptions () const;
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewId PublisherParams::GetDefaultViewId(DgnDbR db) const
    {
    if (!m_viewName.empty())
        return ViewDefinition::QueryViewId(db, m_viewName);

    // Try default view
    DgnViewId viewId;
    if (BeSQLite::BE_SQLITE_ROW == db.QueryProperty(&viewId, sizeof(viewId), DgnViewProperty::DefaultView()) && viewId.IsValid())
        return viewId;

    // Try first spatial view
    for (auto const& entry : ViewDefinition::MakeIterator(db))
        {
        auto view = ViewDefinition::Get(db, entry.GetId());
        if (view.IsValid() && (view->IsSpatialView() || view->IsDrawingView()))
            {
            viewId = view->GetViewId();
            break;
            }
        }

    return viewId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewId PublisherParams::GetViewIds(DgnViewIdSet& viewIds, DgnDbR db)
    {
    bool publishSingleView = !m_viewName.empty();

    DgnViewId defaultViewId = GetDefaultViewId(db);
    ViewDefinitionCPtr view = ViewDefinition::Get(db, defaultViewId);
    if (view.IsNull())
        {
        printf("View not found\n");
        return DgnViewId();
        }

    viewIds.insert(defaultViewId);
    if (publishSingleView)
        return defaultViewId;

    m_viewName = view->GetName();

    for (auto const& entry : ViewDefinition::MakeIterator(db))
        {
        view = ViewDefinition::Get(db, entry.GetId());
        if (view.IsValid() && (view->IsSpatialView() || view->IsDrawingView()))
            viewIds.insert(entry.GetId());
        }

    return defaultViewId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr PublisherParams::OpenDgnDb() const
    {
    DgnDb::OpenParams openParams(DgnDb::OpenMode::Readonly);
    DgnDbPtr db = DgnDb::OpenDgnDb(nullptr, m_inputFileName, openParams);
    if (db.IsNull())
        printf("Failed to open file %ls\n", m_inputFileName.c_str());

    return db;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value  PublisherParams::GetViewerOptions () const
    {
    Json::Value viewerOptions;

    viewerOptions["displayInPlace"] = m_displayGlobe;
    if (!m_imageryProvider.empty())
        viewerOptions["imageryProvider"] = m_imageryProvider.c_str();

    if (!m_terrainProvider.empty())
        viewerOptions["terrainProvider"] = m_terrainProvider.c_str();

    return viewerOptions;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool PublisherParams::ParseArgs(int ac, wchar_t const** av)
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
                m_displayGlobe = true;
                break;
            case ParamId::GlobeImagery:
                m_imageryProvider = Utf8String(arg.m_value.c_str());
                m_displayGlobe = true;
                break;
            case ParamId::GlobeTerrain:
                m_terrainProvider = Utf8String(arg.m_value.c_str());
                m_displayGlobe = true;
                break;
            case ParamId::DisplayGlobe:
                m_displayGlobe = true;
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
            case ParamId::Incremental:
                m_publish = true;
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

//=======================================================================================
//! Publishes the contents of a DgnDb view as a Cesium tileset.
// @bsistruct                                                   Paul.Connelly   08/16
//=======================================================================================
struct TilesetPublisher : PublisherContext
{
private:
    TileGeneratorP              m_generator = nullptr;
    Status                      m_acceptTileStatus = Status::Success;
    uint32_t                    m_publishedTileDepth;
    BeMutex                     m_mutex;
    DgnViewId                   m_defaultViewId;
    bool                        m_verbose;
    bset<Utf8String>            m_modelsInProgress;
    Utf8String                  m_modelNameList;
    StopWatch                   m_timer;

    TileGeneratorStatus _AcceptTile(TileNodeCR tile) override;
    WString _GetTileUrl(TileNodeCR tile, WCharCP fileExtension) const override { return tile.GetFileName(TileUtil::GetRootNameForModel(tile.GetModel()).c_str(), fileExtension); }
    virtual bool _AllTilesPublished() const { return true; }

    TileGeneratorStatus _BeginProcessModel(DgnModelCR) override;
    TileGeneratorStatus _EndProcessModel(DgnModelCR, TileNodeP, TileGeneratorStatus) override;

    Status  GetViewsJson (Json::Value& value, DPoint3dCR groundPoint);

    template<typename T> Json::Value GetIdsJson(Utf8CP tableName, T const& ids);

    Status WriteWebApp(DPoint3dCR groundPoint, PublisherParams const& params);
    void OutputStatistics(TileGenerator::Statistics const& stats) const;
    void GenerateModelNameList();

    //=======================================================================================
    // @bsistruct                                                   Paul.Connelly   08/16
    //=======================================================================================
    struct ProgressMeter : ITileGenerationProgressMonitor
    {
    private:
        TilesetPublisher&   m_publisher;
        uint32_t            m_lastPercentCompleted = 0xffffffff;
        
        bool _WasAborted() override { return PublisherContext::Status::Success != m_publisher.GetTileStatus(); }
    public:
        explicit ProgressMeter(TilesetPublisher& publisher) : m_publisher(publisher) { }
        void _IndicateProgress(uint32_t completed, uint32_t total) override;
    };
public:
    TilesetPublisher(DgnDbR db, DgnViewIdSet const& viewIds, DgnViewId defaultViewId, BeFileNameCR outputDir, WStringCR tilesetName, GeoPointCP geoLocation, size_t maxTilesetDepth,  uint32_t publishDepth, bool publishNonSurfaces, bool publishIncremental, bool verbose, TextureMode textureMode)
        : PublisherContext(db, viewIds, outputDir, tilesetName, geoLocation, publishNonSurfaces, maxTilesetDepth, publishIncremental, textureMode),
          m_publishedTileDepth(publishDepth), m_defaultViewId(defaultViewId), m_verbose(verbose), m_timer(true)
        {
        // Put the scripts dir + html files in outputDir. Put the tiles in a subdirectory thereof.
        m_dataDir.AppendSeparator().AppendToPath(m_rootName.c_str()).AppendSeparator();
        }

    Status Publish(PublisherParams const& params);

    Status GetTileStatus() const { return m_acceptTileStatus; }

    bool WantVerboseStatistics() const { return m_verbose; }

    struct VerboseStatistics
        {
        Utf8String      m_modelNames;
        uint32_t        m_numModels;
        };

    VerboseStatistics GetVerboseStatistics()
        {
        BeMutexHolder lock(m_mutex);
        VerboseStatistics stats;
        stats.m_modelNames = m_modelNameList;
        stats.m_numModels = static_cast<uint32_t>(m_modelsInProgress.size());
        return stats;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeneratorStatus TilesetPublisher::_AcceptTile(TileNodeCR tile)
    {
    if (Status::Success != m_acceptTileStatus || tile.GetDepth() > m_publishedTileDepth)
        return TileGeneratorStatus::Aborted;

    TilePublisher publisher(tile, *this);
    auto publisherStatus = publisher.Publish();
    switch (publisherStatus)
        {
        case Status::Success:
        case Status::NoGeometry:   
            break;
        default:
            m_acceptTileStatus = publisherStatus;
            break;
        }

    return ConvertStatus(publisherStatus);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PublisherContext::Status TilesetPublisher::GetViewsJson (Json::Value& json, DPoint3dCR groundPoint)
    {
    // URL of tileset .json
    Utf8String rootNameUtf8(m_rootName.c_str()); // NEEDSWORK: Why can't we just use utf-8 everywhere...
    Utf8String tilesetUrl = rootNameUtf8;
    tilesetUrl.append(1, '/');
    tilesetUrl.append(rootNameUtf8);
    tilesetUrl.append(".json");

    json["tilesetUrl"] = tilesetUrl;

    return GetViewsetJson(json, groundPoint, m_defaultViewId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
PublisherContext::Status TilesetPublisher::WriteWebApp (DPoint3dCR groundPoint, PublisherParams const& params)
    {
    Json::Value json;
    Status      status;

    if (Status::Success != (status = GetViewsJson (json, groundPoint)))
        return status;


    Json::Value viewerOptions = params.GetViewerOptions();

    // If we are displaying "in place" but don't have a real geographic location - default to natural earth.
    if (IsGeolocated())
        {
        viewerOptions["displayInPlace"]= true;
        }
    else
        {
        if (viewerOptions["displayInPlace"].asBool() &&
            viewerOptions["imageryProvider"].isNull())
            viewerOptions["imageryProvider"] = "NaturalEarth";
        }

    json["viewerOptions"] = viewerOptions;

    WString     jsonRootName = m_rootName + L"_AppData";
    BeFileName  jsonFileName (nullptr, m_dataDir.c_str(), jsonRootName.c_str(), L"json");

    Utf8String jsonFileNameUtf8(jsonFileName.c_str());
    jsonFileNameUtf8.ReplaceAll("\\", "//");

    std::FILE* jsonFile = std::fopen(jsonFileNameUtf8.c_str(), "w");
    if (NULL == jsonFile)
        return Status::CantWriteToBaseDirectory;

    Utf8String jsonStr = Json::FastWriter().write(json);
    std::fwrite(jsonStr.c_str(), 1, jsonStr.size(), jsonFile);
    std::fclose(jsonFile);

    // Produce the html file contents
    BeFileName htmlFileName = m_outputDir;
    htmlFileName.AppendString(m_rootName.c_str()).AppendExtension(L"html");
    std::FILE* htmlFile = std::fopen(Utf8String(htmlFileName.c_str()).c_str(), "w");
    if (NULL == htmlFile)
        return Status::CantWriteToBaseDirectory;

    Utf8String jsonFileUrl = Utf8String (m_rootName) + "/" + Utf8String(jsonRootName.c_str());
    jsonFileUrl.append(".json");
    std::fwrite(s_viewerHtmlPrefix, 1, sizeof(s_viewerHtmlPrefix)-1, htmlFile);
    std::fwrite(jsonFileUrl.c_str(), 1, jsonFileUrl.size(), htmlFile);
    std::fwrite(s_viewerHtmlSuffix, 1, sizeof(s_viewerHtmlSuffix)-1, htmlFile);
    std::fclose(htmlFile);

    // Symlink the scripts, if not already present
    BeFileName scriptsSrcDir(T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
    scriptsSrcDir.AppendToPath(L"scripts");
    BeFileName scriptsDstDir(m_outputDir);
    scriptsDstDir.AppendToPath(L"scripts");
    BeFileName::CloneDirectory(scriptsSrcDir.c_str(), scriptsDstDir.c_str());

    return Status::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TilesetPublisher::OutputStatistics(TileGenerator::Statistics const& stats) const
    {
    printf("\nStatistics:\n"
           "Tile count: %u\n"
           "Tile generation time: %.4f seconds\n"
           "Average per-tile: %.4f seconds\n",
           static_cast<uint32_t>(stats.m_tileCount),
           stats.m_tileGenerationTime,
           0 != stats.m_tileCount ? stats.m_tileGenerationTime / stats.m_tileCount : 0.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TilesetPublisher::ProgressMeter::_IndicateProgress(uint32_t completed, uint32_t total)
    {
    uint32_t    pctComplete = static_cast<double>(completed)/total * 100;

    if (m_lastPercentCompleted == pctComplete && 99 != m_lastPercentCompleted)
        {
        printf("...");
        }
    else
        {
        if (m_publisher.WantVerboseStatistics())
            {
            auto stats = m_publisher.GetVerboseStatistics();
            printf("\n%u models in progress: %s", stats.m_numModels, stats.m_modelNames.c_str());
            }

        printf("\nGenerating Tiles: %3u%% (%u/%u models completed)%s", pctComplete, completed, total, completed == total ? "\n" : "");
        m_lastPercentCompleted = pctComplete;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
PublisherContext::Status TilesetPublisher::Publish(PublisherParams const& params)
    {
    auto status = InitializeDirectories(GetDataDirectory());
    if (Status::Success != status)
        return status;

    ProgressMeter progressMeter(*this);
    TileGenerator generator (m_dbToTile, GetDgnDb(), nullptr, &progressMeter);

    DRange3d            range;

    m_generator = &generator;
    status = PublishViewModels(generator, range, params.GetTolerance(), params.SurfacesOnly(), progressMeter);
    m_generator = nullptr;

    if (Status::Success != status)
        {
        CleanDirectories(GetDataDirectory());
        return Status::Success != m_acceptTileStatus ? m_acceptTileStatus : status;
        }

    OutputStatistics(generator.GetStatistics());

    DPoint3d        groundPoint;

    if (GroundMode::FixedPoint == params.GetGroundMode())
        {
        groundPoint.SumOf (params.GetGroundPoint(), GetDgnDb().Units().GetGlobalOrigin());
        }
    else
        {
        Transform   tileToDb;

        tileToDb.InverseOf (m_dbToTile);
        
        groundPoint = DPoint3d::FromInterpolate (range.low, .5, range.high);
        tileToDb.Multiply (groundPoint);
        groundPoint.z = params.GetGroundHeight();
        }

    return WriteWebApp(groundPoint, params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeneratorStatus TilesetPublisher::_BeginProcessModel(DgnModelCR model)
    {
    auto status = PublisherContext::_BeginProcessModel(model);
    if (TileGeneratorStatus::Success == status)
        {
        BeMutexHolder lock(m_mutex);
        m_modelsInProgress.insert(model.GetName());
        GenerateModelNameList();
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeneratorStatus TilesetPublisher::_EndProcessModel(DgnModelCR model, TileNodeP rootTile, TileGeneratorStatus status)
    {
        {
        BeMutexHolder lock(m_mutex);
        auto const& modelName = model.GetName();
        m_modelsInProgress.erase(modelName);
        printf("\nCompleted model %s (%f seconds elapsed)\n", modelName.c_str(), m_timer.GetCurrentSeconds());
        GenerateModelNameList();
        }

    return PublisherContext::_EndProcessModel(model, rootTile, status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TilesetPublisher::GenerateModelNameList()
    {
    m_modelNameList = "[";
    for (auto const& modelName : m_modelsInProgress)
        {
        m_modelNameList.append(modelName);
        m_modelNameList.append(1, ',');
        }

    auto len = m_modelNameList.length();
    if (len > 1)
        m_modelNameList[len-1] = ']';
    else
        m_modelNameList.append(1, ']');
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
    IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override { return *new WindowsKnownLocationsAdmin(); }
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
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
int wmain(int ac, wchar_t const** av)
    {
#if defined(TILE_PUBLISHER_PROFILE)
    printf("Press a key to start...\n");
    _getch();
#endif

    PublisherParams createParams;
    if (!createParams.ParseArgs(ac, av))
        {
        printUsage(av[0]);
        return 1;
        }

    Host host;
    DgnPlatformLib::Initialize(host, false);

    DgnDomains::RegisterDomain(ThreeMx::ThreeMxDomain::GetDomain());
    DgnDomains::RegisterDomain(PointCloud::PointCloudDomain::GetDomain());
                                                                                  
    DgnDbPtr db = createParams.OpenDgnDb();
    if (db.IsNull())
        return 1;

    DgnViewIdSet viewsToPublish;
    DgnViewId defaultView = createParams.GetViewIds(viewsToPublish, *db);
    if (!defaultView.IsValid())
        return 1;

    static size_t       s_maxTilesetDepth = 5;          // Limit depth of tileset to avoid lag on initial load (or browser crash) on large tilesets.

    TilesetPublisher publisher(*db, viewsToPublish, defaultView, createParams.GetOutputDirectory(), createParams.GetTilesetName(), createParams.GetGeoLocation(), s_maxTilesetDepth, 
                                createParams.GetDepth(), createParams.SurfacesOnly(), createParams.GetIncremental(), createParams.WantVerboseStatistics(), createParams.GetTextureMode());

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



