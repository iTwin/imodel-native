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
enum class ParamId
{
    Input = 0,
    View,
    Output,
    Name,
    HtmlOnly,
    GroundHeight,
    Invalid
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
        { L"i", L"input", L"Name of the .bim file to publish", true },
        { L"v", L"view", L"Name of the view to publish. If omitted, the default view is used", false },
        { L"o", L"output", L"Directory in which to place the output .html file. If omitted, the output is placed in the .bim file's directory", false },
        { L"n", L"name", L"Name of the .html file and root name of the tileset .json and .b3dm files. If omitted, uses the name of the .bim file", false },
        { L"h", L"htmlonly", L"'Y' to generate only the html file.", false, true },
        { L"g", L"groundheight",L"Ground height (in meters).", false},
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
        bool haveArgValue = nullptr != equalPos;
        WCharCP argValue = haveArgValue ? equalPos+1 : nullptr;
        auto paramNameLen = haveArgValue ? equalPos - raw : wcslen(raw);

        if (0 == paramNameLen || (!verboseParamName && 1 != paramNameLen))
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
    bool            m_htmlOnly;         //!< Generate HTML only (not tileset).
    double          m_groundHeight;     //!< Height of ground plane.


    DgnViewId GetViewId(DgnDbR db) const;
public:
    PublisherParams () : m_htmlOnly (false), m_groundHeight (0.0) { }
    BeFileNameCR GetInputFileName() const { return m_inputFileName; }
    BeFileNameCR GetOutputDirectory() const { return m_outputDir; }
    WStringCR GetTilesetName() const { return m_tilesetName; }
    Utf8StringCR GetViewName() const { return m_viewName; }
    double  GetGroundHeight() const { return m_groundHeight; }
    bool GetHtmlOnly() const { return m_htmlOnly; }

