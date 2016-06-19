/*-------------------------------------------------------------------------------------+
|
|     $Source: PointCloudSchema/PointCloudHandler.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <PointCloudSchemaInternal.h>
#include <PointCloudSchema/PointCloudHandler.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_POINTCLOUDSCHEMA
USING_NAMESPACE_BENTLEY_BEPOINTCLOUD

HANDLER_DEFINE_MEMBERS(PointCloudModelHandler)

static Utf8CP JSON_PointCloudModel = "PointCloudModel";
static Utf8CP PROPERTYJSON_FileId = "FileId";
static Utf8CP PROPERTYJSON_SceneToWorld = "SceneToWorld";
static Utf8CP PROPERTYJSON_Description = "Description";
static Utf8CP PROPERTYJSON_Wkt = "Wkt";
static Utf8CP PROPERTYJSON_Density = "Density";
static Utf8CP PROPERTYJSON_Color = "Color";
static Utf8CP PROPERTYJSON_Weight = "Weight";

//----------------------------------------------------------------------------------------
//                                  PointCloudModelHandler
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
PointCloudModelPtr PointCloudModelHandler::CreatePointCloudModel(PointCloudModel::CreateParams const& params)
    {
    // Find resolved file name for the point cloud
    BeFileName fileName;
    BentleyStatus status = T_HOST.GetPointCloudAdmin()._ResolveFileName(fileName, params.m_fileId, params.m_dgndb);
    if (status != SUCCESS)
        {
        ERROR_PRINTF("Failed to resolve filename = %s", fileName.GetNameUtf8().c_str());
        return nullptr;
        }
    Utf8String resolvedName(fileName);
    Utf8String modelName(fileName.GetFileNameWithoutExtension().c_str());

    // Try to open point cloud file
    PointCloudScenePtr pPointCloudScene = PointCloudScene::Create(fileName.c_str());
    if (pPointCloudScene.IsNull())
        {
        ERROR_PRINTF("Failed to create PointCloudScene = %s", fileName.GetNameUtf8().c_str());
        // Can't create model; probably that file name is invalid.
        return nullptr;
        }

    WString wkt = pPointCloudScene->GetSurveyGeoreferenceMetaTag();

    PointCloudModel::Properties props;
    props.m_fileId = params.m_fileId;
    props.m_wkt.Assign(wkt.c_str());
    
    DRange3d sceneRange;
    pPointCloudScene->GetRange(sceneRange);

    // Compute sceneToWorld including units change and reprojection.
    if (SUCCESS != PointCloudGcsFacility::ComputeSceneToWorldTransform(props.m_sceneToWorld, wkt, sceneRange, params.m_dgndb))
        props.m_sceneToWorld.InitIdentity();
        
    // Create model in DgnDb
    PointCloudModelPtr model = new PointCloudModel(params, props);

    return model;
    }

//----------------------------------------------------------------------------------------
//                                  PointCloudModel
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// @bsimethod                                                   
//----------------------------------------------------------------------------------------
Utf8StringCR PointCloudModel::GetSpatialReferenceWkt() const   { return m_properties.m_wkt; }
void PointCloudModel::SetSpatialReferenceWkt(Utf8CP wktString) { m_properties.m_wkt=wktString; }

Utf8StringCR PointCloudModel::GetDescription() const     { return m_properties.m_description; }
void PointCloudModel::SetDescription(Utf8CP description) { m_properties.m_description=description; }

TransformCR PointCloudModel::GetSceneToWorld() const    { return m_properties.m_sceneToWorld; }
void PointCloudModel::SetSceneToWorld(TransformCR trans){ m_properties.m_sceneToWorld = trans;}

float PointCloudModel::GetViewDensity() const       { return m_properties.m_density; }
void PointCloudModel::SetViewDensity(float density) { BeAssert(IN_RANGE(density, 0.0f, 1.0f)); m_properties.m_density = BOUND(density, 0.0f, 1.0f);}

ColorDef PointCloudModel::GetColor() const              {return m_properties.m_color;}
void PointCloudModel::SetColor(ColorDef const& newColor){m_properties.m_color = newColor;}

uint32_t PointCloudModel::GetWeight() const                { return m_properties.m_weight; }
void PointCloudModel::SetWeight(uint32_t const& newWeight) { m_properties.m_weight = newWeight; }


//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
PointCloudModel::PointCloudModel(CreateParams const& params) : T_Super (params)
    {
    m_loadSceneStatus = LoadStatus::Unloaded;
    m_pointCloudScenePtr = nullptr;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
PointCloudModel::PointCloudModel(CreateParams const& params, PointCloudModel::Properties const& properties) 
:T_Super (params),
 m_properties(properties)
    {
    m_loadSceneStatus = LoadStatus::Unloaded;
    m_pointCloudScenePtr = nullptr;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
PointCloudModel::~PointCloudModel()
    {
    m_viewportCache.clear();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2016
//----------------------------------------------------------------------------------------
Dgn::Render::Graphic* PointCloudModel::GetLowDensityGraphicP(DgnViewportCR vp) const
    {
    auto viewportItr = m_viewportCache.find(&vp);
    if (viewportItr != m_viewportCache.end())
        return viewportItr->second.m_lowDensityGraphic.get();

    return nullptr;
    }


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2016
//----------------------------------------------------------------------------------------
void PointCloudModel::SaveLowDensityGraphic(DgnViewportCR vp, Dgn::Render::Graphic* pGraphic)
    {
    // Insert or replace in cache.
    m_viewportCache[&vp].m_lowDensityGraphic = pGraphic;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2016
//----------------------------------------------------------------------------------------
PtViewport* PointCloudModel::GetPtViewportP(DgnViewportCR vp) const
    {
    auto viewportItr = m_viewportCache.find(&vp);
    if (viewportItr != m_viewportCache.end())
        return viewportItr->second.m_ptViewport.get();
    
    ViewportCacheEntry entry;
    entry.m_ptViewport = PtViewport::Create();
    if (entry.m_ptViewport.IsNull())
        return nullptr;    

    m_viewportCache.insert(std::make_pair(&vp, entry));
    return entry.m_ptViewport.get();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2016
//----------------------------------------------------------------------------------------
void PointCloudModel::_DropGraphicsForViewport(Dgn::DgnViewportCR viewport)
    {
    m_viewportCache.erase(&viewport);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
PointCloudSceneP PointCloudModel::GetPointCloudSceneP() const
    {
    if (LoadStatus::Unloaded == m_loadSceneStatus && m_pointCloudScenePtr == nullptr)
        {
        m_loadSceneStatus = LoadStatus::UnknownError;

        // Find resolved file name for the point cloud
        BeFileName fileName;
        BentleyStatus status = T_HOST.GetPointCloudAdmin()._ResolveFileName(fileName, m_properties.m_fileId, GetDgnDb());
        if (status != SUCCESS)
            {
            ERROR_PRINTF("Failed to resolve filename = %s", fileName.GetNameUtf8().c_str());
            return nullptr;
            }
            
        m_pointCloudScenePtr = PointCloudScene::Create(fileName.c_str());

        if (m_pointCloudScenePtr.IsValid())
            {
            m_loadSceneStatus = PointCloudModel::LoadStatus::Loaded;
            DEBUG_PRINTF("PointCloudScene loaded file = %s", fileName.GetNameUtf8().c_str());
            }
        }

    return m_pointCloudScenePtr.get();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2016
//----------------------------------------------------------------------------------------
void PointCloudModel::_OnFitView(Dgn::FitContextR context)
    {
    DRange3d rangeWorld;
    if (!GetRange(rangeWorld, Unit::Scene))
        return;
      
    ElementAlignedBox3d box;
    box.InitFrom(rangeWorld.low, rangeWorld.high);
    context.ExtendFitRange(box, GetSceneToWorld());
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     5/2015
//----------------------------------------------------------------------------------------
bool PointCloudModel::GetRange(DRange3dR range, PointCloudModel::Unit const& unit) const
    {
    BePointCloud::PointCloudSceneP pScene = GetPointCloudSceneP();
    if (nullptr == pScene)
        return false;

    pScene->GetRange(range);
    if (PointCloudModel::Unit::World == unit)
        GetSceneToWorld().Multiply(range, range);

    return true;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
AxisAlignedBox3d PointCloudModel::_QueryModelRange() const
    {
    DRange3d rangeWorld;
    if (!GetRange(rangeWorld, Unit::World))
        return AxisAlignedBox3d();

    return AxisAlignedBox3d(rangeWorld);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void PointCloudModel::_AddSceneGraphics(Dgn::SceneContextR context) const
    {
    if (GetPointCloudSceneP() == nullptr || NULL == context.GetViewport() ||
        !PointCloudProgressiveDisplay::ShouldDrawInContext(context))
        return;

    PtViewport* ptViewport = GetPtViewportP(context.GetViewportR());
    if (nullptr == ptViewport)
        {
        ERROR_PRINTF("Error: no more pt viewport");
        return;     // We ran out of viewport.
        }

    RefCountedPtr<PointCloudProgressiveDisplay> display = new PointCloudProgressiveDisplay(*this, *ptViewport);
    display->DrawView(context);
    }


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2016
//----------------------------------------------------------------------------------------
PointCloudModel::Properties::Properties()
    {
    m_sceneToWorld.InitIdentity();
    m_density = 1.0;
    m_color = ColorDef::White();
    m_weight = 0;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void PointCloudModel::Properties::ToJson(Json::Value& v) const
    {
    v[PROPERTYJSON_FileId] = m_fileId.c_str();

    if (!m_description.empty())
        v[PROPERTYJSON_Description] = m_description.c_str();

    JsonUtils::TransformToJson(v[PROPERTYJSON_SceneToWorld], m_sceneToWorld);

    if(!m_wkt.empty())
        v[PROPERTYJSON_Wkt] = m_wkt.c_str();

    v[PROPERTYJSON_Density] = m_density;
    
    v[PROPERTYJSON_Color] = m_color.GetValue();
    v[PROPERTYJSON_Weight] = m_weight;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void PointCloudModel::Properties::FromJson(Json::Value const& v)
    {
    m_fileId = v[PROPERTYJSON_FileId].asString();
    m_description = v[PROPERTYJSON_Description].asString();

    JsonUtils::TransformFromJson(m_sceneToWorld, v[PROPERTYJSON_SceneToWorld]);
    m_wkt = v[PROPERTYJSON_Wkt].asString();
    m_density = v[PROPERTYJSON_Density].asFloat();

    m_color = ColorDef(v[PROPERTYJSON_Color].asUInt());
    m_weight = v[PROPERTYJSON_Weight].asUInt();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void PointCloudModel::_WriteJsonProperties(Json::Value& val) const
    {
    T_Super::_WriteJsonProperties(val);
    m_properties.ToJson(val[JSON_PointCloudModel]);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void PointCloudModel::_ReadJsonProperties(Json::Value const& val)
    {
    BeAssert(val.isMember(JSON_PointCloudModel));
    T_Super::_ReadJsonProperties(val);
    m_properties.FromJson(val[JSON_PointCloudModel]);
    }

