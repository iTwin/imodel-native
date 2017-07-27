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
#include <ScalableMesh\IScalableMeshProgressiveQuery.h>
#include <ScalableMesh\IScalableMeshQuery.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/TileTree.h>
#include <DgnPlatform/MeshTile.h>
#include <DgnPlatform\Render.h>
#include <forward_list>



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

class ScalableMeshDrawingInfo : public RefCountedBase
    {
private : 

    DrawPurpose m_drawPurpose;            
    DMatrix4d   m_localToViewTransformation;    
    DRange3d    m_range; 
    
public : 

    int m_currentQuery;
    bvector<BentleyB0200::ScalableMesh::IScalableMeshCachedDisplayNodePtr> m_meshNodes;
    bvector<BentleyB0200::ScalableMesh::IScalableMeshCachedDisplayNodePtr> m_overviewNodes;

public : 
        
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
//! A mesh and a Render::Graphic to draw it. Both are optional - we don't need the mesh except for picking, and sometimes we create Geometry objects for exporting (in which case we don't need the Graphic).
// @bsiclass                                                    Keith.Bentley   06/16
//=======================================================================================
struct SMGeometry : RefCountedBase, NonCopyableClass
{
protected:
    bvector<FPoint3d> m_points;
    bvector<FPoint3d> m_normals;
    bvector<FPoint2d> m_textureUV;
    bvector<int32_t> m_indices;
    Dgn::Render::GraphicPtr m_graphic;    

public:
    SMGeometry() {}
    SMGeometry(Dgn::Render::TriMeshArgs const& args, SMSceneR scene, Dgn::Render::SystemP renderSys);
    PolyfaceHeaderPtr GetPolyface() const;
    void GetGraphics(Dgn::TileTree::DrawArgsR);
    void Pick(Dgn::TileTree::PickArgsR);
    void ClearGraphic() { m_graphic = nullptr; }
    bvector<FPoint3d> const& GetPoints() const { return m_points; }
    bool IsEmpty() const { return m_points.empty(); }


    BentleyB0200::RefCountedPtr<BentleyB0200::Dgn::Render::Texture> m_texture;
};



struct SMNode : Dgn::TileTree::Tile
{
    DEFINE_T_SUPER(Dgn::TileTree::Tile);
    friend struct SMScene;
    typedef std::forward_list<SMGeometryPtr> GeometryList;

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

        virtual folly::Future<BentleyStatus> _GetFromSource() override
            {
            //ScalableMesh has his own loader
            return SUCCESS;
            }
    };

private:
    double m_maxDiameter; // maximum diameter
    double m_factor = 0.5;  // by default, 1/2 of diameter

    IScalableMeshNodePtr m_scalableMeshNodePtr;

    GeometryList m_geometry;
    //Utf8String m_childPath;     // this is the name of the file (relative to path of this node) to load the children of this node.

    bool ReadHeader(DPoint3d& centroid);
    BentleyStatus Read3SMTile(Dgn::TileTree::StreamBuffer&, SMSceneR, Dgn::Render::SystemP renderSys, bool loadChildren);
    BentleyStatus DoRead(Dgn::TileTree::StreamBuffer& in, SMSceneR scene, Dgn::Render::SystemP renderSys, bool loadChildren);

    //! Called when tile data is required. The loader will be added to the IOPool and will execute asynchronously.
    Dgn::TileTree::TileLoaderPtr _CreateTileLoader(Dgn::TileTree::TileLoadStatePtr, Dgn::Render::SystemP renderSys) override;

    void _DrawGraphics(Dgn::TileTree::DrawArgsR) const override;
    void _PickGraphics(Dgn::TileTree::PickArgsR args, int depth) const override;
    Utf8String _GetTileCacheKey() const override; 

public:
    SMNode(Dgn::TileTree::RootR root, SMNodeP parent, IScalableMeshNodePtr& smNodePtr) : Dgn::TileTree::Tile(root, parent), m_maxDiameter(0.0), m_scalableMeshNodePtr(smNodePtr) {}
    Utf8String GetFilePath(SMSceneR) const;

    bool _HasChildren() const override { return m_scalableMeshNodePtr->GetChildrenNodes().size() > 0; }
    void ClearGeometry() { m_geometry.clear(); }
    ChildTiles const* _GetChildren(bool load) const override { return IsReady() ? &m_children : nullptr; }
    double _GetMaximumSize() const override { return m_factor * m_maxDiameter; }
    void _OnChildrenUnloaded() const override { m_loadStatus.store(LoadStatus::NotLoaded); }
    void _UnloadChildren(BeTimePoint olderThan) const override { if (IsReady()) T_Super::_UnloadChildren(olderThan); }
    Dgn::ElementAlignedBox3d ComputeRange();
    GeometryList& GetGeometry() { return m_geometry; }
};

