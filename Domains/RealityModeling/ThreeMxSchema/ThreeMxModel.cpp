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
    DrawArgs::MissingNodes m_missing;
    uint64_t m_nextShow = 0;

    ThreeMxProgressive(SceneR scene, DrawArgs::MissingNodes& nodes) : m_scene(scene), m_missing(std::move(nodes)){}
    Completion _DoProgressive(ProgressiveContext& context, WantShow&) override;
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
ProgressiveTask::Completion ThreeMxProgressive::_DoProgressive(ProgressiveContext& context, WantShow& wantShow) 
    {
    DrawArgs args(context, m_scene);

    DEBUG_PRINTF("3MX progressive %d missing", m_missing.size());

    for (auto const& node: m_missing)
        {
        auto stat = node.second->GetChildLoadStatus();
        if (stat == Node::ChildLoad::Ready)
            node.second->Draw(args, node.first);
        else if (stat != Node::ChildLoad::NotFound)
            args.m_missing.Insert(node.first, node.second);
        }

    // all of the nodes that newly arrived are now in the graphics of the DrawArgs. Draw them now.
    args.DrawGraphics();                                                                                        

    // swap the list of missing tiles from this pass with those that are still missing.
    m_missing.swap(args.m_missing);

    DEBUG_PRINTF("3MX after progressive still %d missing", m_missing.size());
    if (m_missing.empty())
        {
        context.GetViewport()->SetNeedsHeal();
        return Completion::Finished;
        }

    uint64_t now = BeTimeUtilities::QueryMillisecondsCounter();
    if (now > m_nextShow)
        {
        m_nextShow = now + 1000;
        wantShow = WantShow::Yes;
        }

    return Completion::Aborted;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DrawArgs::DrawGraphics()
    {
    if (m_graphics.m_entries.empty())
        return;

    DEBUG_PRINTF("3MX drawing %d 3mx nodes", m_graphics.m_entries.size());
                       
    auto group = m_context.CreateGroupNode(Graphic::CreateParams(nullptr, m_scene.GetLocation()), m_graphics, nullptr);
    BeAssert(m_graphics.m_entries.empty()); // the CreateGroupNode should have moved them
    m_context.OutputGraphic(*group, nullptr);
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

    ElementAlignedBox3d range = m_scene->ComputeRange();
    if (!range.IsValid())
        return AxisAlignedBox3d();

    Frustum box(range);
    box.Multiply(m_scene->GetLocation());

    AxisAlignedBox3d aaRange;
    aaRange.Extend(box.m_pts, 8);

    return aaRange;
    }   

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ThreeMxModel::_AddTerrainGraphics(TerrainContextR context) const
    {
    Load(&context.GetTargetR().GetSystem());

    if (!m_scene.IsValid())
        {
        BeAssert(false);
        return;
        }

    DrawArgs args(context, *m_scene);
    m_scene->Draw(args);
    DEBUG_PRINTF("3MX draw %d graphics, %d missing nodes", args.m_graphics.m_entries.size(), args.m_missing.size());

    args.DrawGraphics();

    if (!args.m_missing.empty())
        context.GetViewport()->ScheduleTerrainProgressiveTask(*new ThreeMxProgressive(*m_scene, args.m_missing));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ThreeMxModel::_OnFitView(FitContextR context)
    {
    Load(nullptr);
    if (!m_scene.IsValid())
        return;

    ElementAlignedBox3d rangeWorld = m_scene->ComputeRange();
    context.ExtendFitRange(rangeWorld, m_scene->GetLocation());
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

