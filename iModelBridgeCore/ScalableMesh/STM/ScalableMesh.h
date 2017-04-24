/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMesh.h $
|    $RCSfile: ScalableMesh.h,v $
|   $Revision: 1.54 $
|       $Date: 2012/01/06 16:30:13 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
|                                                                        |
|    ScalableMeshNewFileCreator.h                              (C) Copyright 2001.        |
|                                                BCIVIL Corporation.        |
|                                                All Rights Reserved.    |
|                                                                       |
+----------------------------------------------------------------------*/

#pragma once

/*__PUBLISH_SECTION_START__*/
#include <ScalableMesh/IScalableMesh.h>
#include <ScalableMesh/IScalableMeshClipContainer.h>
#include "ScalableMeshDraping.h"

/*----------------------------------------------------------------------+
| This template class must be exported, so we instanciate and export it |
+----------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
| CLASS definitions                                                     |
+----------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
| Core DTM Methods                                                      |
+----------------------------------------------------------------------*/

#include "ScalableMeshCoreDefs.h"
#include "ScalableMeshCoreFns.h"


USING_NAMESPACE_BENTLEY_TERRAINMODEL

/*__PUBLISH_SECTION_END__*/

//#include "SMPointIndex.h"
//#include "SMMeshIndex.h"


#include "ScalableMeshMemoryPools.h"

#include "Stores/SMSQLiteStore.h"

#include "ScalableMeshVolume.h"

#include <CloudDataSource/DataSourceManager.h>

//extern DataSourceManager s_dataSourceManager;


/*__PUBLISH_SECTION_START__*/
using namespace BENTLEY_NAMESPACE_NAME::GeoCoordinates;


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

namespace GeoCoords
    {
    struct GCS;
    };

extern bool s_useSQLFormat;
//typedef ISMStore::Extent3d64f        Extent3dType;
typedef DRange3d Extent3dType;
typedef HGF3DExtent<double> YProtFeatureExtentType;

/*----------------------------------------------------------------------------+
|Class ScalableMeshBase
+----------------------------------------------------------------------------*/
struct ScalableMeshBase : public RefCounted<IScalableMesh>
    {

    protected:
/*__PUBLISH_SECTION_END__*/

    SMSQLiteFilePtr                     m_smSQLitePtr;
    bool                                m_isDgnDB;

    size_t                              m_workingLayer;

    GeoCoords::GCS                      m_sourceGCS;
    DRange3d                            m_contentExtent;

    WString                             m_baseExtraFilesPath;

    DataSourceAccount               *   m_dataSourceAccount;

    // NOTE: Stored in order to make it possible for the creator to use this. Remove when creator does not depends on
    // this interface anymore (take only a path).
    const WString                        m_path; 


    explicit                            ScalableMeshBase(SMSQLiteFilePtr& smSQLiteFile, const WString&             filePath);


/*__PUBLISH_SECTION_START__*/

    virtual                             ~ScalableMeshBase                 () = 0;


    bool                                LoadGCSFrom(WString wktStr);
    bool                                LoadGCSFrom();


public:

    const SMSQLiteFilePtr&              GetDbFile() const;

    const WChar*                        GetPath                 () const;

    static DataSourceManager &          GetDataSourceManager    (void)                                  {return *DataSourceManager::Get();}
    void                                SetDataSourceAccount    (DataSourceAccount *dataSourceAccount)  {m_dataSourceAccount = dataSourceAccount;}
    DataSourceAccount *                 GetDataSourceAccount    (void) const                            {return m_dataSourceAccount;}
   
    };


