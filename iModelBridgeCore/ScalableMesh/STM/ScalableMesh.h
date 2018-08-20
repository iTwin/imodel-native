/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMesh.h $
|    $RCSfile: ScalableMesh.h,v $
|   $Revision: 1.54 $
|       $Date: 2012/01/06 16:30:13 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
#include <ScalableMesh/IScalableMeshProgress.h>
#include <ScalableMesh/IScalableMeshClipContainer.h>
#include <ScalableMesh/IScalableMeshRDSProvider.h>
#include "ScalableMeshDraping.h"
#include "ScalableMeshClippingOptions.h"

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

#ifndef LINUX_SCALABLEMESH_BUILD
#include <CloudDataSource/DataSourceManager.h>
#endif

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
    private:

#ifndef VANCOUVER_API
        //Avoid assert added on Bim02    
        virtual uint32_t _GetExcessiveRefCountThreshold() const override { return std::numeric_limits<uint32_t>::max(); }
#endif


    protected:
/*__PUBLISH_SECTION_END__*/

    SMSQLiteFilePtr                     m_smSQLitePtr;
    bool                                m_isDgnDB;
    
    size_t                              m_workingLayer;

    GeoCoords::GCS                      m_sourceGCS;
    DRange3d                            m_contentExtent;

    WString                             m_baseExtraFilesPath;
    bool                                m_useTempPath;
 
 #ifndef LINUX_SCALABLEMESH_BUILD  
    DataSourceAccount*                  m_dataSourceAccount;
#endif

    // NOTE: Stored in order to make it possible for the creator to use this. Remove when creator does not depends on
    // this interface anymore (take only a path).
    const WString                        m_path; 


    explicit                            ScalableMeshBase(SMSQLiteFilePtr& smSQLiteFile, const WString&             filePath);


/*__PUBLISH_SECTION_START__*/

    virtual                             ~ScalableMeshBase                 () = 0;


    bool                                LoadGCSFrom(WString wktStr);
    bool                                LoadGCSFrom();

public:

    BENTLEY_SM_EXPORT const SMSQLiteFilePtr&              GetDbFile() const;

    BENTLEY_SM_EXPORT const WChar*                        GetPath                 () const;

#ifndef LINUX_SCALABLEMESH_BUILD
    static DataSourceManager &          GetDataSourceManager    (void)                                  {return *DataSourceManager::Get();}
    void                                SetDataSourceAccount    (DataSourceAccount *dataSourceAccount)  {m_dataSourceAccount = dataSourceAccount;}
    DataSourceAccount *                 GetDataSourceAccount    (void) const                            {return m_dataSourceAccount;}
#endif

    void                                SetUseTempPath(bool useTempPath)                                {m_useTempPath = useTempPath;}
   
    };


