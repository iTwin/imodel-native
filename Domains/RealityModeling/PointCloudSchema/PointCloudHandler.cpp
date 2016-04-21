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

//----------------------------------------------------------------------------------------
//                                  PointCloudModel::JsonUtils
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void PointCloudModel::JsonUtils::DPoint3dToJson(JsonValueR outValue, DPoint3dCR point)
    {
    outValue[0] = point.x;
    outValue[1] = point.y;
    outValue[2] = point.z;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void PointCloudModel::JsonUtils::DPoint3dFromJson(DPoint3dR point, Json::Value const& inValue)
    {
    point.x = inValue[0].asDouble();
    point.y = inValue[1].asDouble();
    point.z = inValue[2].asDouble();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson     02/14
//---------------------------------------------------------------------------------------
void PointCloudModel::JsonUtils::TransformRowFromJson(double* row, JsonValueCR inValue)
    {
    for (int y = 0; y < 4; ++y)
        row[y] = inValue[y].asDouble();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson     02/14
//---------------------------------------------------------------------------------------
void PointCloudModel::JsonUtils::TransformRowToJson(JsonValueR outValue, double const* row)
    {
    for (int y = 0; y < 4; ++y)
        outValue[y] = row[y];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson     02/14
//---------------------------------------------------------------------------------------
void PointCloudModel::JsonUtils::TransformFromJson(TransformR trans, JsonValueCR inValue)
    {
    for (int x = 0; x < 3; ++x)
        TransformRowFromJson(trans.form3d[x], inValue[x]);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson     02/14
//---------------------------------------------------------------------------------------
void PointCloudModel::JsonUtils::TransformToJson(JsonValueR outValue, TransformCR trans)
    {
    for (int x = 0; x < 3; ++x)
        TransformRowToJson(outValue[x], trans.form3d[x]);
    }

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
        return nullptr;
        }
    Utf8String resolvedName(fileName);
    Utf8String modelName(fileName.GetFileNameWithoutExtension().c_str());

    // Try to open point cloud file
    PointCloudScenePtr pPointCloudScene = PointCloudScene::Create(fileName.c_str());
    if (pPointCloudScene.IsNull())
        {
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
void PointCloudModel::SetViewDensity(float density) { m_properties.m_density = density; }

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
    m_cachedPtViewport.clear();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2016
//----------------------------------------------------------------------------------------
PtViewport* PointCloudModel::GetPtViewportP(DgnViewportCR vp) const
    {
    auto viewportItr = m_cachedPtViewport.find(&vp);
    if (viewportItr != m_cachedPtViewport.end())
        return viewportItr->second.get();
    
    auto ptVp = PtViewport::Create();
    if (ptVp.IsNull())
        return nullptr;

    m_cachedPtViewport.insert(std::make_pair(&vp, ptVp));
    return ptVp.get();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2016
//----------------------------------------------------------------------------------------
void PointCloudModel::_DropGraphicsForViewport(Dgn::DgnViewportCR viewport)
    {
    m_cachedPtViewport.erase(&viewport);
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
            return nullptr;
            
        m_pointCloudScenePtr = PointCloudScene::Create(fileName.c_str());

        if (m_pointCloudScenePtr.IsValid())
            m_loadSceneStatus = PointCloudModel::LoadStatus::Loaded;
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
        return;     // We ran out of viewport.

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