class ScalableMeshDTM : public RefCounted<BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM>
    {
    ScalableMeshDraping* m_draping;
    IScalableMesh* m_scMesh;
    Transform m_transformToUors;
    IDTMVolume* m_dtmVolume;
    TerrainModel::BcDTMPtr m_dtm; //Maximum 5M points bcDtm representation of a ScalableMesh
    bool                   m_tryCreateDtm; 


    protected:

    virtual IDTMDrapingP     _GetDTMDraping() override;
    virtual IDTMDrainageP    _GetDTMDrainage() override;
    virtual IDTMContouringP  _GetDTMContouring() override;
    virtual int64_t _GetPointCount() override;
    virtual DTMStatusInt _GetRange(DRange3dR range) override;
    virtual BcDTMP _GetBcDTM() override;
    virtual DTMStatusInt _GetBoundary(DTMPointArray& ret) override;
    virtual DTMStatusInt _CalculateSlopeArea(double& flatArea, double& slopeArea, DPoint3dCP pts, int numPoints) override;
    virtual DTMStatusInt _CalculateSlopeArea(double& flatArea, double& slopeArea, DPoint3dCP pts, int numPoints, DTMAreaValuesCallback progressiveCallback, DTMCancelProcessCallback isCancelledCallback) override;
    virtual DTMStatusInt _GetTransformDTM(DTMPtr& transformedDTM, TransformCR transformation) override;
    virtual bool _GetTransformation(TransformR transformation) override;
    virtual IDTMVolumeP _GetDTMVolume() override;

    virtual DTMStatusInt _ExportToGeopakTinFile(WCharCP fileNameP, TransformCP transformation) override;

    public:

        ScalableMeshDTM(IScalableMeshPtr& scMesh);            

        virtual ~ScalableMeshDTM()
            {
            delete m_draping;
            delete m_dtmVolume;
            }

        static RefCountedPtr<ScalableMeshDTM> Create(IScalableMeshPtr scMesh)
            {
            return new ScalableMeshDTM(scMesh);
            }        

        void SetAnalysisType(DTMAnalysisType type)
            {
            m_draping->SetAnalysisType(type);
            }

        void SetStorageToUors(DMatrix4d& storageToUors);
    };
