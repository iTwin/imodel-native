/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ScalableMeshSchema/ScalableMeshHandler.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <ScalableMeshSchema/ScalableMeshSchemaCommon.h>
#include <ScalableMeshSchema/ExportMacros.h>
#include <ScalableMesh/IScalableMesh.h>
#include <ScalableMeshSchema/IMeshSpatialModel.h>
#include <TerrainModel/TerrainModel.h>
#include <ScalableMesh/IScalableMeshProgressiveQuery.h>
#include <ScalableMesh/IScalableMeshQuery.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/TileTree.h>
#include <DgnPlatform/MeshTile.h>
#include <DgnPlatform/Render.h>

SCALABLEMESH_SCHEMA_TYPEDEFS(ScalableMeshModel)

USING_NAMESPACE_BENTLEY_SCALABLEMESH

BEGIN_BENTLEY_SCALABLEMESH_SCHEMA_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(SMGeometry)
DEFINE_POINTER_SUFFIX_TYPEDEFS(SMNode)
DEFINE_POINTER_SUFFIX_TYPEDEFS(SMScene)

DEFINE_REF_COUNTED_PTR(SMGeometry)
DEFINE_REF_COUNTED_PTR(SMNode)
DEFINE_REF_COUNTED_PTR(SMScene)

class ScalableMeshDrawingInfo;

typedef RefCountedPtr<ScalableMeshDrawingInfo> ScalableMeshDrawingInfoPtr;

//=======================================================================================
// @bsistruct                                                   Mathieu.St-Pierre   07/17
//=======================================================================================
class ScalableMeshDrawingInfo : public RefCountedBase
{
private:
    DrawPurpose m_drawPurpose;
    DMatrix4d   m_localToViewTransformation;
    DRange3d    m_range;

public:
    int m_currentQuery;
    bvector<BentleyB0200::ScalableMesh::IScalableMeshCachedDisplayNodePtr> m_meshNodes;
    bvector<BentleyB0200::ScalableMesh::IScalableMeshCachedDisplayNodePtr> m_overviewNodes;

    ScalableMeshDrawingInfo(BentleyB0200::Dgn::ViewContextP viewContext)
        {
        //m_drawPurpose = viewContext->GetDrawPurpose();
        const DMatrix4d localToView(viewContext->GetViewport()->GetWorldToViewMap()->M0);
        memcpy(&m_localToViewTransformation, &localToView, sizeof(DMatrix4d));
        //m_range = viewContext->GetViewport()->GetViewCorners();
        }

    ~ScalableMeshDrawingInfo()
        {
        }

    DrawPurpose GetDrawPurpose()
        {
        return m_drawPurpose;
        }

    int GetViewNumber()
        {
        //NEEDS_WORK_SM : Default to 0
        return 0;
        }

    bool HasAppearanceChanged(const ScalableMeshDrawingInfoPtr& smDrawingInfoPtr)
        {
        return (0 != memcmp(&smDrawingInfoPtr->m_localToViewTransformation, &m_localToViewTransformation, sizeof(DMatrix4d))) ||
            (0 != memcmp(&smDrawingInfoPtr->m_range, &m_range, sizeof(DRange3d)));
        }

    const DMatrix4d& GetLocalToViewTransform() {return m_localToViewTransformation;}
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   07/17
//=======================================================================================
struct SMGeometry : Dgn::TileTree::TriMeshTree::TriMesh
{
    Dgn::Render::TexturePtr m_texture;

    SMGeometry() { }
    SMGeometry(CreateParams const& params, SMSceneR scene, Dgn::Render::SystemP renderSys);
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   07/17
//=======================================================================================
struct SMNode : Dgn::TileTree::TriMeshTree::Tile
{
    DEFINE_T_SUPER(Dgn::TileTree::TriMeshTree::Tile);
    friend struct SMScene;

    //=======================================================================================
    // @bsiclass                                                    Mathieu.Marchand  11/2016
    //=======================================================================================
    struct SMLoader : Dgn::TileTree::TileLoader
    {
        SMLoader(Dgn::TileTree::TileR tile, Dgn::TileTree::TileLoadStatePtr loads, Dgn::Render::SystemP renderSys);

        BentleyStatus _LoadTile() override
            {
            return static_cast<SMNodeR>(*m_tile).Read3SMTile(m_tileBytes, (SMSceneR)m_tile->GetRootR(), GetRenderSystem(), true);
            };

        folly::Future<BentleyStatus> _GetFromSource() override
            {
            //ScalableMesh has his own loader
            return SUCCESS;
            }
    };

private:
    IScalableMeshNodePtr m_scalableMeshNodePtr;

