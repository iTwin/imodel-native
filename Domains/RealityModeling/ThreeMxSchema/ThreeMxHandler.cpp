/*-------------------------------------------------------------------------------------+
|
|     $Source: ThreeMxSchema/ThreeMxHandler.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ThreeMxSchemaInternal.h"

DOMAIN_DEFINE_MEMBERS(ThreeMxDomain)

HANDLER_DEFINE_MEMBERS(ThreeMxModelHandler)

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Ray.Bentley       09/2015
//-----------------------------------------------------------------------------------------
ThreeMxDomain::ThreeMxDomain() : DgnDomain(BENTLEY_THREEMX_SCHEMA_NAME, "3MX Domain", 1) 
    {
    RegisterHandler(ThreeMxModelHandler::GetHandler());
    }
 
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
//========================================================================================
// @bsiclass                                                        Ray.Bentley     09/2015
//========================================================================================
struct ThreeMxSchema::ThreeMxProgressiveDisplay : Dgn::IProgressiveDisplay, NonCopyableClass
{
    DEFINE_BENTLEY_REF_COUNTED_MEMBERS

protected:
    ThreeMxModelR        m_model;

    ThreeMxProgressiveDisplay(ThreeMxModelR model) : m_model(model) {DEFINE_BENTLEY_REF_COUNTED_MEMBER_INIT }

public:

//----------------------------------------------------------------------------------------
// @bsimethod                                                      Ray.Bentley     09/2015
//----------------------------------------------------------------------------------------
virtual Completion _Process(ViewContextR viewContext) override
    {
    switch (MRMeshCacheManager::GetManager().ProcessRequests())
        {
        default:
        case MRMeshCacheManager::RequestStatus::None:
        case MRMeshCacheManager::RequestStatus::Processed:
            return Completion::HealRequired;

        case MRMeshCacheManager::RequestStatus::Finished:
            return Completion::Finished;
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                      Ray.Bentley     09/2015
//----------------------------------------------------------------------------------------
static void Schedule(ThreeMxModelR model, ViewContextR viewContext)
    {
    ThreeMxProgressiveDisplay*   progressiveDisplay = new ThreeMxProgressiveDisplay(model);

    viewContext.GetViewport()->ScheduleProgressiveDisplay(*progressiveDisplay);
    }

};  //  ThreeMxProgressiveDisplay
#endif

//----------------------------------------------------------------------------------------
// @bsimethod                                                      Ray.Bentley     09/2015
//----------------------------------------------------------------------------------------
MRMeshScenePtr ThreeMxModel::ReadScene(BeFileNameCR fileName, DgnDbR db, SystemP system) 
    {
    S3SceneInfo sceneInfo;
    if (SUCCESS != sceneInfo.Read3MX(fileName))
        return nullptr;

    MRMeshScenePtr scene = new MRMeshScene(db, sceneInfo, fileName);
    return SUCCESS==scene->Load(sceneInfo, system) ? scene : nullptr;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                      Ray.Bentley     09/2015
//----------------------------------------------------------------------------------------
DgnModelId ThreeMxModelHandler::CreateModel(DgnDbR db, Utf8CP modelName, Utf8CP sceneUrl)
    {
    DgnClassId classId(db.Schemas().GetECClassId(BENTLEY_THREEMX_SCHEMA_NAME, "ThreeMxModel"));
    BeAssert(classId.IsValid());

    BeFileName fileName;
    BentleyStatus status = T_HOST.GetPointCloudAdmin()._ResolveFileName(fileName, sceneUrl, db);
    if (SUCCESS != status)
        return DgnModelId();

    MRMeshScenePtr scene = ThreeMxModel::ReadScene(fileName, db, nullptr);
    if (!scene.IsValid())
        return DgnModelId();

    // Create model in DgnDb
    ThreeMxModelPtr model = new ThreeMxModel(DgnModel::CreateParams(db, classId, ThreeMxModel::CreateModelCode(modelName)));
    
    model->SetSceneUrl(sceneUrl);
    model->SetScene(scene.get());
    model->Insert();
    return model->GetModelId();
    }

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
//----------------------------------------------------------------------------------------
// @bsimethod                                                      Ray.Bentley     09/2015
//----------------------------------------------------------------------------------------
MRMeshScenePtr ThreeMxModel::GetScene() const
    {
    if (!m_scene.IsValid())
        {
        BeFileName fileName;
        ReadScene(fileName, GetDgnDb(), m_fileId);
        }

    return m_scene;
    }
#endif

//----------------------------------------------------------------------------------------
// @bsimethod                                               Nicholas.Woodfield     01/2016
//----------------------------------------------------------------------------------------
void ThreeMxModel::GetTiles(TileCallback& callback, double resolution) 
    {
    if (resolution < 0)
        return;
        
    if (!m_scene.IsValid())
        {
        BeAssert(false);
        return;
        }
  
    m_scene->GetTiles(callback, resolution);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                      Ray.Bentley     09/2015
//----------------------------------------------------------------------------------------
AxisAlignedBox3d ThreeMxModel::_QueryModelRange() const
    {
    if (!m_scene.IsValid())
        {
        BeAssert(false);
        return AxisAlignedBox3d();
        }

    DRange3d range;
    m_scene->_GetRange(range);

    return AxisAlignedBox3d(range);
    }   

//----------------------------------------------------------------------------------------
// @bsimethod                                                      Ray.Bentley     09/2015
//----------------------------------------------------------------------------------------
void ThreeMxModel::_AddTerrainGraphics(TerrainContextR context) const
    {
    GetScene();

    if (!m_scene.IsValid())
        {
        BeAssert(false);
        return;
        }

    MRMeshContext meshContext(Transform::FromIdentity(), context, 0.0);
    bool childrenScheduled = false;

    m_scene->Draw(childrenScheduled, context, meshContext);

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    if (childrenScheduled)
        ThreeMxProgressiveDisplay::Schedule(*this, context);
#endif
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                      Ray.Bentley     09/2015
//----------------------------------------------------------------------------------------
void ThreeMxModel::_WriteJsonProperties(Json::Value& val) const
    {
    T_Super::_WriteJsonProperties(val);
    val["SceneUrl"] = m_sceneUrl;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                      Ray.Bentley     09/2015
//----------------------------------------------------------------------------------------
void ThreeMxModel::_ReadJsonProperties(Json::Value const& val)
    {
    T_Super::_ReadJsonProperties(val);
    m_sceneUrl = val["SceneUrl"].asString();
    }

