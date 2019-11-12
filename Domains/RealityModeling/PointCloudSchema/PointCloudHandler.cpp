/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <PointCloudInternal.h>
#include <PointCloud/PointCloudHandler.h>
#include "PointCloudTileTree.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_POINTCLOUD
USING_NAMESPACE_BENTLEY_BEPOINTCLOUD

HANDLER_DEFINE_MEMBERS(PointCloudModelHandler)

BE_JSON_NAME(PointCloudModel)
static Utf8CP PROPERTYJSON_FileUri = "FileUri";
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
    BentleyStatus status = T_HOST.GetPointCloudAdmin()._ResolveFileUri(fileName, params.m_fileUri, params.m_dgndb);
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
    props.m_fileUri = params.m_fileUri;
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
    //
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
        BentleyStatus status = T_HOST.GetPointCloudAdmin()._ResolveFileUri(fileName, m_properties.m_fileUri, GetDgnDb());
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
// @bsimethod                                                       Ray.Bentley     10/2019
//----------------------------------------------------------------------------------------
AxisAlignedBox3d PointCloudModel::_QueryNonElementModelRange() const 
    {
    AxisAlignedBox3d   range;
    
    GetRange(range, PointCloudModel::Unit::World);
    return range;
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
    v[PROPERTYJSON_FileUri] = m_fileUri.c_str();

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
    m_fileUri = v[PROPERTYJSON_FileUri].asString();
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
void PointCloudModel::_OnSaveJsonProperties() 
    {
    T_Super::_OnSaveJsonProperties();
    Json::Value val;
    m_properties.ToJson(val);
    SetJsonProperties(json_PointCloudModel(), val);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void PointCloudModel::_OnLoadedJsonProperties()
    {
    T_Super::_OnLoadedJsonProperties();
    m_properties.FromJson(GetJsonProperties(json_PointCloudModel()));
    }

    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::Cesium::RootPtr PointCloudModel::_CreateCesiumTileTree(Dgn::Cesium::OutputR output, Utf8StringCR)
    {
    return PointCloudTileTree::Root::Create(*this, output);
    }
