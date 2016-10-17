/*--------------------------------------------------------------------------------------+
|
|     $Source: TilePublisher/exe/main.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatform/DesktopTools/WindowsKnownLocationsAdmin.h>
#include <ThreeMx/ThreeMxApi.h>
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
    Polylines,
    GeographicLocation,
    GlobeImagery,
    Standalone,
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
        { L"pl", L"polylines", L"Publish polylines", false, true },
        { L"l",  L"geographicLocation", L"Geographic location (longitude, latitude)", false },
        { L"ip", L"imageryProvider" L"Imagery Provider", false, false },
        { L"s",  L"standalone", L"Display in \"standalone\" mode, without globe, sky etc.)", false, true },
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
    BeFileName      m_inputFileName;    //!< Path to the .bim
    Utf8String      m_viewName;         //!< Name of the view definition from which to publish
    BeFileName      m_outputDir;        //!< Directory in which to place the output
    WString         m_tilesetName;      //!< Root name of the output tileset files
    double          m_groundHeight;     //!< Height of ground plane.
    DPoint3d        m_groundPoint;      //!< Ground point. (if m_groundMode == GroundMode::FixedOrigin
    GroundMode      m_groundMode;
    double          m_tolerance;
    uint32_t        m_depth = 0xffffffff;
    bool            m_polylines = false;
    Utf8String      m_imageryProvider;
    bool            m_standalone = false;
    GeoPoint        m_geoLocation = {-75.686844444444444444444444444444, 40.065702777777777777777777777778, 0.0 };   // Bentley Exton flagpole...

    DgnViewId GetViewId(DgnDbR db) const;
public:
    PublisherParams () : m_groundHeight(0.0), m_groundPoint(DPoint3d::FromZero()), m_groundMode(GroundMode::FixedHeight), m_tolerance (.001) { }
    BeFileNameCR GetInputFileName() const { return m_inputFileName; }
    BeFileNameCR GetOutputDirectory() const { return m_outputDir; }
    WStringCR GetTilesetName() const { return m_tilesetName; }
    Utf8StringCR GetViewName() const { return m_viewName; }
    double  GetGroundHeight() const { return m_groundHeight; }
    GroundMode GetGroundMode() const { return m_groundMode; }
    DPoint3dCR GetGroundPoint() const { return  m_groundPoint; }
    double GetTolerance() const { return m_tolerance; }
    uint32_t GetDepth() const { return m_depth; }
    bool WantPolylines() const { return m_polylines; }
    GeoPointCR GetGeoLocation() const { return m_geoLocation; }

    Utf8StringCR GetImageryProvider() const { return m_imageryProvider; }

    bool ParseArgs(int ac, wchar_t const** av);
    DgnDbPtr OpenDgnDb() const;
    ViewControllerPtr LoadViewController(DgnDbR db);
    Json::Value  GetViewerOptions () const;
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewId PublisherParams::GetViewId(DgnDbR db) const
    {
    if (!m_viewName.empty())
        return ViewDefinition::QueryViewId(m_viewName, db);

    // Try default view
    DgnViewId viewId;
    if (BeSQLite::BE_SQLITE_ROW == db.QueryProperty(&viewId, sizeof(viewId), DgnViewProperty::DefaultView()) && viewId.IsValid())
        return viewId;

    // Try first spatial view
    for (auto const& entry : ViewDefinition::MakeIterator(db))
        {
        auto view = ViewDefinition::QueryView(entry.GetId(), db);
        if (view.IsValid() && view->IsSpatialView())
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
ViewControllerPtr PublisherParams::LoadViewController(DgnDbR db)
    {
    DgnViewId viewId = GetViewId(db);
    ViewDefinitionCPtr view = ViewDefinition::QueryView(viewId, db);
    if (view.IsNull())
        {
        printf("View not found\n");
        return false;
        }

    m_viewName = view->GetName();

    ViewControllerPtr controller = view->LoadViewController();
    if (controller.IsNull())
        {
        printf("Failed to load view %hs\n", view->GetName().c_str());
        return false;
        }

    return controller;
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

    viewerOptions["displayInPlace"] = !m_standalone;
    if (!m_imageryProvider.empty())
        viewerOptions["imageryProvider"] = m_imageryProvider.c_str();

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
            case ParamId::Polylines:
                m_polylines = true;
                break;
            case ParamId::GeographicLocation:
                if (2 != swscanf (arg.m_value.c_str(), L"%lf,%lf", &m_geoLocation.longitude, &m_geoLocation.latitude))
                    {
                    printf ("Unrecognized geographic location: %ls\n", av[i]);
                    return false;
                    }
                m_standalone = false;
                break;
            case ParamId::GlobeImagery:
                m_imageryProvider = Utf8String(arg.m_value.c_str());
                break;
            case ParamId::Standalone:
                m_standalone = true;
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
struct TilesetPublisher : PublisherContext, TileGenerator::ITileCollector
{
private:
    TileGeneratorP              m_generator = nullptr;
    DgnModelIdSet               m_allModels;
    DgnCategoryIdSet            m_allCategories;
    Status                      m_acceptTileStatus = Status::Success;
    uint32_t                    m_publishedTileDepth;
    BeMutex                     m_mutex;
    bvector<TileNodeCP>         m_emptyNodes;

    virtual TileGenerator::Status _AcceptTile(TileNodeCR tile) override;
    virtual WString _GetTileUrl(TileNodeCR tile, WCharCP fileExtension) const override { return tile.GetRelativePath(GetRootName().c_str(), fileExtension); }
    virtual TileGenerationCacheCR _GetCache() const override { BeAssert(nullptr != m_generator); return m_generator->GetCache(); }
    virtual bool _OmitFromTileset(TileNodeCR tile) const override { return m_emptyNodes.end() != std::find(m_emptyNodes.begin(), m_emptyNodes.end(), &tile); }
    virtual bool _AllTilesPublished() const { return true; }

    Status  GetViewsJson (Json::Value& value, TransformCR transform, DPoint3dCR groundPoint);

    template<typename T> Json::Value GetIdsJson(Utf8CP tableName, T const& ids);

    Status WriteWebApp(TransformCR transform, DPoint3dCR groundPoint, PublisherParams const& params);
    void OutputStatistics(TileGenerator::Statistics const& stats) const;

    //=======================================================================================
    // @bsistruct                                                   Paul.Connelly   08/16
    //=======================================================================================
    struct ProgressMeter : ITileGenerationProgressMonitor
    {
    private:
        Utf8String          m_taskName;
        TilesetPublisher&   m_publisher;
        uint32_t            m_lastPercentCompleted = 0xffffffff;
        DgnModelCP          m_model;
        
        virtual bool _WasAborted() override { return PublisherContext::Status::Success != m_publisher.GetTileStatus(); }
    public:
        explicit ProgressMeter(TilesetPublisher& publisher) : m_publisher(publisher), m_model (nullptr) { }
        virtual void _SetModel (DgnModelCP model) { m_model = model; }
        virtual void _SetTaskName(ITileGenerationProgressMonitor::TaskName task) override;
        virtual void _IndicateProgress(uint32_t completed, uint32_t total) override;
    };
public:
    TilesetPublisher(ViewControllerR viewController, BeFileNameCR outputDir, WStringCR tilesetName, GeoPointCP geoLocation, size_t maxTilesetDepth, size_t maxTilesPerDirectory, uint32_t publishDepth, bool publishPolylines)
        : PublisherContext(viewController, outputDir, tilesetName, geoLocation, publishPolylines, maxTilesetDepth, maxTilesPerDirectory), m_publishedTileDepth(publishDepth)
        {
        // Put the scripts dir + html files in outputDir. Put the tiles in a subdirectory thereof.
        m_dataDir.AppendSeparator().AppendToPath(m_rootName.c_str()).AppendSeparator();

        auto& db = viewController.GetDgnDb();
        for (auto& view : ViewDefinition::MakeIterator(db))
            {
            auto viewDefinition = ViewDefinition::QueryView(view.GetId(), db);
            auto spatialView = viewDefinition.IsValid() ? viewDefinition->ToSpatialView() : nullptr;
            if (nullptr == spatialView)
                continue;

            auto modelSelector = db.Elements().Get<ModelSelector>(spatialView->GetModelSelectorId());
            if (modelSelector.IsValid())
                {
                auto viewModels = modelSelector->GetModelIds();
                m_allModels.insert(viewModels.begin(), viewModels.end());
                }

            auto categorySelector = db.Elements().Get<CategorySelector>(spatialView->GetCategorySelectorId());
            if (categorySelector.IsValid())
                {
                auto viewCats = categorySelector->GetCategoryIds();
                m_allCategories.insert(viewCats.begin(), viewCats.end());
                }
            }
        }

    Status Publish(PublisherParams const& params);

    Status GetTileStatus() const { return m_acceptTileStatus; }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerator::Status TilesetPublisher::_AcceptTile(TileNodeCR tile)
    {
    if (Status::Success != m_acceptTileStatus || tile.GetDepth() > m_publishedTileDepth)
        return TileGenerator::Status::Aborted;

    TilePublisher publisher(tile, *this);
    auto publisherStatus = publisher.Publish();
    switch (publisherStatus)
        {
        case Status::Success:
            break;
        case Status::NoGeometry:    // ok for tile to have no geometry - but mark as empty so we avoid including in json
            if (tile.GetChildren().empty())
                {
                // Leaf nodes with no children should not be published.
                // Reality models often contain empty parents with non-empty children - they must be published.
                BeMutexHolder lock(m_mutex);
                m_emptyNodes.push_back(&tile);
                }
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
PublisherContext::Status TilesetPublisher::GetViewsJson (Json::Value& json, TransformCR transform, DPoint3dCR groundPoint)
    {
    // URL of tileset .json
    Utf8String rootNameUtf8(m_rootName.c_str()); // NEEDSWORK: Why can't we just use utf-8 everywhere...
    Utf8String tilesetUrl = rootNameUtf8;
    tilesetUrl.append(1, '/');
    tilesetUrl.append(rootNameUtf8);
    tilesetUrl.append(".json");
    json["tilesetUrl"] = tilesetUrl;
    json["name"] = rootNameUtf8;

    // Geolocation
    bool geoLocated = !m_tileToEcef.IsIdentity();
    if (geoLocated)
        {
        DPoint3d    groundEcefPoint;

        transform.Multiply (groundEcefPoint, groundPoint);
        json["geolocated"] = true;
        json["groundPoint"] = PointToJson(groundEcefPoint);
        }

    auto& viewsJson =  json["views"] = Json::Value (Json::objectValue); 
    bset<DgnViewId> publishedViews;

    for (auto& view : ViewDefinition::MakeIterator(GetDgnDb()))
        {
        auto    viewDefinition = ViewDefinition::QueryView(view.GetId(), GetDgnDb());

        SpatialViewDefinitionCP spatialView;

        if (!viewDefinition.IsValid() || nullptr == (spatialView = viewDefinition->ToSpatialView()))
            continue;

        Json::Value     entry (Json::objectValue);

        if (nullptr != view.GetName())
            entry["name"] = view.GetName();

        GetSpatialViewJson (entry, *spatialView, transform);
        auto modelSelector = GetDgnDb().Elements().Get<ModelSelector>(spatialView->GetModelSelectorId());

        if (modelSelector.IsValid())
            entry["models"] = IdSetToJson(modelSelector->GetModelIds());

        auto categorySelector = GetDgnDb().Elements().Get<CategorySelector>(spatialView->GetCategorySelectorId());

        if (categorySelector.IsValid())
            entry["categories"] = IdSetToJson (categorySelector->GetCategoryIds());

        auto displayStyle = GetDgnDb().Elements().Get<DisplayStyle>(spatialView->GetDisplayStyleId());

        if (displayStyle.IsValid())
            {
            ColorDef    backgroundColor = displayStyle->GetBackgroundColor();
            auto&       colorJson = entry["backgroundColor"] = Json::objectValue;
            colorJson["red"]   = backgroundColor.GetRed()   / 255.0;            
            colorJson["green"] = backgroundColor.GetGreen() / 255.0;            
            colorJson["blue"]  = backgroundColor.GetBlue()  / 255.0;            
            }

        publishedViews.insert (view.GetId());
        viewsJson[view.GetId().ToString()] = entry;
        }

    if (m_allModels.empty())
        return Status::NoGeometry;

    json["models"] = GetModelsJson (m_allModels);
    json["categories"] = GetCategoriesJson (m_allCategories);

    if (publishedViews.find(GetViewController().GetViewId()) == publishedViews.end())
        json["defaultView"] = publishedViews.begin()->ToString();
    else
        json["defaultView"] = GetViewController().GetViewId().ToString();
    
    return Status::Success; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
PublisherContext::Status TilesetPublisher::WriteWebApp (TransformCR transform, DPoint3dCR groundPoint, PublisherParams const& params)
    {
    Json::Value json;
    Status      status;

    if (Status::Success != (status = GetViewsJson (json, transform, groundPoint)))
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

    BeFileName jsonFileName = m_outputDir;
    jsonFileName.AppendString(m_rootName.c_str()).AppendExtension(L"json");

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

    Utf8String jsonFileUrl(m_rootName.c_str());
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
    printf("Statistics:\n"
           "Tile count: %u\n"
           "Tile depth: %u\n"
           "Cache population time: %.4f seconds\n"
           "Tile node generation time: %.4f seconds\n"
           "Tile publishing: %.4f seconds Average per-tile: %.4f seconds\n",
           static_cast<uint32_t>(stats.m_tileCount),
           static_cast<uint32_t>(stats.m_tileDepth),
           stats.m_cachePopulationTime,
           stats.m_collectionTime,
           stats.m_tileCreationTime,
           0 != stats.m_tileCount ? stats.m_tileCreationTime / stats.m_tileCount : 0.0);
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
        Utf8String  modelNameString;   

        if (nullptr != m_model)
            modelNameString = " (" +  m_model->GetName() + ")";

        printf("\n%s%s: %u%% (%u/%u)%s", m_taskName.c_str(), modelNameString.c_str(), pctComplete, completed, total, completed == total ? "\n" : "");
        m_lastPercentCompleted = pctComplete;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TilesetPublisher::ProgressMeter::_SetTaskName(ITileGenerationProgressMonitor::TaskName task)
    {
    Utf8String newTaskName;
    switch (task)
        {
        case ITileGenerationProgressMonitor::TaskName::PopulatingCache:         newTaskName = "Populating cache"; break;
        case ITileGenerationProgressMonitor::TaskName::GeneratingTileNodes:     newTaskName = "Generating tile tree"; break;
        case ITileGenerationProgressMonitor::TaskName::CollectingTileMeshes:    newTaskName = "Publishing tiles"; break;
        default:                                                                BeAssert(false); newTaskName = "Unknown task"; break;
        }

    if (!m_taskName.Equals(newTaskName))
        {
        m_lastPercentCompleted = 0xffffffff;
        m_taskName = newTaskName;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
PublisherContext::Status TilesetPublisher::Publish(PublisherParams const& params)
    {
    auto status = Setup();
    if (Status::Success != status)
        return status;

    TileModelCategoryFilter filter(GetDgnDb(), &m_allModels, &m_allCategories);
    ProgressMeter progressMeter(*this);
    TileGenerator generator (m_dbToTile, GetDgnDb(), &filter, &progressMeter);

    DRange3d            range;

    m_generator = &generator;
    PublishViewModels(generator, *this, range, params.GetTolerance(), progressMeter);
    m_generator = nullptr;

    if (Status::Success != status)
        return Status::Success != m_acceptTileStatus ? m_acceptTileStatus : status;

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

    return WriteWebApp(Transform::FromProduct(m_tileToEcef, m_dbToTile), groundPoint, params);
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
    virtual void _SupplyProductName(Utf8StringR name) override { name.assign("TilePublisher"); }
    virtual IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override { return *new WindowsKnownLocationsAdmin(); }
    virtual BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() override
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

    DgnDbPtr db = createParams.OpenDgnDb();
    if (db.IsNull())
        return 1;


    ViewControllerPtr viewController = createParams.LoadViewController(*db);
    if (viewController.IsNull())
        return 1;

    static size_t       s_maxTilesetDepth = 5;          // Limit depth of tileset to avoid lag on initial load (or browser crash) on large tilesets.
    static size_t       s_maxTilesPerDirectory = 0;     // Put all files in same directory

    TilesetPublisher publisher(*viewController, createParams.GetOutputDirectory(), createParams.GetTilesetName(), &createParams.GetGeoLocation(), s_maxTilesetDepth, s_maxTilesPerDirectory, createParams.GetDepth(), createParams.WantPolylines());

    printf("Publishing:\n"
           "\tInput: View %s from %ls\n"
           "\tOutput: %ls%ls.html\n"
           "\tData: %ls\n",
            createParams.GetViewName().c_str(), createParams.GetInputFileName().c_str(), publisher.GetOutputDirectory().c_str(), publisher.GetRootName().c_str(), publisher.GetDataDirectory().c_str());
            
    auto status = publisher.Publish (createParams);
    printStatus(status);

    return static_cast<int>(status);
    }