/*----------------------------------------------------------------------------+
|Class ScalableMesh
+----------------------------------------------------------------------------*/
template <class INDEXPOINT> class ScalableMesh : public ScalableMeshBase
    {
/*__PUBLISH_SECTION_END__*/
    private : 
             
        friend struct IScalableMesh;        
        friend class auto_ptr<ScalableMesh>;
        friend class ScalableMeshProgressiveQueryEngine;        
                
        typedef SMMeshIndex<INDEXPOINT, Extent3dType>
                                        MeshIndexType;
        

        bool                            m_areDataCompressed; 
        bool                            m_computeTileBoundary;
        bool                            m_isInvertingClips;
        bool                            m_needsNeighbors;
        bool                            m_isCesium3DTiles;
        double                          m_minScreenPixelsPerPoint;            

        HFCPtr<MeshIndexType>          m_scmIndexPtr;              
        HFCPtr<MeshIndexType>          m_scmTerrainIndexPtr; //Optional. Scalable Mesh representing only terrain
        IScalableMeshPtr               m_terrainP; 
        RefCountedPtr<ScalableMeshDTM>  m_scalableMeshDTM[DTMAnalysisType::Qty];

        IScalableMeshPtr m_groupP;

        bvector<IScalableMeshNodePtr> m_viewedNodes;

        Transform                     m_reprojectionTransform; //approximation of reprojection used for live transforms.


        explicit                        ScalableMesh(SMSQLiteFilePtr& smSQLiteFile,const WString&             path);


        virtual                         ~ScalableMesh                         ();


        static IScalableMeshPtr       Open                           (SMSQLiteFilePtr& smSQLiteFile,
                                                                        const WString&             filePath,
                                                                        const Utf8String&     baseEditsFilePath,
                                                                      bool                    needsNeighbors,
                                                                      StatusInt&                      status);


        int                             Open                           ();                
                    
        int                             Close                          ();

  
        unsigned __int64                CountPointsInExtent(Extent3dType&  extent, 
                                                            HFCPtr<SMPointIndexNode<INDEXPOINT, Extent3dType>> nodePtr,
                                                            const unsigned __int64& maxNumberCountedPoints) const;

        unsigned __int64                CountLinearsInExtent(YProtFeatureExtentType& extent, unsigned __int64 maxFeatures) const;


        static DRange3d                 ComputeTotalExtentFor          (const MeshIndexType*           pointIndexP);

        bool                            AreDataCompressed              ();

        void CreateSpatialIndexFromExtents(list<HVE2DSegment>& pi_rpBreaklineList, 
                                           BC_DTM_OBJ**        po_ppBcDtmObj);
        
        bool                            IsValidInContextOf             (const GeoCoords::GCS&           gcs) const;

        bool                            IsEmpty                        () const;

        void                            SaveEditFiles();

    protected : 

        HFCPtr<SMPointIndexNode<INDEXPOINT, Extent3dType>> GetRootNode();                    
        virtual void                               _TextureFromRaster(ITextureProviderPtr provider) override;
 
        virtual __int64          _GetPointCount() override;

        virtual bool          _IsTerrain() override;

        virtual bool          _IsTextured() override;

        virtual bool          _IsCesium3DTiles() override;
        

        virtual BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM*  _GetDTMInterface(DTMAnalysisType type) override;

        virtual BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM*  _GetDTMInterface(DMatrix4d& storageToUors, DTMAnalysisType type) override;

        virtual DTMStatusInt     _GetRange(DRange3dR range) override;

        virtual StatusInt         _GetBoundary(bvector<DPoint3d>& boundary) override;


    // Inherited from IMRDTM                   
        virtual Count                  _GetCountInRange (const DRange2d& range, const CountType& type, const unsigned __int64& maxNumberCountedPoints) const;
        virtual int                    _GenerateSubResolutions() override;
        virtual __int64                _GetBreaklineCount() const override;
        virtual ScalableMeshCompressionType   _GetCompressionType() const override;          
        virtual int                    _GetNbResolutions() const override;    
        virtual size_t                    _GetTerrainDepth() const override;
        virtual IScalableMeshPointQueryPtr         _GetQueryInterface(ScalableMeshQueryType     queryType) const override;
        virtual IScalableMeshPointQueryPtr         _GetQueryInterface(ScalableMeshQueryType                queryType,                                                           
                                                                      BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCS,
                                                                      const DRange3d&                      extentInTargetGCS) const override;        
        virtual IScalableMeshMeshQueryPtr     _GetMeshQueryInterface(MeshQueryType queryType) const override;
        virtual IScalableMeshMeshQueryPtr     _GetMeshQueryInterface(MeshQueryType queryType,
                                                              BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCS,
                                                              const DRange3d&                      extentInTargetGCS) const override;
        virtual IScalableMeshNodeRayQueryPtr     _GetNodeQueryInterface() const override;

        virtual IScalableMeshEditPtr    _GetMeshEditInterface() const override;

        virtual const GeoCoords::GCS&  _GetGCS() const override;
        virtual StatusInt              _SetGCS(const GeoCoords::GCS& sourceGCS) override;        
        virtual ScalableMeshState             _GetState() const override;     
        virtual bool                   _IsProgressive() const override;    
        virtual bool                   _IsReadOnly() const override;
        virtual bool                   _IsShareable() const override;
        virtual int                    _GetRangeInSpecificGCS(DPoint3d& lowPt, DPoint3d& highPt, BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCS) const;


        virtual uint64_t                           _AddClip(const DPoint3d* pts, size_t ptsSize) override;
        virtual bool                               _ModifyClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID) override;
        virtual bool                               _AddClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive) override;
        virtual bool                               _ModifyClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive) override;
        virtual bool                               _AddClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID, bool alsoAddOnTerrain = true) override;
        virtual bool                               _RemoveClip(uint64_t clipID) override;
        virtual bool                               _GetClip(uint64_t clipID, bvector<DPoint3d>& clipData) override;
        virtual void                               _SetIsInsertingClips(bool toggleInsertMode) override;

        virtual bool                               _ShouldInvertClips() override;

        virtual void                               _SetInvertClip(bool invertClips) override;

        virtual void                               _ModifyClipMetadata(uint64_t clipId, double importance, int nDimensions) override;
        virtual void                               _GetAllClipsIds(bvector<uint64_t>& allClipIds) override;

        virtual void                               _SetClipOnOrOff(uint64_t id, bool isActive) override;
        virtual void                               _GetIsClipActive(uint64_t id, bool& isActive) override;

        virtual void                               _GetClipType(uint64_t id, SMNonDestructiveClipType& type) override;
        virtual void                               _SynchronizeClipData(const bvector<bpair<uint64_t, bvector<DPoint3d>>>& listOfClips, const bvector<bpair<uint64_t, bvector<bvector<DPoint3d>>>>& listOfSkirts) override;


        virtual bool                               _ModifySkirt(const bvector<bvector<DPoint3d>>& skirt, uint64_t skirtID) override;
        virtual bool                               _AddSkirt(const bvector<bvector<DPoint3d>>& skirt, uint64_t skirtID, bool alsoAddOnTerrain = true) override;
        virtual bool                               _RemoveSkirt(uint64_t skirtID) override;
        virtual int                                _ConvertToCloud(const WString& outContainerName, const WString& outDatasetName, SMCloudServerType server) const override;
        virtual void                               _ImportTerrainSM(WString terrainPath) override;
        virtual IScalableMeshPtr                    _GetTerrainSM() override;

        virtual BentleyStatus                      _SetReprojection(GeoCoordinates::BaseGCSCR targetCS, TransformCR approximateTransform) override;
        virtual BentleyStatus                      _Reproject(GeoCoordinates::BaseGCSCR targetCS, DgnModelRefP dgnModel) override;
        virtual Transform                          _GetReprojectionTransform() const override;

        virtual BentleyStatus                      _DetectGroundForRegion(BeFileName& createdTerrain, const BeFileName& coverageTempDataFolder, const bvector<DPoint3d>& coverageData, uint64_t id, IScalableMeshGroundPreviewerPtr groundPreviewer) override;
        virtual BentleyStatus                      _CreateCoverage(const bvector<DPoint3d>& coverageData, uint64_t id, const Utf8String& coverageName) override;
        virtual void                               _GetAllCoverages(bvector<bvector<DPoint3d>>& coverageData) override;
        virtual void                               _GetCoverageIds(bvector<uint64_t>& ids) const override;        
        virtual BentleyStatus                      _DeleteCoverage(uint64_t id) override;
        virtual void                               _GetCoverageName(Utf8String& name, uint64_t id) const override;

        virtual void                               _GetCurrentlyViewedNodes(bvector<IScalableMeshNodePtr>& nodes) override;
        virtual void                               _SetCurrentlyViewedNodes(const bvector<IScalableMeshNodePtr>& nodes) override;

        virtual  IScalableMeshPtr             _GetGroup() override;

        virtual void                          _AddToGroup(IScalableMeshPtr& sMesh, bool isRegionRestricted = false, const DPoint3d* region = nullptr, size_t nOfPtsInRegion = 0) override;


        virtual void                          _RemoveFromGroup(IScalableMeshPtr& sMesh) override;

        virtual void                          _SetGroupSelectionFromPoint(DPoint3d firstPoint) override {}
        virtual void                          _ClearGroupSelection() override {}
        
