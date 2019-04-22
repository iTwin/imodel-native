/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
#include <ScalableMesh/IScalableMeshProgressiveQuery.h>
#include <ScalableMesh/IScalableMeshQuery.h>
#include <ScalableMesh/IScalableMeshPublisher.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/Render.h>
#include <DgnPlatform/ModelSpatialClassifier.h>


//#define ACTIVATE_MOBILE_CODE_ON_DESKTOP 1

SCALABLEMESH_SCHEMA_TYPEDEFS(ScalableMeshModel)

USING_NAMESPACE_BENTLEY_SCALABLEMESH

BEGIN_BENTLEY_SCALABLEMESH_SCHEMA_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(SMGeometry)
DEFINE_POINTER_SUFFIX_TYPEDEFS(SMNode)
DEFINE_POINTER_SUFFIX_TYPEDEFS(SMScene)

DEFINE_REF_COUNTED_PTR(SMGeometry)
DEFINE_REF_COUNTED_PTR(SMNode)
DEFINE_REF_COUNTED_PTR(SMScene)

struct ScalableMeshModel;

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

        virtual BentleyStatus _GetExtraFileDirectory(BeFileNameR extraFileDir, BentleyApi::Dgn::DgnDbCR dgnDb) const = 0;

    public:

        SCALABLEMESH_SCHEMA_EXPORT BentleyStatus GetExtraFileDirectory(BeFileNameR extraFileDir, BentleyApi::Dgn::DgnDbCR dgnDb) const;
    };

typedef RefCountedPtr<IScalableMeshLocationProvider> IScalableMeshLocationProviderPtr;


//=======================================================================================
// @bsiclass
//=======================================================================================
struct SMModelClipInfo
    {    

    SMModelClipInfo(const bvector<DPoint3d>& shape, ScalableMesh::SMNonDestructiveClipType type)
        {
        m_shape = shape;
        m_type = type;
        m_isActive = true;
        m_geomType = 0;
        }

    SMModelClipInfo()
        {
        m_type = SMNonDestructiveClipType::Mask;       
        m_isActive = true;
        m_geomType = 0;
        }

    void FromBlob(size_t& currentBlobInd, const uint8_t* pClipData);

    void ToBlob(bvector<uint8_t>& clipData);

    bvector<DPoint3d>                      m_shape;
    ScalableMesh::SMNonDestructiveClipType m_type;
    bool                                   m_isActive;
    uint32_t                               m_geomType;
    };

struct SMModelClipVectorInfo
{

	SMModelClipVectorInfo(const CLIP_VECTOR_NAMESPACE::ClipVectorPtr& clip, ScalableMesh::SMNonDestructiveClipType type)
	{
		m_bounds = ClipVector::Create();
		*m_bounds = *clip;
		m_type = type;
		m_isActive = true;
		m_geomType = 0;
	}

	SMModelClipVectorInfo()
	{
		m_type = SMNonDestructiveClipType::Mask;
		m_isActive = true;
		m_geomType = 0;
	}

	void FromBlob(size_t& currentBlobInd, const uint8_t* pClipData);

	void ToBlob(bvector<uint8_t>& clipData);

	CLIP_VECTOR_NAMESPACE::ClipVectorPtr                    m_bounds;
	ScalableMesh::SMNonDestructiveClipType m_type;
	bool                                   m_isActive;
	uint32_t                               m_geomType;
};


//=======================================================================================
// @bsiclass
//=======================================================================================
struct SMClipProvider : public ScalableMesh::IClipDefinitionDataProvider
    {
    private:

        ScalableMeshModel* m_smModel = nullptr;

    public:

        SMClipProvider(ScalableMeshModel* smModel);
        virtual void GetClipPolygon(bvector<DPoint3d>& poly, uint64_t id);
        virtual void GetClipPolygon(bvector<DPoint3d>& poly, uint64_t id, ScalableMesh::SMNonDestructiveClipType& type);
        virtual void SetClipPolygon(const bvector<DPoint3d>& poly, uint64_t id, ScalableMesh::SMNonDestructiveClipType type);
        virtual void SetClipPolygon(const bvector<DPoint3d>& poly, uint64_t id);
		virtual void GetClipVector(CLIP_VECTOR_NAMESPACE::ClipVectorPtr& poly, uint64_t id, SMNonDestructiveClipType& type);
		virtual void SetClipVector(const CLIP_VECTOR_NAMESPACE::ClipVectorPtr& poly, uint64_t id, SMNonDestructiveClipType type);
        virtual void RemoveClipPolygon(uint64_t id);

        virtual void RemoveTerrainRegion(uint64_t id);
        virtual void GetTerrainRegion(bvector<DPoint3d>& poly, uint64_t id);
        virtual void SetTerrainRegion(const bvector<DPoint3d>& poly, uint64_t id);

        virtual void ListClipIDs(bvector<uint64_t>& ids);
        virtual void ListTerrainRegionIDs(bvector<uint64_t>& ids);

        virtual void SetTerrainRegionName(const Utf8String& name, uint64_t id);
        virtual void GetTerrainRegionName(Utf8String& name, uint64_t id);
        virtual void RemoveTerrainRegionName(uint64_t id) {};
    };


