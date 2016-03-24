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

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
DgnModelId PointCloudModelHandler::CreatePointCloudModel(DgnDbR db, Utf8StringCR fileId)
    {
    DgnClassId classId(db.Schemas().GetECClassId(POINTCLOUD_SCHEMA_NAME, "PointCloudModel"));
    BeAssert(classId.IsValid());

    // Find resolved file name for the point cloud
    BeFileName fileName;
    BentleyStatus status = T_HOST.GetPointCloudAdmin()._ResolveFileName(fileName, fileId, db);
    if (status != SUCCESS)
        {
        // Return an invalid model id.
        return DgnModelId();
        }
    Utf8String resolvedName(fileName);
    Utf8String modelName(fileName.GetFileNameWithoutExtension().c_str());

    // Try to open point cloud file
    PointCloudScenePtr pointCloudScenePtr = PointCloudScene::Create(fileName.c_str());
    if (pointCloudScenePtr == nullptr)
        {
        // Can't create model; probably that file name is invalid. Return an invalid model id.
        return DgnModelId();
        }

    PointCloudModel::Properties props;
    props.m_fileId = fileId;
    pointCloudScenePtr->GetRange (props.m_range, true);

    // Create model in DgnDb
    PointCloudModelPtr model = new PointCloudModel(DgnModel::CreateParams(db, classId, DgnModel::CreateModelCode(modelName)), props);
    model->Insert();
    return model->GetModelId();
    }

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
    m_sceneToWorld.InitIdentity();
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
            {
            // Get transformation to UOR (including transformation for the coordinate system)
            DRange3d rangeUOR = GetRange();
            WString wktString(m_pointCloudScenePtr->GetSurveyGeoreferenceMetaTag());
            if (SUCCESS != PointCloudGcsFacility::GetTransformToUor(m_sceneToWorld, wktString, rangeUOR, GetDgnDb()))
                m_sceneToWorld.InitIdentity();

            m_loadSceneStatus = PointCloudModel::LoadStatus::Loaded;
            }
        }

    return m_pointCloudScenePtr.get();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2016
//----------------------------------------------------------------------------------------
void PointCloudModel::_OnFitView(Dgn::FitContextR context)
    {
    PointCloudScene* pScene = GetPointCloudSceneP();
    if(nullptr != pScene)
        {
        DRange3d pcRange;
        pScene->GetRange(pcRange, true);
        DRange3d pcRangeWorld;
        GetSceneToWorld().Multiply(pcRangeWorld, pcRange);

        ElementAlignedBox3d box;
        box.InitFrom(pcRangeWorld.low, pcRangeWorld.high);
        context.ExtendFitRange(box, Transform::FromIdentity());
        }
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
// @bsimethod                                                       Eric.Paquet     5/2015
//----------------------------------------------------------------------------------------
DRange3d PointCloudModel::GetSceneRange() const
    {
    DRange3d range = DRange3d::From (0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
    if (GetPointCloudSceneP() != nullptr)
        GetPointCloudSceneP()->GetRange (range, true);
        
    return range;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
AxisAlignedBox3d PointCloudModel::_QueryModelRange() const
    {
    return AxisAlignedBox3d(m_properties.m_range);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void PointCloudModel::JsonUtils::DPoint3dToJson (JsonValueR outValue, DPoint3dCR point)
    {
    outValue[0] = point.x;
    outValue[1] = point.y;
    outValue[2] = point.z;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void PointCloudModel::JsonUtils::DPoint3dFromJson (DPoint3dR point, Json::Value const& inValue)
    {
    point.x = inValue[0].asDouble();
    point.y = inValue[1].asDouble();
    point.z = inValue[2].asDouble();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void PointCloudModel::Properties::ToJson(Json::Value& v) const
    {
    JsonUtils::DPoint3dToJson(v["RangeLow"], m_range.low);
    JsonUtils::DPoint3dToJson(v["RangeHigh"], m_range.high);

    v["FileId"] = m_fileId.c_str();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void PointCloudModel::Properties::FromJson(Json::Value const& v)
    {
    JsonUtils::DPoint3dFromJson(m_range.low, v["RangeLow"]);
    JsonUtils::DPoint3dFromJson(m_range.high, v["RangeHigh"]);

    m_fileId = v["FileId"].asString();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void PointCloudModel::_WriteJsonProperties(Json::Value& v) const
    {
    T_Super::_WriteJsonProperties(v);
    m_properties.ToJson(v);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void PointCloudModel::_ReadJsonProperties(Json::Value const& v)
    {
    T_Super::_ReadJsonProperties(v);
    m_properties.FromJson(v);
    }

