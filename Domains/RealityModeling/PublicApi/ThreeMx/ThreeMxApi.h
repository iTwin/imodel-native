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

DEFINE_POINTER_SUFFIX_TYPEDEFS(DrawArgs)
DEFINE_POINTER_SUFFIX_TYPEDEFS(Geometry)
DEFINE_POINTER_SUFFIX_TYPEDEFS(Node)
DEFINE_POINTER_SUFFIX_TYPEDEFS(Scene)
DEFINE_POINTER_SUFFIX_TYPEDEFS(ThreeMxModel)

DEFINE_REF_COUNTED_PTR(Geometry)
DEFINE_REF_COUNTED_PTR(Node)
DEFINE_REF_COUNTED_PTR(Scene)
DEFINE_REF_COUNTED_PTR(ThreeMxModel)

//=======================================================================================
// A ByteStream with a "current position". Used for reading 3MX files
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
* A mesh, plus optionally a texture
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
    void Draw(DrawArgsR);
    DRange3d GetRange(TransformCR) const;
    size_t GetMemorySize() const;
    void ClearGraphic() {m_graphic = nullptr;}
    bool IsCached() const {return m_graphic.IsValid();}
};

/*=================================================================================**//**
* Data about the 3mx scene read from the "Scene" file. It holds the filename of the "root node" (relative to the location of the scene file.)
* @bsiclass                                                     Ray.Bentley     03/2015
+===============+===============+===============+===============+===============+======*/
struct SceneInfo
{
    Utf8String m_sceneName;
    Utf8String m_reprojectionSystem;
    DPoint3d m_origin = DPoint3d::FromZero();
    Utf8String m_rootNodePath;;
    BentleyStatus Read(MxStreamBuffer&);
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   05/16
//=======================================================================================
struct DrawArgs
{
    Dgn::RenderContextR m_context;
    SceneR m_scene;
    Dgn::Render::GraphicArray m_graphics;
    DrawArgs(Dgn::RenderContextR context, SceneR scene) : m_context(context), m_scene(scene) {}
};

/*=================================================================================**//**
* A node in the 3mx scene. Each node has a range (from which we store a center/radius) and a "maxScreenDiameter" value.
*
* It can optionally have:
*  1) a Geometry ojbect. If present, it is used to draw this node if the size of the node in pixes is less than maxScreenDiameter. The first few nodes in the scene
*     are merely present to segregate the scene and do not have Geometry (that is, thier maxScreenDiameter is 0, so they are not displayable)
*  2) a list of child nodes. The child nodes are read from the "child file" whose name, relative to the parent of this node, is stored in the member "m_childPath". When a node
*     is first created (by its parent), the list of child nodes is empty. Only when/if we determine that the geometry of a node is not fine enough
*     (that is, it is too large in pixels) for a view do we load its children.
*
* Multi-threaded loading of children:
* The loading of children of a node involves reading a file (and potentially downloading from an external reality server). We always do that asynchronously via the RealityCache service on
* the reality cache thread(s). That means that sometimes we'll attempt to draw a node and discover that it is too coarse for the current view, but its children are not loaded yet. In that
* case we draw thw geometry of the parent and queue its children to be loaded. The inter-thread synchronization is via the BeAtomic member variable "m_childLoad". Only when the value
* of m_childLoad==Ready is it safe to use the m_childNodes member.
*
// @bsiclass                                                    Keith.Bentley   03/16
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
    NodeP m_parent;
    GeometryPtr m_geometry;
    Utf8String m_childPath;     // this is the name of the file (relative to path of this node) to load the children of this node.

    enum ChildLoad {Invalid=0, Queued=1, Ready=2, NotFound=3};
    BeAtomic<int> m_childLoad;
    ChildNodes m_childNodes;

    bool ReadHeader(JsonValueCR pt, Utf8String&, bvector<Utf8String>& nodeResources);
    BentleyStatus DoRead(MxStreamBuffer&, SceneR);

public:
    Node(NodeP parent) : m_parent(parent), m_childLoad(ChildLoad::Invalid) {m_center.Zero();}
    double GetMaxDiameter() const {return m_maxScreenDiameter;}
    double GetRadius() const {return m_radius;}
    DPoint3dCR GetCenter() const {return m_center;}
    DRange3dCR GetRange() const {return m_range;}
    Utf8String GetChildFile () const;
    BentleyStatus Read3MXB(MxStreamBuffer&, SceneR);
    BentleyStatus Read3MXB(BeFileNameCR filename, SceneR);
    Utf8String GetFilePath(SceneR) const;
    BentleyStatus LoadChildren(SceneR);
    void Dump(Utf8CP prefix) const;
    bool Draw(DrawArgsR);
    void DrawGeometry(DrawArgsR);
    DRange3d GetRange(TransformCR) const;
    void SetIsReady() {return m_childLoad.store(ChildLoad::Ready);}
    void SetNotFound() {BeAssert(false); return m_childLoad.store(ChildLoad::NotFound);}
    bool IsQueued() const {return m_childLoad.load() == ChildLoad::Queued;}
    bool AreChildrenValid() const {return m_childLoad.load() == ChildLoad::Ready;}
    bool NeedLoadChildren() const {return m_childLoad.load() == ChildLoad::Invalid && !m_childPath.empty();}
    bool IsDisplayable() const {return GetMaxDiameter() > 0.0;}
    void UnloadChildren();
    size_t GetNodeCount() const;
    bool TestVisibility(Dgn::ViewContextR viewContext, SceneR);
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
    struct CompareNode{bool operator()(NodePtr a, NodePtr b) const {return a.get() < b.get();}};
    typedef bset<NodePtr, CompareNode> Requests;

    bool m_loadSynchronous = false;
    bool m_useFixedResolution = false;
    bool m_isUrl = false;
    double m_fixedResolution = 0.0;
    Dgn::DgnDbR m_db;
    Utf8String m_rootUrl;
    Utf8String m_rootDir;
    BeFileName m_localCacheName;
    Transform m_location;
    double m_scale = 1.0;
    NodePtr m_rootNode;
    Dgn::RealityDataCachePtr m_cache;
    Dgn::Render::SystemP m_renderSystem = nullptr;
    Requests m_requests;

    BentleyStatus ReadGeoLocation(SceneInfo const&);
    void QueueLoadChildren(Node&);
    BentleyStatus LoadScene();
    void RemoveRequest(NodeR node);
    bool HasPendingRequests() const {return !m_requests.empty();}
    bool IsUrl() const {return m_isUrl;}
    Dgn::RealityDataCacheResult RequestData(Node* node, bool synchronous, MxStreamBuffer**);

public:
    THREEMX_EXPORT Scene(Dgn::DgnDbR, TransformCR location, Utf8CP realityCacheName, Utf8CP rootUrl, Dgn::Render::SystemP);
    Dgn::Render::SystemP GetRenderSystem() const {return m_renderSystem;}
    DPoint3d GetNodeCenter(Node const& node) const {return DPoint3d::FromProduct(m_location, node.GetCenter());}
    double GetNodeRadius(Node const& node) const {return m_scale * node.GetRadius();}
    bool Draw(Dgn::RenderContextR);
    DRange3d GetRange(TransformCR) const;
    size_t GetNodeCount() const;
    bool GetLoadSynchronous() const {return m_loadSynchronous;}
    bool UseFixedResolution()const {return m_useFixedResolution;}
    double GetFixedResolution() const {return m_fixedResolution;}
    TransformCR GetLocation() const {return m_location;}
    double GetScale() const {return m_scale;}
    THREEMX_EXPORT BentleyStatus ReadRoot(SceneInfo& sceneInfo);
    THREEMX_EXPORT BentleyStatus DeleteRealityCache();
    Utf8String ConstructNodeName(Node& node);

    enum class RequestStatus {Finished, Pending};
    RequestStatus ProcessRequests();

    static void GetMemoryStatistics(size_t& memoryLoad, size_t& total, size_t& available);
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
    Transform m_location;
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