    bool ReadHeader(Transform& locationTransform);
    BentleyStatus Read3SMTile(Dgn::TileTree::StreamBuffer&, SMSceneR, Dgn::Render::SystemP renderSys, bool loadChildren);
    BentleyStatus DoRead(Dgn::TileTree::StreamBuffer& in, SMSceneR scene, Dgn::Render::SystemP renderSys, bool loadChildren);

    //! Called when tile data is required. The loader will be added to the IOPool and will execute asynchronously.
    Dgn::TileTree::TileLoaderPtr _CreateTileLoader(Dgn::TileTree::TileLoadStatePtr, Dgn::Render::SystemP renderSys) override;
    Utf8String _GetTileCacheKey() const override;
    bool _WantDebugRangeGraphics() const override;
    void _DrawGraphics(Dgn::TileTree::DrawArgsR args) const override;

public:
    SMNode(Dgn::TileTree::TriMeshTree::Root& root, SMNodeP parent, IScalableMeshNodePtr& smNodePtr) : T_Super(root, parent), m_scalableMeshNodePtr(smNodePtr) {}
    Utf8String GetFilePath(SMSceneR) const;

    bool _HasChildren() const override { return m_scalableMeshNodePtr->GetChildrenNodes().size() > 0 || !IsReady(); }
    ChildTiles const* _GetChildren(bool load) const override; 


    Dgn::ElementAlignedBox3d ComputeRange();
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   07/17
//=======================================================================================
struct SMScene : Dgn::TileTree::TriMeshTree::Root
{
    DEFINE_T_SUPER(Dgn::TileTree::TriMeshTree::Root);
    friend struct SMNode;

private:
    IScalableMeshPtr m_smPtr;
    Transform        m_toFloatTransform; 

    //SceneInfo   m_sceneInfo;
    BentleyStatus LocateFromSRS(); // compute location transform from spatial reference system in the sceneinfo
    Dgn::TileTree::TriMeshTree::TriMeshPtr _CreateGeometry(Dgn::TileTree::TriMeshTree::TriMesh::CreateParams const& args, Dgn::Render::SystemP renderSys) override { return new SMGeometry(args, *this, renderSys); }
    Utf8CP _GetName() const override { return "3SM"; }

public:
    SMScene(Dgn::DgnDbR db, IScalableMeshPtr& smPtr, TransformCR location, TransformCR toFloatTransform, Utf8CP sceneFile, Dgn::Render::SystemP system) : T_Super(db, location, sceneFile, system), m_smPtr(smPtr), m_toFloatTransform(toFloatTransform) {}

    ~SMScene() { ClearAllTiles(); }

    //SceneInfo const& GetSceneInfo() const { return m_sceneInfo; }
    BentleyStatus LoadNodeSynchronous(SMNodeR);
    BentleyStatus LoadScene(); // synchronous

    Transform GetToFloatTransform() { return m_toFloatTransform;}