//=======================================================================================
// @bsiclass
//=======================================================================================
struct ScalableMeshModel : IMeshSpatialModel
{
    DGNMODEL_DECLARE_MEMBERS("ScalableMeshModel", IMeshSpatialModel)

    BE_JSON_NAME(scalablemesh)
    BE_JSON_NAME(clip)
    BE_JSON_NAME(classifiers)
    BE_JSON_NAME(publishing)
    BE_JSON_NAME(tilesetUrl)

    friend struct SMClipProvider;

private:

    RefCountedPtr<SMClipProvider> m_clipProvider;


public: //MST_TEMP
    static IScalableMeshLocationProviderPtr m_locationProviderPtr;

    SMSceneP Load(Dgn::Render::SystemP) const;
    //NEEDS_WORK_MS : Modify remove mutable
    mutable IScalableMeshPtr                m_smPtr;
    Transform                               m_smToModelUorTransform;
    Transform                               m_modelUorToSmTransform;
    mutable bool                            m_tryOpen;
    mutable BentleyApi::AxisAlignedBox3d    m_range;
    mutable bool                            m_displayTexture;       
	mutable IScalableMeshTextureInfoPtr     m_textureInfo;            
    mutable bool                            m_forceRedraw;
    mutable bset<uint64_t>                  m_activeClips;
    mutable bset<uint64_t>                  m_notActiveClips;
    BeFileName                              m_path;
    bool                                    m_isProgressiveDisplayOn;
    bool                                    m_isInsertingClips;
	mutable Dgn::ClipVectorCPtr             m_clip;
	int                                     m_startClipCount;
    ModelSpatialClassifiers                 m_classifiers;
    bvector<ScalableMeshModel*>             m_terrainParts;
    bmap<uint64_t, bpair<ClipMode, bool>>   m_currentClips;
    
    bool                                    m_subModel;
    ScalableMeshModel*                      m_parentModel;
    uint64_t                                m_associatedRegion;

    bool                                    m_loadedAllModels;
    BeFileName                              m_basePath;

    struct QueuedRegionOp
        {
        uint64_t id;
        bvector<DPoint3d> regionData;
        };

    bvector<QueuedRegionOp> m_queuedRegions;

    void Cleanup(bool isModelDelete);

    IScalableMeshProgressiveQueryEnginePtr GetProgressiveQueryEngine(); 
    void InitializeTerrainRegions(/*Dgn::ViewContextR*/);
		
	bool HasClipBoundary(const bvector<DPoint3d>& clipBoundary, uint64_t clipID);

private:
		    
    uint32_t _GetExcessiveRefCountThreshold() const override { return 0x7fffffff; }

protected:
    struct Properties
    {
        Utf8String          m_fileId;

        void ToJson(Json::Value&) const;
        void FromJson(Json::Value const&);
    };

    bmap <uint64_t, SMModelClipInfo>   m_scalableClipDefs;
	bmap <uint64_t, SMModelClipVectorInfo>   m_scalableClipVectorDefs;

    //bpair<Model id, clip id>
    bvector<bpair<uint64_t, uint64_t>> m_linksToGroundModels;

    Properties      m_properties;

    Dgn::DgnDbStatus _ReadSelectParams(BeSQLite::EC::ECSqlStatement& statement, Dgn::ECSqlClassParamsCR params) override;
    void _BindWriteParams(BeSQLite::EC::ECSqlStatement&, ForInsert) override;

    void _OnSaveJsonProperties() override;
    void _OnLoadedJsonProperties() override;

    virtual bool _IsMultiResolution() const override { return true; };
    BentleyApi::AxisAlignedBox3d _GetRange() const override;
    AxisAlignedBox3d _QueryNonElementModelRange() const override { return _QueryElementsRange(); }


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
    BentleyStatus _GetSpatialClassifiers(Dgn::ModelSpatialClassifiersR classifiers) const override { classifiers = m_classifiers; return SUCCESS; }
        virtual BentleyApi::Dgn::DgnDbStatus _OnDelete() override;
        void RefreshClips();

    BeFileName GenerateClipFileName(BeFileNameCR smFilename, BentleyApi::Dgn::DgnDbR dgnProject);

public:

    SCALABLEMESH_SCHEMA_EXPORT static BentleyStatus SetLocationProvider(IScalableMeshLocationProvider& locationProviderPtr);

    //! Create a new TerrainPhysicalModel object, in preparation for loading it from the DgnDb.
    ScalableMeshModel(BentleyApi::Dgn::DgnModel::CreateParams const& params);

    virtual ~ScalableMeshModel();

    SCALABLEMESH_SCHEMA_EXPORT static ScalableMeshModelP CreateModel(BentleyApi::Dgn::DgnDbR dgnDb);

    SCALABLEMESH_SCHEMA_EXPORT static ScalableMeshModelP CreateModel(BentleyApi::Dgn::DgnDbR dgnDb, WString terrainName, BeFileName terrainPath);
    void OpenFile(BeFileNameCR smFilename, BentleyApi::Dgn::DgnDbR dgnProject);