#ifdef SCALABLE_MESH_ATP
        virtual int                    _ChangeGeometricError(const WString& outContainerName, const WString& outDatasetName, SMCloudServerType server, const double& newGeometricErrorValue) const override;
        virtual int                    _LoadAllNodeHeaders(size_t& nbLoadedNodes, int level) const override;
        virtual int                    _LoadAllNodeData(size_t& nbLoadedNodes, int level) const override;
        virtual int                    _SaveGroupedNodeHeaders(const WString& pi_pOutputDirPath, const short& pi_pGroupMode) const override;
#endif

        virtual void _SetUserFilterCallback(MeshUserFilterCallback callback) override;
        virtual void _ReFilter() override;
        
        virtual void                               _SetEditFilesBasePath(const Utf8String& path) override;

        virtual Utf8String                         _GetEditFilesBasePath() override;

        virtual void                               _GetExtraFileNames(bvector<BeFileName>& extraFileNames) const override;        
        
        virtual IScalableMeshNodePtr               _GetRootNode() override;

#ifdef WIP_MESH_IMPORT
        virtual void _GetAllTextures(bvector<IScalableMeshTexturePtr>& textures) override;
#endif

        //Data source synchronization functions.
        virtual bool                   _InSynchWithSources() const override; 
        virtual bool                   _LastSynchronizationCheck(time_t& last) const override;        
        virtual int                    _SynchWithSources() override;           

