/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/TilePublisher/CesiumTilePublisher.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <TilePublisher/TilePublisher.h>
#include <TilePublisher/CesiumPublisher.h>
#include <DgnPlatform/TileWriter.h>

#define USING_NAMESPACE_BENTLEY_TILEPUBLISHER_CESIUMDIRECT using namespace BentleyApi::Dgn::TilePublish::CesiumDirect;


BEGIN_BENTLEY_TILEPUBLISHER_NAMESPACE


namespace CesiumDirect
{

//=======================================================================================
//! Publishes the contents of a set of models viewed by a DgnDb view to a Json file and
//!  a series of Cesium tilesets suitable for viewing by the Bimium client. 
//! rendering tile tree.
// @bsistruct                                                   Ray.Bentley     08/2017
//=======================================================================================
struct DirectPublisher 
{

    enum class Status
        {
        Success = SUCCESS,
        NoGeometry,
        Aborted,
        CantWriteToBaseDirectory,
        CantCreateSubDirectory,
        ErrorWritingScene,
        ErrorWritingNode,
        CantOpenOutputFile,
        };

    typedef folly::Future<Status> FutureStatus;


protected:
    BeMutex                                     m_mutex;
    DgnViewId                                   m_defaultViewId;
    bset<Utf8String>                            m_modelsInProgress;
    Utf8String                                  m_modelNameList;
    StopWatch                                   m_timer;
    bool                                        m_verbose;
    bool                                        m_wantProgressOutput;
    bmap<DgnModelId, DRange3d>                  m_modelRanges;
    bmap<DgnModelId, ModelSpatialClassifiers>   m_classifierMap;
    double                                      m_leafTolerance;
    DgnDbR                                      m_db;
    BeFileName                                  m_outputDir;
    BeFileName                                  m_dataDir;
    DgnViewIdSet                                m_viewIds;
    WString                                     m_rootName;
    Transform                                   m_dbToTile;
    Transform                                   m_spatialToEcef;
    bool                                        m_publishAsClassifier;


    template<typename T> Json::Value GetIdsJson(Utf8CP tableName, T const& ids);

    void            AddModelsJson(Json::Value&, DgnElementIdSet const& allModelSelectors, DgnModelIdSet const& all2dModels);
    void            AddCategoriesJson(Json::Value&, DgnElementIdSet const& allCategorySelectors);
    Status          GetViewsJson (Json::Value& value, DPoint3dCR groundPoint);
    Json::Value     GetViewAttachmentsJson(Sheet::ModelCR sheet);
    Json::Value     GetDisplayStylesJson(DgnElementIdSet const& styleIds);
    Json::Value     GetDisplayStyleJson(DisplayStyleCR style);
    Json::Value     GetClassifiersJson(ModelSpatialClassifiersCR classifiers);
    Json::Value     GetAllClassifiersJson();                                                                                                                                
    void            AddViewedModel(DgnModelIdSet& viewedModels, DgnModelId modelId);
    void            GetViewedModelsFromView (DgnModelIdSet& viewedModels, DgnViewId viewId);
    Status          GetViewsetJson(Json::Value& json, DPoint3dCR groundPoint, DgnViewId defaultViewId);
    void            GetViewJson (Json::Value& json, ViewDefinitionCR view, TransformCR transform);
    Json::Value     GetModelsJson (DgnModelIdSet const& modelIds);
    Json::Value     GetCategoriesJson(DgnCategoryIdSet const& categoryIds);
    BeFileName      GetTilesetFileName(DgnModelId modelId);
    Utf8String      GetTilesetName(DgnModelId modelId, bool asClassifier);
    Status          PublishViewModels (DRange3dR range);
    void            WriteModelMetadataTree (DRange3dR range, Json::Value& root, TileTree::TileWriter::PublishedTileCR tile, GeometricModelCR model);
    Status          WriteWebApp(DPoint3dCR groundPoint, TilePublish::Cesium::PublisherParams const& params);
    void            OutputStatistics(TileGenerator::Statistics const& stats) const;
    FutureStatus    GenerateTilesFromModels(DgnModelIdSet const& modelIds);
    Utf8String      GetTileUrl(TileTree::TileWriter::PublishedTileCR tile, GeometricModelCR model) const;
    DgnDbR          GetDgnDb() const { return m_db; }
    bool            IsGeolocated () const;

public:
    DirectPublisher(DgnDbR db, Cesium::PublisherParamsR params, DgnViewIdSet const& viewsToPublish, DgnViewId defaultView) :                                                          
        DirectPublisher(db, viewsToPublish, defaultView, params.GetOutputDirectory(), params.GetTilesetName(), params.GetGeoLocation(), params.WantVerboseStatistics(), params.WantProgressOutput(), params.GetTolerance()) { }

    TILEPUBLISHER_EXPORT DirectPublisher(DgnDbR db, DgnViewIdSet const& viewIds, DgnViewId defaultViewId, BeFileNameCR outputDir, WStringCR tilesetName, GeoPointCP geoLocation, bool verbose, bool wantProgressOutput, double leafTolerance);
    TILEPUBLISHER_EXPORT Status Publish(Cesium::PublisherParams const& params);

    bool            WantVerboseStatistics() const { return m_verbose; }
    bool            WantProgressOutput() const { return m_wantProgressOutput; }
    BeFileName      GetDataDirForModel(DgnModelCR model, WStringP rootName=nullptr) const;
    Status          InitializeDirectories(BeFileNameCR dataDir);
    void            CleanDirectories(BeFileNameCR dataDir);
    double          GetLeafTolerance() const { return m_leafTolerance; }
    void            WriteModelTileset(GeometricModelCR model, TileTree::TileWriter::PublishedTileCR tile);
    BeFileNameCR    GetDataDirectory() const { return m_dataDir; }
    WStringCR       GetRootName() const { return m_rootName; }
    BeFileNameCR    GetOutputDirectory() const { return m_outputDir; }
    static Status   ConvertStatus(TileTree::TileIO::WriteStatus status);
    void            AddModelRange(DgnModelId modelId, DRange3dCR modelRange);


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


};  // DirectPublisher


} // namespace CesiumDirect

END_BENTLEY_TILEPUBLISHER_NAMESPACE




                                                                               
























                                            
