class ScalableMeshDTM : public RefCounted<BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM>
    {
    ScalableMeshDraping* m_draping;
    IScalableMesh* m_scMesh;
    Transform m_transformToUors;
    IDTMVolume* m_dtmVolume;
    TerrainModel::BcDTMPtr m_dtm; //Maximum 5M points bcDtm representation of a ScalableMesh
    bool                   m_tryCreateDtm; 
	bool m_useBcLibDrape;


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
    virtual DTMStatusInt _ExportToGeopakTinFile(BENTLEY_NAMESPACE_NAME::WCharCP fileNameP, TransformCP transformation) override;

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

		void SetUseBcLibForDraping(bool useBcLib);
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
        bool                            m_isFromStubFile;
        double                          m_minScreenPixelsPerPoint;

        HFCPtr<MeshIndexType>          m_scmIndexPtr;              
        HFCPtr<MeshIndexType>          m_scmTerrainIndexPtr; //Optional. Scalable Mesh representing only terrain
        IScalableMeshPtr               m_terrainP; 
        RefCountedPtr<ScalableMeshDTM>  m_scalableMeshDTM[DTMAnalysisType::Qty];

        IScalableMeshPtr m_groupP;

        bvector<IScalableMeshNodePtr> m_viewedNodes;

        Transform                     m_reprojectionTransform; //approximation of reprojection used for live transforms.

        IScalableMeshRDSProviderPtr   m_smRDSProvider = nullptr;

		IScalableMeshClippingOptionsPtr m_clippingOptions;



        explicit                        ScalableMesh(SMSQLiteFilePtr& smSQLiteFile,const WString&             path);


        virtual                         ~ScalableMesh                         ();


        static IScalableMeshPtr       Open                           (SMSQLiteFilePtr&  smSQLiteFile,
                                                                      const WString&    filePath,
                                                                      const Utf8String& baseEditsFilePath,
                                                                      bool              useTempFolderForEditFiles,
                                                                      bool              needsNeighbors,
                                                                      StatusInt&        status);


        int                             Open                           ();                
                    
        int                             Close                          ();

  
        uint64_t                CountPointsInExtent(Extent3dType&  extent, 
                                                            HFCPtr<SMPointIndexNode<INDEXPOINT, Extent3dType>> nodePtr,
                                                            const uint64_t& maxNumberCountedPoints) const;

        uint64_t                CountLinearsInExtent(YProtFeatureExtentType& extent, uint64_t maxFeatures) const;        

        bool                            AreDataCompressed              ();

        void CreateSpatialIndexFromExtents(list<HVE2DSegment>& pi_rpBreaklineList, 
                                           BC_DTM_OBJ**        po_ppBcDtmObj);
        
        bool                            IsValidInContextOf             (const GeoCoords::GCS&           gcs) const;

        bool                            IsEmpty                        () const;

        void                            SaveEditFiles();

    protected : 

        HFCPtr<SMPointIndexNode<INDEXPOINT, Extent3dType>> GetRootNode();                    
        virtual void                               _TextureFromRaster(ITextureProviderPtr provider) override;
 
        virtual int64_t          _GetPointCount() override;

        virtual uint64_t          _GetNodeCount() override;

        virtual bool          _IsTerrain() override;

        virtual bool          _IsTextured() override;

        virtual StatusInt     _GetTextureInfo(IScalableMeshTextureInfoPtr& textureInfo) const override;

        virtual bool          _IsCesium3DTiles() override;

        virtual bool          _IsStubFile() override;

        virtual Utf8String    _GetProjectWiseContextShareLink() override;
        

        virtual BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM*  _GetDTMInterface(DTMAnalysisType type) override;

        virtual BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM*  _GetDTMInterface(DMatrix4d& storageToUors, DTMAnalysisType type) override;

        virtual DTMStatusInt     _GetRange(DRange3dR range) override;

        virtual StatusInt         _GetBoundary(bvector<DPoint3d>& boundary) override;


    // Inherited from IMRDTM                   
        virtual Count                  _GetCountInRange (const DRange2d& range, const CountType& type, const uint64_t& maxNumberCountedPoints) const;
        virtual int                    _GenerateSubResolutions() override;
        virtual int64_t                _GetBreaklineCount() const override;
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
        virtual IScalableMeshAnalysisPtr    _GetMeshAnalysisInterface() override;

        virtual const GeoCoords::GCS&  _GetGCS() const override;
        virtual StatusInt              _SetGCS(const GeoCoords::GCS& sourceGCS) override;        
        virtual ScalableMeshState             _GetState() const override;             
        virtual bool                   _IsReadOnly() const override;
        virtual bool                   _IsShareable() const override;
        virtual int                    _GetRangeInSpecificGCS(DPoint3d& lowPt, DPoint3d& highPt, BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCS) const;


        virtual uint64_t                           _AddClip(const DPoint3d* pts, size_t ptsSize) override;
        virtual bool                               _ModifyClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID) override;
        virtual bool                               _AddClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive) override;
        virtual bool                               _ModifyClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive) override;
        virtual bool                               _AddClip(const ClipVectorPtr& clip, uint64_t clipID, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive) override;
        virtual bool                               _ModifyClip(const ClipVectorPtr& clip, uint64_t clipID, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive) override;
        virtual bool                               _AddClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID, bool alsoAddOnTerrain = true) override;
        virtual bool                               _RemoveClip(uint64_t clipID) override;
        virtual bool                               _GetClip(uint64_t clipID, bvector<DPoint3d>& clipData) override;
        virtual bool                               _IsInsertingClips() override;
        virtual void                               _SetIsInsertingClips(bool toggleInsertMode) override;

        virtual bool                               _ShouldInvertClips() override;

        virtual void                               _SetInvertClip(bool invertClips) override;

        virtual void                               _ModifyClipMetadata(uint64_t clipId, double importance, int nDimensions) override;
        virtual void                               _GetAllClipsIds(bvector<uint64_t>& allClipIds) override;

        virtual void                               _SetClipOnOrOff(uint64_t id, bool isActive) override;
        virtual void                               _GetIsClipActive(uint64_t id, bool& isActive) override;

        virtual void                               _GetClipType(uint64_t id, SMNonDestructiveClipType& type) override;
        virtual void                               _SynchronizeClipData(const bvector<bpair<uint64_t, bvector<DPoint3d>>>& listOfClips, const bvector<bpair<uint64_t, bvector<bvector<DPoint3d>>>>& listOfSkirts) override;

		virtual void                               _CompactExtraFiles() override;

		virtual void                               _WriteExtraFiles() override;

        virtual bool                               _GetSkirt(uint64_t skirtID, bvector<bvector<DPoint3d>>& skirt) override;
        virtual bool                               _ModifySkirt(const bvector<bvector<DPoint3d>>& skirt, uint64_t skirtID) override;
        virtual bool                               _AddSkirt(const bvector<bvector<DPoint3d>>& skirt, uint64_t skirtID, bool alsoAddOnTerrain = true) override;
        virtual bool                               _RemoveSkirt(uint64_t skirtID) override;
        virtual int                                _SaveAs(const WString& destination, ClipVectorPtr clips, IScalableMeshProgressPtr progress) override;
        virtual int                                _Generate3DTiles(const WString& outContainerName, const WString& outDatasetName, SMCloudServerType server, IScalableMeshProgressPtr progress, ClipVectorPtr clips, uint64_t coverageId) const override;
        virtual void                               _ImportTerrainSM(WString terrainPath) override;
        virtual IScalableMeshPtr                    _GetTerrainSM() override;

        virtual BentleyStatus                      _SetReprojection(GeoCoordinates::BaseGCSCR targetCS, TransformCR approximateTransform) override;
