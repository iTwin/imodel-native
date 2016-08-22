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
#include <DgnPlatform/RealityDataCache.h>
#include <forward_list>

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

typedef std::chrono::steady_clock::time_point TimePoint;

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

//=======================================================================================
// A mesh and a Render::Graphic to draw it. Both are optional - we don't need the mesh except for picking, and sometimes we create Geometry objects for exporting (in which case we don't need the Graphic).
// @bsiclass                                                    Keith.Bentley   06/16
//=======================================================================================
struct Geometry : RefCountedBase, NonCopyableClass
{
private:
    bvector<FPoint3d> m_points;
    bvector<FPoint3d> m_normals;
    bvector<int32_t> m_indices;
    Dgn::Render::GraphicPtr m_graphic;

public:
    Geometry(Dgn::Render::IGraphicBuilder::TriMeshArgs const&, SceneR);
    PolyfaceHeaderPtr GetPolyface() const;
    void Draw(DrawArgsR);
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
    Utf8String m_rootNodePath;
    BentleyStatus Read(MxStreamBuffer&);
};

//=======================================================================================
// Arguments for drawing a node. As nodes are drawn, their Render::Graphics go into the GraphicArray member of this object. After all
// in-view nodes are drawn, the accumulated list of Render::Graphics are placed in a Render::GroupNode with the "location"
// transform for the scene (that is, the tile graphics are always in the local coordinate system of the 3mx scene.)
// If higher resolution tiles are needed but missing, the graphics for lower resolution tiles are
// drawn and the missing tiles are requested for download (if necessary.) They are then added to the MissingNodes member. If the
// MissingNodes list is not empty, we schedule a ProgressiveDisplay that checks for the arrival of the missing nodes and draws them (using
// this class). Each iteration of ProgressiveDisplay starts with a list of previously-missing tiles and generates a new list of
// still-missing tiles until all have arrived (or the view changes.)
// @bsiclass                                                    Keith.Bentley   05/16
//=======================================================================================
struct DrawArgs
{
    typedef bmultimap<int, NodePtr> MissingNodes;
    Dgn::RenderContextR m_context;
    SceneR m_scene;
    Dgn::Render::GraphicBranch m_graphics;
    MissingNodes m_missing;
    TimePoint m_now;
    TimePoint m_purgeOlderThan;

    DrawArgs(Dgn::RenderContextR context, SceneR scene, TimePoint now, TimePoint purgeOlderThan) : m_context(context), m_scene(scene), m_now(now), m_purgeOlderThan(purgeOlderThan) {}
    void DrawGraphics(Dgn::ViewContextR); // place all entries into a GraphicBranch and send it to the RenderContext.
};

/*=================================================================================**//**
* A node in the 3mx scene. Each node has a range (from which we store a center/radius) and a "maxScreenDiameter" value.
*
* It can optionally have:
*  1) a Geometry object. If present, it is used to draw this node if the size of the node in pixels is less than maxScreenDiameter. The first few nodes in the scene
*     are merely present to segregate the scene and do not have Geometry (that is, their maxScreenDiameter is 0, so they are not displayable)
*  2) a list of child nodes. The child nodes are read from the "child file" whose name, relative to the parent of this node, is stored in the member "m_childPath". When a node
*     is first created (by its parent), the list of child nodes is empty. Only when/if we determine that the geometry of a node is not fine enough
*     (that is, it is too large in pixels) for a view do we load its children.
*
* Multi-threaded loading of children:
* The loading of children of a node involves reading a file (and potentially downloading from an external reality server). We always do that asynchronously via the RealityCache service on
* the reality cache thread(s). That means that sometimes we'll attempt to draw a node and discover that it is too coarse for the current view, but its children are not loaded yet. In that
* case we draw the geometry of the parent and queue its children to be loaded. The inter-thread synchronization is via the BeAtomic member variable "m_childLoad". Only when the value
* of m_childLoad==Ready is it safe to use the m_childNodes member.
*
// @bsiclass                                                    Keith.Bentley   03/16
+===============+===============+===============+===============+===============+======*/
struct Node : RefCountedBase, NonCopyableClass
{
    friend struct Scene;
    typedef bvector<NodePtr> ChildNodes;
    enum ChildLoad {NotLoaded=0, Queued=1, Loading=2, Ready=3, NotFound=4, Abandoned=5};

private:
    Dgn::ElementAlignedBox3d m_range;
    DPoint3d m_center;
    double m_radius = 0.0;
    double m_maxScreenDiameter = 0.0;
    NodeP m_parent;
    std::forward_list<GeometryPtr> m_geometry;
    Utf8String m_childPath;     // this is the name of the file (relative to path of this node) to load the children of this node.
    BeAtomic<int> m_childLoad;
    ChildNodes m_childNodes;
    mutable TimePoint m_childrenLastUsed;

