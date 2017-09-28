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


#include <ScalableMesh/ScalableMeshDefs.h>
#include <ScalableMesh/IScalableMesh.h>

#include <ScalableMeshSchema/IMeshSpatialModel.h>

#include <TerrainModel/TerrainModel.h>
#include <ScalableMesh\IScalableMeshProgressiveQuery.h>
#include <ScalableMesh\IScalableMeshQuery.h>


SCALABLEMESH_SCHEMA_TYPEDEFS(ScalableMeshModel)



USING_NAMESPACE_BENTLEY_SCALABLEMESH



BEGIN_BENTLEY_SCALABLEMESH_SCHEMA_NAMESPACE

class ScalableMeshDrawingInfo;

typedef RefCountedPtr<ScalableMeshDrawingInfo> ScalableMeshDrawingInfoPtr;     

class ScalableMeshDrawingInfo : public RefCountedBase
    {
private : 

    DrawPurpose m_drawPurpose;            
    DMatrix4d   m_localToViewTransformation;    
    DPoint3d    m_low;
    DPoint3d    m_high;
public : 

    int m_currentQuery;
    int m_terrainQuery;
    bvector<ClipVectorPtr> m_coverageClips;
    bool m_hasCoverage;
    bvector<BentleyG06::ScalableMesh::IScalableMeshCachedDisplayNodePtr> m_meshNodes;
    bvector<BentleyG06::ScalableMesh::IScalableMeshCachedDisplayNodePtr> m_overviewNodes;
    bvector<BentleyG06::ScalableMesh::IScalableMeshCachedDisplayNodePtr> m_terrainMeshNodes;
    bvector<BentleyG06::ScalableMesh::IScalableMeshCachedDisplayNodePtr> m_terrainOverviewNodes;
    BentleyG06::ScalableMesh::IScalableMeshPtr m_smPtr;
public : 
        
    ScalableMeshDrawingInfo(ViewContextP viewContext)        
        {
        m_drawPurpose = viewContext->GetDrawPurpose();   
        const DMatrix4d localToView(viewContext->GetLocalToView());     
        memcpy(&m_localToViewTransformation, &localToView, sizeof(DMatrix4d));                
        viewContext->GetViewport()->GetViewCorners(m_low, m_high);
        m_hasCoverage = false;
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
               (0 != memcmp(&smDrawingInfoPtr->m_low, &m_low, sizeof(DPoint3d))) ||
               (0 != memcmp(&smDrawingInfoPtr->m_high, &m_high, sizeof(DPoint3d))) || m_hasCoverage != smDrawingInfoPtr->m_hasCoverage
               || m_coverageClips != smDrawingInfoPtr->m_coverageClips;
        }

    const DMatrix4d& GetLocalToViewTransform() {return m_localToViewTransformation;}   
    };

enum class ClipMode
    {
    Clip = 0,
    Mask
    };


//=======================================================================================
// @bsiclass                                                  
//=======================================================================================
struct IScalableMeshLocationProvider : public RefCountedBase
    {
    protected:

        virtual BentleyStatus _GetExtraFileDirectory(BeFileNameR extraFileDir, DgnDbCR dgnDb) const = 0;

    public:

        SCALABLEMESH_SCHEMA_EXPORT BentleyStatus GetExtraFileDirectory(BeFileNameR extraFileDir, DgnDbCR dgnDb) const;
    };

typedef RefCountedPtr<IScalableMeshLocationProvider> IScalableMeshLocationProviderPtr;