#ifdef VANCOUVER_API
        virtual BentleyStatus                      _Reproject(GeoCoordinates::BaseGCSCP targetCS, DgnModelRefP dgnModel) override;
#endif
        virtual Transform                          _GetReprojectionTransform() const override;

        virtual SMStatus                      _DetectGroundForRegion(BeFileName& createdTerrain, const BeFileName& coverageTempDataFolder, const bvector<DPoint3d>& coverageData, uint64_t id, IScalableMeshGroundPreviewerPtr groundPreviewer, BaseGCSCPtr& destinationGcs, bool limitResolution) override;
        virtual BentleyStatus                      _CreateCoverage(const bvector<DPoint3d>& coverageData, uint64_t id, const Utf8String& coverageName) override;
        virtual void                               _GetAllCoverages(bvector<bvector<DPoint3d>>& coverageData) override;
        virtual void                               _GetCoverageIds(bvector<uint64_t>& ids) const override;        
        virtual BentleyStatus                      _DeleteCoverage(uint64_t id) override;
        virtual void                               _GetCoverageName(Utf8String& name, uint64_t id) const override;

		virtual IScalableMeshClippingOptions&      _EditClippingOptions() override;

        virtual void                               _GetCurrentlyViewedNodes(bvector<IScalableMeshNodePtr>& nodes) override;
        virtual void                               _SetCurrentlyViewedNodes(const bvector<IScalableMeshNodePtr>& nodes) override;

        virtual  IScalableMeshPtr             _GetGroup() override;

        virtual void                          _AddToGroup(IScalableMeshPtr& sMesh, bool isRegionRestricted = false, const DPoint3d* region = nullptr, size_t nOfPtsInRegion = 0) override;


        virtual void                          _RemoveFromGroup(IScalableMeshPtr& sMesh) override;

        virtual void                          _SetGroupSelectionFromPoint(DPoint3d firstPoint) override {}
        virtual void                          _ClearGroupSelection() override {}

        virtual void                          _RemoveAllDisplayData() override;

        