    bool ParseArgs(int ac, wchar_t const** av);
    DgnDbPtr OpenDgnDb() const;
    ViewControllerPtr LoadViewController(DgnDbR db);
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
            case ParamId::HtmlOnly:
                m_htmlOnly = true;
                break;
            case ParamId::GroundHeight:
                {
                double groundHeight;

                if (1 == swscanf (arg.m_value.c_str(), L"%lf", &groundHeight))
                    {
                    m_groundHeight = groundHeight;
                    }
                else
                    {
                    printf ("Unrecognized ground height: %ls\n", av[i]);
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
    TileGeneratorP      m_generator = nullptr;
    Status              m_acceptTileStatus = Status::Success;

    virtual TileGenerator::Status _AcceptTile(TileNodeCR tile) override;
    virtual WString _GetTileUrl(TileNodeCR tile, WCharCP fileExtension) const override { return tile.GetRelativePath(GetRootName().c_str(), fileExtension); }

    Status WriteWebApp(TransformCR transform, DPoint3dCR groundPoint);
    void OutputStatistics(TileGenerator::Statistics const& stats) const;

    //=======================================================================================
    // @bsistruct                                                   Paul.Connelly   08/16
    //=======================================================================================
    struct ProgressMeter : TileGenerator::IProgressMeter
    {
    private:
        Utf8String          m_taskName;
        TilesetPublisher&   m_publisher;
        uint32_t            m_lastNumCompleted = 0xffffffff;
        DgnModelCP          m_model;
        
        virtual bool _WasAborted() override { return PublisherContext::Status::Success != m_publisher.GetTileStatus(); }
    public:
        explicit ProgressMeter(TilesetPublisher& publisher) : m_publisher(publisher), m_model (nullptr) { }
        virtual void _SetModel (DgnModelCP model) { m_model = model; }
        virtual void _SetTaskName(TileGenerator::TaskName task) override;
        virtual void _IndicateProgress(uint32_t completed, uint32_t total) override;
    };
public:
    TilesetPublisher(ViewControllerR viewController, BeFileNameCR outputDir, WStringCR tilesetName, size_t maxTilesetDepth, size_t maxTilesPerDirectory)
        : PublisherContext(viewController, outputDir, tilesetName, maxTilesetDepth, maxTilesPerDirectory)
        {
        // Put the scripts dir + html files in outputDir. Put the tiles in a subdirectory thereof.
        m_dataDir.AppendSeparator().AppendToPath(m_rootName.c_str()).AppendSeparator();
        }

    Status Publish(bool appOnly, DPoint3dCR groundPoint);

    Status GetTileStatus() const { return m_acceptTileStatus; }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerator::Status TilesetPublisher::_AcceptTile(TileNodeCR tile)
    {
    if (Status::Success != m_acceptTileStatus)
        return TileGenerator::Status::Aborted;

    TilePublisher publisher(tile, *this);
    auto publisherStatus = publisher.Publish();
    switch (publisherStatus)
        {
        case Status::Success:
        case Status::NoGeometry:    // ok for tile to have no geometry
            break;
        default:
            m_acceptTileStatus = publisherStatus;
            break;
        }

    return ConvertStatus(publisherStatus);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
PublisherContext::Status TilesetPublisher::WriteWebApp (TransformCR transform, DPoint3dCR groundPoint)
    {
    // Set up initial view based on view controller settings
    DVec3d xVec, yVec, zVec;
    m_viewController.GetRotation().GetRows(xVec, yVec, zVec);

    auto cameraView = m_viewController._ToCameraView();
    bool useCamera = nullptr != cameraView /* ###TODO? Apparently no longer possible to turn camera off && cameraView->IsCameraOn() */;
    DPoint3d viewDest = useCamera ? cameraView->GetControllerCamera().GetEyePoint() : m_viewController.GetCenter();
    if (!useCamera)
        {
        static const double s_zRatio = 1.5;
        DVec3d viewDelta = m_viewController.GetDelta();
        viewDest = DPoint3d::FromSumOf(viewDest, zVec, std::max(viewDelta.x, viewDelta.y) * s_zRatio);
        }

    transform.Multiply(viewDest);
    transform.MultiplyMatrixOnly(yVec);
    transform.MultiplyMatrixOnly(zVec);

    yVec.Normalize();
    zVec.Normalize();
    zVec.Negate();      // Towards target.

    bool geoLocated = !m_tileToEcef.IsIdentity();
    Utf8CP viewOptionString = geoLocated ? "" : "globe: false, scene3DOnly:true, skyBox: false, skyAtmosphere: false";
    Utf8CP viewFrameString = geoLocated ? s_geoLocatedViewingFrameJs : s_3dOnlyViewingFrameJs; 
    Utf8String adjustHeightString;
    
    if (geoLocated)
        {
        DPoint3d    groundEcefPoint;

        transform.Multiply (groundEcefPoint, groundPoint);
        adjustHeightString = Utf8PrintfString (s_adjustTerrainString, groundEcefPoint.x, groundEcefPoint.y, groundEcefPoint.z);
        }

    Utf8String       tileSetHtml = Utf8PrintfString (s_tilesetHtml, m_rootName.c_str(), m_rootName.c_str());

    // Produce the html file contents
    Utf8PrintfString html(s_viewerHtml, viewOptionString, tileSetHtml.c_str(), adjustHeightString.c_str(), viewFrameString,  viewDest.x, viewDest.y, viewDest.z, zVec.x, zVec.y, zVec.z, yVec.x, yVec.y, yVec.z);

    BeFileName htmlFileName = m_outputDir;
    htmlFileName.AppendString(m_rootName.c_str()).AppendExtension(L"html");

    std::FILE* htmlFile = std::fopen(Utf8String(htmlFileName.c_str()).c_str(), "w");
    std::fwrite(html.data(), 1, html.size(), htmlFile);
    std::fclose(htmlFile);

    // Symlink the Cesium dir, if not already present
    BeFileName cesiumSrcDir(T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
    cesiumSrcDir.AppendToPath(L"scripts");
    cesiumSrcDir.AppendToPath(L"Cesium");
    BeFileName cesiumDstDir(m_outputDir);
    cesiumDstDir.AppendToPath(L"Cesium");
    BeFileName::CloneDirectory(cesiumSrcDir.c_str(), cesiumDstDir.c_str());

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
           "Range tree generation time: %.4f seconds\n"
           "Tile creation: %.4f seconds Average per-tile: %.4f seconds\n",
           static_cast<uint32_t>(stats.m_tileCount),
           static_cast<uint32_t>(stats.m_tileDepth),
           stats.m_collectionTime,
           stats.m_tileCreationTime,
           0 != stats.m_tileCount ? stats.m_tileCreationTime / stats.m_tileCount : 0.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TilesetPublisher::ProgressMeter::_IndicateProgress(uint32_t completed, uint32_t total)
    {
    if (m_lastNumCompleted == completed)
        {
        printf("...");
        }
    else
        {
        m_lastNumCompleted = completed;
        uint32_t    pctComplete = static_cast<double>(completed)/total * 100;
        Utf8String  modelNameString;   

        if (nullptr != m_model)
            modelNameString = " (" +  m_model->GetName() + ")";

        printf("\n%s%s: %u%% (%u/%u)%s", m_taskName.c_str(), modelNameString.c_str(), pctComplete, completed, total, completed == total ? "\n" : "");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TilesetPublisher::ProgressMeter::_SetTaskName(TileGenerator::TaskName task)
    {
    Utf8String newTaskName = (TileGenerator::TaskName::CollectingTileMeshes == task) ? "Publishing tiles" : "Generating range tree";
    if (!m_taskName.Equals(newTaskName))
        {
        m_lastNumCompleted = 0xffffffff;
        m_taskName = newTaskName;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
PublisherContext::Status TilesetPublisher::Publish(bool appOnly, DPoint3dCR groundPoint)
    {
    if (!appOnly)
        {
        auto status = Setup();
        if (Status::Success != status)
            return status;

        ProgressMeter progressMeter(*this);
        TileGenerator generator (m_dbToTile, &progressMeter);

        static double       s_toleranceInMeters = 0.01;

        m_generator = &generator;
        PublishViewModels(generator, *this, s_toleranceInMeters, progressMeter);
        m_generator = nullptr;

        if (Status::Success != status)
            return Status::Success != m_acceptTileStatus ? m_acceptTileStatus : status;

        OutputStatistics(generator.GetStatistics());
        }

    return WriteWebApp(Transform::FromProduct(m_tileToEcef, m_dbToTile), groundPoint);
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

    TilesetPublisher publisher(*viewController, createParams.GetOutputDirectory(), createParams.GetTilesetName(), s_maxTilesetDepth, s_maxTilesPerDirectory);

    printf("Publishing:\n"
           "\tInput: View %s from %ls\n"
           "\tOutput: %ls%ls.html\n"
           "\tData: %ls\n",
            createParams.GetViewName().c_str(), createParams.GetInputFileName().c_str(), publisher.GetOutputDirectory().c_str(), publisher.GetRootName().c_str(), publisher.GetDataDirectory().c_str());
            
    auto status = publisher.Publish (createParams.GetHtmlOnly(), DPoint3d::From (0.0, 0.0, createParams.GetGroundHeight()));
    printStatus(status);

    return static_cast<int>(status);
    }

