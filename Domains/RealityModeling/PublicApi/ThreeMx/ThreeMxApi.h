/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/TileTree.h>
#include <DgnPlatform/MeshTile.h>
#include <DgnPlatform/ModelSpatialClassifier.h>

#define BEGIN_BENTLEY_THREEMX_NAMESPACE      BEGIN_BENTLEY_NAMESPACE namespace ThreeMx {
#define END_BENTLEY_THREEMX_NAMESPACE        } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_THREEMX      using namespace BentleyApi::ThreeMx;

#define THREEMX_SCHEMA_NAME "ThreeMx"
#define THREEMX_SCHEMA_FILE L"ThreeMx.ecschema.xml"
#define THREEMX_SCHEMA(className)   THREEMX_SCHEMA_NAME "." className

#ifdef __THREEMX_BUILD__
#define THREEMX_EXPORT EXPORT_ATTRIBUTE
#else
#define THREEMX_EXPORT IMPORT_ATTRIBUTE
#endif

BEGIN_BENTLEY_THREEMX_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(Node)
DEFINE_POINTER_SUFFIX_TYPEDEFS(Scene)
DEFINE_POINTER_SUFFIX_TYPEDEFS(ThreeMxModel)

DEFINE_REF_COUNTED_PTR(Node)
DEFINE_REF_COUNTED_PTR(Scene)
DEFINE_REF_COUNTED_PTR(ThreeMxModel)

/*=================================================================================**//**
* Data about the 3mx scene read from the scene (.3mx) file. It holds the filename of the "root node" (relative to the location of the scene file.)
* @bsiclass                                                     Ray.Bentley     03/2015
+===============+===============+===============+===============+===============+======*/
struct SceneInfo
{
    Utf8String m_sceneName;
    Utf8String m_logo;
    Utf8String m_reprojectionSystem;
    Utf8String m_rootNodePath;
    DPoint3d m_origin = DPoint3d::FromZero();
    BentleyStatus Read(Dgn::TileTree::StreamBuffer&);
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
* case we draw the geometry of the parent and queue its children to be loaded. The inter-thread synchronization is via the BeAtomic member variable "m_loadStatus". Only when the value
* of m_loadStatus==Ready is it safe to use the m_children member.
*
// @bsiclass                                                    Keith.Bentley   03/16
+===============+===============+===============+===============+===============+======*/
struct Node : Dgn::TileTree::TriMeshTree::Tile
{
    DEFINE_T_SUPER(Dgn::TileTree::TriMeshTree::Tile);
    friend struct Scene;

    //=======================================================================================
    // @bsiclass                                                    Mathieu.Marchand  11/2016
    //=======================================================================================
    struct Loader : Dgn::TileTree::TileLoader
        {
        Loader(Utf8StringCR url, Dgn::TileTree::TileR tile, Dgn::TileTree::TileLoadStatePtr loads, Dgn::Render::SystemP renderSys) : TileLoader(url, tile, loads, tile._GetTileCacheKey(), renderSys) {}
        BentleyStatus _LoadTile() override {return static_cast<NodeR>(*m_tile).Read3MXB(m_tileBytes, (SceneR)m_tile->GetRootR(), GetRenderSystem());};
        };

private:
    Utf8String m_childPath;     // this is the name of the file (relative to path of this node) to load the children of this node.

    bool ReadHeader(JsonValueCR pt, Utf8String&, bvector<Utf8String>& nodeResources);
    BentleyStatus Read3MXB(Dgn::TileTree::StreamBuffer&, SceneR, Dgn::Render::SystemP renderSys);
    Utf8String GetChildFile() const;
    BentleyStatus DoRead(Dgn::TileTree::StreamBuffer& in, SceneR scene, Dgn::Render::SystemP renderSys);

