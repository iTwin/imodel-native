/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ThreeMx/ThreeMxApi.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnPlatformApi.h>

#define BEGIN_BENTLEY_THREEMX_NAMESPACE      BEGIN_BENTLEY_NAMESPACE namespace ThreeMx {
#define END_BENTLEY_THREEMX_NAMESPACE        } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_THREEMX      using namespace BentleyApi::ThreeMx;

#define BENTLEY_THREEMX_SCHEMA_NAME "ThreeMx"
#define BENTLEY_THREEMX_SCHEMA_PATH L"ECSchemas/Domain/ThreeMx.01.00.ecschema.xml"
#define THREEMX_SCHEMA(className)   BENTLEY_THREEMX_SCHEMA_NAME "." className

#ifdef __THREEMX_BUILD__
#define THREEMX_EXPORT EXPORT_ATTRIBUTE
#else
#define THREEMX_EXPORT IMPORT_ATTRIBUTE
#endif

BEGIN_BENTLEY_THREEMX_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(CacheManager)
DEFINE_POINTER_SUFFIX_TYPEDEFS(Geometry)
DEFINE_POINTER_SUFFIX_TYPEDEFS(LoadContext)
DEFINE_POINTER_SUFFIX_TYPEDEFS(Node)
DEFINE_POINTER_SUFFIX_TYPEDEFS(Scene)
DEFINE_POINTER_SUFFIX_TYPEDEFS(Scene)
DEFINE_POINTER_SUFFIX_TYPEDEFS(ThreeMxModel)

DEFINE_REF_COUNTED_PTR(Geometry)
DEFINE_REF_COUNTED_PTR(Node)
DEFINE_REF_COUNTED_PTR(Scene)
DEFINE_REF_COUNTED_PTR(ThreeMxModel)

//=======================================================================================
// A ByteStream with a "current position".
// @bsiclass                                                    Keith.Bentley   03/16
//=======================================================================================
struct MxStreamBuffer : ByteStream
    {
    uint32_t m_currPos = 0;
    Dgn::ByteCP GetCurrent() const {return (m_currPos > GetSize()) ? nullptr : GetData() + m_currPos;}
    Dgn::ByteCP Advance(uint32_t size) {m_currPos += size; return GetCurrent();} // returns nullptr if advanced past end.
    void SetPos(uint32_t pos) {m_currPos=pos;}
    };

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     03/2015
+===============+===============+===============+===============+===============+======*/
struct NodeInfo
{
    DPoint3d m_center;
    double m_radius;
    double m_dMax;
    bvector<Utf8String> m_children;
    NodeInfo() 
        {
        m_center.Zero();
        m_radius = 1.0e10;
        m_dMax   = 0.0;
        }
};

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     03/2015
+===============+===============+===============+===============+===============+======*/
struct SceneInfo
{
    Utf8String m_sceneName;
    Utf8String m_SRS;
    bvector<double> m_SRSOrigin;
Utf8String m_navigationMode;
    bvector<Utf8String> m_meshChildren;

    BentleyStatus Read3MX(MxStreamBuffer&);
    BentleyStatus Read3MX(BeFileNameCR filename);
};

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     03/2015
+===============+===============+===============+===============+===============+======*/
struct Geometry : RefCountedBase, NonCopyableClass
{
private:
    bvector<FPoint3d> m_points;
    bvector<FPoint3d> m_normals;
    bvector<FPoint2d> m_textureUV;
    bvector<int32_t> m_indices;
    Dgn::Render::GraphicPtr m_graphic;

public:
    Geometry(Dgn::Render::Graphic::TriMeshArgs const&, LoadContextCR);
    PolyfaceHeaderPtr GetPolyface() const;
    void Draw(Dgn::RenderContextR);
    DRange3d GetRange() const;
    size_t GetMemorySize() const;
    void ClearGraphic() {m_graphic = nullptr;}
    bool IsCached() const {return m_graphic.IsValid();}
};

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     03/2015
+===============+===============+===============+===============+===============+======*/
struct LoadContext
{
    Transform      m_placement;
    Dgn::Render::SystemP m_system = 0;
    bool           m_loadSynchronous = false;
    bool           m_useFixedResolution = false;
    double         m_fixedResolution = 0.0;
    mutable size_t m_lastPumpTicks = 0;
    mutable size_t m_nodeCount = 0;
    mutable size_t m_pointCount = 0;

