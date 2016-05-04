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

#define THREEMX_SCHEMA_NAME "ThreeMx"
#define THREEMX_SCHEMA_FILE L"ThreeMx.01.00.ecschema.xml"
#define THREEMX_SCHEMA(className)   THREEMX_SCHEMA_NAME "." className

#ifdef __THREEMX_BUILD__
#define THREEMX_EXPORT EXPORT_ATTRIBUTE
#else
#define THREEMX_EXPORT IMPORT_ATTRIBUTE
#endif

BEGIN_BENTLEY_THREEMX_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(Geometry)
DEFINE_POINTER_SUFFIX_TYPEDEFS(Node)
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
    MxStreamBuffer() {}
    MxStreamBuffer(ByteStream const& other) : ByteStream(other) {}
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
    Geometry(Dgn::Render::Graphic::TriMeshArgs const&, SceneR);
    PolyfaceHeaderPtr GetPolyface() const;
    void Draw(Dgn::RenderContextR);
    DRange3d GetRange(TransformCR) const;
    size_t GetMemorySize() const;
    void ClearGraphic() {m_graphic = nullptr;}
    bool IsCached() const {return m_graphic.IsValid();}
};

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     03/2015
+===============+===============+===============+===============+===============+======*/
struct SceneInfo
{
    Utf8String m_sceneName;
    Utf8String m_reprojectionSystem;
    bvector<double> m_origin;
    Utf8String m_rootNodePath;;
    BentleyStatus Read(MxStreamBuffer&);
};

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     03/2015
+===============+===============+===============+===============+===============+======*/
struct Node : RefCountedBase, NonCopyableClass
{
    friend struct Scene;
    typedef bvector<NodePtr> ChildNodes;

private:
    DRange3d m_range = DRange3d::NullRange();
    DPoint3d m_center;
    double m_radius = 0.0;
    double m_maxScreenDiameter = 0.0;
    Utf8String m_childPath;     // this is the name of the file, relative to m_dir of this node, to load the children of this node. 
    NodeP m_parent;
    ChildNodes m_childNodes;
    GeometryPtr m_geometry;
    Utf8String m_nodePath;      // the file of this node. This is not set until we attempt to load its children.

    enum ChildLoad {Invalid=0, Queued=1, Ready=2};
    BeAtomic<int> m_childLoad;

    bool ReadHeader(JsonValueCR pt, Utf8String&, bvector<Utf8String>& nodeResources);

public:
    Node(NodeP parent) : m_parent(parent), m_childLoad(ChildLoad::Invalid) {m_center.Zero();}
    ~Node();

    double GetMaxDiameter() const {return m_maxScreenDiameter;}
    double GetRadius() const {return m_radius;}
    DPoint3dCR GetCenter() const {return m_center;}
    DRange3dCR GetRange() const {return m_range;}
    Utf8StringCR GetChildPath() const {return m_childPath;}

    void SetNodePath(Utf8StringCR path) {m_nodePath = path;}
    BentleyStatus Read3MXB(MxStreamBuffer&, SceneR);
    BentleyStatus Read3MXB(BeFileNameCR filename, SceneR);
    Utf8String GetFilePath(SceneR) const;
    BentleyStatus LoadChildren(SceneR);
    bool LoadedUntilDisplayable(SceneR);
    void Dump(Utf8CP prefix) const;
    bool Draw(Dgn::RenderContextR, SceneR);
    void DrawGeometry(Dgn::RenderContextR);
    void DrawBoundingSphere(Dgn::RenderContextR, SceneCR) const;
    DRange3d GetRange(TransformCR) const;
    void SetIsReady() {return m_childLoad.store(ChildLoad::Ready);}
    bool IsQueued() const {return m_childLoad.load() == ChildLoad::Queued;}
    bool AreChildrenValid() const {return m_childLoad.load() == ChildLoad::Ready;}
    bool NeedLoadChildren() const {return m_childLoad.load() == ChildLoad::Invalid && !m_childPath.empty();}
    bool IsDisplayable() const {return GetMaxDiameter() > 0.0;}
    size_t GetTextureMemorySize() const {return 0;}
    size_t GetMeshMemorySize() const;
    size_t GetMemorySize() const {return GetMeshMemorySize() + GetTextureMemorySize();}
    size_t GetNodeCount() const;
    size_t GetMeshCount() const;
    bool TestVisibility(Dgn::ViewContextR viewContext, SceneR);
    void RemoveChild(NodeP child);
    void FlushStale(uint64_t staleTime);
    ChildNodes const* GetChildren() const {return AreChildrenValid() ? &m_childNodes : nullptr;}
    NodeCP GetParent() const {return m_parent;}
};

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     04/2015
+===============+===============+===============+===============+===============+======*/
struct Scene : RefCountedBase, NonCopyableClass
{
    friend struct Node;
    friend struct Geometry;
    friend struct ThreeMxModel;

private:
    struct NodeRequest
    {
        bset<Dgn::DgnViewportP> m_viewports;
        NodeRequest() {}
        NodeRequest(Dgn::DgnViewportP viewport) {m_viewports.insert(viewport);}
    };
    struct CompareNode{bool operator()(NodePtr a, NodePtr b) const {return a.get() < b.get();}};
    typedef bmap<NodePtr, NodeRequest, CompareNode> RequestMap;

