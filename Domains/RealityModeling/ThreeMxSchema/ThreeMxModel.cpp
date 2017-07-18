/*-------------------------------------------------------------------------------------+
|
|     $Source: ThreeMxSchema/ThreeMxModel.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ThreeMxInternal.h"
#include <DgnPlatform/JsonUtils.h>

DOMAIN_DEFINE_MEMBERS(ThreeMxDomain)
HANDLER_DEFINE_MEMBERS(ModelHandler)

USING_NAMESPACE_TILETREE


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Scene::ReadSceneFile()
    {
    StreamBuffer rootStream;

    if (IsHttp())
        {
        TileTree::HttpDataQuery query(m_sceneFile, nullptr);
        query.Perform().wait();

        rootStream = std::move(query.GetData());
        }
    else
        {
        TileTree::FileDataQuery query(m_sceneFile, nullptr);
        rootStream = std::move(query.Perform().get());
        }

    return rootStream.HasData() ? m_sceneInfo.Read(rootStream) : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Scene::LoadScene()
    {
    if (SUCCESS != ReadSceneFile())
        return ERROR;
    
    CreateCache(m_sceneInfo.m_sceneName.c_str(), 1024*1024*1024); // 1 GB

    Node* root = new Node(*this, nullptr);
    root->m_childPath = m_sceneInfo.m_rootNodePath;
    m_rootTile = root;

    auto result = _RequestTile(*root, nullptr);
    result.wait(BeDuration::Seconds(2)); // only wait for 2 seconds
    return result.isReady() ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Scene::LoadNodeSynchronous(NodeR node)
    {
    auto result = _RequestTile(node, nullptr);
    result.wait();
    return result.isReady() ? SUCCESS : ERROR;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                      Ray.Bentley     09/2015
//----------------------------------------------------------------------------------------
BentleyStatus Scene::LocateFromSRS()
    {
    DgnGCSPtr bimGCS = m_db.GeoLocation().GetDgnGCS();
    if (!bimGCS.IsValid())
        return ERROR; // BIM is not geolocated, can't use geolocation in 3mx scene

    if (m_sceneInfo.m_reprojectionSystem.empty())
        return SUCCESS;  // scene has no spatial reference system, give up.

    WString    warningMsg;
    StatusInt  status, warning;

    DgnGCSPtr threeMxGCS = DgnGCS::CreateGCS(m_db);

    int epsgCode;
    double latitude, longitude;
    if (1 == sscanf(m_sceneInfo.m_reprojectionSystem.c_str(), "EPSG:%d", &epsgCode))
        status = threeMxGCS->InitFromEPSGCode(&warning, &warningMsg, epsgCode);
    else if (2 == sscanf(m_sceneInfo.m_reprojectionSystem.c_str(), "ENU:%lf,%lf", &latitude, &longitude))
        {
        // ENU specification does not impose any projection method so we use the first azimuthal available using values that will
        // mimic the intent (North is Y positive, no offset)
        // Note that we could have injected the origin here but keeping it in the transform as for other GCS specs
        if (latitude < 90.0 && latitude > -90.0 && longitude < 180.0 && longitude > -180.0)
            status = threeMxGCS->InitAzimuthalEqualArea(&warningMsg, L"WGS84", L"METER", longitude, latitude, 0.0, 1.0, 0.0, 0.0, 1);
        else
            status = ERROR;
        }
    else
        status = threeMxGCS->InitFromWellKnownText(&warning, &warningMsg, DgnGCS::wktFlavorEPSG, WString(m_sceneInfo.m_reprojectionSystem.c_str(), false).c_str());

    if (SUCCESS != status)
        {
        BeAssert(false && warningMsg.c_str());
        return ERROR;
        }

    // Compute a linear transform that approximates the reprojection transformation at the origin.
    Transform localTransform;
    status = threeMxGCS->GetLocalTransform(&localTransform, m_sceneInfo.m_origin, nullptr, true/*doRotate*/, true/*doScale*/, *bimGCS);

    // 0 == SUCCESS, 1 == Warning, 2 == Severe Warning,  Negative values are severe errors.
    if (status == 0 || status == 1)
        {
        m_location = localTransform;
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
    BeTimePoint m_nextShow;
    TileLoadStatePtr m_loads;
    ClipVectorCPtr m_clip;

    ThreeMxProgressive(SceneR scene, DrawArgs::MissingNodes& nodes, TileLoadStatePtr loads, ClipVectorCP clip) : m_scene(scene), m_missing(std::move(nodes)), m_loads(loads), m_clip(clip) {}
    ~ThreeMxProgressive() {if (nullptr != m_loads) m_loads->SetCanceled();}
    Completion _DoProgressive(RenderListContext& context, WantShow&) override;
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
ProgressiveTask::Completion ThreeMxProgressive::_DoProgressive(RenderListContext& context, WantShow& wantShow)
    {
    auto now = BeTimePoint::Now();
    DrawArgs args(context, m_scene.GetLocation(), m_scene, now, now-m_scene.GetExpirationTime(), m_clip.get());

    DEBUG_PRINTF("3MX progressive %d missing, ", m_missing.size());

    for (auto const& node: m_missing)
        {
        auto stat = node.second->GetLoadStatus();
        if (stat == Tile::LoadStatus::Ready)
            node.second->Draw(args, node.first);        // now ready, draw it (this potentially generates new missing nodes)
        else if (stat != Tile::LoadStatus::NotFound)
            args.m_missing.Insert(node.first, node.second);     // still not ready, put into new missing list
        }

    args.RequestMissingTiles(m_scene, m_loads);
    args.DrawGraphics();     // the nodes that newly arrived are in the GraphicBranch in the DrawArgs. Add them to the context 

    m_missing.swap(args.m_missing); // swap the list of missing tiles we were waiting for with those that are still missing.

    DEBUG_PRINTF("3MX after progressive still %d missing", m_missing.size());
    if (m_missing.empty()) // when we have no missing tiles, the progressive task is done.
        {
        m_loads = nullptr; // for debugging
        context.GetViewport()->SetNeedsHeal(); // unfortunately the newly drawn tiles may be obscured by lower resolution ones
        return Completion::Finished;
        }

    if (now > m_nextShow)
        {
        m_nextShow = now + BeDuration::Seconds(1); // once per second
        wantShow = WantShow::Yes;
        }

    return Completion::Aborted;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
ProgressiveTaskPtr Scene::_CreateProgressiveTask(DrawArgsR args, TileLoadStatePtr loads) 
    {
    return new ThreeMxProgressive(*this, args.m_missing, loads, args.m_clip);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ThreeMxModel::Load(SystemP renderSys) const
    {
    if (m_scene.IsValid() && (nullptr==renderSys || m_scene->GetRenderSystem()==renderSys))
        return;

    // if we ask for the model with a different Render::System, we just throw the old one away.
    m_scene = new Scene(m_dgndb, m_location, m_sceneFile.c_str(), renderSys);
    m_scene->SetPickable(true);
    if (SUCCESS != m_scene->LoadScene())
        m_scene = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelId ModelHandler::CreateModel(RepositoryLinkCR modeledElement, Utf8CP sceneFile, TransformCP trans, ClipVectorCP clip, ModelSpatialClassifiersCP classifiers)
    {
    DgnDbR db = modeledElement.GetDgnDb();
    DgnClassId classId(db.Schemas().GetClassId(THREEMX_SCHEMA_NAME, "ThreeMxModel"));
    BeAssert(classId.IsValid());

    ThreeMxModelPtr model = new ThreeMxModel(DgnModel::CreateParams(db, classId, modeledElement.GetElementId()));

    model->SetSceneFile(sceneFile);
    if (trans)
        model->SetLocation(*trans);
    
    if (clip)
        model->SetClip(ClipVector::CreateCopy(*clip).get());

    if (nullptr != classifiers)
        model->SetClassifiers(*classifiers);

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

    if (m_scene.IsValid())
        m_scene->DrawInView(context, m_scene->GetLocation(), m_clip.get());
    }
                                                                  

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ThreeMxModel::_PickTerrainGraphics(PickContextR context) const
    {
    if (!m_scene.IsValid())
        return;
    
    PickContext::ActiveDescription descr(context, GetName());
    m_scene->Pick(context, m_scene->GetLocation(), m_clip.get());
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ThreeMxModel::_OnSaveJsonProperties() 
    {
    T_Super::_OnSaveJsonProperties();

    Json::Value val;
    val[json_sceneFile()] = m_sceneFile;
    if (!m_location.IsIdentity())
        JsonUtils::TransformToJson(val[json_location()], m_location);

    if (m_clip.IsValid())
        val[json_clip()] = m_clip->ToJson();

    if (!m_classifiers.empty())
        val[json_classifiers()] = m_classifiers.ToJson();

    SetJsonProperties(json_threemx(), val);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ThreeMxModel::_OnLoadedJsonProperties()
    {
    T_Super::_OnLoadedJsonProperties();

    JsonValueCR val = GetJsonProperties(json_threemx());
    m_sceneFile = val[json_sceneFile()].asString();

    if (val.isMember(json_location()))
        JsonUtils::TransformFromJson(m_location, val[json_location()]);
    else
        m_location.InitIdentity();

    if (val.isMember(json_clip()))
        m_clip = ClipVector::FromJson(val[json_clip()]);

    if (val.isMember(json_classifiers()))
        m_classifiers.FromJson(val[json_classifiers()]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ThreeMxModel::GeolocateFromSceneFile()
    {
    Load(nullptr);
    auto stat = m_scene->LocateFromSRS();
    if (SUCCESS == stat)
        m_location = m_scene->GetLocation();
    return stat;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ray.Bentley                     04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::TileTree::RootCPtr ThreeMxModel::_GetPublishingTileTree(Dgn::Render::SystemP renderSys) const
    { 
    Load(renderSys);

    return m_scene.get();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ray.Bentley                     04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::ClipVectorPtr ThreeMxModel::_GetPublishingClip () const
    {
    return m_clip.IsValid() ? m_clip->Clone(nullptr) : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ray.Bentley                     04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ThreeMxModel::_GetSpatialClassifiers(Dgn::ModelSpatialClassifiersR classifiers) const 
    {
    classifiers = m_classifiers;
    return SUCCESS;
    }