    LoadContext(TransformCR placement=Transform::FromIdentity()) : m_placement(placement){}
    TransformCR GetPlacement() const {return m_placement;}
    bool GetLoadSynchronous() const {return m_loadSynchronous;}
    bool UseFixedResolution()const {return m_useFixedResolution;}
    double GetFixedResolution() const {return m_fixedResolution;}
};

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     03/2015
+===============+===============+===============+===============+===============+======*/
struct Node :  RefCountedBase, NonCopyableClass
{
    typedef bvector<NodePtr>  MeshNodes;
    NodeInfo m_info;
    NodeP m_parent;
    MeshNodes m_children;
    bvector<GeometryPtr> m_meshes;
    bvector<ByteStream> m_jpegTextures;
    BeFileName m_dir;
    bool m_primary;  // The root node and all descendents until displayable are marked as primary and never flushed.
    bool m_childrenRequested;
    uint64_t m_lastUsed;

public:
    Node(NodeInfo const& info, NodeP parent) : m_info(info), m_parent(parent), m_primary(false), m_childrenRequested(false), m_lastUsed(0) {}
    ~Node();

    void SetDirectory(BeFileNameCR dir) {m_dir = dir;}
    void AddGeometry(int32_t nodeId, Dgn::Render::Graphic::TriMeshArgs& args, int32_t textureIndex, LoadContextCR);
    void PushNode(const NodeInfo& nodeInfo) ;

    BentleyStatus Read3MXB(MxStreamBuffer&, LoadContextCR);
    BentleyStatus Read3MXB(BeFileNameCR filename, LoadContextCR);

    BeFileName GetFileName() const;
    BentleyStatus Load(LoadContextCR);
    BentleyStatus LoadUntilDisplayable(LoadContextCR);
    void RequestLoadUntilDisplayable(LoadContextCR);
    bool LoadedUntilDisplayable() const;
    void Dump(Utf8StringCR prefix);
    bool Draw(Dgn::RenderContextR, LoadContextCR );
    void DrawMeshes(Dgn::RenderContextR);
    void DrawBoundingSphere(Dgn::RenderContextR) const;
    DRange3d GetRange() const;
    DRange3d GetSphereRange() const;
    bool IsLoaded() const {return !m_dir.empty();}
    bool AreChildrenLoaded() const;
    bool AreVisibleChildrenLoaded(MeshNodes& visibleChildren, Dgn::ViewContextR viewContext, LoadContextCR meshContext) const;
    bool IsDisplayable() const {return m_info.m_dMax > 0.0;}
    ByteStream* GetTextureData(int textureIndex) {return textureIndex >= 0 && textureIndex < (int) m_jpegTextures.size() ? &m_jpegTextures[textureIndex] : nullptr;}
    size_t GetTextureMemorySize() const;
    size_t GetMeshMemorySize() const;
    size_t GetMemorySize() const {return GetMeshMemorySize() + GetTextureMemorySize();}
    size_t GetNodeCount() const;
    size_t GetMeshCount() const;
    size_t GetMaxDepth() const;
    bool TestVisibility(bool& isUnderMaximumSize, Dgn::ViewContextR viewContext, LoadContextCR meshContext);
    void RemoveChild(NodeP child);
    void Clone(Node const& other);
    void ClearGraphics();
    bool IsCached() const;
    bool Validate(NodeCP parent) const;
    void GetDepthMap(bvector<size_t>& map, bvector <bset<BeFileName>>& fileNames, size_t depth);
    void Clear();
    void FlushStale(uint64_t staleTime);
    MeshNodes const& GetChildren() const {return m_children;}
    NodeCP GetParent() const {return m_parent;}
    NodeInfo const& GetInfo() const {return m_info;}
};

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     04/2015
+===============+===============+===============+===============+===============+======*/
struct Scene : RefCountedBase, NonCopyableClass
{
private:
    Dgn::DgnDbR m_db;
    Utf8String m_sceneName;
    Utf8String m_srs;
    BeFileName m_fileName;
    DPoint3d m_srsOrigin;
    Transform m_placement;
    bvector<NodePtr> m_children;
    BentleyStatus GetProjectionTransform();

public:
    Scene(Dgn::DgnDbR, SceneInfo const& sceneInfo, BeFileNameCR fileName);
    BentleyStatus Load(SceneInfo const& sceneInfo, LoadContextCR);
    bool Draw(Dgn::RenderContextR, LoadContextCR);
    DRange3d GetRange() const;
    DRange3d GetSphereRange() const;
    void DrawBoundingSpheres(Dgn::RenderContextR);
    size_t GetTextureMemorySize() const;
    size_t GetMeshMemorySize() const;
    size_t GetMemorySize() const {return GetMeshMemorySize() + GetTextureMemorySize();}
    size_t GetMeshCount() const;
    size_t GetNodeCount() const;
    size_t GetMaxDepth() const;
    void FlushStale(uint64_t staleTime);
    TransformCR GetPlacement() const {return m_placement;}
};

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     03/2015
+===============+===============+===============+===============+===============+======*/
struct  CacheManager
{
    struct Cache*         m_cache;
    static CacheManagerR  GetManager();