    bool m_loadSynchronous = false;
    bool m_useFixedResolution = false;
    bool m_isUrl = false;
    double m_fixedResolution = 0.0;
    Dgn::DgnDbR m_db;
    Utf8String m_rootUrl;
    BeFileName m_localCacheName;
    Transform m_location;
    double m_scale = 1.0;
    NodePtr m_rootNode;
    Dgn::RealityDataCachePtr m_cache;
    Dgn::Render::SystemP m_renderSystem = nullptr;
    RequestMap m_requests;

    BentleyStatus ReadGeoLocation(SceneInfo const&);
    void QueueLoadChildren(Node&, Dgn::DgnViewportP viewport);
    BentleyStatus LoadScene();
    BentleyStatus SynchronousRead(NodeR node, Utf8StringCR fileName);
    void RemoveRequest(NodeR node);
    bool HasPendingRequests() const {return !m_requests.empty();}
    bool IsUrl() const {return m_isUrl;}
    Dgn::RealityDataCacheResult RequestData(Node* node, Utf8StringCR path, bool synchronous, MxStreamBuffer*);

public:
    THREEMX_EXPORT Scene(Dgn::DgnDbR, TransformCR location, Utf8CP realityCacheName, Utf8CP rootUrl, Dgn::Render::SystemP);
    Dgn::Render::SystemP GetRenderSystem() const {return m_renderSystem;}
    DPoint3d GetNodeCenter(Node const& node) const {return DPoint3d::FromProduct(m_location, node.GetCenter());}
    double GetNodeRadius(Node const& node) const {return m_scale * node.GetRadius();}
    bool Draw(Dgn::RenderContextR);
    DRange3d GetRange(TransformCR) const;
    void DrawBoundingSpheres(Dgn::RenderContextR);
    size_t GetTextureMemorySize() const;
    size_t GetMeshMemorySize() const;
    size_t GetMemorySize() const {return GetMeshMemorySize() + GetTextureMemorySize();}
    size_t GetMeshCount() const;
    size_t GetNodeCount() const;
    bool GetLoadSynchronous() const {return m_loadSynchronous;}
    bool UseFixedResolution()const {return m_useFixedResolution;}
    double GetFixedResolution() const {return m_fixedResolution;}
    TransformCR GetLocation() const {return m_location;}
    double GetScale() const {return m_scale;}
    THREEMX_EXPORT BentleyStatus ReadRoot(SceneInfo& sceneInfo);
    THREEMX_EXPORT BentleyStatus DeleteRealityCache();
    void DisplayNodeFailureWarning(Utf8StringCR fileName) {BeAssert(false);};
    Utf8String ConstructNodeName(Utf8StringCR childName, Utf8StringCR parentName);

    enum class RequestStatus {Finished, None, Processed};
    RequestStatus ProcessRequests();

#if defined (BENTLEYCONFIG_OS_WINDOWS) && !defined (BENTLEYCONFIG_OS_WINRT)
    static void GetMemoryStatistics(size_t& memoryLoad, size_t& total, size_t& available);
#endif
    static double CalculateResolutionRatio();
};

//=======================================================================================
// @bsiclass                                                    Ray.Bentley     09/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ThreeMxDomain : Dgn::DgnDomain
{
    DOMAIN_DECLARE_MEMBERS(ThreeMxDomain, THREEMX_EXPORT)

public:
    THREEMX_EXPORT ThreeMxDomain();
};

//=======================================================================================
// @bsiclass                                                    Ray.Bentley     09/2015
//=======================================================================================
struct ThreeMxModel : Dgn::SpatialModel
{
    DGNMODEL_DECLARE_MEMBERS("ThreeMxModel", SpatialModel);
    friend struct ModelHandler;

private:
    Utf8String m_rootUrl;
    Transform m_location; // only used if scene is not geolocated
    mutable ScenePtr m_scene;

    DRange3d GetSceneRange();
    void Load(Dgn::Render::SystemP) const;

public:
    ThreeMxModel(CreateParams const& params) : T_Super(params) {m_location = Transform::FromIdentity();}
    double GetDefaultExportResolution() const {return 0.0;}
    THREEMX_EXPORT void _AddTerrainGraphics(Dgn::TerrainContextR) const override;
    THREEMX_EXPORT void _WriteJsonProperties(Json::Value&) const override;
    THREEMX_EXPORT void _ReadJsonProperties(Json::Value const&) override;
    THREEMX_EXPORT Dgn::AxisAlignedBox3d _QueryModelRange() const override;
    void SetRootUrl(Utf8CP url) {m_rootUrl = url;}
    void SetLocation(TransformCR trans) {m_location = trans;}
};

//=======================================================================================
// @bsiclass                                                    Ray.Bentley     09/2015
//=======================================================================================
struct ModelHandler :  Dgn::dgn_ModelHandler::Spatial
{
    MODELHANDLER_DECLARE_MEMBERS ("ThreeMxModel", ThreeMxModel, ModelHandler, Dgn::dgn_ModelHandler::Spatial, THREEMX_EXPORT)
    THREEMX_EXPORT static Dgn::DgnModelId CreateModel(Dgn::DgnDbR db, Utf8CP modelName, Utf8CP rootUrl, TransformCP);
};

END_BENTLEY_THREEMX_NAMESPACE