    SCALABLEMESH_SCHEMA_EXPORT void CloseFile();
    void SetFileNameProperty(BeFileNameCR smFilename);
    SCALABLEMESH_SCHEMA_EXPORT BentleyStatus UpdateFilename(BeFileNameCR newFilename);

    SCALABLEMESH_SCHEMA_EXPORT BentleyStatus UpdateExtractedTerrainLocation(BeFileNameCR oldLocation, BeFileNameCR newLocation);
    BeFileName GetPath() const { return m_path; }

	SCALABLEMESH_SCHEMA_EXPORT void ClearExtraFiles();
    void SetClassifiers(Dgn::ModelSpatialClassifiersCR classifiers) { m_classifiers = classifiers; }
	SCALABLEMESH_SCHEMA_EXPORT void CompactExtraFiles();
		
	SCALABLEMESH_SCHEMA_EXPORT void SetClip(Dgn::ClipVectorCP clip);
		
	SCALABLEMESH_SCHEMA_EXPORT void SetScalableClips(bmap <uint64_t, SMModelClipInfo>& clipInfo);

    SCALABLEMESH_SCHEMA_EXPORT void SetGroundModelLinks(bvector<bpair<uint64_t, uint64_t>>& linksToGroundModels);

    //! A DgnDb can have only one terrain.
    SCALABLEMESH_SCHEMA_EXPORT static IMeshSpatialModelP GetTerrainModelP(BentleyApi::Dgn::DgnDbCR dgnDb);

    SCALABLEMESH_SCHEMA_EXPORT static void GetAllScalableMeshes(BentleyApi::Dgn::DgnDbCR dgnDb, bvector<IMeshSpatialModelP>& models);

    SCALABLEMESH_SCHEMA_EXPORT static void GetScalableMeshTypes(BentleyApi::Dgn::DgnDbCR dgnDb, bool& has3D, bool& hasTerrain, bool& hasExtractedTerrain, bool& hasCesium3DTiles);
		
    SCALABLEMESH_SCHEMA_EXPORT static WString GetTerrainModelPath(BentleyApi::Dgn::DgnDbCR dgnDb, bool createDir = true);

    SCALABLEMESH_SCHEMA_EXPORT IScalableMesh* GetScalableMesh(bool wantGroup = true);        

    SCALABLEMESH_SCHEMA_EXPORT Transform GetUorsToStorage();

    SCALABLEMESH_SCHEMA_EXPORT void SetActiveClipSets(bset<uint64_t>& activeClips, bset<uint64_t>& previouslyActiveClips);

    SCALABLEMESH_SCHEMA_EXPORT void ClearOverviews(IScalableMeshPtr& targetSM);

    SCALABLEMESH_SCHEMA_EXPORT void LoadOverviews(IScalableMeshPtr& targetSM);

    SCALABLEMESH_SCHEMA_EXPORT void GetClipSetIds(bvector<uint64_t>& allShownIds);

    SCALABLEMESH_SCHEMA_EXPORT void GetActiveClipSetIds(bset<uint64_t>& allShownIds);

    SCALABLEMESH_SCHEMA_EXPORT bool IsTerrain();

    SCALABLEMESH_SCHEMA_EXPORT bool HasTerrain();

    SCALABLEMESH_SCHEMA_EXPORT void SetProgressiveDisplay(bool isProgressiveOn);  

    SCALABLEMESH_SCHEMA_EXPORT void SetDisplayTexture(bool displayTexture);
    
    SCALABLEMESH_SCHEMA_EXPORT bool GetDisplayTexture() const { return m_displayTexture; }
    
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

    SCALABLEMESH_SCHEMA_EXPORT void WriteCesiumTileset(BeFileName outFileName, BeFileNameCR outputDir, const Transform& dbToECEF) const;
    SCALABLEMESH_SCHEMA_EXPORT bool AllowPublishing() const;

    uint64_t GetAssociatedRegionId() const { return m_associatedRegion; }
};

struct EXPORT_VTABLE_ATTRIBUTE ScalableMeshModelHandler : Dgn::dgn_ModelHandler::Spatial
{
    MODELHANDLER_DECLARE_MEMBERS("ScalableMeshModel", ScalableMeshModel, ScalableMeshModelHandler, Dgn::dgn_ModelHandler::Spatial, SCALABLEMESH_SCHEMA_EXPORT)
public :
    //NEEDS_WORK_SM : Currently for testing only
    SCALABLEMESH_SCHEMA_EXPORT static IMeshSpatialModelP AttachTerrainModel(BentleyApi::Dgn::DgnDb& db, Utf8StringCR modelName, BeFileNameCR smFilename, RepositoryLinkCR modeledElement, bool openFile = true, Dgn::ClipVectorCP clip = nullptr, ModelSpatialClassifiersCP classifiers = nullptr);

    virtual void _GetClassParams(Dgn::ECSqlClassParamsR params) override;
};
END_BENTLEY_SCALABLEMESH_SCHEMA_NAMESPACE
