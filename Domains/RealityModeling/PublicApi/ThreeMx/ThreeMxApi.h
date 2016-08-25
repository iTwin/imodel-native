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
#include <DgnPlatform/TileTree.h>
#include <DgnPlatform/MeshTile.h>
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

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_RENDER
USING_NAMESPACE_TILETREE

DEFINE_POINTER_SUFFIX_TYPEDEFS(Geometry)
DEFINE_POINTER_SUFFIX_TYPEDEFS(Node)
DEFINE_POINTER_SUFFIX_TYPEDEFS(Scene)
DEFINE_POINTER_SUFFIX_TYPEDEFS(ThreeMxModel)

DEFINE_REF_COUNTED_PTR(Geometry)
DEFINE_REF_COUNTED_PTR(Node)
DEFINE_REF_COUNTED_PTR(Scene)
DEFINE_REF_COUNTED_PTR(ThreeMxModel)

//=======================================================================================
// A mesh and a Render::Graphic to draw it. Both are optional - we don't need the mesh except for picking, and sometimes we create Geometry objects for exporting (in which case we don't need the Graphic).
// @bsiclass                                                    Keith.Bentley   06/16
//=======================================================================================
struct Geometry : RefCountedBase, NonCopyableClass
{
protected:
    bvector<FPoint3d> m_points;
    bvector<FPoint3d> m_normals;
    bvector<FPoint2d> m_textureUV;
    bvector<int32_t> m_indices;
    GraphicPtr m_graphic;

public:
    Geometry() {}
    THREEMX_EXPORT Geometry(IGraphicBuilder::TriMeshArgs const&, SceneR);
    PolyfaceHeaderPtr GetPolyface() const;
    void Draw(TileTree::DrawArgsR);
    void ClearGraphic() {m_graphic = nullptr;}
    bool IsEmpty() const {return m_points.empty();}
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
    BentleyStatus Read(TileTree::StreamBuffer&);
};

/*=================================================================================**//**
* A node in the 3mx scene. Each node has a range (from which we store a center/radius) and a "maxScreenDiameter" value.
*
* It can optionally have:
*  1) a list of Geometry objects. If present, these are used to draw this node if the size of the node in pixels is less than maxScreenDiameter. The first few nodes in the scene
*     are merely present to segregate the scene and do not have Geometry (that is, their maxScreenDiameter is 0, so they are not displayable)
*  2) a list of child nodes. The child nodes are read from the "child file" whose name, relative to the parent of this node, is stored in the member "m_childPath". When a node
*     is first created (by its parent), the list of child nodes is empty. Only when/if we determine that the geometry of a node is not fine enough
*     (that is, it is too large in pixels) for a view do we load its children.
*
* Multi-threaded loading of children:
* The loading of children of a node involves reading a file (and potentially downloading from an external reality server). We always do that asynchronously via the RealityCache service on
* the reality cache thread(s). That means that sometimes we'll attempt to draw a node and discover that it is too coarse for the current view, but its children are not loaded yet. In that
* case we draw the geometry of the parent and queue its children to be loaded. The inter-thread synchronization is via the BeAtomic member variable "m_loadState". Only when the value
* of m_loadState==Ready is it safe to use the m_children member.
*
// @bsiclass                                                    Keith.Bentley   03/16
+===============+===============+===============+===============+===============+======*/
struct Node : TileTree::Tile
{
    friend struct Scene;
    typedef std::forward_list<GeometryPtr> GeometryList;

private:
    GeometryList m_geometry;
    Utf8String m_childPath;     // this is the name of the file (relative to path of this node) to load the children of this node.

    bool ReadHeader(JsonValueCR pt, Utf8String&, bvector<Utf8String>& nodeResources);
    BentleyStatus Read3MXB(TileTree::StreamBuffer&, SceneR);
    Utf8String GetChildFile() const;
    BentleyStatus DoRead(StreamBuffer& in, SceneR scene);

