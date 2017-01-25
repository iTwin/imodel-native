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
        TileTree::HttpDataQuery query(m_rootUrl, nullptr);
        query.Perform().wait();

        rootStream = std::move(query.GetData());
        }
    else
        {
        TileTree::FileDataQuery query(m_rootUrl, nullptr);
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ThreeMxModel::Load(SystemP renderSys) const
    {
    if (m_scene.IsValid() && (nullptr==renderSys || m_scene->GetRenderSystem()==renderSys))
        return;

    // if we ask for the model with a different Render::System, we just throw the old one away.
    m_scene = new Scene(m_dgndb, m_location, m_sceneFile.c_str(), renderSys);
    if (SUCCESS != m_scene->LoadScene())
        m_scene = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelId ModelHandler::CreateModel(RepositoryLinkCR modeledElement, Utf8CP sceneFile, TransformCP trans, ClipVectorCP clip)
    {
    DgnDbR db = modeledElement.GetDgnDb();
    DgnClassId classId(db.Schemas().GetECClassId(THREEMX_SCHEMA_NAME, "ThreeMxModel"));
    BeAssert(classId.IsValid());

    ThreeMxModelPtr model = new ThreeMxModel(DgnModel::CreateParams(db, classId, modeledElement.GetElementId()));

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
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ThreeMxModel::SetClip(Dgn::ClipVectorCP clip)
    {
    m_clip = clip;
    if (m_scene.IsValid())
        m_scene->SetClip(clip);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TileTree::RootPtr ThreeMxModel::_CreateTileTree(RenderContextR context, ViewControllerCR view)
    {
    Load(&context.GetTargetR().GetSystem());
    if (m_scene.IsValid())
        m_scene->SetClip(m_clip.get());

    return m_scene;
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
        val[JSON_CLIP()] = m_clip->ToJson();
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
        m_clip = ClipVector::FromJson(val[JSON_CLIP()]);
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
    SceneCP             m_scene;
    NodePtr             m_node;
    ClipVectorCP        m_clip;

    PublishTileNode(DgnModelCR model, SceneR scene, NodeR node, TransformCR transformDbToTile, size_t depth, size_t siblingIndex, TileNodeP parent, ClipVectorCP clip)
        : ModelTileNode(model, DRange3d::NullRange(), transformDbToTile, depth, siblingIndex, parent, 0.0), m_scene(&scene), m_node(&node), m_clip(clip) { }

    virtual WString _GetFileExtension() const override { return L"b3dm"; }

    void    SetTolerance (double tolerance) { m_tolerance = tolerance; }
    struct ClipOutputCollector : PolyfaceQuery::IClipToPlaneSetOutput
        {
        TileMeshBuilderR    m_builder;
        DgnModelId          m_modelId;
        DgnDbR              m_dgnDb;
        bool                m_twoSidedTriangles;
        
        ClipOutputCollector(DgnModelId modelId, DgnDbR dgnDb, TileMeshBuilderR builder, bool twoSidedTriangles) : m_builder(builder), m_modelId(modelId), m_dgnDb (dgnDb), m_twoSidedTriangles(twoSidedTriangles) { }

        virtual StatusInt   _ProcessUnclippedPolyface(PolyfaceQueryCR polyfaceQuery) override { m_builder.AddPolyface(polyfaceQuery, DgnMaterialId(), m_dgnDb, DgnElementId(), m_twoSidedTriangles, true); return SUCCESS; }
        virtual StatusInt   _ProcessClippedPolyface(PolyfaceHeaderR polyfaceHeader) override  { m_builder.AddPolyface(polyfaceHeader, DgnMaterialId(), m_dgnDb, DgnElementId(), m_twoSidedTriangles, true); return SUCCESS; }
        };
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
virtual PublishableTileGeometry _GeneratePublishableGeometry(DgnDbR dgnDb, TileGeometry::NormalMode normalMode, bool twoSidedTriangles, bool doPolylines, ITileGenerationFilterCP filter = nullptr) const override
    {
    PublishableTileGeometry publishableGeometry;
    TileMeshList&           tileMeshes =  publishableGeometry.Meshes();
    Transform               sceneToTile = Transform::FromProduct(GetTransformFromDgn(), m_scene->GetLocation());

    if (nullptr == m_node->_GetChildren(true))
        {
        BeAssert(false);
        return publishableGeometry;
        }

    if (0.0 == m_node->_GetMaximumSize())       // Don't bother returning meshes - this node will never be displayed.
        return publishableGeometry;

    bmap <Publish3mxTexture*, TileMeshBuilderPtr>  builderMap;

    for (auto& child : *m_node->_GetChildren(true))
        {
        NodeR                   node = (NodeR) *child;
        ClipPlaneContainment    clipContainment = ClipPlaneContainment_StronglyInside;

        if (nullptr != m_clip)
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

            // Rather than flipping the textures to match the bottom up - flip the parameters here.
            for (auto& param : polyface->Param())
                param.y = 1.0 - param.y;

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
                ClipOutputCollector clipOutputCollector(m_model->GetModelId(), dgnDb, *builder, twoSidedTriangles);

                m_clip->ClipPolyface(*polyface, clipOutputCollector, true);
                }
            else
                {
                builder->AddPolyface(*polyface, DgnMaterialId(), dgnDb, DgnElementId(), twoSidedTriangles, true);
                }
            }
        node.ClearGeometry();       // No longer needed.... reduce memory usage.
        }                                                                                                                                                                                                                        

    for (auto& builder : builderMap)
        if (!builder.second->GetMesh()->IsEmpty())
            tileMeshes.push_back(builder.second->GetMesh());
        
    return publishableGeometry;
    }

};  //  PublishTileNode

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   08/16
//=======================================================================================
struct Publish3mxScene : Scene
{
    using Scene::Scene;

    TexturePtr _CreateTexture(ImageSourceCR source, Image::Format targetFormat=Image::Format::Rgb, Image::BottomUp bottomUp=Image::BottomUp::No) const override {return new Publish3mxTexture(source, targetFormat, bottomUp);}
    GeometryPtr _CreateGeometry(IGraphicBuilder::TriMeshArgs const& args) override {return new Publish3mxGeometry(args, *this);}
};
typedef RefCountedPtr<PublishTileNode>  T_PublishTilePtr;

//=======================================================================================
// @bsiclass                                                    Ray.Bentley     10/2016
//=======================================================================================
struct Publish3MxContext
{
    SceneR                          m_scene;
    DgnModelCPtr                    m_model;
    TransformCR                     m_transformDbToTile;
    ClipVectorCP                    m_tileClip;
    TileGenerator::ITileCollector&  m_collector;
    ITileGenerationProgressMonitorR m_progressMeter;
    uint32_t                        m_totalTiles;
    BeAtomic<uint32_t>              m_completedTiles;
    StopWatch                       m_progressTimer;

    Publish3MxContext (SceneR scene, DgnModelCR model, TransformCR transformDbToTile, ClipVectorCP tileClip, TileGenerator::ITileCollector& collector, ITileGenerationProgressMonitorR progressMeter) :
                        m_scene(scene), m_model(&model), m_transformDbToTile(transformDbToTile), m_tileClip(tileClip), m_collector(collector), m_progressMeter(progressMeter), m_totalTiles(1), m_progressTimer(true) { }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ProcessTile(PublishTileNode& tile, NodeR node, size_t depth)
    { 
    double          tolerance = (0.0 == node._GetMaximumSize()) ? 1.0E6 : (2.0 * node.GetRadius() / node._GetMaximumSize());
    DRange3d        dgnRange;

    m_scene.GetLocation().Multiply (dgnRange, node.GetRange());

#ifdef IGNORE_CLIPPED_TILES
    // This seems like a good idea but doesn't work -- Revisit later (Clark Island);
    DRange3d        tileRange;
    Transform       sceneToTile = Transform::FromProduct(transformDbToTile, scene.GetLocation());

    sceneToTile.Multiply(tileRange, node.GetRange());

    if (nullptr != tileClip &&
        ClipPlaneContainment_StronglyOutside == tileClip->ClassifyRangeContainment(tileRange))
        return nullptr;
#endif

    tile.SetDgnRange (dgnRange);
    tile.SetTolerance (tolerance);

    if (node._HasChildren() && node.IsNotLoaded())
        m_scene.LoadNodeSynchronous(node);


    static size_t       s_depthLimit = 0xffff;                    // Useful for limiting depth when debugging...

    if (nullptr != node._GetChildren(false) && depth < s_depthLimit)
        {
        size_t      childIndex = 0;
        for (auto& child : *node._GetChildren(true))
            {
            NodeR   childNode = (NodeR) *child;

            if (childNode._HasChildren())
                {
                T_PublishTilePtr    childTile = new PublishTileNode(*m_model, m_scene, (NodeR) *child, m_transformDbToTile, depth, childIndex++, &tile, m_tileClip);

                m_totalTiles++;
                tile.GetChildren().push_back(childTile);
                ProcessTile(*childTile, (NodeR) *child,  depth+1);
                }
            }
        }
    folly::via(&BeFolly::ThreadPool::GetIoPool(), [&]()  
        {  
        m_collector._AcceptTile(tile);  
        m_completedTiles++;
        });

    IndicateProgress();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void    IndicateProgress()
    {
    // No.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool    ProcessingRemains()
    {
    if (m_completedTiles >= m_totalTiles)
        return false;

    IndicateProgress();
    return true;
    }


};  // Publish3MxContext

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TileGeneratorStatus ThreeMxModel::_GenerateMeshTiles(TileNodePtr& rootTile, TransformCR transformDbToTile, double leafTolerance, TileGenerator::ITileCollector& collector, ITileGenerationProgressMonitorR progressMeter) 
    {
    ScenePtr  scene = new Publish3mxScene(m_dgndb, m_location, m_sceneFile.c_str(), nullptr);
    if (SUCCESS != scene->LoadScene())
        return TileGeneratorStatus::NoGeometry;

    ClipVectorPtr       tileClip;

    if (m_clip.IsValid())
        {
        tileClip = ClipVector::CreateCopy(*m_clip);
        tileClip->TransformInPlace(transformDbToTile);
        }
    
    Publish3MxContext   publishContext (*scene, *this, transformDbToTile, tileClip.get(), collector, progressMeter);
    T_PublishTilePtr    rootPublishTile =  new PublishTileNode(*this, *scene, (NodeR) *scene->GetRootTile(), transformDbToTile, 0, 0, nullptr, tileClip.get());

    rootTile = rootPublishTile;

    publishContext.ProcessTile(*rootPublishTile, (NodeR) *scene->GetRootTile(), 0);

    while (publishContext.ProcessingRemains())
        BeDuration::FromSeconds(1).Sleep();

    return progressMeter._WasAborted() ? TileGeneratorStatus::Aborted : TileGeneratorStatus::Success;
    }

