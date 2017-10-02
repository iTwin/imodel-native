/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/TilePublisher/CesiumPublisher.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <TilePublisher/TilePublisher.h>

#define USING_NAMESPACE_BENTLEY_TILEPUBLISHER_CESIUM using namespace BentleyApi::Dgn::TilePublish::Cesium;
;
BEGIN_BENTLEY_TILEPUBLISHER_NAMESPACE

namespace Cesium
{
DEFINE_POINTER_SUFFIX_TYPEDEFS(PublisherParams);

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   08/16
//=======================================================================================
enum class GroundMode
{
    Absolute,
    FixedHeight,        // Point at center of range and fixed (zero default) height is located at ground level.
    FixedPoint,         // Specified point is located at ground level.
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   08/16
//=======================================================================================
struct PublisherParams
{
protected:
    BeFileName                      m_inputFileName;    //!< Path to the .bim
    Utf8String                      m_viewName;         //!< Name of the view definition from which to publish
    BeFileName                      m_outputDir;        //!< Directory in which to place the output
    WString                         m_tilesetName;      //!< Root name of the output tileset files
    double                          m_groundHeight = 0.0; //!< Height of ground plane.
    DPoint3d                        m_groundPoint = DPoint3d::FromZero(); //!< Ground point. (if m_groundMode == GroundMode::FixedOrigin
    GroundMode                      m_groundMode = GroundMode::FixedHeight;
    double                          m_tolerance = 0.001;
    uint32_t                        m_depth = 0xffffffff;
    Utf8String                      m_imageryProvider;
    Utf8String                      m_terrainProvider;
    GeoPoint                        m_geoLocation = {-75.686844444444444444444444444444, 40.065702777777777777777777777778, 0.0 };   // Bentley Exton flagpole...
    PublisherContext::TextureMode   m_textureMode = PublisherContext::TextureMode::Embedded;
    bool                            m_geoLocated = false;
    bool                            m_surfacesOnly = false;
    bool                            m_verbose = false;
    bool                            m_overwriteExisting = true;
    bool                            m_wantProgressOutput = true;

    // History (WIP) requires IModel Hub connection.
    bool                            m_wantHistory = false;
    Utf8String                      m_userName;
    Utf8String                      m_password;

    TILEPUBLISHER_EXPORT DgnViewId GetDefaultViewId(DgnDbR db) const;
public:
    PublisherParams () { }
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
    bool WantProgressOutput() const { return m_wantProgressOutput; }
    bool WantHistory() const { return m_wantHistory; }
    GeoPointCP GetGeoLocation() const { return m_geoLocated ? &m_geoLocation : nullptr; }
    bool GetOverwriteExistingOutputFile() const { return m_overwriteExisting; }
    PublisherContext::TextureMode GetTextureMode() const { return m_textureMode; }
    Utf8StringCR GetImageryProvider() const { return m_imageryProvider; }
    Utf8StringCR GetTerrainProvider() const { return m_terrainProvider; }

    TILEPUBLISHER_EXPORT DgnViewId GetViewIds(DgnViewIdSet& viewIds, DgnDbR db);
    TILEPUBLISHER_EXPORT Json::Value GetViewerOptions () const;

    // For History publishing...
    Utf8StringCR GetUser() const { return m_userName; }
    Utf8StringCR GetPassword() const { return m_password; }

};

//=======================================================================================
//! Publishes the contents of a DgnDb view as a Cesium tileset.
// @bsistruct                                                   Paul.Connelly   08/16
//=======================================================================================
struct TilesetPublisher : PublisherContext
{
protected:
    TileGeneratorP              m_generator = nullptr;
    uint32_t                    m_publishedTileDepth;
    BeMutex                     m_mutex;
    DgnViewId                   m_defaultViewId;
    bset<Utf8String>            m_modelsInProgress;
    Utf8String                  m_modelNameList;
    StopWatch                   m_timer;
    Status                      m_acceptTileStatus = Status::Success;
    bool                        m_verbose;
    bool                        m_wantProgressOutput;
    Json::Value                 m_revisionsJson;

    TILEPUBLISHER_EXPORT TileGeneratorStatus _AcceptTile(TileNodeCR tile) override;
    TILEPUBLISHER_EXPORT TileGeneratorStatus _AcceptPublishedTilesetInfo(DgnModelCR, IGetPublishedTilesetInfoR) override;
    TILEPUBLISHER_EXPORT WString _GetTileUrl(TileNodeCR tile, WCharCP fileExtension, PublisherContext::ClassifierInfo const* classifier) const override;
    bool _AllTilesPublished() const override { return true; }

    TILEPUBLISHER_EXPORT TileGeneratorStatus _BeginProcessModel(DgnModelCR) override;
    TILEPUBLISHER_EXPORT TileGeneratorStatus _EndProcessModel(DgnModelCR, TileNodeP, TileGeneratorStatus) override;

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
    TilesetPublisher(DgnDbR db, DgnViewIdSet const& viewIds, DgnViewId defaultViewId, BeFileNameCR outputDir, WStringCR tilesetName, GeoPointCP geoLocation, size_t maxTilesetDepth,  uint32_t publishDepth, bool publishNonSurfaces, bool verbose, TextureMode textureMode, bool wantProgressOutput)
        : PublisherContext(db, viewIds, outputDir, tilesetName, geoLocation, publishNonSurfaces, maxTilesetDepth, textureMode),
          m_publishedTileDepth(publishDepth), m_defaultViewId(defaultViewId), m_verbose(verbose), m_timer(true), m_wantProgressOutput(wantProgressOutput)
        {
        // Put the scripts dir + html files in outputDir. Put the tiles in a subdirectory thereof.
        m_dataDir.AppendSeparator().AppendToPath(L"TileSets").AppendSeparator().AppendToPath(m_rootName.c_str()).AppendSeparator();
        }

    TilesetPublisher(DgnDbR db, PublisherParamsR params, DgnViewIdSet const& viewsToPublish, DgnViewId defaultView, size_t maxTilesetDepth=5)
        : TilesetPublisher(db, viewsToPublish, defaultView, params.GetOutputDirectory(), params.GetTilesetName(), params.GetGeoLocation(), maxTilesetDepth,
            params.GetDepth(), params.SurfacesOnly(), params.WantVerboseStatistics(), params.GetTextureMode(), params.WantProgressOutput()) { }

    TILEPUBLISHER_EXPORT Status Publish(PublisherParams const& params, bool initializeDirectories = true);

    Status GetTileStatus() const { return m_acceptTileStatus; }

    bool WantVerboseStatistics() const { return m_verbose; }
    bool WantProgressOutput() const { return m_wantProgressOutput; }
    void SetRevisionsJson(Json::Value const&& revisionsJson) { m_revisionsJson = std::move(revisionsJson); }


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

//=======================================================================================
// @bsistruct                                                   Ray.Bentley     09/2017
//=======================================================================================
struct TilesetHistoryPublisher : TilesetPublisher
{
    TILEPUBLISHER_EXPORT static Status PublishTilesetWithHistory(PublisherParamsR params);




};  // TilesetHistoryPublisher

} // namespace Cesium

END_BENTLEY_TILEPUBLISHER_NAMESPACE

