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

USING_NAMESPACE_TILETREE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Scene::ReadSceneFile(SceneInfo& sceneInfo)
    {
    StreamBuffer rootStream;
    auto result = _RequestFile(m_rootUrl, rootStream);

    result.wait(std::chrono::seconds(2)); // only wait for 2 seconds
    return result.isReady() ? sceneInfo.Read(rootStream) : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Scene::LoadScene()
    {
    CreateCache();

    SceneInfo sceneInfo;
    if (SUCCESS != ReadSceneFile(sceneInfo))
        return ERROR;

    Node* root = new Node(nullptr);
    root->m_childPath = sceneInfo.m_rootNodePath;
    m_rootTile = root;

    auto result = _RequestTile(*root);
    result.wait(std::chrono::seconds(2)); // only wait for 2 seconds
    return result.isReady() ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Scene::LoadNodeSynchronous(NodeR node)
    {
    auto result = _RequestTile(node);
    result.wait();
    return result.isReady() ? SUCCESS : ERROR;
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
    else if (2 == sscanf(sceneInfo.m_reprojectionSystem.c_str(), "ENU:%lf,%lf", &latitude, &longitude))
        {
        // ENU specification does not impose any projection method so we use the first azimuthal available using values that will
        // mimic the intent (North is Y positive, no offset)
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
    TimePoint m_nextShow;

    ThreeMxProgressive(SceneR scene, DrawArgs::MissingNodes& nodes) : m_scene(scene), m_missing(std::move(nodes)){}
    Completion _DoProgressive(ProgressiveContext& context, WantShow&) override;
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
ProgressiveTask::Completion ThreeMxProgressive::_DoProgressive(ProgressiveContext& context, WantShow& wantShow)
    {
    auto now = std::chrono::steady_clock::now();
    DrawArgs args(context, m_scene.GetLocation(), now, now-m_scene.GetExpirationTime());

    DEBUG_PRINTF("3MX progressive %d missing", m_missing.size());

    for (auto const& node: m_missing)
        {
        auto stat = node.second->GetLoadState();
        if (stat == Tile::LoadState::Ready)
            node.second->Visit(args, node.first);        // now ready, draw it (this potentially generates new missing nodes)
        else if (stat != Tile::LoadState::NotFound)
            args.m_missing.Insert(node.first, node.second);     // still not ready, put into new missing list
        }

    args.RequestMissingTiles(m_scene);
    args.DrawGraphics(context);  // the nodes that newly arrived are in the GraphicBranch in the DrawArgs. Add them to the context 

    m_missing.swap(args.m_missing); // swap the list of missing tiles we were waiting for with those that are still missing.

    DEBUG_PRINTF("3MX after progressive still %d missing", m_missing.size());
    if (m_missing.empty()) // when we have no missing tiles, the progressive task is done.
        {
        context.GetViewport()->SetNeedsHeal(); // unfortunately the newly drawn tiles may be obscured by lower resolution ones
        return Completion::Finished;
        }

    if (now > m_nextShow)
        {
        m_nextShow = now + std::chrono::seconds(1); // once per second
        wantShow = WantShow::Yes;
        }

    return Completion::Aborted;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ThreeMxModel::Load(SystemP renderSys) const
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
DgnModelId ModelHandler::CreateModel(DgnDbR db, Utf8CP modelNameIn, Utf8CP sceneFile, TransformCP trans)
    {
    DgnClassId classId(db.Schemas().GetECClassId(THREEMX_SCHEMA_NAME, "ThreeMxModel"));
    BeAssert(classId.IsValid());

    Utf8String modelName(db.Models().GetUniqueModelName(modelNameIn));

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

    auto now = std::chrono::steady_clock::now();
    DrawArgs args(context, m_scene->GetLocation(), now, now-m_scene->GetExpirationTime());
    m_scene->Draw(args);
    DEBUG_PRINTF("3MX draw %d graphics, %d total, %d missing ", args.m_graphics.m_entries.size(), m_scene->m_rootTile->CountTiles(), args.m_missing.size());

    args.DrawGraphics(context);

    if (!args.m_missing.empty())
        {
        args.RequestMissingTiles(*m_scene);
        context.GetViewport()->ScheduleTerrainProgressiveTask(*new ThreeMxProgressive(*m_scene, args.m_missing));
        }
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

static Utf8CP JSON_SCENE_FILE() {return "SceneFile";}
static Utf8CP JSON_LOCATION() {return "Location";}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ThreeMxModel::_WriteJsonProperties(Json::Value& val) const
    {
    T_Super::_WriteJsonProperties(val);

    val[JSON_SCENE_FILE()] = m_sceneFile;
    if (!m_location.IsIdentity())
        JsonUtils::TransformToJson(val[JSON_LOCATION()], m_location);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ThreeMxModel::_ReadJsonProperties(JsonValueCR val)
    {
    T_Super::_ReadJsonProperties(val);
    m_sceneFile = val[JSON_SCENE_FILE()].asString();

    if (val.isMember(JSON_LOCATION()))
        JsonUtils::TransformFromJson(m_location, val[JSON_LOCATION()]);
    else
        m_location.InitIdentity();
    }

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   08/16
//=======================================================================================
struct Publish3mxGeometry : Geometry
{
    TexturePtr m_texture;

    Publish3mxGeometry(IGraphicBuilder::TriMeshArgs const& args, SceneR scene)
        {
        m_texture = args.m_texture;

        m_indices.resize(args.m_numIndices);
        memcpy(&m_indices.front(), args.m_vertIndex, args.m_numIndices * sizeof(int32_t));

        m_points.resize(args.m_numPoints);
        memcpy(&m_points.front(), args.m_points, args.m_numPoints * sizeof(FPoint3d));

        if (nullptr != args.m_normals)
            {
            m_normals.resize(args.m_numPoints);
            memcpy(&m_normals.front(), args.m_normals, args.m_numPoints * sizeof(FPoint3d));
            }

        if (nullptr != args.m_textureUV)
            {
            m_textureUV.resize(args.m_numPoints);
            memcpy(&m_textureUV.front(), args.m_textureUV, args.m_numPoints * sizeof(FPoint2d));
            }
            
        }
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   08/16
//=======================================================================================
struct Publish3mxTexture : Render::Texture
{
    ImageSource         m_source;
    Image::Format       m_format;
    Image::BottomUp     m_bottomUp;
    Publish3mxTexture(ImageSourceCR source, Image::Format format, Image::BottomUp bottomUp) : m_source(std::move(source)), m_format(format), m_bottomUp(bottomUp) {}
};

//=======================================================================================
// @bsiclass                                                    Ray.Bentley     08/2016
//=======================================================================================
struct  PublishTileNode : TileNode
{
    ScenePtr            m_scene;
    NodePtr             m_node;
    Transform           m_transform;

    PublishTileNode(SceneR scene, NodeR node, TransformCR transform,  DRange3dCR range, size_t depth, size_t siblingIndex, double tolerance, TileNodeP parent) : 
                    m_scene (&scene), m_node(&node), m_transform(transform), TileNode(range, depth, siblingIndex, tolerance, parent) { }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
virtual TileMeshList _GenerateMeshes(TileGeometryCacheR geometryCache, double tolerance, TileGeometry::NormalMode normalMode=TileGeometry::NormalMode::CurvedSurfacesOnly, bool twoSidedTriangles=false) const override
    {
    TileMeshList        tileMeshes;

    for (auto& geometry : m_node->GetGeometry())
        {
        if (!geometry->GetPolyface().IsValid())
            continue;

        PolyfaceHeaderPtr   polyface = geometry->GetPolyface()->Clone();
        static bool         s_supplyNormalsForLighting = true;         // Not needed as we are going to ignore lighting (it is baked into the capture).

        if (s_supplyNormalsForLighting && 0 == polyface->GetNormalCount())
            polyface->BuildPerFaceNormals();

        polyface->Transform (m_transform);

        Publish3mxGeometry*     publishGeometry = dynamic_cast <Publish3mxGeometry*> (geometry.get());
        Publish3mxTexture*      publishTexture;
        TileTextureImagePtr     tileTexture;
        static bool             s_ignoreLighting = true;           // Acute3d models use the lighing at time of capture...

        if (nullptr != publishGeometry &&
            nullptr != (publishTexture = dynamic_cast <Publish3mxTexture*> (publishGeometry->m_texture.get())))
            tileTexture = new TileTextureImage (publishTexture->m_source);

        TileDisplayParamsPtr    displayParams = new TileDisplayParams (0xffffff, tileTexture, s_ignoreLighting);
        TileMeshBuilderPtr      builder = TileMeshBuilder::Create(displayParams, NULL, 0.0);

        for (PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(*polyface); visitor->AdvanceToNextFace(); )
            builder->AddTriangle(*visitor,DgnElementId(), false, twoSidedTriangles);

        tileMeshes.push_back(builder->GetMesh());
        }
    return tileMeshes;
    }

};  //  PublishTileNode


//=======================================================================================
// @bsiclass                                                    Keith.Bentley   08/16
//=======================================================================================
struct Publish3mxScene : Scene
{
    using Scene::Scene;

    TexturePtr _CreateTexture(ImageSourceCR source, Image::Format targetFormat=Image::Format::Rgb, Image::BottomUp bottomUp=Image::BottomUp::No) const {return new Publish3mxTexture(source, targetFormat, bottomUp);}
    GeometryPtr _CreateGeometry(IGraphicBuilder::TriMeshArgs const& args) override {return new Publish3mxGeometry(args, *this);}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<PublishTileNode> tileFromNode (NodeR node, SceneR scene, TransformCR toTile, size_t depth, size_t siblingIndex, TileNodeP parent)
    { 
    double                  tolerance = (0.0 == node.GetMaximumSize()) ? 1.0E6 : (2.0 * node.GetRadius() / node.GetMaximumSize());
    DRange3d                range = node.GetRange();;

    if (node._HasChildren() && node.IsNotLoaded())
        scene.LoadNodeSynchronous(node);

    if (range.IsNull() && nullptr != node._GetChildren())     // No range set on root node...
        for (auto& child : *node._GetChildren())
            range.Extend (child->GetRange());

    toTile.Multiply (range, range);

    RefCountedPtr<PublishTileNode>     tileNode = new PublishTileNode (scene, node, toTile, range, depth, siblingIndex, tolerance, parent);
    static size_t                   s_depthLimit = 0xffff;                    // Useful for limiting depth when debugging...

    if (nullptr != node._GetChildren() && depth < s_depthLimit)
        {
        size_t                  childIndex = 0;

        depth++;
        for (auto& child : *node._GetChildren())
            tileNode->GetChildren().push_back (tileFromNode ((NodeR) *child, scene, toTile, depth, childIndex++, tileNode.get()));
        }

    return tileNode;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerator::Status ThreeMxModel::_GenerateMeshTiles(TileNodePtr& rootTile, TransformCR transformDbToTile) 
    {
    ScenePtr  scene = new Publish3mxScene(m_dgndb, m_location, GetName().c_str(), m_sceneFile.c_str(), nullptr);
    
    if (SUCCESS != scene->LoadScene())                                                                                                                                                                
        return TileGenerator::Status::NoGeometry;

    Transform               modelToTile = Transform::FromProduct (transformDbToTile, scene->GetLocation());
    
    RefCountedPtr<PublishTileNode>  rootPublishTile = tileFromNode ((NodeR) *scene->GetRoot(), *scene, modelToTile, 0, 0, nullptr);
    
    rootTile = rootPublishTile;
    return TileGenerator::Status::Success;
    }













                                                                          
