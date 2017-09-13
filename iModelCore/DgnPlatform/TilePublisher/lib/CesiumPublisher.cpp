/*--------------------------------------------------------------------------------------+                  
|
|     $Source: TilePublisher/lib/CesiumPublisher.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <TilePublisher/CesiumPublisher.h>
#include "CesiumConstants.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_RENDER
USING_NAMESPACE_BENTLEY_TILEPUBLISHER
USING_NAMESPACE_BENTLEY_TILEPUBLISHER_CESIUM

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewId PublisherParams::GetDefaultViewId(DgnDbR db) const
    {
    if (!m_viewName.empty())
        return ViewDefinition::QueryViewId(db.GetDictionaryModel(), m_viewName);

    // Try default view
    DgnViewId viewId;
    if (BeSQLite::BE_SQLITE_ROW == db.QueryProperty(&viewId, sizeof(viewId), DgnViewProperty::DefaultView()) && viewId.IsValid())
        return viewId;

    // Try first spatial view
    for (auto const& entry : ViewDefinition::MakeIterator(db))
        {
        auto view = ViewDefinition::Get(db, entry.GetId());
        if (view.IsValid() && !view->IsPrivate() && (view->IsSpatialView() || view->IsDrawingView() || view->IsSheetView()))
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
        if (view.IsValid() &&!view->IsPrivate() && (view->IsSpatialView() || view->IsDrawingView() || view->IsSheetView()))
            viewIds.insert(entry.GetId());
        }

    return defaultViewId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value  PublisherParams::GetViewerOptions () const
    {
    Json::Value viewerOptions(Json::objectValue);

    if (!m_imageryProvider.empty())
        viewerOptions["imageryProvider"] = m_imageryProvider.c_str();

    if (!m_terrainProvider.empty())
        viewerOptions["terrainProvider"] = m_terrainProvider.c_str();

    return viewerOptions;
    }

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
* @bsimethod                                                    Paul.Connelly   08/17
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeneratorStatus TilesetPublisher::_AcceptPublishedTilesetInfo(DgnModelCR model, IGetPublishedTilesetInfoR getUrl)
    {
    PublishedTilesetInfo info = getUrl._GetPublishedTilesetInfo();
    if (!info.IsValid())
        return TileGeneratorStatus::NoGeometry;

    BeMutexHolder lock(m_mutex);
    m_directUrls.Insert(model.GetModelId(), info.m_url);
    m_modelRanges.Insert(model.GetModelId(), ModelRange(info.m_ecefRange, true));

    return TileGeneratorStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PublisherContext::Status TilesetPublisher::GetViewsJson (Json::Value& json, DPoint3dCR groundPoint)
    {
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
    if (!IsGeolocated() && viewerOptions["imageryProvider"].isNull())
        viewerOptions["imageryProvider"] = "NaturalEarth";

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

    Utf8String jsonFileUrl = "TileSets/"  + Utf8String (m_rootName) + "/" + Utf8String(jsonRootName.c_str());
    jsonFileUrl.append(".json");
    std::fwrite(s_viewerHtmlPrefix, 1, sizeof(s_viewerHtmlPrefix)-1, htmlFile);
    std::fwrite(jsonFileUrl.c_str(), 1, jsonFileUrl.size(), htmlFile);
    std::fwrite(s_viewerHtmlSuffix, 1, sizeof(s_viewerHtmlSuffix)-1, htmlFile);
    std::fclose(htmlFile);

    // Symlink the scripts, if not already present
    BeFileName scriptsSrcDir(T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
    scriptsSrcDir.AppendToPath(L"scripts");
    BeFileName scriptsDstDir(m_outputDir);
    scriptsDstDir.AppendToPath(L"PublishedScripts");
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
    if (!m_publisher.WantProgressOutput())
        return;

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
    TileGenerator generator (GetDgnDb(), nullptr, &progressMeter);

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
        groundPoint.SumOf (params.GetGroundPoint(), GetDgnDb().GeoLocation().GetGlobalOrigin());
        }
    else
        {
        if (range.IsNull())
            {
            groundPoint.x = groundPoint.y = 0.0;
            }
        else
            {
            Transform   tileToDb;

            tileToDb.InverseOf (m_dbToTile);
            
            groundPoint = DPoint3d::FromInterpolate (range.low, .5, range.high);
            tileToDb.Multiply (groundPoint);
            }

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
* @bsimethod                                                    Ray.Bentley     04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
WString TilesetPublisher::_GetTileUrl(TileNodeCR tile, WCharCP fileExtension, PublisherContext::ClassifierInfo const* classifier) const
    {
    WString     modelRootName = nullptr == classifier ? TileUtil::GetRootNameForModel(tile.GetModel().GetModelId(), false) : classifier->GetRootName();

    return modelRootName + L"//" + tile.GetFileName(modelRootName.c_str(), fileExtension); 
    }


