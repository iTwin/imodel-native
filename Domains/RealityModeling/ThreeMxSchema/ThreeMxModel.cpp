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
Scene::Scene(DgnDbR db, TransformCR location, Utf8CP realityCacheName, Utf8CP rootUrl, Render::SystemP renderSys) : m_db(db), m_rootUrl(rootUrl), m_location(location), m_renderSystem(renderSys)
    {
    m_isHttp = (0 == strncmp("http:", rootUrl, 5) || 0 == strncmp("https:", rootUrl, 6));
    if (m_isHttp)
        m_rootDir = m_rootUrl.substr(0, m_rootUrl.find_last_of("/"));
    else
        {
        BeFileName rootUrl(BeFileName::DevAndDir, BeFileName(m_rootUrl));
        BeFileName::FixPathName(rootUrl, rootUrl, false);
        m_rootDir = rootUrl.GetNameUtf8();
        }

    m_scale = location.ColumnXMagnitude();

    m_localCacheName = T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectoryBaseName();
    m_localCacheName.AppendToPath(BeFileName(realityCacheName));
    m_localCacheName.AppendExtension(L"3MXcache");

    CreateCache();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Scene::DeleteCacheFile()
    {
    m_cache = nullptr;
    return BeFileNameStatus::Success == m_localCacheName.BeDeleteFile() ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Scene::ReadSceneFile(SceneInfo& sceneInfo)
    {
    MxStreamBuffer rootStream;
    return RealityDataCacheResult::Success != RequestData(nullptr, true, &rootStream) ? ERROR : sceneInfo.Read(rootStream);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Scene::LoadScene()
    {
    SceneInfo sceneInfo;
    if (SUCCESS != ReadSceneFile(sceneInfo))
        return ERROR;

    m_rootNode = new Node(nullptr);
    m_rootNode->m_childPath = sceneInfo.m_rootNodePath;

    return RealityDataCacheResult::Success == RequestData(m_rootNode.get(), true, nullptr) ? SUCCESS : ERROR;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                      Ray.Bentley     09/2015
//----------------------------------------------------------------------------------------
BentleyStatus Scene::ReadGeoLocation(SceneInfo const& sceneInfo)
    {
    DRange3d  range = ComputeRange();
    if (range.IsNull())
        return ERROR;

    DgnGCSPtr databaseGCS = m_db.Units().GetDgnGCS();
    if (!databaseGCS.IsValid())
        return ERROR;

    if (sceneInfo.m_reprojectionSystem.empty())
        return SUCCESS;         // No GCS...

    Transform transform = Transform::From(sceneInfo.m_origin);
    WString    warningMsg;
    StatusInt  status, warning;

    DgnGCSPtr acute3dGCS = DgnGCS::CreateGCS(m_db);

    int epsgCode;
    double latitude, longitude;
    if (1 == sscanf(sceneInfo.m_reprojectionSystem.c_str(), "EPSG:%d", &epsgCode))
        status = acute3dGCS->InitFromEPSGCode(&warning, &warningMsg, epsgCode);
    else if (2 == sscanf (sceneInfo.m_reprojectionSystem.c_str(), "ENU:%lf,%lf", &latitude, &longitude))
        {
        // ENU specification does not impose any projection method so we use the first azimuthal available using values that will
        // mimick the intent (North is Y positive, no offset)
        // Note that we could have injected the origin here but keeping it in the transform as for other GCS specs
        if (latitude < 90.0 && latitude > -90.0 && longitude < 180.0 && longitude > -180.0)
            status = acute3dGCS->InitAzimuthalEqualArea(&warningMsg, L"WGS84", L"METER", longitude, latitude, 0.0, 1.0, 0.0, 0.0, 1);
        else
            status = ERROR;
        }
    else
        status = acute3dGCS->InitFromWellKnownText(&warning, &warningMsg, DgnGCS::wktFlavorEPSG, WString(sceneInfo.m_reprojectionSystem.c_str(), false).c_str());

    if (SUCCESS != status)
        {
        BeAssert(false && warningMsg.c_str());
        return ERROR;
        }

    DRange3d sourceRange;
    transform.Multiply(sourceRange, range);
    
    DPoint3d extent;
    extent.DifferenceOf(sourceRange.high, sourceRange.low);

    // Compute a linear transform that approximate the reprojection transformation.
    Transform localTransform;
    status = acute3dGCS->GetLocalTransform(&localTransform, sourceRange.low, &extent, true/*doRotate*/, true/*doScale*/, *databaseGCS);

    // 0 == SUCCESS, 1 == Warning, 2 == Severe Warning,  Negative values are severe errors.
    if (status == 0 || status == 1)
        {
        m_location = Transform::FromProduct(localTransform, transform);
        return SUCCESS;
        }

    return ERROR;
    }

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
    uint64_t now = BeTimeUtilities::QueryMillisecondsCounter();
    DrawArgs args(context, m_scene, now, now-m_scene.GetNodeExpirationTime());

    DEBUG_PRINTF("3MX progressive %d missing", m_missing.size());

    for (auto const& node: m_missing)
        {
        auto stat = node.second->GetChildLoadStatus();
        if (stat == Node::ChildLoad::Ready)
            node.second->Draw(args, node.first);        // now ready, draw it (this potentially generates new missing nodes)
        else if (stat != Node::ChildLoad::NotFound)
            args.m_missing.Insert(node.first, node.second);     // still not ready, put into new missing list
        }

    args.DrawGraphics();  // the nodes that newly arrived drew into the GraphicsArray in the DrawArgs. Add them to the context in a GroupNode

    m_missing.swap(args.m_missing); // swap the list of missing tiles we were waiting for with those that are still missing.

    DEBUG_PRINTF("3MX after progressive still %d missing", m_missing.size());
    if (m_missing.empty()) // when we have no missing tiles, the progressive task is done. 
        {
        context.GetViewport()->SetNeedsHeal(); // unfortunately the newly drawn tiles may be obscured by lower resolution ones
        return Completion::Finished;
        }

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

    // if we ask for the model with a different Render::System, we just throw the old one away. 
    m_scene = new Scene(m_dgndb, m_location, GetName().c_str(), m_sceneFile.c_str(), renderSys);
    if (SUCCESS != m_scene->LoadScene())
        m_scene = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelId ModelHandler::CreateModel(DgnDbR db, Utf8CP modelName, Utf8CP sceneFile, TransformCP trans)
    {
    DgnClassId classId(db.Schemas().GetECClassId(THREEMX_SCHEMA_NAME, "ThreeMxModel"));
    BeAssert(classId.IsValid());

    ThreeMxModelPtr model = new ThreeMxModel(DgnModel::CreateParams(db, classId, ThreeMxModel::CreateModelCode(modelName)));
    
    model->SetSceneFile(sceneFile);
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
* Called whenever the camera moves. Must be fast.
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

    uint64_t now = BeTimeUtilities::QueryMillisecondsCounter();
    DrawArgs args(context, *m_scene, now, now-m_scene->GetNodeExpirationTime());
    m_scene->Draw(args);
    DEBUG_PRINTF("3MX draw %d graphics, %d total, %d missing ", args.m_graphics.m_entries.size(), m_scene->CountNodes(), args.m_missing.size());

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

#define JSON_SCENE_FILE "SceneFile"
#define JSON_LOCATION "Location"
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ThreeMxModel::_WriteJsonProperties(Json::Value& val) const
    {
    T_Super::_WriteJsonProperties(val);

    val[JSON_SCENE_FILE] = m_sceneFile;
    if (!m_location.IsIdentity())
        JsonUtils::TransformToJson(val[JSON_LOCATION], m_location);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ThreeMxModel::_ReadJsonProperties(Json::Value const& val)
    {
    T_Super::_ReadJsonProperties(val);
    m_sceneFile = val[JSON_SCENE_FILE].asString();

    if (val.isMember(JSON_LOCATION))
        JsonUtils::TransformFromJson(m_location, val[JSON_LOCATION]);
    else
        m_location.InitIdentity();
    }