/*=================================================================================**//**
//! A 3mx scene, constructed for a single Render::System. The graphics held by this scene are only useful for that Render::System.
// @bsiclass                                                    Keith.Bentley   03/16
+===============+===============+===============+===============+===============+======*/
struct SMScene : Dgn::TileTree::Root
{
    DEFINE_T_SUPER(Dgn::TileTree::Root);
    friend struct SMNode;
    friend struct SMGeometry;    

private:

    IScalableMeshPtr m_smPtr;
    
    //SceneInfo   m_sceneInfo;
    BentleyStatus LocateFromSRS(); // compute location transform from spatial reference system in the sceneinfo
    virtual SMGeometryPtr _CreateGeometry(Dgn::Render::TriMeshArgs const& args, Dgn::Render::SystemP renderSys) { return new SMGeometry(args, *this, renderSys); }
    virtual Dgn::Render::TexturePtr _CreateTexture(Dgn::Render::ImageSourceCR source, Dgn::Render::Image::Format targetFormat, Dgn::Render::Image::BottomUp bottomUp, Dgn::Render::SystemP renderSys) const { return renderSys ? renderSys->_CreateTexture(source, bottomUp) : nullptr; }
    Utf8CP _GetName() const override { return "3SM"; }

public:
    SMScene(Dgn::DgnDbR db, IScalableMeshPtr& smPtr, TransformCR location, Utf8CP sceneFile, Dgn::Render::SystemP system) : T_Super(db, location, sceneFile, system), m_smPtr(smPtr) {}

    ~SMScene() { ClearAllTiles(); }

    //SceneInfo const& GetSceneInfo() const { return m_sceneInfo; }
    BentleyStatus LoadNodeSynchronous(SMNodeR);
    BentleyStatus LoadScene(); // synchronous

    SCALABLEMESH_SCHEMA_EXPORT BentleyStatus ReadSceneFile(); //!< Read the scene file synchronously
};

//=======================================================================================
// @bsiclass                                                  
//=======================================================================================
struct ScalableMeshModel : IMeshSpatialModel //, Dgn::Render::IGenerateMeshTiles
    {
        DGNMODEL_DECLARE_MEMBERS("ScalableMeshModel", IMeshSpatialModel)

        BE_JSON_NAME(scalablemesh)

    private:

        mutable SMScenePtr m_scene;

        void Load(Dgn::Render::SystemP) const;
        

        //NEEDS_WORK_MS : Modify remove mutable
        mutable IScalableMeshPtr                m_smPtr;
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

        virtual void _OnSaveJsonProperties() override;
        virtual void _OnLoadedJsonProperties() override;
             
        virtual bool _IsMultiResolution() const { return true; };
        virtual BentleyApi::Dgn::AxisAlignedBox3d _GetRange() const override;        

        virtual BentleyStatus _QueryTexturesLod(bvector<ITerrainTexturePtr>& textures, size_t maxSizeBytes) const override;
        virtual BentleyStatus _QueryTexture(ITextureTileId const& tileId, ITerrainTexturePtr& texture) const override;

        virtual BentleyStatus _ReloadClipMask(const BentleyApi::Dgn::DgnElementId& clipMaskElementId, bool isNew) override;        
        virtual BentleyStatus _ReloadAllClipMasks() override;
        virtual BentleyStatus _StartClipMaskBulkInsert() override;
        virtual BentleyStatus _StopClipMaskBulkInsert() override;
        virtual BentleyStatus _CreateIterator(ITerrainTileIteratorPtr& iterator) override;
        virtual TerrainModel::IDTM* _GetDTM(ScalableMesh::DTMAnalysisType type) override;
        virtual void _RegisterTilesChangedEventListener(ITerrainTileChangedHandler* eventListener) override;
        virtual bool _UnregisterTilesChangedEventListener(ITerrainTileChangedHandler* eventListener) override;
        
        SCALABLEMESH_SCHEMA_EXPORT void _PickTerrainGraphics(Dgn::PickContextR) const override;
        SCALABLEMESH_SCHEMA_EXPORT void _OnFitView(FitContextR context) override;
                        
        
    public:

        //virtual TileGeneratorStatus _GenerateMeshTiles(TileNodePtr& rootTile, TransformCR transformDbToTile, double leafTolerance, TileGenerator::ITileCollector& collector, ITileGenerationProgressMonitorR progressMeter) override;

        //! Create a new TerrainPhysicalModel object, in preparation for loading it from the DgnDb.
        ScalableMeshModel(BentleyApi::Dgn::DgnModel::CreateParams const& params);

        virtual ~ScalableMeshModel();

        SCALABLEMESH_SCHEMA_EXPORT static ScalableMeshModelP CreateModel(BentleyApi::Dgn::DgnDbR dgnDb);
                
        void OpenFile(BeFileNameCR smFilename, DgnDbR dgnProject);        

        void SetFileNameProperty(BeFileNameCR smFilename);
		
        SCALABLEMESH_SCHEMA_EXPORT BeFileName GetPath();

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