#ifdef SCALABLE_MESH_ATP
        virtual int                    _ChangeGeometricError(const WString& outContainerName, const WString& outDatasetName, SMCloudServerType server, const double& newGeometricErrorValue) const override;
        virtual int                    _LoadAllNodeHeaders(size_t& nbLoadedNodes, int level) const override;
        virtual int                    _LoadAllNodeData(size_t& nbLoadedNodes, int level) const override;
        virtual int                    _SaveGroupedNodeHeaders(const WString& pi_pOutputDirPath, const short& pi_pGroupMode) const override;
#endif
        
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

/*__PUBLISH_SECTION_START__*/

    public:    

        void                ComputeTileBoundaryDuringQuery(bool doCompute);    

        double              GetMinScreenPixelsPerPoint();   
    
        void                SetMinScreenPixelsPerPoint(double pi_minScreenPixelsPerPoint);     

        HFCPtr<MeshIndexType> GetMainIndexP() { return m_scmIndexPtr; }

        void SetMainIndexP(HFCPtr<MeshIndexType> newIndex) { m_scmIndexPtr = newIndex; }

        void SetNeedsNeighbors(bool needsNeighbors) { m_needsNeighbors = needsNeighbors; }

        static DRange3d ComputeTotalExtentFor(const MeshIndexType*           pointIndexP);
                                    
    };

template <class POINT>
DRange3d ScalableMesh<POINT>::ComputeTotalExtentFor(const MeshIndexType*   pointIndexP)
    {
    typedef ExtentOp<Extent3dType>         PtExtentOpType;

    DRange3d totalExtent;
    memset(&totalExtent, 0, sizeof(totalExtent));

    if ((pointIndexP != 0) && (!pointIndexP->IsEmpty()))
        {
        Extent3dType ExtentPoints = pointIndexP->GetContentExtent();
        totalExtent.low.x = PtExtentOpType::GetXMin(ExtentPoints);
        totalExtent.high.x = PtExtentOpType::GetXMax(ExtentPoints);
        totalExtent.low.y = PtExtentOpType::GetYMin(ExtentPoints);
        totalExtent.high.y = PtExtentOpType::GetYMax(ExtentPoints);
        totalExtent.low.z = PtExtentOpType::GetZMin(ExtentPoints);
        totalExtent.high.z = PtExtentOpType::GetZMax(ExtentPoints);
        }

    return totalExtent;
    }


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
		ScalableMeshClippingOptions m_options;
        
    protected : 

        ScalableMeshSingleResolutionPointIndexView(HFCPtr<SMPointIndex<POINT, Extent3dType>> scmPointIndexPtr, 
                                                   int                                             resolutionIndex, 
                                                   GeoCoords::GCS                             sourceGCS);

        virtual ~ScalableMeshSingleResolutionPointIndexView();

        virtual void                               _TextureFromRaster(ITextureProviderPtr provider) override;

        // Inherited from IDTM   
        virtual int64_t          _GetPointCount() override;

        virtual uint64_t          _GetNodeCount() override;

        virtual bool          _IsTerrain() override;

        virtual bool          _IsTextured() override { return false; }

        virtual StatusInt     _GetTextureInfo(IScalableMeshTextureInfoPtr& textureInfo) const override {return ERROR;}

        virtual bool           _IsCesium3DTiles() override { return false; }

        virtual bool           _IsStubFile() override { return false; }

        virtual Utf8String    _GetProjectWiseContextShareLink() override { return Utf8String(); }

        virtual BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM*  _GetDTMInterface(DTMAnalysisType type) override;

        virtual BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM*  _GetDTMInterface(DMatrix4d& storageToUors, DTMAnalysisType type) override;

        virtual DTMStatusInt     _GetRange(DRange3dR range) override;

        virtual StatusInt         _GetBoundary(bvector<DPoint3d>& boundary) override;

        // Inherited from IMRDTM             
        virtual Count                  _GetCountInRange (const DRange2d& range, const CountType& type, const uint64_t& maxNumberCountedPoints) const override;
        virtual int                    _GenerateSubResolutions() override;
        virtual int64_t                _GetBreaklineCount() const override;
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
        virtual IScalableMeshAnalysisPtr    _GetMeshAnalysisInterface() override { return nullptr; };

        const GeoCoords::GCS&          _GetGCS() const override;
        StatusInt                      _SetGCS(const GeoCoords::GCS& sourceGCS) override;
        virtual ScalableMeshState             _GetState() const override;           
        virtual bool                   _IsReadOnly() const override;
        virtual bool                   _IsShareable() const override;


        virtual uint64_t                           _AddClip(const DPoint3d* pts, size_t ptsSize) override;
        virtual bool                               _AddClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive)
            {
            return false;
            }
        virtual bool                               _AddClip(const ClipVectorPtr& clip, uint64_t clipID, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive)
        {
            return false;
        }
        virtual bool                               _ModifyClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID) override;
        virtual bool                               _ModifyClip(const ClipVectorPtr& clip, uint64_t clipID, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive)
            {
            return false;
            }
        virtual bool                               _ModifyClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive)
        {
            return false;
        }
        virtual bool                               _GetClip(uint64_t clipID, bvector<DPoint3d>& clipData) override { return false; }
        virtual bool                               _AddClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID, bool alsoAddOnTerrain = true) override;
        virtual bool                               _RemoveClip(uint64_t clipID) override;
        virtual bool                               _IsInsertingClips() override;
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
		virtual void                               _CompactExtraFiles() override {}

		virtual void                               _WriteExtraFiles() override {}

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

        virtual void                               _GetExtraFileNames(bvector<BeFileName>& extraFileNames) const override { assert(!"Should not be called"); }
        
        virtual int                    _GetRangeInSpecificGCS(DPoint3d& lowPt, DPoint3d& highPt, BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCS) const override;
        virtual int                                _SaveAs(const WString& destination, ClipVectorPtr clips, IScalableMeshProgressPtr progress) override { return ERROR; }
        virtual int                    _Generate3DTiles(const WString& outContainerName, const WString& outDatasetName, SMCloudServerType server, IScalableMeshProgressPtr progress, ClipVectorPtr clips, uint64_t coverageId) const override { return ERROR; }
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
#ifdef VANCOUVER_API
        virtual BentleyStatus                      _Reproject(GeoCoordinates::BaseGCSCP targetCS, DgnModelRefP dgnModel) override
            {
            return ERROR;
            }