    SCALABLEMESH_SCHEMA_EXPORT BentleyStatus ReadSceneFile(); //!< Read the scene file synchronously
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ScalableMeshModel : IMeshSpatialModel //, Dgn::Render::IGetPublishedTilesetInfo
{
    DGNMODEL_DECLARE_MEMBERS("ScalableMeshModel", IMeshSpatialModel)

    BE_JSON_NAME(scalablemesh)

private:
    SMSceneP Load(Dgn::Render::SystemP) const;
    //NEEDS_WORK_MS : Modify remove mutable
    mutable IScalableMeshPtr                m_smPtr;
    Transform                               m_smToModelUorTransform;
    Transform                               m_modelUorToSmTransform;
    mutable bool                            m_tryOpen;
    mutable BentleyApi::Dgn::AxisAlignedBox3d       m_range;

    mutable IScalableMeshDisplayCacheManagerPtr     m_displayNodesCache;
    mutable IScalableMeshProgressiveQueryEnginePtr  m_progressiveQueryEngine;
    mutable ScalableMeshDrawingInfoPtr              m_currentDrawingInfoPtr;
    mutable DMatrix4d                               m_storageToUorsTransfo;
    mutable bool m_forceRedraw;
    mutable bset<uint64_t>                          m_activeClips;

    BeFileName                              m_path;
    bool                                    m_isProgressiveDisplayOn;
    bool                                    m_isInsertingClips;

    IScalableMeshProgressiveQueryEnginePtr GetProgressiveQueryEngine();
    void MakeTileSubTree(Render::TileNodePtr& rootTile, IScalableMeshNodePtr& node, TransformCR transformDbToTile, size_t childIndex=0, Render::TileNode* parent=nullptr);
protected:
    struct Properties
    {
        Utf8String          m_fileId;

        void ToJson(Json::Value&) const;
        void FromJson(Json::Value const&);
    };

    Properties      m_properties;

    void _OnSaveJsonProperties() override;
    void _OnLoadedJsonProperties() override;

    virtual bool _IsMultiResolution() const { return true; };
    BentleyApi::Dgn::AxisAlignedBox3d _GetRange() const override;
    SCALABLEMESH_SCHEMA_EXPORT BentleyApi::Dgn::AxisAlignedBox3d _QueryModelRange() const override;

    BentleyStatus _QueryTexturesLod(bvector<ITerrainTexturePtr>& textures, size_t maxSizeBytes) const override;
    BentleyStatus _QueryTexture(ITextureTileId const& tileId, ITerrainTexturePtr& texture) const override;

    BentleyStatus _ReloadClipMask(const BentleyApi::Dgn::DgnElementId& clipMaskElementId, bool isNew) override;
    BentleyStatus _ReloadAllClipMasks() override;
    BentleyStatus _StartClipMaskBulkInsert() override;
    BentleyStatus _StopClipMaskBulkInsert() override;
    BentleyStatus _CreateIterator(ITerrainTileIteratorPtr& iterator) override;
    TerrainModel::IDTM* _GetDTM(ScalableMesh::DTMAnalysisType type) override;
    void _RegisterTilesChangedEventListener(ITerrainTileChangedHandler* eventListener) override;
    bool _UnregisterTilesChangedEventListener(ITerrainTileChangedHandler* eventListener) override;

    SCALABLEMESH_SCHEMA_EXPORT Dgn::TileTree::RootPtr _CreateTileTree(Dgn::Render::SystemP) override;
    SCALABLEMESH_SCHEMA_EXPORT void _PickTerrainGraphics(Dgn::PickContextR) const override;
    SCALABLEMESH_SCHEMA_EXPORT void _OnFitView(FitContextR context) override;
public:
    //Dgn::Render::PublishedTilesetInfo _GetPublishedTilesetInfo() override;

    //! Create a new TerrainPhysicalModel object, in preparation for loading it from the DgnDb.
    ScalableMeshModel(BentleyApi::Dgn::DgnModel::CreateParams const& params);

    virtual ~ScalableMeshModel();

    SCALABLEMESH_SCHEMA_EXPORT static ScalableMeshModelP CreateModel(BentleyApi::Dgn::DgnDbR dgnDb);

    void OpenFile(BeFileNameCR smFilename, DgnDbR dgnProject);

    void SetFileNameProperty(BeFileNameCR smFilename);

    BeFileName GetPath() const { return m_path; }

    //! A DgnDb can have only one terrain.
    SCALABLEMESH_SCHEMA_EXPORT static IMeshSpatialModelP GetTerrainModelP(BentleyApi::Dgn::DgnDbCR dgnDb);

    SCALABLEMESH_SCHEMA_EXPORT static void GetAllScalableMeshes(BentleyApi::Dgn::DgnDbCR dgnDb, bvector<IMeshSpatialModelP>& models);

    SCALABLEMESH_SCHEMA_EXPORT static WString GetTerrainModelPath(BentleyApi::Dgn::DgnDbCR dgnDb);

    SCALABLEMESH_SCHEMA_EXPORT IScalableMesh* GetScalableMesh();

    SCALABLEMESH_SCHEMA_EXPORT Transform GetUorsToStorage();

    SCALABLEMESH_SCHEMA_EXPORT void SetActiveClipSets(bset<uint64_t>& activeClips, bset<uint64_t>& previouslyActiveClips);

    SCALABLEMESH_SCHEMA_EXPORT void GetClipSetIds(bvector<uint64_t>& allShownIds);

    SCALABLEMESH_SCHEMA_EXPORT bool IsTerrain();

    SCALABLEMESH_SCHEMA_EXPORT void SetProgressiveDisplay(bool isProgressiveOn);

    SCALABLEMESH_SCHEMA_EXPORT void ClearOverviews(IScalableMeshPtr& targetSM);

    SCALABLEMESH_SCHEMA_EXPORT void LoadOverviews(IScalableMeshPtr& targetSM);
};

struct EXPORT_VTABLE_ATTRIBUTE ScalableMeshModelHandler : Dgn::dgn_ModelHandler::Spatial
{
    MODELHANDLER_DECLARE_MEMBERS("ScalableMeshModel", ScalableMeshModel, ScalableMeshModelHandler, Dgn::dgn_ModelHandler::Spatial, SCALABLEMESH_SCHEMA_EXPORT)
public :
    //NEEDS_WORK_SM : Currently for testing only
    SCALABLEMESH_SCHEMA_EXPORT static IMeshSpatialModelP AttachTerrainModel(DgnDb& db, Utf8StringCR modelName, BeFileNameCR smFilename, RepositoryLinkCR modeledElement, bool openFile = true);
};
END_BENTLEY_SCALABLEMESH_SCHEMA_NAMESPACE