    //! Called when tile data is required. The loader will be added to the IOPool and will execute asynchronously.
    Dgn::TileTree::TileLoaderPtr _CreateTileLoader(Dgn::TileTree::TileLoadStatePtr, Dgn::Render::SystemP renderSys) override;

    Utf8String _GetTileCacheKey() const override {return GetChildFile();}
    bool _WantDebugRangeGraphics() const override;
public:
    Node(Dgn::TileTree::TriMeshTree::Root& root, NodeP parent) : T_Super(root, parent) {}
    Utf8String GetFilePath(SceneR) const;
    bool _HasChildren() const override {return !m_childPath.empty();}
    ElementAlignedBox3d ComputeRange();
};

/*=================================================================================**//**
//! A 3mx scene, constructed for a single Render::System. The graphics held by this scene are only useful for that Render::System.
// @bsiclass                                                    Keith.Bentley   03/16
+===============+===============+===============+===============+===============+======*/
struct Scene : Dgn::TileTree::TriMeshTree::Root
{
    DEFINE_T_SUPER(Dgn::TileTree::TriMeshTree::Root);
    friend struct Node;
    friend struct ThreeMxModel;

private:
    Utf8String  m_sceneFile;
    SceneInfo   m_sceneInfo;
    Dgn::ClipVectorCPtr m_clip;
    BentleyStatus LocateFromSRS(); // compute location transform from spatial reference system in the sceneinfo

    Utf8CP _GetName() const override { return m_rootResource.c_str(); }
    Dgn::ClipVectorCP _GetClipVector() const override { return m_clip.get(); }

public:
    THREEMX_EXPORT Scene(ThreeMxModelR model, TransformCR location, Utf8CP sceneFile, Dgn::Render::SystemP system);
    THREEMX_EXPORT Scene(Dgn::DgnDbR db, Dgn::DgnModelId modelId, TransformCR location, Utf8CP sceneFile, Dgn::Render::SystemP system);

    ~Scene() {ClearAllTiles();}

    SceneInfo const& GetSceneInfo() const {return m_sceneInfo;}
    BentleyStatus LoadNodeSynchronous(NodeR);
    BentleyStatus LoadScene(); // synchronous
    void SetClip(Dgn::ClipVectorCP clip) { m_clip = clip; }

    THREEMX_EXPORT BentleyStatus ReadSceneFile(); //!< Read the scene file synchronously
};

//=======================================================================================
// @bsiclass                                                    Ray.Bentley     09/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ThreeMxDomain : Dgn::DgnDomain
{
    DOMAIN_DECLARE_MEMBERS(ThreeMxDomain, THREEMX_EXPORT)
    THREEMX_EXPORT ThreeMxDomain();

private:
    WCharCP _GetSchemaRelativePath() const override { return L"ECSchemas/Domain/" THREEMX_SCHEMA_FILE; }
};

//=======================================================================================
//! A DgnModel to reference a 3mx scene. This holds the name of the scenefile, plus a "location" transform
//! to position the scene relative to the BIM.
//! Note that the scenefile may also have a "Spatial Reference System" stored in it,
//! so the location can be calculated by geo-referncing it to the one in the BIM (via #GeolocateFromSceneFile).
//! But, since not all 3mx files are geo-referenced, and sometimes users may want to "tweak" the location relative
//! to their BIM, we store it in the model and use that.
// @bsiclass                                                    Keith.Bentley   03/16
//=======================================================================================
struct ThreeMxModel : Dgn::SpatialModel, Dgn::Render::IGetTileTreeForPublishing
{
    DGNMODEL_DECLARE_MEMBERS("ThreeMxModel", SpatialModel);
    friend struct ModelHandler;

    BE_JSON_NAME(threemx)
    BE_JSON_NAME(sceneFile)
    BE_JSON_NAME(location)
    BE_JSON_NAME(clip)
    BE_JSON_NAME(classifiers)
private:
    Utf8String                              m_sceneFile;
    Transform                               m_location;