    CacheManager();
    ~CacheManager();
    void QueueChildLoad(Node::MeshNodes const& children, Dgn::DgnViewportP viewport, LoadContextCR);
    void SetRoot(SceneR root, Dgn::Render::SystemP);
    BentleyStatus SynchronousRead(NodeR node, BeFileNameCR fileName, LoadContextCR);
    void RemoveRequest(NodeR node);
    void Debug();
    void Flush(uint64_t staleTime);

    enum class RequestStatus {Finished, None, Processed};
    RequestStatus ProcessRequests();
};

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     03/2015
+===============+===============+===============+===============+===============+======*/
struct  Util
{
    static void DisplayNodeFailureWarning(WCharCP fileName) {BeAssert(false);};
    static BeFileName ConstructNodeName(Utf8StringCR childName, BeFileNameCP parentName);
#if defined (BENTLEYCONFIG_OS_WINDOWS) && !defined (BENTLEY_WINRT)
    static void GetMemoryStatistics(size_t& memoryLoad, size_t& total, size_t& available);
    static double CalculateResolutionRatio();
#endif
    static BentleyStatus ParseTileId(Utf8StringCR name, uint32_t& tileX, uint32_t& tileY);
};

//=======================================================================================
// @bsiclass                                                    Eric.Paquet     05/15
//=======================================================================================
struct ThreeMxDomain : Dgn::DgnDomain
{
    DOMAIN_DECLARE_MEMBERS(ThreeMxDomain, THREEMX_EXPORT)

public:
    THREEMX_EXPORT ThreeMxDomain();
};

#if defined (NEEDS_WORK_RENDER_SYSTEM)
//=======================================================================================
// @bsiclass                                                    Ray.Bentley     09/2015
//=======================================================================================
struct TileCallback
{
    virtual void _OnTile(uint32_t x, uint32_t y, bvector<bpair<PolyfaceHeaderPtr, int>>& meshes, bvector<bpair<Byte*, Point2d>>& textures) = 0;
};
#endif

//=======================================================================================
// @bsiclass                                                    Ray.Bentley     09/2015
//=======================================================================================
struct ThreeMxModel : Dgn::SpatialModel
{
    DGNMODEL_DECLARE_MEMBERS("ThreeMxModel", SpatialModel);
    friend struct ModelHandler;

private:
    Utf8String m_sceneUrl;
    ScenePtr m_scene;

    DRange3d GetSceneRange();
    static ScenePtr ReadScene(BeFileNameCR fileName, Dgn::DgnDbR db, LoadContextCR);

public:
    ThreeMxModel(CreateParams const& params) : T_Super(params) {}
    double GetDefaultExportResolution() const {return 0.0;}
    THREEMX_EXPORT void _AddTerrainGraphics(Dgn::TerrainContextR) const override;
    THREEMX_EXPORT virtual void _WriteJsonProperties(Json::Value&) const override;
    virtual void THREEMX_EXPORT _ReadJsonProperties(Json::Value const&) override;
    THREEMX_EXPORT Dgn::AxisAlignedBox3d _QueryModelRange() const override;
    void SetSceneUrl(Utf8CP url) {m_sceneUrl = url;}
    SceneP GetScene() const {return m_scene.get();}
};

//=======================================================================================
// @bsiclass                                                    Ray.Bentley     09/2015
//=======================================================================================
struct ModelHandler :  Dgn::dgn_ModelHandler::Spatial
{
    MODELHANDLER_DECLARE_MEMBERS ("ThreeMxModel", ThreeMxModel, ModelHandler, Dgn::dgn_ModelHandler::Spatial, THREEMX_EXPORT)
    THREEMX_EXPORT static Dgn::DgnModelId CreateModel(Dgn::DgnDbR db, Utf8CP modelName, Utf8CP fileName);
};

END_BENTLEY_THREEMX_NAMESPACE