#endif
        virtual SMStatus                      _DetectGroundForRegion(BeFileName& createdTerrain, const BeFileName& coverageTempDataFolder, const bvector<DPoint3d>& coverageData, uint64_t id, IScalableMeshGroundPreviewerPtr groundPreviewer, BaseGCSCPtr& destinationGcs, bool limitResolution) override
            {
            return SMStatus::S_ERROR;
            }

        virtual BentleyStatus                   _CreateCoverage( const bvector<DPoint3d>& coverageData, uint64_t id, const Utf8String& coverageName) override { return ERROR; };
        virtual void                           _GetAllCoverages(bvector<bvector<DPoint3d>>& coverageData) override {};
        virtual void                               _GetCoverageIds(bvector<uint64_t>& ids) const override {};
        virtual BentleyStatus                      _DeleteCoverage(uint64_t id) override { return SUCCESS; };        
        virtual void                               _GetCoverageName(Utf8String& name, uint64_t id) const override { assert(false); };
		virtual IScalableMeshClippingOptions&      _EditClippingOptions() override { assert(false); return m_options; };
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

        virtual void                          _RemoveAllDisplayData() override {}

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
        
        virtual void _ReFilter() override {};
           
    };

   BENTLEY_SM_EXPORT void MergePolygonSets(bvector<bvector<DPoint3d>>& polygons);
   BENTLEY_SM_EXPORT void MergePolygonSets(bvector<bvector<DPoint3d>>& polygons, std::function<bool(const size_t i, const bvector<DPoint3d>& element)> choosePolygonInSet, std::function<void(const bvector<DPoint3d>& element)> afterPolygonAdded);


/*__PUBLISH_SECTION_START__*/
END_BENTLEY_SCALABLEMESH_NAMESPACE