    BentleyStatus _Read(TileTree::StreamBuffer& buffer, TileTree::RootR root) override {return Read3MXB(buffer, (SceneR) root);}
    VisitComplete _Draw(TileTree::DrawArgsR, int depth) const override;
    Utf8String _GetTileName() const override {return GetChildFile();}

public:
    Node(NodeP parent) : TileTree::Tile(parent) {}
    Utf8String GetFilePath(SceneR) const;
    bool _HasChildren() const override {return !m_childPath.empty();}
    ChildTiles const* _GetChildren() const override {return IsReady() ? &m_children : nullptr;}
    ElementAlignedBox3d ComputeRange();
    GeometryList& GetGeometry() {return m_geometry;}
};

/*=================================================================================**//**
* A 3mx scene, constructed for a single Render::System. The graphics held by this scene are only useful for that Render::System.
// @bsiclass                                                    Keith.Bentley   03/16
+===============+===============+===============+===============+===============+======*/
struct Scene : TileTree::Root
{
    friend struct Node;
    friend struct Geometry;
    friend struct ThreeMxModel;

private:
    SystemP m_renderSystem = nullptr;

    BentleyStatus ReadGeoLocation(SceneInfo const&);
    virtual Utf8String _ConstructTileName(TileTree::TileCR tile) {return m_rootDir + tile._GetTileName();}
    virtual GeometryPtr _CreateGeometry(IGraphicBuilder::TriMeshArgs const& args) {return new Geometry(args, *this);}
    virtual TexturePtr _CreateTexture(ImageSourceCR source, Image::Format targetFormat, Image::BottomUp bottomUp) const {return m_renderSystem->_CreateTexture(source, targetFormat, bottomUp);}

public:
    SystemP GetRenderSystem() const {return m_renderSystem;}
    BentleyStatus LoadNodeSynchronous(NodeR);
    BentleyStatus LoadScene(); // synchronous
    
    THREEMX_EXPORT BentleyStatus ReadSceneFile(SceneInfo& sceneInfo); //! Read the scene file synchronously
    Scene(DgnDbR db, TransformCR location, Utf8CP cacheName, Utf8CP sceneFile, SystemP system) : Root(db, location, cacheName, sceneFile), m_renderSystem(system) {}
};

//=======================================================================================
// @bsiclass                                                    Ray.Bentley     09/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ThreeMxDomain : DgnDomain
{
    DOMAIN_DECLARE_MEMBERS(ThreeMxDomain, THREEMX_EXPORT)
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
struct ThreeMxModel : SpatialModel, IPublishModelTiles
{
    DGNMODEL_DECLARE_MEMBERS("ThreeMxModel", SpatialModel);
    friend struct ModelHandler;

private:
    Utf8String m_sceneFile;
    Transform m_location;
    mutable ScenePtr m_scene;

    DRange3d GetSceneRange();
    void Load(SystemP) const;

public:
    ThreeMxModel(CreateParams const& params) : T_Super(params) {m_location = Transform::FromIdentity();}
    ~ThreeMxModel() {}

    THREEMX_EXPORT void _AddTerrainGraphics(TerrainContextR) const override;
    THREEMX_EXPORT void _WriteJsonProperties(Json::Value&) const override;
    THREEMX_EXPORT void _ReadJsonProperties(Json::Value const&) override;
    THREEMX_EXPORT AxisAlignedBox3d _QueryModelRange() const override;
    THREEMX_EXPORT void _OnFitView(FitContextR) override;
    THREEMX_EXPORT TileGenerator::Status _PublishModelTiles(TileGenerator& generator, TileGenerator::ITileCollector& collector, TransformCR transformToTile) override;

    //! Set the name of the scene file for this 3MX model
    void SetSceneFile(Utf8CP name) {m_sceneFile = name;}

    //! Set the location transform (from scene coordinates to BIM coordinates)
    void SetLocation(TransformCR trans) {m_location = trans;}
};

//=======================================================================================
// @bsiclass                                                    Ray.Bentley     09/2015
//=======================================================================================
struct ModelHandler :  dgn_ModelHandler::Spatial
{
    MODELHANDLER_DECLARE_MEMBERS ("ThreeMxModel", ThreeMxModel, ModelHandler, dgn_ModelHandler::Spatial, THREEMX_EXPORT)
    THREEMX_EXPORT static DgnModelId CreateModel(DgnDbR db, Utf8CP modelName, Utf8CP sceneFile, TransformCP);
};

END_BENTLEY_THREEMX_NAMESPACE
