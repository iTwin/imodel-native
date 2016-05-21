/*-------------------------------------------------------------------------------------+
|
|     $Source: ThreeMxSchema/ThreeMxModel.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ThreeMxInternal.h"
#include <DgnPlatform/JsonUtils.h>

DOMAIN_DEFINE_MEMBERS(ThreeMxDomain)
HANDLER_DEFINE_MEMBERS(ModelHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
ThreeMxDomain::ThreeMxDomain() : DgnDomain(THREEMX_SCHEMA_NAME, "3MX Domain", 1) 
    {
    RegisterHandler(ModelHandler::GetHandler());
    }
 
//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/16
//=======================================================================================
struct ThreeMxProgressive : ProgressiveTask
{
    SceneR m_scene;
    ThreeMxProgressive(SceneR scene) : m_scene(scene) {}
    Completion _DoProgressive(ProgressiveContext& context, WantShow&) override;
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
ProgressiveTask::Completion ThreeMxProgressive::_DoProgressive(ProgressiveContext& context, WantShow&) 
    {
    auto stat = m_scene.ProcessRequests();
    m_scene.Draw(context);

    return stat==Scene::RequestStatus::Finished ? Completion::Finished : Completion::Aborted;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ThreeMxModel::Load(Dgn::Render::SystemP renderSys) const
    {
    if (m_scene.IsValid() && (nullptr==renderSys || m_scene->GetRenderSystem()==renderSys))
        return;

    // if we ask for the model with a different rendersys, we just throw the old one away. 
    m_scene = new Scene(m_dgndb, m_location, GetName().c_str(), m_rootUrl.c_str(), renderSys);
    m_scene->LoadScene();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelId ModelHandler::CreateModel(DgnDbR db, Utf8CP modelName, Utf8CP rootUrl, TransformCP trans)
    {
    DgnClassId classId(db.Schemas().GetECClassId(THREEMX_SCHEMA_NAME, "ThreeMxModel"));
    BeAssert(classId.IsValid());

    ThreeMxModelPtr model = new ThreeMxModel(DgnModel::CreateParams(db, classId, ThreeMxModel::CreateModelCode(modelName)));
    
    model->SetRootUrl(rootUrl);
    if (trans)
        model->SetLocation(*trans);

    model->Insert();
    return model->GetModelId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
AxisAlignedBox3d ThreeMxModel::_QueryModelRange() const
    {
    Load(nullptr);
    if (!m_scene.IsValid())
        {
        BeAssert(false);
        return AxisAlignedBox3d();
        }

    return AxisAlignedBox3d(m_scene->GetRange(m_scene->GetLocation()));
    }   

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ThreeMxModel::_AddTerrainGraphics(TerrainContextR context) const
    {
#if defined (NOT_YET) // This was not ready on the dgndb61-16q2-dev branch. Just disable it for now DO NOT MERGE FORWARD!
    Load(&context.GetTargetR().GetSystem());

    if (!m_scene.IsValid())
        {
        BeAssert(false);
        return;
        }

    m_scene->Draw(context);

    if (m_scene->HasPendingRequests())
        context.GetViewport()->ScheduleTerrainProgressiveTask(*new ThreeMxProgressive(*m_scene));
#endif
    }

#define JSON_ROOT_URL "RootUrl"
#define JSON_LOCATION "Location"
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ThreeMxModel::_WriteJsonProperties(Json::Value& val) const
    {
    T_Super::_WriteJsonProperties(val);

    val[JSON_ROOT_URL] = m_rootUrl;
    if (!m_location.IsIdentity())
        JsonUtils::TransformToJson(val[JSON_LOCATION], m_location);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ThreeMxModel::_ReadJsonProperties(Json::Value const& val)
    {
    T_Super::_ReadJsonProperties(val);
    m_rootUrl = val[JSON_ROOT_URL].asString();

    if (val.isMember(JSON_LOCATION))
        JsonUtils::TransformFromJson(m_location, val[JSON_LOCATION]);
    }

