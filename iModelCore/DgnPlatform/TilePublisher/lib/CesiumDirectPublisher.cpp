/*--------------------------------------------------------------------------------------+                  
|
|     $Source: TilePublisher/lib/CesiumDirectPublisher.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <TilePublisher/CesiumTilePublisher.h>
#include <BeSqLite/BeSqLite.h>
#include "Constants.h"
#include "CesiumConstants.h"

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_RENDER
USING_NAMESPACE_TILETREE
USING_NAMESPACE_TILEWRITER
USING_NAMESPACE_BENTLEY_TILEPUBLISHER
USING_NAMESPACE_BENTLEY_TILEPUBLISHER_CESIUMDIRECT

/*---------------------------------------------------------------------------------**//**
* @bsimethod    
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> static Json::Value idSetToJson(T const& ids)
    {
    Json::Value json(Json::arrayValue);
    for (auto const& id : ids)
        json.append(id.ToString());
    return json;

    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static Json::Value pointToJson(DPoint3dCR pt)
    {
    Json::Value json(Json::objectValue);
    json["x"] = pt.x;
    json["y"] = pt.y;
    json["z"] = pt.z;
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static Json::Value transformToJson(TransformCR tf)
    {
    auto matrix = DMatrix4d::From(tf);
    Json::Value json(Json::arrayValue);
    for (size_t i=0;i<4; i++)
        for (size_t j=0; j<4; j++)
            json.append (matrix.coff[j][i]);


    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static Json::Value rangeToJson(DRange3dCR range)
    {
    Json::Value json(Json::objectValue);
    json["low"] = pointToJson(range.low);
    json["high"] = pointToJson(range.high);
    return json;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod    
+---------------+---------------+---------------+---------------+---------------+------*/
static void appendPoint(Json::Value& val, DPoint3dCR pt)
    { 
    val.append(pt.x); val.append(pt.y); val.append(pt.z); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void writeBoundingVolume(Json::Value& val, DRange3dCR range)
    {
    BeAssert (!range.IsNull());
    DPoint3d    center = DPoint3d::FromInterpolate (range.low, .5, range.high);
    DVec3d      diagonal = range.DiagonalVector();

    // Range identified by center point and axes
    // Axes are relative to origin of box, not center
    static double      s_minSize = .001;   // Meters...  Don't allow degenerate box.

    auto& volume = val[JSON_BoundingVolume];
    auto& box = volume[JSON_Box];

    appendPoint(box, center);
    appendPoint(box, DPoint3d::FromXYZ (std::max(s_minSize, diagonal.x)/2.0, 0.0, 0.0));
    appendPoint(box, DPoint3d::FromXYZ (0.0, std::max(s_minSize, diagonal.y)/2.0, 0.0));
    appendPoint(box, DPoint3d::FromXYZ (0.0, 0.0, std::max(s_minSize, diagonal.z/2.0)));
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static DPoint3d  cartesianFromRadians (double longitude, double latitude, double height = 0.0)
    {
    DPoint3d    s_wgs84RadiiSquared = DPoint3d::From (6378137.0 * 6378137.0, 6378137.0 * 6378137.0, 6356752.3142451793 * 6356752.3142451793);
    double      cosLatitude = cos(latitude);
    DPoint3d    normal, scratchK;

    normal.x = cosLatitude * cos(longitude);
    normal.y = cosLatitude * sin(longitude);
    normal.z = sin(latitude);

    normal.Normalize();
    scratchK.x = normal.x * s_wgs84RadiiSquared.x;
    scratchK.y = normal.y * s_wgs84RadiiSquared.y;
    scratchK.z = normal.z * s_wgs84RadiiSquared.z;

    double  gamma = sqrt(normal.DotProduct (scratchK));

    DPoint3d    earthPoint = DPoint3d::FromScale(scratchK, 1.0 / gamma);
    DPoint3d    heightDelta = DPoint3d::FromScale (normal, height);

    return DPoint3d::FromSumOf (earthPoint, heightDelta);
    };



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DirectPublisher::DirectPublisher(DgnDbR db, DgnViewIdSet const& viewIds, DgnViewId defaultViewId, BeFileNameCR outputDir, WStringCR tilesetName, GeoPointCP geoLocation, bool verbose, bool wantProgressOutput, double leafTolerance)
    : m_db(db), m_viewIds(viewIds), m_outputDir(outputDir), m_rootName(tilesetName), m_publishAsClassifier(false), m_verbose(verbose), m_wantProgressOutput(wantProgressOutput), m_leafTolerance(leafTolerance)
    {
    // By default, output dir is where we put the json/b3dm files.
    m_outputDir.AppendSeparator();
    m_dataDir = m_outputDir;

    // Put the scripts dir + html files in outputDir. Put the tiles in a subdirectory thereof.
    m_dataDir.AppendSeparator().AppendToPath(m_rootName.c_str()).AppendSeparator();


 
    // ###TODO: Probably want a separate db-to-tile per model...will differ for non-spatial models...
    DPoint3d        origin = db.GeoLocation().GetProjectExtents().GetCenter();
    m_dbToTile = Transform::From (-origin.x, -origin.y, -origin.z);

    DgnGCS*         dgnGCS = db.GeoLocation().GetDgnGCS();
    DPoint3d        ecfOrigin, ecfNorth;

    if (nullptr == dgnGCS)
        {
        double  longitude = -75.686844444444444444444444444444, latitude = 40.065702777777777777777777777778;

        if (nullptr != geoLocation)
            {
            longitude = geoLocation->longitude;
            latitude  = geoLocation->latitude;
            }

        // NB: We have to translate to surface of globe even if we're not using the globe, because
        // Cesium's camera freaks out if it approaches the origin (aka the center of the earth)

        ecfOrigin = cartesianFromRadians (longitude * msGeomConst_radiansPerDegree, latitude * msGeomConst_radiansPerDegree);
        ecfNorth  = cartesianFromRadians (longitude * msGeomConst_radiansPerDegree, 1.0E-4 + latitude * msGeomConst_radiansPerDegree);
        }
    else
        {
        GeoPoint        originLatLong, northLatLong;
        DPoint3d        north = origin;
    
        north.y += 100.0;

        dgnGCS->LatLongFromUors (originLatLong, origin);
        dgnGCS->XYZFromLatLong(ecfOrigin, originLatLong);

        dgnGCS->LatLongFromUors (northLatLong, north);
        dgnGCS->XYZFromLatLong(ecfNorth, northLatLong);
        }

    RotMatrix   rMatrix;
    rMatrix.InitIdentity();

    DVec3d      zVector, yVector;

    zVector.Normalize ((DVec3dCR) ecfOrigin);
    yVector.NormalizedDifference (ecfNorth, ecfOrigin);

    rMatrix.SetColumn (yVector, 1);
    rMatrix.SetColumn (zVector, 2);
    rMatrix.SquareAndNormalizeColumns (rMatrix, 1, 2);

    m_spatialToEcef =  Transform::From (rMatrix, ecfOrigin);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DirectPublisher::AddModelsJson(Json::Value& json, DgnElementIdSet const& allModelSelectors, DgnModelIdSet const& all2dModels)
    {
    DgnModelIdSet allModels = all2dModels;
    Json::Value& selectorsJson = (json["modelSelectors"] = Json::objectValue);
    for (auto const& selectorId : allModelSelectors)
        {
        auto selector = GetDgnDb().Elements().Get<ModelSelector>(selectorId);
        if (selector.IsValid())
            {
            auto models = selector->GetModels();
            selectorsJson[selectorId.ToString()] = idSetToJson(models);
            allModels.insert(models.begin(), models.end());
            }
        }

    // create a fake model selector for each 2d model
    for (auto const& modelId : all2dModels)
        {
        DgnModelIdSet modelIdSet;
        modelIdSet.insert(modelId);
        selectorsJson[modelId.ToString()+"_2d"] = idSetToJson(modelIdSet);
        }

    json["models"] = GetModelsJson(allModels);
    json["classifiers"] = GetAllClassifiersJson();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value DirectPublisher::GetClassifiersJson(ModelSpatialClassifiersCR classifiers)
    {
    Json::Value classifiersValue = Json::arrayValue;

    for (auto& classifier : classifiers)
        {
        Json::Value     classifierValue = classifier.ToJson();

        classifierValue["tilesetUrl"] = GetTilesetName(classifier.GetModelId(), true);

        classifiersValue.append(classifierValue);
        }
    return classifiersValue;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value DirectPublisher::GetAllClassifiersJson()
    {
    Json::Value classifiersValue = Json::objectValue;

    for (auto& curr : m_classifierMap)
        {
        size_t      index = 0;
        for (auto& classifier : curr.second)
            {
            Json::Value     classifierValue = classifier.ToJson();

            classifierValue["tilesetUrl"] = GetTilesetName(classifier.GetModelId(), true);

            Utf8PrintfString    classifierId("%s_%d", classifier.GetModelId().ToString(), index++);
            classifiersValue[classifierId] = classifierValue;
            }
        }
    return classifiersValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DirectPublisher::AddCategoriesJson(Json::Value& json, DgnElementIdSet const& selectorIds)
    {
    DgnCategoryIdSet allCategories;                                                                                                                      
    Json::Value& selectorsJson = (json["categorySelectors"] = Json::objectValue);
    for (auto const& selectorId : selectorIds)
        {
        auto selector = GetDgnDb().Elements().Get<CategorySelector>(selectorId);
        if (selector.IsValid())
            {
            auto cats = selector->GetCategories();
            selectorsJson[selectorId.ToString()] = idSetToJson(cats);
            allCategories.insert(cats.begin(), cats.end());
            }
        }

    json["categories"] = GetCategoriesJson(allCategories);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DirectPublisher::Status DirectPublisher::GetViewsetJson(Json::Value& json, DPoint3dCR groundPoint, DgnViewId defaultViewId)
    {
    Utf8String rootNameUtf8(m_rootName.c_str());
    json["name"] = rootNameUtf8;

    Transform spatialTransform = Transform::FromProduct(m_spatialToEcef, m_dbToTile);

    if (!m_spatialToEcef.IsIdentity())
        {
        DPoint3d groundEcefPoint;
        spatialTransform.Multiply(groundEcefPoint, groundPoint);
        json["groundPoint"] = pointToJson(groundEcefPoint);
        }

    DgnElementIdSet allModelSelectors;
    DgnElementIdSet allCategorySelectors;
    DgnElementIdSet allDisplayStyles;
    DgnModelIdSet   all2dModelIds;

    auto& viewsJson = (json["views"] = Json::objectValue);
    for (auto const& viewId : m_viewIds)
        {
        auto viewDefinition = ViewDefinition::Get(GetDgnDb(), viewId);
        if (!viewDefinition.IsValid())
            continue;

        auto                        spatialView = viewDefinition->ToSpatialView();
        DrawingViewDefinitionCP     drawingView;
        SheetViewDefinitionCP       sheetView;

        if (nullptr != spatialView)
            {
            allModelSelectors.insert(spatialView->GetModelSelectorId());
            }
        else if (nullptr != (drawingView = viewDefinition->ToDrawingView()))    
            {
            all2dModelIds.insert(drawingView->GetBaseModelId());
            }
        else if (nullptr != (sheetView = viewDefinition->ToSheetView()))
            {
            all2dModelIds.insert(sheetView->GetBaseModelId());

            auto const&  model = GetDgnDb().Models().GetModel (sheetView->GetBaseModelId());

            if (model.IsValid() && nullptr != model->ToSheetModel())
                {
                auto   attachedViews = model->ToSheetModel()->GetSheetAttachmentViews(GetDgnDb());
                for (auto& attachedView : attachedViews)
                    {
                    allCategorySelectors.insert(attachedView->GetCategorySelectorId());
                    allDisplayStyles.insert(attachedView->GetDisplayStyleId());
                    if (nullptr != attachedView->ToView2d())
                        all2dModelIds.insert(attachedView->ToView2d()->GetBaseModelId());
                    }
                }
            }

        Json::Value entry(Json::objectValue);
 
        allCategorySelectors.insert(viewDefinition->GetCategorySelectorId());
        allDisplayStyles.insert(viewDefinition->GetDisplayStyleId());

        GetViewJson(entry, *viewDefinition, nullptr != spatialView ? spatialTransform : Transform::FromIdentity());
        viewsJson[viewId.ToString()] = entry;

        // If for some reason the default view is not in the published set, we'll use the first view as the default
        if (!defaultViewId.IsValid())
            defaultViewId = viewId;
        }

    if (!defaultViewId.IsValid())
        return Status::NoGeometry;

    json["defaultView"] = defaultViewId.ToString();

    AddModelsJson(json, allModelSelectors, all2dModelIds);
    AddCategoriesJson(json, allCategorySelectors);
    json["displayStyles"] = GetDisplayStylesJson(allDisplayStyles);

    AxisAlignedBox3d projectExtents = GetDgnDb().GeoLocation().GetProjectExtents();
    spatialTransform.Multiply(projectExtents, projectExtents);
    json["projectExtents"] = rangeToJson(projectExtents);
    json["projectTransform"] = transformToJson(m_spatialToEcef);
    
    return Status::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DirectPublisher::Status DirectPublisher::GetViewsJson (Json::Value& json, DPoint3dCR groundPoint)
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
DirectPublisher::Status DirectPublisher::WriteWebApp (DPoint3dCR groundPoint, Cesium::PublisherParams const& params)
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
void DirectPublisher::OutputStatistics(TileGenerator::Statistics const& stats) const
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
* @bsimethod                                                    Ray.Bentley     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DirectPublisher::Status DirectPublisher::Publish(Cesium::PublisherParams const& params)
    {
    auto status = InitializeDirectories(GetDataDirectory());
    if (Status::Success != status)
        return status;

    DRange3d            range;

    status = PublishViewModels(range);

    if (Status::Success != status)
        {
        CleanDirectories(GetDataDirectory());
        return status;
        }

#ifdef WIP
    OutputStatistics(generator.GetStatistics());
#endif

    DPoint3d        groundPoint;

    if (Cesium::GroundMode::FixedPoint == params.GetGroundMode())
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
* @bsimethod                                                    Paul.Connelly   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value DirectPublisher::GetDisplayStylesJson(DgnElementIdSet const& styleIds)
    {
    Json::Value json(Json::objectValue);
    for (auto const& styleId : styleIds)
        {
        auto style = GetDgnDb().Elements().Get<DisplayStyle>(styleId);
        if (style.IsValid())
            json[styleId.ToString()] = GetDisplayStyleJson(*style);
        }

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value DirectPublisher::GetDisplayStyleJson(DisplayStyleCR style)
    {
    Json::Value json(Json::objectValue);

    ColorDef bgColor = style.GetBackgroundColor();
    auto& bgColorJson = (json["backgroundColor"] = Json::objectValue);
    bgColorJson["red"] = bgColor.GetRed() / 255.0;
    bgColorJson["green"] = bgColor.GetGreen() / 255.0;
    bgColorJson["blue"] = bgColor.GetBlue() / 255.0;

    json["viewFlags"] = style.GetViewFlags().ToJson();

    auto style3d = style.ToDisplayStyle3d();
    if (nullptr != style3d)
        json["isGlobeVisible"] = style3d->IsGroundPlaneEnabled();

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void DirectPublisher::GetViewJson(Json::Value& json, ViewDefinitionCR view, TransformCR transform)
    {
    auto                spatialView = view.ToSpatialView();
    ViewDefinition2dCP  view2d;
    if (nullptr != spatialView)
        {
        auto selectorId = spatialView->GetModelSelectorId().ToString();
        json["modelSelector"] = selectorId;
        }
    else if (nullptr != (view2d = view.ToDrawingView()) ||
             nullptr != (view2d = view.ToSheetView()))
        {
        auto fakeModelSelectorId = view2d->GetBaseModelId().ToString();
        fakeModelSelectorId.append("_2d");
        json["modelSelector"] = fakeModelSelectorId;
        }
    else
        {
        BeAssert(false && "Unexpected view type");
        return;
        }

    json["name"] = view.GetName();
    json["categorySelector"] = view.GetCategorySelectorId().ToString();
    json["displayStyle"] = view.GetDisplayStyleId().ToString();

    DPoint3d viewOrigin = view.GetOrigin();
    transform.Multiply(viewOrigin);
    json["origin"] = pointToJson(viewOrigin);
    
    DVec3d viewExtents = view.GetExtents();
    json["extents"] = pointToJson(viewExtents);

    DVec3d xVec, yVec, zVec;
    view.GetRotation().GetRows(xVec, yVec, zVec);
    transform.MultiplyMatrixOnly(xVec);
    transform.MultiplyMatrixOnly(yVec);
    transform.MultiplyMatrixOnly(zVec);

    RotMatrix columnMajorRotation = RotMatrix::FromColumnVectors(xVec, yVec, zVec);
    auto& rotJson = (json["rotation"] = Json::arrayValue);
    for (size_t i = 0; i < 3; i++)
        for (size_t j = 0; j < 3; j++)
            rotJson.append(columnMajorRotation.form3d[i][j]);

    auto cameraView = nullptr != spatialView ? view.ToView3d() : nullptr;
    if (nullptr != cameraView)
        {
        json["type"] = "camera";
        json["isCameraOn"] = cameraView->IsCameraOn();
        DPoint3d eyePoint = cameraView->GetEyePoint();
        transform.Multiply(eyePoint);
        json["eyePoint"] = pointToJson(eyePoint);

        json["lensAngle"] = cameraView->GetLensAngle().Radians();
        json["focusDistance"] = cameraView->GetFocusDistance();
        }
    else if (nullptr != spatialView)
        {
        json["type"] = "ortho";
        }
    else
        {
        json["type"] = nullptr != view.ToDrawingView() ? "drawing" : "sheet";;
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value DirectPublisher::GetModelsJson (DgnModelIdSet const& modelIds)
    {
    Json::Value     modelsJson (Json::objectValue);
    
    for (auto& modelId : modelIds)
        {
        auto const&  model = GetDgnDb().Models().GetModel (modelId);
        if (model.IsValid())
            {
            auto spatialModel = model->ToSpatialModel();
            auto model2d = nullptr == spatialModel ? dynamic_cast<GraphicalModel2dCP>(model.get()) : nullptr;
            if (nullptr == spatialModel && nullptr == model2d)
                {
                BeAssert(false && "Unsupported model type");
                continue;
                }

            auto modelRangeIter = m_modelRanges.find(modelId);
            if (m_modelRanges.end() == modelRangeIter)
                continue; // this model produced no tiles. ignore it.

            DRange3d modelRange = modelRangeIter->second;
            if (modelRange.IsNull())
                {
                BeAssert(false && "Null model range");
                continue;
                }

            Json::Value modelJson(Json::objectValue);

            auto sheetModel = model->ToSheetModel();
            modelJson["name"] = model->GetName();
            modelJson["type"] = nullptr != spatialModel ? "spatial" : (nullptr != sheetModel ? "sheet" : "drawing");

            if (nullptr != spatialModel)
                {
                m_spatialToEcef.Multiply(modelRange, modelRange);
                modelJson["transform"] = transformToJson(m_spatialToEcef);
                }
            else if (nullptr != sheetModel)
                {
                modelJson["attachedViews"] = GetViewAttachmentsJson(*sheetModel);
                }

            modelJson["extents"] = rangeToJson(modelRange);
            modelJson["tilesetUrl"] = GetTilesetName(modelId, false);

#ifdef PER_MODEL_CLASSIFIER
            // Cesium doesn't support classifying a single model as we do in JSon.
            auto const& foundClassifier = m_classifierMap.find(modelId);
            if (foundClassifier != m_classifierMap.end())
                modelJson["classifiers"] = GetClassifiersJson(foundClassifier->second);
#endif

            modelsJson[modelId.ToString()] = modelJson;
            }
        }

    return modelsJson;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value DirectPublisher::GetViewAttachmentsJson(Sheet::ModelCR sheet)
    {
    bvector<DgnElementId> attachmentIds = sheet.GetSheetAttachmentIds();
    Json::Value attachmentsJson(Json::arrayValue);
    for (DgnElementId attachmentId : attachmentIds)
        {
        auto attachment = GetDgnDb().Elements().Get<Sheet::ViewAttachment>(attachmentId);
        DrawingViewDefinitionCPtr view = attachment.IsValid() ? GetDgnDb().Elements().Get<DrawingViewDefinition>(attachment->GetAttachedViewId()) : nullptr;
        if (view.IsNull())
            continue;

        Json::Value viewJson;
        viewJson["baseModelId"] = view->GetBaseModelId().ToString();
        viewJson["categorySelector"] = view->GetCategorySelectorId().ToString();
        viewJson["displayStyle"] = view->GetDisplayStyleId().ToString();

        DPoint3d            viewOrigin = view->GetOrigin();
        AxisAlignedBox3d    sheetRange = attachment->GetPlacement().CalculateRange();
        double              sheetScale = sheetRange.XLength() / view->GetExtents().x;
        Transform           subtractViewOrigin = Transform::From(DPoint3d::From(-viewOrigin.x, -viewOrigin.y, -viewOrigin.z)),
                            viewRotation = Transform::From(view->GetRotation()),
                            scaleToSheet = Transform::FromScaleFactors (sheetScale, sheetScale, sheetScale),
                            addSheetOrigin = Transform::From(DPoint3d::From(sheetRange.low.x, sheetRange.low.y, attachment->GetDisplayPriority()/500.0)),
                            tileToSheet = Transform::FromProduct(Transform::FromProduct(addSheetOrigin, scaleToSheet), Transform::FromProduct(viewRotation, subtractViewOrigin)),
                            sheetToTile;

        sheetToTile.InverseOf(tileToSheet);
        viewJson["transform"] = transformToJson(tileToSheet);

        attachmentsJson.append(std::move(viewJson));
        }

    return attachmentsJson;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value DirectPublisher::GetCategoriesJson (DgnCategoryIdSet const& categoryIds)
    {
    Json::Value categoryJson (Json::objectValue); 
    
    for (auto& categoryId : categoryIds)
        {
        auto const& category = DgnCategory::Get(GetDgnDb(), categoryId);

        if (category.IsValid())
            categoryJson[categoryId.ToString()] = category->GetCategoryName();
        }

    return categoryJson;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String  DirectPublisher::GetTilesetName(DgnModelId modelId, bool asClassifier)
    {
    WString         modelRootName = TileUtil::GetRootNameForModel(modelId, asClassifier);
    BeFileName      tilesetFileName (nullptr, m_rootName.c_str(), modelRootName.c_str(), s_metadataExtension);
    auto            utf8FileName = tilesetFileName.GetNameUtf8();

    utf8FileName.ReplaceAll("\\", "//");
    return utf8FileName;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void DirectPublisher::AddViewedModel(DgnModelIdSet& viewedModels, DgnModelId modelId)
    {
    viewedModels.insert(modelId);

    // Scan for viewAttachments...
    auto stmt = GetDgnDb().GetPreparedECSqlStatement("SELECT ECInstanceId FROM " BIS_SCHEMA(BIS_CLASS_ViewAttachment) " WHERE Model.Id=?");
    stmt->BindId(1, modelId);

    while (BE_SQLITE_ROW == stmt->Step())
        {
        auto attachId = stmt->GetValueId<DgnElementId>(0);
        auto attach   = GetDgnDb().Elements().Get<Sheet::ViewAttachment>(attachId);

        if (!attach.IsValid())
            {
            BeAssert(false);
            continue;
            }

        GetViewedModelsFromView (viewedModels, attach->GetAttachedViewId());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void    DirectPublisher::GetViewedModelsFromView (DgnModelIdSet& viewedModels, DgnViewId viewId)
    {
    SpatialViewDefinitionPtr spatialView = nullptr;
    auto view2d = GetDgnDb().Elements().Get<ViewDefinition2d>(viewId);
    if (view2d.IsValid())
        {
        AddViewedModel (viewedModels, view2d->GetBaseModelId()); 
        }
    else if ((spatialView = GetDgnDb().Elements().GetForEdit<SpatialViewDefinition>(viewId)).IsValid())
        {
        for (auto& modelId : spatialView->GetModelSelector().GetModels())
            AddViewedModel (viewedModels, modelId);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DirectPublisher::Status   DirectPublisher::PublishViewModels (DRange3dR range)
    {
    DgnModelIdSet viewedModels;

    for (auto const& viewId : m_viewIds)
        GetViewedModelsFromView (viewedModels, viewId);

    static size_t           s_maxPointsPerTile = 250000;
    auto status = GenerateTilesFromModels(viewedModels);
    if (Status::Success != status.value())
        return status.value();

#if defined(TODO_TILE_PUBLISH)
    for (auto& modelId : viewedModels)
        {
        auto                        getTileTree = dynamic_cast<IGetTileTreeForPublishing*>(GetDgnDb().Models().GetModel(modelId).get());
        ModelSpatialClassifiers     classifiers;

        if (nullptr != getTileTree && 
            SUCCESS == getTileTree->_GetSpatialClassifiers(classifiers))
            {
            for (auto& classifier : classifiers)
                classifierModels.insert(classifier.GetModelId());

            m_classifierMap.Insert(modelId, classifiers);
            }
        }


    if (!classifierModels.empty())
        {
        AutoRestore<bool> savePublishAsVectors (&m_publishAsClassifier, true);

        auto status = generator.GenerateTiles(*this, classifierModels, toleranceInMeters, surfacesOnly, s_maxPointsPerTile);
        if (TileGeneratorStatus::Success != status)
            return ConvertStatus(status);
        }
#endif
    return Status::Success;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void DirectPublisher::WriteModelMetadataTree (DRange3dR range, Json::Value& root, PublishedTileCR tile, GeometricModelCR model)
    {
    if (tile.GetIsEmpty() && tile.GetChildren().empty())
        {
        range = DRange3d::NullRange();
        return;
        }

    WString         rootName = TileUtil::GetRootNameForModel(model.GetModelId(), m_publishAsClassifier);
    DRange3d        contentRange, publishedRange = tile.GetPublishedRange();

    
    // the published range represents the actual range of the published meshes. - This may be smaller than the 
    // range estimated when we built the tile tree. -- However we do not clip the meshes to the tile range.
    // so start the range out as the intersection of the tile range and the published range.
    range = contentRange = DRange3d::FromIntersection (tile.GetTileRange(), publishedRange, true);

    if (!tile.GetChildren().empty())
        {
        root[JSON_Children] = Json::arrayValue;

        // Append children to this tileset.
        for (auto& childTile : tile.GetChildren())
            {
            Json::Value         child;
            DRange3d            childRange;

            WriteModelMetadataTree (childRange, child, *childTile, model);
            if (!childRange.IsNull())
                {
                root[JSON_Children].append(child);
                range.Extend (childRange);
                }
            }
        }
    if (range.IsNull())
        return;

    root["refine"] = "REPLACE";
    root[JSON_GeometricError] = tile.GetTolerance();
    TilePublisher::WriteBoundingVolume(root, range);

    if (!contentRange.IsNull() && !tile.GetIsEmpty())
        {
        root[JSON_Content]["url"] = GetTileUrl(tile, model).c_str();
        TilePublisher::WriteBoundingVolume (root[JSON_Content], contentRange);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void DirectPublisher::WriteModelTileset(GeometricModelCR model, PublishedTileCR rootTile)
    {
    BeFileName      fileName = GetTilesetFileName(model.GetModelId());

    Json::Value val, modelRoot;

    val["asset"]["version"] = "0.0";
    val["asset"]["gltfUpAxis"] = "Z";
 
    DRange3d    rootRange;
    WriteModelMetadataTree (rootRange, modelRoot, rootTile, model);

    val[JSON_Root] = std::move(modelRoot);

    if (model.IsSpatialModel())
        val[JSON_Root][JSON_Transform] = transformToJson(m_spatialToEcef);

    TileUtil::WriteJsonToFile (fileName.c_str(), val);
    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DirectPublisher::Status DirectPublisher::InitializeDirectories(BeFileNameCR dataDir)
    {
    // Ensure directories exist and are writable
    if (m_outputDir != dataDir && BeFileNameStatus::Success != BeFileName::CheckAccess(m_outputDir, BeFileNameAccess::Write))
        return Status::CantWriteToBaseDirectory;

    bool dataDirExists = BeFileName::DoesPathExist(dataDir);
    if (dataDirExists && BeFileNameStatus::Success != BeFileName::EmptyDirectory(dataDir.c_str()))
        return Status::CantCreateSubDirectory;
    else if (!dataDirExists && BeFileNameStatus::Success != BeFileName::CreateNewDirectory(dataDir))
        return Status::CantCreateSubDirectory;

    if (BeFileNameStatus::Success != BeFileName::CheckAccess(dataDir, BeFileNameAccess::Write))
        return Status::CantCreateSubDirectory;

    return Status::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DirectPublisher::CleanDirectories(BeFileNameCR dataDir)
    {
    BeFileName::EmptyAndRemoveDirectory (dataDir);
    }
 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DirectPublisher::Status DirectPublisher::ConvertStatus(TileIO::WriteStatus status)
    {
    switch (status)
        {
        case TileIO::WriteStatus::Success:           return Status::Success;
        case TileIO::WriteStatus::NoGeometry:        return Status::NoGeometry;
        case TileIO::WriteStatus::UnableToOpenFile:  return Status::CantOpenOutputFile;
        case TileIO::WriteStatus::Aborted:           return Status::Aborted;

        default:   
            BeAssert(false);
            return Status::Aborted;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName DirectPublisher::GetDataDirForModel(DgnModelCR model, WStringP pTilesetName) const
    {
    WString tmpTilesetName;
    WStringR tilesetName = nullptr != pTilesetName ? *pTilesetName : tmpTilesetName;

    tilesetName = TileUtil::GetRootNameForModel(model.GetModelId(), m_publishAsClassifier);

    BeFileName dataDir = m_dataDir;
    dataDir.AppendToPath(tilesetName.c_str());                                                                                

    return dataDir;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool DirectPublisher::IsGeolocated () const
    {
    return nullptr != GetDgnDb().GeoLocation().GetDgnGCS();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName DirectPublisher::GetTilesetFileName(DgnModelId modelId)
    {
    return BeFileName(nullptr, m_dataDir.c_str(), TileUtil::GetRootNameForModel(modelId, m_publishAsClassifier).c_str(), s_metadataExtension);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
WString DirectPublisher::GetTileUrl(PublishedTileCR tile, GeometricModelCR model) const
    {
    WString     modelRootName = TileUtil::GetRootNameForModel(model.GetModelId(), false);
    BeFileName  nameAndExtensionOnly(BeFileName::NameAndExt, tile.GetFileName().c_str());

    return modelRootName + L"//" + nameAndExtensionOnly.c_str(); 
    }

//=======================================================================================
//! Generates the contents of a single model to a Cesium tileset
// @bsistruct                                                   Ray.Bentley     08/2017
//=======================================================================================
struct TilesetGenerator : ICesiumPublisher
{

protected:
    BeMutex                 m_mutex;
    DirectPublisher&        m_publisher;

    TilesetGenerator(DirectPublisher& publisher) : m_publisher(publisher) { }

    BeFileName  _GetOutputDirectory(GeometricModelCR model) const override { return m_publisher.GetDataDirForModel(model); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileIO::WriteStatus _BeginProcessModel(GeometricModelCR model)
    {
    if (DirectPublisher::Status::Success != m_publisher.InitializeDirectories(m_publisher.GetDataDirForModel(model)))
        return TileIO::WriteStatus::UnableToOpenFile;

    return TileIO::WriteStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TileIO::WriteStatus _EndProcessModel(GeometricModelCR model, PublishedTileCR rootTile, TileIO::WriteStatus status)
    {
    if (TileIO::WriteStatus::Success == status)
        m_publisher.WriteModelTileset(model, rootTile);
    else
        m_publisher.CleanDirectories(m_publisher.GetDataDirForModel(model));

    return status;
    }
public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static DirectPublisher::FutureStatus GenerateTilesFromModel(DirectPublisher* publisher, GeometricModelCP model)
    {
    Transform           transformFromDgn = Transform::FromIdentity();    // TBD.
    TilesetGenerator    tilesetGenerator(*publisher);

    return folly::makeFuture(DirectPublisher::ConvertStatus(ICesiumPublisher::WriteCesiumTileset(tilesetGenerator, *model, transformFromDgn, publisher->GetLeafTolerance())));
    }
};  // TilesetGenerator.

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DirectPublisher::FutureStatus DirectPublisher::GenerateTilesFromModels(DgnModelIdSet const& modelIds)
    {
    std::vector<FutureStatus> modelFutures;
    for (auto const& modelId : modelIds)
        {
        auto model = GetDgnDb().Models().GetModel(modelId);
        if (model.IsValid() && nullptr != model->ToGeometricModel())
            modelFutures.push_back(TilesetGenerator::GenerateTilesFromModel(this, model->ToGeometricModel()));
        }

    return folly::unorderedReduce(modelFutures, DirectPublisher::Status::Success, [=](DirectPublisher::Status reduced, DirectPublisher::Status next)
        {
        return DirectPublisher::Status::Aborted == reduced || DirectPublisher::Status::Aborted == next ? DirectPublisher::Status::Aborted : DirectPublisher::Status::Success;
        });
    }