/*__PUBLISH_SECTION_START__*/

    public:    

        void                ComputeTileBoundaryDuringQuery(bool doCompute);    

        double              GetMinScreenPixelsPerPoint();   
    
        void                SetMinScreenPixelsPerPoint(double pi_minScreenPixelsPerPoint);     

        HFCPtr<MeshIndexType> GetMainIndexP() { return m_scmIndexPtr; }

        void SetMainIndexP(HFCPtr<MeshIndexType> newIndex) { m_scmIndexPtr = newIndex; }

        void SetNeedsNeighbors(bool needsNeighbors) { m_needsNeighbors = needsNeighbors; }
                                    
    };


/*__PUBLISH_SECTION_END__*/

template<class POINT> class ScalableMeshFixResolutionViewPointQuery;

//MS It should inherit from IDTM but the query interface is not implemented at that level. 
template <class POINT> class ScalableMeshSingleResolutionPointIndexView : public RefCounted<IScalableMesh>
    {        
    friend class ScalableMeshFixResolutionViewPointQuery<POINT>;

    private : 
                
        HFCPtr<SMPointIndex<POINT, Extent3dType>> m_scmIndexPtr;
        int                                             m_resolutionIndex;
        GeoCoords::GCS                                  m_sourceGCS;
        
    protected : 

        ScalableMeshSingleResolutionPointIndexView(HFCPtr<SMPointIndex<POINT, Extent3dType>> scmPointIndexPtr, 
                                                   int                                             resolutionIndex, 
                                                   GeoCoords::GCS                             sourceGCS);

        virtual ~ScalableMeshSingleResolutionPointIndexView();

        virtual void                               _TextureFromRaster(ITextureProviderPtr provider) override;

        // Inherited from IDTM   
        virtual __int64          _GetPointCount() override;

        virtual bool          _IsTerrain() override;

        virtual bool          _IsTextured() override { return false; }

        virtual bool           _IsCesium3DTiles() override { return false; }

        virtual BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM*  _GetDTMInterface(DTMAnalysisType type) override;

        virtual BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM*  _GetDTMInterface(DMatrix4d& storageToUors, DTMAnalysisType type) override;

        virtual DTMStatusInt     _GetRange(DRange3dR range) override;

        virtual StatusInt         _GetBoundary(bvector<DPoint3d>& boundary) override;

        // Inherited from IMRDTM             
        virtual Count                  _GetCountInRange (const DRange2d& range, const CountType& type, const unsigned __int64& maxNumberCountedPoints) const override;
        virtual int                    _GenerateSubResolutions() override;
        virtual __int64                _GetBreaklineCount() const override;
        virtual ScalableMeshCompressionType   _GetCompressionType() const override;           
        virtual int                    _GetNbResolutions() const override;    
        virtual size_t                    _GetTerrainDepth() const override;
        virtual IScalableMeshPointQueryPtr         _GetQueryInterface(ScalableMeshQueryType     queryType) const override;
        virtual IScalableMeshPointQueryPtr         _GetQueryInterface(ScalableMeshQueryType                queryType, 
                                                                      BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCS,
                                                                      const DRange3d&                      extentInTargetGCS) const override;                
        virtual IScalableMeshMeshQueryPtr     _GetMeshQueryInterface(MeshQueryType queryType) const override; 
        virtual IScalableMeshMeshQueryPtr     _GetMeshQueryInterface(MeshQueryType queryType,
                                                              BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCS,
                                                              const DRange3d&                      extentInTargetGCS) const override;
        virtual IScalableMeshNodeRayQueryPtr     _GetNodeQueryInterface() const override;

        virtual IScalableMeshEditPtr    _GetMeshEditInterface() const override { return nullptr; };

        const GeoCoords::GCS&          _GetGCS() const override;
        StatusInt                      _SetGCS(const GeoCoords::GCS& sourceGCS) override;
        virtual ScalableMeshState             _GetState() const override;   
        virtual bool                   _IsProgressive() const override;       
        virtual bool                   _IsReadOnly() const override;
        virtual bool                   _IsShareable() const override;


        virtual uint64_t                           _AddClip(const DPoint3d* pts, size_t ptsSize) override;
        virtual bool                               _AddClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive)
            {
            return false;
            }
        virtual bool                               _ModifyClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID) override;
        virtual bool                               _ModifyClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive)
            {
            return false;
            }
        virtual bool                               _GetClip(uint64_t clipID, bvector<DPoint3d>& clipData) override { return false; }
        virtual bool                               _AddClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID, bool alsoAddOnTerrain = true) override;
        virtual bool                               _RemoveClip(uint64_t clipID) override;
        virtual void                               _SetIsInsertingClips(bool toggleInsertMode) override;
        virtual void                               _ModifyClipMetadata(uint64_t clipId, double importance, int nDimensions) override;
        virtual void                               _GetAllClipsIds(bvector<uint64_t>& allClipIds) override;

        virtual void                               _SetClipOnOrOff(uint64_t id, bool isActive) override
            {
            return;
            }
        virtual void                               _GetIsClipActive(uint64_t id, bool& isActive) override
            {
            return;
            }

        virtual void                               _GetClipType(uint64_t id, SMNonDestructiveClipType& type) override
            {
            return;
            }

        virtual bool                               _ShouldInvertClips() override { return false; }

        virtual void                               _SetInvertClip(bool invertClips) override {}

        virtual bool                               _ModifySkirt(const bvector<bvector<DPoint3d>>& skirt, uint64_t skirtID) override;
        virtual bool                               _AddSkirt(const bvector<bvector<DPoint3d>>& skirt, uint64_t skirtID, bool alsoAddOnTerrain = true) override;
        virtual bool                               _RemoveSkirt(uint64_t skirtID) override;
        virtual void                               _SynchronizeClipData(const bvector<bpair<uint64_t, bvector<DPoint3d>>>& listOfClips, const bvector<bpair<uint64_t, bvector<bvector<DPoint3d>>>>& listOfSkirts) override {}

        
        virtual void                               _GetCurrentlyViewedNodes(bvector<IScalableMeshNodePtr>& nodes) override;
        virtual void                               _SetCurrentlyViewedNodes(const bvector<IScalableMeshNodePtr>& nodes) override;
        //Data source synchronization functions.
        virtual bool                   _InSynchWithSources() const override; 
        virtual bool                   _LastSynchronizationCheck(time_t& last) const override;        
        virtual int                    _SynchWithSources() override;

        virtual void                               _GetExtraFileNames(bvector<BeFileName>& extraFileNames) const override { assert(!"Should not be called"); }
        
        virtual int                    _GetRangeInSpecificGCS(DPoint3d& lowPt, DPoint3d& highPt, BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCS) const override;
        virtual int                    _ConvertToCloud(const WString& outContainerName, const WString& outDatasetName, SMCloudServerType server) const override { return ERROR; }
        virtual void                               _ImportTerrainSM(WString terrainPath) override {};
        virtual IScalableMeshPtr                    _GetTerrainSM() override
            {
            return nullptr;
            }

        virtual Transform                          _GetReprojectionTransform() const override
            {
            return Transform();

            }

        virtual BentleyStatus                      _SetReprojection(GeoCoordinates::BaseGCSCR targetCS, TransformCR approximateTransform) override
            {
            return ERROR;
            }
        virtual BentleyStatus                      _Reproject(GeoCoordinates::BaseGCSCR targetCS, DgnModelRefP dgnModel) override
            {
            return ERROR;
            }
        virtual BentleyStatus                      _DetectGroundForRegion(BeFileName& createdTerrain, const BeFileName& coverageTempDataFolder, const bvector<DPoint3d>& coverageData, uint64_t id, IScalableMeshGroundPreviewerPtr groundPreviewer) override
            {
            return ERROR;
            }

        virtual BentleyStatus                   _CreateCoverage( const bvector<DPoint3d>& coverageData, uint64_t id, const Utf8String& coverageName) override { return ERROR; };
        virtual void                           _GetAllCoverages(bvector<bvector<DPoint3d>>& coverageData) override {};
        virtual void                               _GetCoverageIds(bvector<uint64_t>& ids) const override {};
        virtual BentleyStatus                      _DeleteCoverage(uint64_t id) override { return SUCCESS; };        
        virtual void                               _GetCoverageName(Utf8String& name, uint64_t id) const override { assert(false); };
        virtual void                               _SetEditFilesBasePath(const Utf8String& path) override { assert(false); };
        virtual Utf8String                         _GetEditFilesBasePath() override { assert(false); return Utf8String(); };
        virtual IScalableMeshNodePtr               _GetRootNode() override
            {
            assert(false);
            auto ptr = HFCPtr<SMPointIndexNode<POINT, Extent3dType>>(nullptr);
           #ifndef VANCOUVER_API
            return new ScalableMeshNode<POINT>(ptr);
            #else
            return ScalableMeshNode<POINT>::CreateItem(ptr);
            #endif
            }

        virtual  IScalableMeshPtr             _GetGroup() override { return nullptr;  }

        virtual void                          _AddToGroup(IScalableMeshPtr& sMesh, bool isRegionRestricted = false, const DPoint3d* region = nullptr, size_t nOfPtsInRegion = 0) override
            {}


        virtual void                          _RemoveFromGroup(IScalableMeshPtr& sMesh) override {}

        virtual void                          _SetGroupSelectionFromPoint(DPoint3d firstPoint) override {}
        virtual void                          _ClearGroupSelection() override{}

#ifdef WIP_MESH_IMPORT
        virtual void _GetAllTextures(bvector<IScalableMeshTexturePtr>& textures) override
            {}
#endif

#ifdef SCALABLE_MESH_ATP
        virtual int                    _ChangeGeometricError(const WString& outContainerName, const WString& outDatasetName, SMCloudServerType server, const double& newGeometricErrorValue) const override { return ERROR; }
        virtual int                    _LoadAllNodeHeaders(size_t& nbLoadedNodes, int level) const override {return ERROR;}
        virtual int                    _LoadAllNodeData(size_t& nbLoadedNodes, int level) const override { return ERROR; }
        virtual int                    _SaveGroupedNodeHeaders(const WString& pi_pOutputDirPath, const short& pi_pGroupMode) const override { return ERROR; }
#endif

        virtual void _SetUserFilterCallback(MeshUserFilterCallback callback) override {};
        virtual void _ReFilter() override {};
           
    };


/*__PUBLISH_SECTION_START__*/
END_BENTLEY_SCALABLEMESH_NAMESPACE
