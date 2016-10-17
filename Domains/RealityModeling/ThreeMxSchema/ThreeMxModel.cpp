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
BentleyStatus Scene::ReadSceneFile()
    {
    StreamBuffer rootStream;
    auto result = _RequestFile(m_rootUrl, rootStream);

    result.wait(); 
    return result.isReady() ? m_sceneInfo.Read(rootStream) : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Scene::LoadScene()
    {
    CreateCache(1024*1024*1024); // 1 GB

    if (SUCCESS != ReadSceneFile())
        return ERROR;

    Node* root = new Node(nullptr);
    root->m_childPath = m_sceneInfo.m_rootNodePath;
    m_rootTile = root;

    auto result = _RequestTile(*root, nullptr);
    result.wait(std::chrono::seconds(2)); // only wait for 2 seconds
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
    DgnGCSPtr bimGCS = m_db.Units().GetDgnGCS();
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
    TimePoint m_nextShow;
    TileLoadsPtr m_loads;
    ClipVectorCPtr m_clip;

    ThreeMxProgressive(SceneR scene, DrawArgs::MissingNodes& nodes, TileLoadsPtr loads, ClipVectorCP clip) : m_scene(scene), m_missing(std::move(nodes)), m_loads(loads), m_clip(clip) {}
    ~ThreeMxProgressive() {if (nullptr != m_loads) m_loads->SetCanceled();}
    Completion _DoProgressive(ProgressiveContext& context, WantShow&) override;
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
ProgressiveTask::Completion ThreeMxProgressive::_DoProgressive(ProgressiveContext& context, WantShow& wantShow)
    {
    auto now = std::chrono::steady_clock::now();
    DrawArgs args(context, m_scene.GetLocation(), now, now-m_scene.GetExpirationTime(), m_clip.get());

    DEBUG_PRINTF("3MX progressive %d missing, ", m_missing.size());

    for (auto const& node: m_missing)
        {
        auto stat = node.second->GetLoadState();
        if (stat == Tile::LoadState::Ready)
            node.second->Draw(args, node.first);        // now ready, draw it (this potentially generates new missing nodes)
        else if (stat != Tile::LoadState::NotFound)
            args.m_missing.Insert(node.first, node.second);     // still not ready, put into new missing list
        }

    args.RequestMissingTiles(m_scene, m_loads);
    args.DrawGraphics(context);     // the nodes that newly arrived are in the GraphicBranch in the DrawArgs. Add them to the context 

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
DgnModelId ModelHandler::CreateModel(RepositoryLinkCR modeledElement, Utf8CP modelNameIn, Utf8CP sceneFile, TransformCP trans, ClipVectorCP clip)
    {
    DgnDbR db = modeledElement.GetDgnDb();
    DgnClassId classId(db.Schemas().GetECClassId(THREEMX_SCHEMA_NAME, "ThreeMxModel"));
    BeAssert(classId.IsValid());

    Utf8String modelName(db.Models().GetUniqueModelName(modelNameIn));

    ThreeMxModelPtr model = new ThreeMxModel(DgnModel::CreateParams(db, classId, modeledElement.GetElementId(), ThreeMxModel::CreateModelCode(modelName)));

    model->SetSceneFile(sceneFile);
    if (trans)
        model->SetLocation(*trans);
    
    if (clip)
        model->SetClip(ClipVector::CreateCopy(*clip).get());

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
    DrawArgs args(context, m_scene->GetLocation(), now, now-m_scene->GetExpirationTime(), m_clip.get());
    m_scene->Draw(args);
    DEBUG_PRINTF("3MX draw %d graphics, %d total, %d missing ", args.m_graphics.m_entries.size(), m_scene->m_rootTile->CountTiles(), args.m_missing.size());

    args.DrawGraphics(context);

    if (!args.m_missing.empty())
        {
        TileLoadsPtr loads = std::make_shared<TileLoads>();
        args.RequestMissingTiles(*m_scene, loads);
        context.GetViewport()->ScheduleTerrainProgressiveTask(*new ThreeMxProgressive(*m_scene, args.m_missing, loads, m_clip.get()));
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
static Utf8CP JSON_CLIP() {return "Clip";}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ThreeMxModel::_WriteJsonProperties(Json::Value& val) const
    {
    T_Super::_WriteJsonProperties(val);

    val[JSON_SCENE_FILE()] = m_sceneFile;
    if (!m_location.IsIdentity())
        JsonUtils::TransformToJson(val[JSON_LOCATION()], m_location);

    if (m_clip.IsValid())
        JsonUtils::ClipVectorToJson(val[JSON_CLIP()], *m_clip);
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

    if (val.isMember(JSON_CLIP()))
        {
        ClipVectorPtr clip = ClipVector::Create();
        JsonUtils::ClipVectorFromJson(*clip, val[JSON_CLIP()]);
        m_clip = clip;
        }
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

BEGIN_UNNAMED_NAMESPACE
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
struct  PublishTileNode : ModelTileNode
{
    ScenePtr            m_scene;
    NodePtr             m_node;
    ClipVectorCPtr      m_clip;
    DgnModelId          m_modelId;

    PublishTileNode(DgnModelId modelId, SceneR scene, NodeR node, TransformCR transformDbToTile, DRange3dCR dgnRange, size_t depth, size_t siblingIndex, double tolerance, TileNodeP parent, ClipVectorCP clip)
        : ModelTileNode(dgnRange, transformDbToTile, depth, siblingIndex, parent, tolerance), m_scene(&scene), m_node(&node), m_clip(clip), m_modelId(modelId) { }


    struct ClipOutputCollector : PolyfaceQuery::IClipToPlaneSetOutput
        {
        TileMeshBuilderR    m_builder;
        DgnModelId          m_modelId;
        DgnDbR              m_dgnDb;
        bool                m_twoSidedTriangles;
        
        ClipOutputCollector(DgnModelId modelId, DgnDbR dgnDb, TileMeshBuilderR builder, bool twoSidedTriangles) : m_builder(builder), m_modelId(modelId), m_dgnDb (dgnDb), m_twoSidedTriangles(twoSidedTriangles) { }

        virtual StatusInt   _ProcessUnclippedPolyface(PolyfaceQueryCR polyfaceQuery) override { m_builder.AddPolyface(polyfaceQuery, DgnMaterialId(), m_dgnDb, m_modelId, m_twoSidedTriangles); return SUCCESS; }
        virtual StatusInt   _ProcessClippedPolyface(PolyfaceHeaderR polyfaceHeader) override  { m_builder.AddPolyface(polyfaceHeader, DgnMaterialId(), m_dgnDb, m_modelId, m_twoSidedTriangles); return SUCCESS; }
        };
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
virtual TileMeshList _GenerateMeshes(TileGenerationCacheCR, DgnDbR dgnDb, TileGeometry::NormalMode normalMode, bool twoSidedTriangles, bool doPolylines) const override
    {
    TileMeshList        tileMeshes;
    Transform           sceneToTile = Transform::FromProduct(GetTransformFromDgn(), m_scene->GetLocation());

    if (nullptr == m_node->_GetChildren(true))
        {
        BeAssert(false);
        return tileMeshes;
        }

    if (0.0 == m_node->_GetMaximumSize())       // Don't bother returning meshes - this node will never be displayed.
        return tileMeshes;

    bmap <Publish3mxTexture*, TileMeshBuilderPtr>  builderMap;

    for (auto& child : *m_node->_GetChildren(true))
        {
        NodeR                   node = (NodeR) *child;
        ClipPlaneContainment    clipContainment = ClipPlaneContainment_StronglyInside;

        if (m_clip.IsValid())
            {
            DRange3d        tileRange;

            sceneToTile.Multiply (tileRange, node.GetRange());
            if (ClipPlaneContainment_StronglyOutside == (clipContainment = m_clip->ClassifyRangeContainment(tileRange)))
                continue;
            }

        for (auto& geometry : node.GetGeometry())
            {
            if (!geometry->GetPolyface().IsValid())
                continue;

            PolyfaceHeaderPtr   polyface = geometry->GetPolyface()->Clone();
            static bool         s_supplyNormalsForLighting = false;         // Not needed as we are going to ignore lighting (it is baked into the capture).

            if (s_supplyNormalsForLighting && 0 == polyface->GetNormalCount())
                polyface->BuildPerFaceNormals();

            polyface->Transform(sceneToTile);

            Publish3mxGeometry*     publishGeometry = dynamic_cast <Publish3mxGeometry*> (geometry.get());
            Publish3mxTexture*      publishTexture;
            TileMeshBuilderPtr      builder;
            static bool             s_ignoreLighting = true;           // Acute3d models use the lighing at time of capture...

            if (nullptr != publishGeometry &&
                nullptr != (publishTexture = dynamic_cast <Publish3mxTexture*> (publishGeometry->m_texture.get())))
                {
                auto const&   found = builderMap.find(publishTexture);
                
                if (found == builderMap.end())
                    {
                    TileTextureImagePtr     tileTexture = TileTextureImage::Create(publishTexture->m_source);
                    TileDisplayParamsPtr    displayParams = TileDisplayParams::Create(0xffffff, tileTexture.get(), s_ignoreLighting);
                    builder = TileMeshBuilder::Create(displayParams, 0.0, 0.0);

                    builderMap.Insert(publishTexture, builder);
                    }
                else
                    {
                    builder = found->second;
                    }
                }

            if (ClipPlaneContainment_StronglyInside != clipContainment)
                {
                ClipOutputCollector clipOutputCollector(m_modelId, dgnDb, *builder, twoSidedTriangles);

                m_clip->ClipPolyface(*polyface, clipOutputCollector, true);
                }
            else
                {
                builder->AddPolyface(*polyface, DgnMaterialId(), dgnDb, m_modelId, twoSidedTriangles);
                }
            }
        }                                                                                                                                                                                                                        

    for (auto& builder : builderMap)
        if (!builder.second->GetMesh()->IsEmpty())
            tileMeshes.push_back(builder.second->GetMesh());
        
    return tileMeshes;
    }

};  //  PublishTileNode

//=======================================================================================                                                             
// @bsiclass                                                    Keith.Bentley   08/16
//=======================================================================================
struct Publish3mxScene : Scene
{
    using Scene::Scene;

    TexturePtr _CreateTexture(ImageSourceCR source, Image::Format targetFormat=Image::Format::Rgb, Image::BottomUp bottomUp=Image::BottomUp::No) const {return new Publish3mxTexture(source, targetFormat, bottomUp);}
    GeometryPtr _CreateGeometry(IGraphicBuilder::TriMeshArgs const& args) override {return new Publish3mxGeometry(args, *this);}
};

typedef RefCountedPtr<PublishTileNode>  T_PublishTilePtr;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static T_PublishTilePtr tileFromNode(DgnModelId modelId, NodeR node, SceneR scene, TransformCR transformDbToTile, ClipVectorCP tileClip, size_t depth, size_t siblingIndex, TileNodeP parent)
    { 
    double          tolerance = (0.0 == node._GetMaximumSize()) ? 1.0E6 : (2.0 * node.GetRadius() / node._GetMaximumSize());
    DRange3d        dgnRange;

    scene.GetLocation().Multiply (dgnRange, node.GetRange());

#ifdef IGNORE_CLIPPED_TILES
    // This seems like a good idea but doesn't work -- Revisit later (Clark Island);
    DRange3d        tileRange;
    Transform       sceneToTile = Transform::FromProduct(transformDbToTile, scene.GetLocation());

    sceneToTile.Multiply(tileRange, node.GetRange());

    if (nullptr != tileClip &&
        ClipPlaneContainment_StronglyOutside == tileClip->ClassifyRangeContainment(tileRange))
        return nullptr;
#endif

    if (node._HasChildren() && node.IsNotLoaded())
        scene.LoadNodeSynchronous(node);

    static size_t       s_depthLimit = 0xffff;                    // Useful for limiting depth when debugging...
    T_PublishTilePtr    tileNode = new PublishTileNode(modelId, scene, node, transformDbToTile, dgnRange, depth, siblingIndex, tolerance, parent, tileClip);

    if (nullptr != node._GetChildren(false) && depth < s_depthLimit)
        {
        size_t      childIndex = 0;

        depth++;
        for (auto& child : *node._GetChildren(true))
            {
            NodeR   childNode = (NodeR) *child;

            if (childNode._HasChildren())
                {
                T_PublishTilePtr    childTile;

                if ((childTile = tileFromNode(modelId, (NodeR) *child, scene, transformDbToTile, tileClip, depth, childIndex++, tileNode.get())).IsValid())
                    tileNode->GetChildren().push_back(childTile);
                }
            }
        }

    return tileNode;
    }
END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TileGenerator::Status ThreeMxModel::_GenerateMeshTiles(TileNodePtr& rootTile, TransformCR transformDbToTile) 
    {
    ScenePtr  scene = new Publish3mxScene(m_dgndb, m_location, GetName().c_str(), m_sceneFile.c_str(), nullptr);
    
    if (SUCCESS != scene->LoadScene())                                                                                                                                                                
        return TileGenerator::Status::NoGeometry;

    ClipVectorPtr       tileClip;

    if (m_clip.IsValid())
        {
        tileClip = ClipVector::CreateCopy(*m_clip);
        tileClip->TransformInPlace(transformDbToTile);
        }

    rootTile = tileFromNode(GetModelId(), (NodeR) *scene->GetRootTile(), *scene, transformDbToTile, tileClip.get(), 0, 0, nullptr).get();

    return TileGenerator::Status::Success;
    }