    mutable Dgn::ClipVectorCPtr             m_clip;
    mutable Dgn::ModelSpatialClassifiers    m_classifiers;

    DRange3d GetSceneRange();
    SceneP Load(Dgn::Render::SystemP) const;

    uint32_t _GetExcessiveRefCountThreshold() const override { return 0xffff; } // tile publisher makes lots of referrents...
    BentleyStatus _GetSpatialClassifiers(Dgn::ModelSpatialClassifiersR classifiers) const override { classifiers = m_classifiers; return SUCCESS; }
public:
    ThreeMxModel(CreateParams const& params) : T_Super(params) {m_location = Transform::FromIdentity();}
    ~ThreeMxModel() {}

    Dgn::TileTree::RootPtr _GetTileTree(Dgn::RenderContextR) override;
    THREEMX_EXPORT Dgn::TileTree::RootPtr _CreateTileTree(Dgn::Render::SystemP) override;
    THREEMX_EXPORT void _PickTerrainGraphics(Dgn::PickContextR) const override;
    THREEMX_EXPORT void _OnSaveJsonProperties() override;
    THREEMX_EXPORT void _OnLoadedJsonProperties() override;
    THREEMX_EXPORT AxisAlignedBox3d _QueryModelRange() const override;
    AxisAlignedBox3d _QueryNonElementModelRange() const override { return _QueryModelRange(); }

    THREEMX_EXPORT void _OnFitView(Dgn::FitContextR) override;
    // ###TODO_TILE_PUBLISH THREEMX_EXPORT BentleyStatus _GetSpatialClassifiers(Dgn::ModelSpatialClassifiersR classifiers) const override;

    //! Set the name of the scene (.3mx) file for this 3MX model. This can either be a local file name or a URL.
    //! @note New models are not valid until the have a scene file.
    void SetSceneFile(Utf8CP name) {m_sceneFile = name;}

    //! Set the location of this 3MS model in the BIM file. This is a transform from scene coordinates to BIM coordinates.
    //! @note Use this method to manually position the 3mx scene in the BIM. Alternatively, use GeolocateFromSceneFile.
    //! @note To save this value for future sessions, you must call this model's Update method.
    void SetLocation(TransformCR trans) {m_location = trans;}

    //! Set or clear a clipping volume for this model.
    //! @note To save this value for future sessions, you must call this model's Update method.
    THREEMX_EXPORT void SetClip(Dgn::ClipVectorCP clip);

    //! Set the spatial classifiers for this reality model.
    void SetClassifiers(Dgn::ModelSpatialClassifiersCR classifiers) { m_classifiers = classifiers; }

    //! Set the location for this ThreeMxModel from the Spatial Reference System (SRS) data in the scene (.3mx) file.
    //! Generally, this should be called once when the model is first created. On success, the location transformation of the model
    //! is established to position the scene's geolocation into the BIM's GCS.
    //! @return SUCCESS if the scene file was successfully read, it has a SRS, the BIM has a GCS, and we were able to compute a transform between them.
    //! @note To save this value for future sessions, you must call this model's Update method.
    THREEMX_EXPORT BentleyStatus GeolocateFromSceneFile();
};

//=======================================================================================
// @bsiclass                                                    Ray.Bentley     09/2015
//=======================================================================================
struct ModelHandler :  Dgn::dgn_ModelHandler::Spatial
{
    MODELHANDLER_DECLARE_MEMBERS ("ThreeMxModel", ThreeMxModel, ModelHandler, Dgn::dgn_ModelHandler::Spatial, THREEMX_EXPORT)
    THREEMX_EXPORT static Dgn::DgnModelId CreateModel(Dgn::RepositoryLinkCR modeledElement, Utf8CP sceneFile, TransformCP, Dgn::ClipVectorCP, Dgn::ModelSpatialClassifiersCP classifiers = nullptr);
};

END_BENTLEY_THREEMX_NAMESPACE
