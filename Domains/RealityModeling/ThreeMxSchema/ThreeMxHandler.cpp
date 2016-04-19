/*-------------------------------------------------------------------------------------+
|
|     $Source: ThreeMxSchema/ThreeMxHandler.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ThreeMxSchemaInternal.h"

DOMAIN_DEFINE_MEMBERS(ThreeMxDomain)
HANDLER_DEFINE_MEMBERS(ModelHandler)

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Ray.Bentley       09/2015
//-----------------------------------------------------------------------------------------
ThreeMxDomain::ThreeMxDomain() : DgnDomain(THREEMX_SCHEMA_NAME, "3MX Domain", 1) 
    {
    RegisterHandler(ModelHandler::GetHandler());
    }
 
//========================================================================================
// @bsiclass                                                        Ray.Bentley     09/2015
//========================================================================================
struct ThreeMxProgressive : ProgressiveTask
{
    SceneR m_scene;
    ThreeMxProgressive(SceneR scene) : m_scene(scene) {}
    Completion _DoProgressive(ProgressiveContext& context, WantShow&) override;
};

//----------------------------------------------------------------------------------------
// @bsimethod                                                      Ray.Bentley     09/2015
//----------------------------------------------------------------------------------------
ProgressiveTask::Completion ThreeMxProgressive::_DoProgressive(ProgressiveContext& context, WantShow&) 
    {
    m_scene.ProcessRequests();
#if defined (NEEDS_WORK_RENDER_SYSTEM)
    switch (m_scene.ProcessRequests())
        {
        default:
        case CacheManager::RequestStatus::None:
        case CacheManager::RequestStatus::Processed:
            return Completion::HealRequired;

        case CacheManager::RequestStatus::Finished:
            return Completion::Finished;
        }
#endif
    return Completion::Finished;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                      Ray.Bentley     09/2015
//----------------------------------------------------------------------------------------
ScenePtr ThreeMxModel::ReadScene(BeFileNameCR fileName, DgnDbR db) 
    {
    SceneInfo sceneInfo;
    if (SUCCESS != sceneInfo.Read3MX(fileName))
        return nullptr;

    ScenePtr scene = new Scene(db, sceneInfo, fileName);
    return SUCCESS==scene->Load(sceneInfo) ? scene : nullptr;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                      Ray.Bentley     09/2015
//----------------------------------------------------------------------------------------
DgnModelId ModelHandler::CreateModel(DgnDbR db, Utf8CP modelName, Utf8CP sceneUrl)
    {
    DgnClassId classId(db.Schemas().GetECClassId(THREEMX_SCHEMA_NAME, "ThreeMxModel"));
    BeAssert(classId.IsValid());

    BeFileName fileName(sceneUrl);

    ScenePtr scene = ThreeMxModel::ReadScene(fileName, db);
    if (!scene.IsValid())
        return DgnModelId();

    // Create model in DgnDb
    ThreeMxModelPtr model = new ThreeMxModel(DgnModel::CreateParams(db, classId, ThreeMxModel::CreateModelCode(modelName)));
    
    model->SetSceneUrl(sceneUrl);
//    model->SetScene(scene.get());
    model->Insert();
    return model->GetModelId();
    }

#if defined (NEEDS_WORK_RENDER_SYSTEM)
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
#endif

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

    return AxisAlignedBox3d(m_scene->GetRange());
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

    if (m_scene->Draw(context))
        context.GetViewport()->ScheduleTerrainProgressiveTask(*new ThreeMxProgressive(*m_scene));
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