    void SetAbandoned();
    bool ReadHeader(JsonValueCR pt, Utf8String&, bvector<Utf8String>& nodeResources);
    BentleyStatus DoRead(MxStreamBuffer&, SceneR);

public:
    Node(NodeP parent) : m_parent(parent), m_childLoad(ChildLoad::NotLoaded) {m_center.Zero();}
    double GetMaxDiameter() const {return m_maxScreenDiameter;}
    double GetRadius() const {return m_radius;}
    DPoint3dCR GetCenter() const {return m_center;}
    Dgn::ElementAlignedBox3d GetRange() const {return m_range;}
    Utf8String GetChildFile () const;
    BentleyStatus Read3MXB(MxStreamBuffer&, SceneR);
    Utf8String GetFilePath(SceneR) const;
    void Draw(DrawArgsR, int depth);
    Dgn::ElementAlignedBox3d ComputeRange();
    ChildLoad GetChildLoadStatus() const {return (ChildLoad) m_childLoad.load();}
    void SetIsReady() {return m_childLoad.store(ChildLoad::Ready);}
    void SetNotFound() {BeAssert(false); return m_childLoad.store(ChildLoad::NotFound);}
    bool HasChildren() const {return !m_childPath.empty();}
    bool IsQueued() const {return m_childLoad.load() == ChildLoad::Queued;}
    bool IsAbandoned() const {return m_childLoad.load() == ChildLoad::Abandoned;}
    bool AreChildrenValid() const {return m_childLoad.load() == ChildLoad::Ready;}
    bool AreChildrenNotLoaded() const {return m_childLoad.load() == ChildLoad::NotLoaded;}
    bool IsDisplayable() const {return GetMaxDiameter() > 0.0;}
    void UnloadChildren(TimePoint olderThan);
    int CountNodes() const;
    ChildNodes const* GetChildren() const {return AreChildrenValid() ? &m_childNodes : nullptr;}
    NodeCP GetParent() const {return m_parent;}
};

/*=================================================================================**//**
* A 3mx scene, constructed for a single Render::System. The graphics held by this scene are only useful for that Render::System.
// @bsiclass                                                    Keith.Bentley   03/16
+===============+===============+===============+===============+===============+======*/
struct Scene : RefCountedBase, NonCopyableClass
{
    friend struct Node;
    friend struct Geometry;
    friend struct ThreeMxModel;

private:
    struct CompareNode{bool operator()(NodePtr a, NodePtr b) const {return a.get() < b.get();}};
    typedef bset<NodePtr, CompareNode> Requests;

    bool m_useFixedResolution = false;
    bool m_isHttp = false;
    bool m_locatable = false;
    double m_fixedResolution = 0.0;
    Dgn::DgnDbR m_db;
    Utf8String m_rootUrl;
    Utf8String m_rootDir;
    BeFileName m_localCacheName;
    Transform m_location;
    double m_scale = 1.0;
    NodePtr m_rootNode;
    std::chrono::seconds m_expirationTime = std::chrono::seconds(20); // save unused nodes for 20 seconds
    Dgn::RealityData::CachePtr m_cache;
    Dgn::Render::SystemP m_renderSystem = nullptr;