//=======================================================================================
// @bsiclass                                                  
//=======================================================================================
struct ScalableMeshModel : IMeshSpatialModel
    {
        DGNMODEL_DECLARE_MEMBERS("ScalableMeshModel", IMeshSpatialModel)



    private:

        static IScalableMeshLocationProviderPtr m_locationProviderPtr;
    
        
        IScalableMeshPtr                        m_smPtr;
        Transform                               m_smToModelUorTransform;
        Transform                               m_modelUorToSmTransform;
        bool                                    m_tryOpen;        
        bool                                    m_displayTexture;       
        bool                                    m_isUsingBingMap; 
                
        
        IScalableMeshDisplayCacheManagerPtr     m_displayNodesCache;
        IScalableMeshProgressiveQueryEnginePtr  m_progressiveQueryEngine;        
        ScalableMeshDrawingInfoPtr              m_currentDrawingInfoPtr;
        DMatrix4d                               m_storageToUorsTransfo; 
        bool m_forceRedraw;
        bset<uint64_t>                          m_activeClips;
        bset<uint64_t>                          m_notActiveClips;

        BeFileName                              m_path;
        bool                                    m_isProgressiveDisplayOn;    
        bool                                    m_isInsertingClips;
        int                                     m_startClipCount;

        bvector<ScalableMeshModel*>             m_terrainParts;
        bmap<uint64_t, bpair<ClipMode, bool>>   m_currentClips;
        
        bool  m_subModel;
        ScalableMeshModel* m_parentModel;
        uint64_t m_associatedRegion;

        bool m_loadedAllModels;
        BeFileName m_basePath;

        struct QueuedRegionOp
            {
            uint64_t id;
            bvector<DPoint3d> regionData;
            };

        bvector<QueuedRegionOp> m_queuedRegions;

        void Cleanup(bool isModelDelete);

        void DrawBingLogo(ViewContextR context, Byte const* pBitmapRGBA, DPoint2d const& bitmapSize);

        IScalableMeshProgressiveQueryEnginePtr GetProgressiveQueryEngine();
        
		void InitializeTerrainRegions(Dgn::ViewContextR);
		
		bool HasClipBoundary(const bvector<DPoint3d>& clipBoundary, uint64_t clipID);

           
    protected:

        struct Properties
            {
            Utf8String          m_fileId;                

            void ToJson(Json::Value&) const;
            void FromJson(Json::Value const&);
            };

        Properties      m_properties;

        virtual void _WriteJsonProperties(Json::Value&) const override;
        virtual void _ReadJsonProperties(Json::Value const&) override;
     
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


        virtual DgnDbStatus _OnDelete() override;


        SCALABLEMESH_SCHEMA_EXPORT virtual void _AddGraphicsToScene(BentleyApi::Dgn::ViewContextR context) override;

        void RefreshClips();

        BeFileName GenerateClipFileName(BeFileNameCR smFilename, DgnDbR dgnProject);


    public:

        SCALABLEMESH_SCHEMA_EXPORT static BentleyStatus SetLocationProvider(IScalableMeshLocationProvider& locationProviderPtr);

        //! Create a new TerrainPhysicalModel object, in preparation for loading it from the DgnDb.
        ScalableMeshModel(BentleyApi::Dgn::DgnModel::CreateParams const& params);

        virtual ~ScalableMeshModel();

        SCALABLEMESH_SCHEMA_EXPORT static ScalableMeshModelP CreateModel(BentleyApi::Dgn::DgnDbR dgnDb);

        SCALABLEMESH_SCHEMA_EXPORT static ScalableMeshModelP CreateModel(BentleyApi::Dgn::DgnDbR dgnDb, WString terrainName, BeFileName terrainPath);
                
        SCALABLEMESH_SCHEMA_EXPORT void OpenFile(BeFileNameCR smFilename, BentleyApi::Dgn::DgnDbR dgnProject);

        SCALABLEMESH_SCHEMA_EXPORT void CloseFile();

        SCALABLEMESH_SCHEMA_EXPORT BentleyStatus UpdateFilename(BeFileNameCR newFilename);

        SCALABLEMESH_SCHEMA_EXPORT BentleyStatus UpdateExtractedTerrainLocation(BeFileNameCR oldLocation, BeFileNameCR newLocation);

        SCALABLEMESH_SCHEMA_EXPORT BeFileName GetPath();

		SCALABLEMESH_SCHEMA_EXPORT void ClearExtraFiles();

		SCALABLEMESH_SCHEMA_EXPORT void CompactExtraFiles();


        //! A DgnDb can have only one terrain. 
        SCALABLEMESH_SCHEMA_EXPORT static IMeshSpatialModelP GetTerrainModelP(BentleyApi::Dgn::DgnDbCR dgnDb);

        SCALABLEMESH_SCHEMA_EXPORT static void GetAllScalableMeshes(BentleyApi::Dgn::DgnDbCR dgnDb, bvector<IMeshSpatialModelP>& models);        

        SCALABLEMESH_SCHEMA_EXPORT static void GetScalableMeshTypes(BentleyApi::Dgn::DgnDbCR dgnDb, bool& has3D, bool& hasTerrain, bool& hasExtractedTerrain, bool& hasCesium3DTiles);
        
        SCALABLEMESH_SCHEMA_EXPORT static WString GetTerrainModelPath(BentleyApi::Dgn::DgnDbCR dgnDb);

        SCALABLEMESH_SCHEMA_EXPORT IScalableMesh* GetScalableMesh(bool wantGroup = true);        

        SCALABLEMESH_SCHEMA_EXPORT void SetActiveClipSets(bset<uint64_t>& activeClips, bset<uint64_t>& previouslyActiveClips);

        SCALABLEMESH_SCHEMA_EXPORT void ClearOverviews(IScalableMeshPtr& targetSM);

        SCALABLEMESH_SCHEMA_EXPORT void LoadOverviews(IScalableMeshPtr& targetSM);

        SCALABLEMESH_SCHEMA_EXPORT void GetClipSetIds(bvector<uint64_t>& allShownIds);

        SCALABLEMESH_SCHEMA_EXPORT void GetActiveClipSetIds(bset<uint64_t>& allShownIds);

        SCALABLEMESH_SCHEMA_EXPORT bool IsTerrain();

        SCALABLEMESH_SCHEMA_EXPORT bool HasTerrain();

        SCALABLEMESH_SCHEMA_EXPORT void SetProgressiveDisplay(bool isProgressiveOn);  

        SCALABLEMESH_SCHEMA_EXPORT void SetDisplayTexture(bool displayTexture);
        
        SCALABLEMESH_SCHEMA_EXPORT void ClearAllDisplayMem();
                
        SCALABLEMESH_SCHEMA_EXPORT void ReloadMesh(); // force to reload the entire mesh data
        
        IScalableMesh* GetScalableMeshHandle();

        SCALABLEMESH_SCHEMA_EXPORT void ActivateClip(uint64_t id, ClipMode clip = ClipMode::Mask);

        SCALABLEMESH_SCHEMA_EXPORT void DeactivateClip(uint64_t clipId);

        SCALABLEMESH_SCHEMA_EXPORT void SetDefaultClipsActive();

        SCALABLEMESH_SCHEMA_EXPORT void AddTerrainRegion(uint64_t id, ScalableMeshModel* terrainModel, const bvector<DPoint3d> region);

        SCALABLEMESH_SCHEMA_EXPORT void FindTerrainRegion(uint64_t id, ScalableMeshModel*& terrainModel);

        SCALABLEMESH_SCHEMA_EXPORT void RemoveRegion(uint64_t id);

        SCALABLEMESH_SCHEMA_EXPORT void SetRegionVisibility(uint64_t id, bool isVisible);

        SCALABLEMESH_SCHEMA_EXPORT void GetPathForTerrainRegion(BeFileNameR terrainName, uint64_t id, const WString& basePath);

        SCALABLEMESH_SCHEMA_EXPORT bool HasQueuedTerrainRegions();

        SCALABLEMESH_SCHEMA_EXPORT void SyncTerrainRegions(bvector<uint64_t>& newModelIds);

        SCALABLEMESH_SCHEMA_EXPORT void QueueDeleteTerrainRegions(uint64_t id);

        SCALABLEMESH_SCHEMA_EXPORT void QueueAddTerrainRegions(uint64_t id, const bvector<DPoint3d>& boundary);

		SCALABLEMESH_SCHEMA_EXPORT void ActivateTerrainRegion(const BentleyApi::Dgn::DgnElementId& id, ScalableMeshModel* terrainModel);

		SCALABLEMESH_SCHEMA_EXPORT void UnlinkTerrainRegion(const BentleyApi::Dgn::DgnElementId& blanketId, const BentleyApi::Dgn::DgnModelId& modelId);

		SCALABLEMESH_SCHEMA_EXPORT void LinkTerrainRegion(const BentleyApi::Dgn::DgnElementId& blanketId, const BentleyApi::Dgn::DgnModelId& modelId, const bvector<DPoint3d> region, const Utf8String& blanketName);

        SCALABLEMESH_SCHEMA_EXPORT void CreateBreaklines(const BeFileName& extraLinearFeatureAbsFileName, bvector<DSegment3d> const& breaklines);

        uint64_t GetAssociatedRegionId() const { return m_associatedRegion; }

    };

struct EXPORT_VTABLE_ATTRIBUTE ScalableMeshModelHandler : Dgn::dgn_ModelHandler::Spatial
    {
    MODELHANDLER_DECLARE_MEMBERS("ScalableMeshModel", ScalableMeshModel, ScalableMeshModelHandler, Dgn::dgn_ModelHandler::Spatial, SCALABLEMESH_SCHEMA_EXPORT)

    public : 
                     
        //NEEDS_WORK_SM : Currently for testing only
        SCALABLEMESH_SCHEMA_EXPORT static IMeshSpatialModelP AttachTerrainModel(BentleyApi::Dgn::DgnDb& db, Utf8StringCR modelName, BeFileNameCR smFilename);
    };
END_BENTLEY_SCALABLEMESH_SCHEMA_NAMESPACE