    BentleyStatus ReadGeoLocation(SceneInfo const&);
    BentleyStatus LoadScene(); // synchronous
    BentleyStatus RequestData(Node* node, bool synchronous, MxStreamBuffer*);
    void CreateCache();

public:
    bool IsHttp() const {return m_isHttp;}
    Utf8String ConstructNodeName(Node& node) {return m_rootDir + node.GetChildFile();}
    Dgn::Render::SystemP GetRenderSystem() const {return m_renderSystem;}
    DPoint3d GetNodeCenter(Node const& node) const {return DPoint3d::FromProduct(m_location, node.GetCenter());}
    double GetNodeRadius(Node const& node) const {return m_scale * node.GetRadius();}
    void Draw(DrawArgs& args) {m_rootNode->Draw(args, 0);}
    Dgn::ElementAlignedBox3d ComputeRange() {return m_rootNode->ComputeRange();}
    void SetNodeExpirationTime(std::chrono::seconds val) {m_expirationTime = val;} //! set expiration time for unused nodes
    std::chrono::seconds GetNodeExpirationTime() const {return m_expirationTime;} //! get expiration time for unused nodes
    int CountNodes() const {return m_rootNode->CountNodes();}
    bool UseFixedResolution()const {return m_useFixedResolution;}
    bool IsLocatable() const {return m_locatable;}
    double GetFixedResolution() const {return m_fixedResolution;}
    TransformCR GetLocation() const {return m_location;}
    double GetScale() const {return m_scale;}
    Dgn::RealityData::CachePtr GetCache() const {return m_cache;}
    THREEMX_EXPORT BentleyStatus ReadSceneFile(SceneInfo& sceneInfo); //! Read the scene file synchronously
    THREEMX_EXPORT BentleyStatus DeleteCacheFile(); //! delete the local SQLite file holding the cache of downloaded tiles.
    THREEMX_EXPORT Scene(Dgn::DgnDbR, TransformCR location, Utf8CP realityCacheName, Utf8CP sceneFile, Dgn::Render::SystemP);
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
// A DgnModel to reference a 3mx scene. This holds the name of the scenefile, plus a "location" transform
// to position the scene relative to the BIM.
// Note that the scenefile may also have a "Spatial Reference System" stored in it,
// so the location can be calculated by geo-referncing it to the one in the BIM. But, since not all 3mx files are geo-referenced,
// and sometimes users may want to "tweak" the location relative to their BIM, we store it in the model and use that.
// @bsiclass                                                    Keith.Bentley   03/16
//=======================================================================================
struct ThreeMxModel : Dgn::SpatialModel
{
    DGNMODEL_DECLARE_MEMBERS("ThreeMxModel", SpatialModel);
    friend struct ModelHandler;

private:
    Utf8String m_sceneFile;
    Transform m_location;
    mutable ScenePtr m_scene;

    DRange3d GetSceneRange();
    void Load(Dgn::Render::SystemP) const;

public:
    ThreeMxModel(CreateParams const& params) : T_Super(params) {m_location = Transform::FromIdentity();}
    ~ThreeMxModel() {}

    THREEMX_EXPORT void _AddTerrainGraphics(Dgn::TerrainContextR) const override;
    THREEMX_EXPORT void _WriteJsonProperties(Json::Value&) const override;
    THREEMX_EXPORT void _ReadJsonProperties(Json::Value const&) override;
    THREEMX_EXPORT Dgn::AxisAlignedBox3d _QueryModelRange() const override;
    THREEMX_EXPORT void _OnFitView(Dgn::FitContextR) override;

    //! Set the name of the scene file for this 3MX model
    void SetSceneFile(Utf8CP name) {m_sceneFile = name;}

    //! Set the location transform (from scene coordinates to BIM coordinates)
    void SetLocation(TransformCR trans) {m_location = trans;}
};

//=======================================================================================
// @bsiclass                                                    Ray.Bentley     09/2015
//=======================================================================================
struct ModelHandler :  Dgn::dgn_ModelHandler::Spatial
{
    MODELHANDLER_DECLARE_MEMBERS ("ThreeMxModel", ThreeMxModel, ModelHandler, Dgn::dgn_ModelHandler::Spatial, THREEMX_EXPORT)
    THREEMX_EXPORT static Dgn::DgnModelId CreateModel(Dgn::DgnDbR db, Utf8CP modelName, Utf8CP sceneFile, TransformCP);
};

END_BENTLEY_THREEMX_NAMESPACE
