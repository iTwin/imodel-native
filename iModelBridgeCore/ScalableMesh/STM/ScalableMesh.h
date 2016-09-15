/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMesh.h $
|    $RCSfile: ScalableMesh.h,v $
|   $Revision: 1.54 $
|       $Date: 2012/01/06 16:30:13 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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

#include "SMPointIndex.h"
#include "SMMeshIndex.h"

#include <ScalableMesh/GeoCoords/Reprojection.h>
#include <ScalableMesh/GeoCoords/GCS.h>
#include "ScalableMeshMemoryPools.h"

#include "Stores/SMSQLiteStore.h"

#include "ScalableMeshVolume.h"

#include <CloudDataSource/DataSourceManager.h>

/*__PUBLISH_SECTION_START__*/
using namespace BENTLEY_NAMESPACE_NAME::GeoCoordinates;


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

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

    static DataSourceManager            s_dataSourceManager;
    DataSourceAccount               *   m_dataSourceAccount;

    // NOTE: Stored in order to make it possible for the creator to use this. Remove when creator does not depends on
    // this interface anymore (take only a path).
    const WString                        m_path; 


    explicit                            ScalableMeshBase(SMSQLiteFilePtr& smSQLiteFile, const WString&             filePath);


/*__PUBLISH_SECTION_START__*/

    virtual                             ~ScalableMeshBase                 () = 0;


    bool                                LoadGCSFrom();


public:

    const SMSQLiteFilePtr&              GetDbFile() const;

    const WChar*                        GetPath                 () const;

    static DataSourceManager &          GetDataSourceManager    (void)                                  {return s_dataSourceManager;}
    void                                SetDataSourceAccount    (DataSourceAccount *dataSourceAccount)  {m_dataSourceAccount = dataSourceAccount;}
    DataSourceAccount *                 GetDataSourceAccount    (void) const                            {return m_dataSourceAccount;}
    
    DataSourceStatus                    InitializeAzureTest     (void);

    };


class ScalableMeshDTM : public RefCounted<BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM>
    {
    ScalableMeshDraping* m_draping;
    IScalableMesh* m_scMesh;
    Transform m_transformToUors;
    IDTMVolume* m_dtmVolume;
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

    virtual DTMStatusInt _ExportToGeopakTinFile(WCharCP fileNameP) override;

    public:
        ScalableMeshDTM(IScalableMeshPtr scMesh)
            {
            m_draping = new ScalableMeshDraping(scMesh);
            m_dtmVolume = new ScalableMeshVolume(scMesh);
            m_scMesh = scMesh.get();
            }

        virtual ~ScalableMeshDTM()
            {
            delete m_draping;
            delete m_dtmVolume;
            }

        static RefCountedPtr<ScalableMeshDTM> Create(IScalableMeshPtr scMesh)
            {
            return new ScalableMeshDTM(scMesh);
            }

        void SetStorageToUors(DMatrix4d& storageToUors)
            {
            m_transformToUors.InitFrom(storageToUors);
            m_draping->SetTransform(m_transformToUors);
            ((ScalableMeshVolume*)m_dtmVolume)->SetTransform(m_transformToUors);
            }

        void SetAnalysisType(DTMAnalysisType type)
            {
            m_draping->SetAnalysisType(type);
            }
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
        double                          m_minScreenPixelsPerPoint;            

        HFCPtr<MeshIndexType>          m_scmIndexPtr;                                                    
        RefCountedPtr<ScalableMeshDTM>  m_scalableMeshDTM[DTMAnalysisType::Qty];

        bvector<IScalableMeshNodePtr> m_viewedNodes;


        explicit                        ScalableMesh(SMSQLiteFilePtr& smSQLiteFile,const WString&             path);


        virtual                         ~ScalableMesh                         ();


        static IScalableMeshPtr       Open                           (SMSQLiteFilePtr& smSQLiteFile,
                                                                        const WString&             filePath,
                                                                        const Utf8String&     baseEditsFilePath,
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

    protected : 

        HFCPtr<SMPointIndexNode<INDEXPOINT, Extent3dType>> GetRootNode();                    
        virtual void                               _TextureFromRaster(HIMMosaic* mosaicP, Transform unitTransform = Transform::FromIdentity()) override;
 
        virtual __int64          _GetPointCount() override;

        virtual bool          _IsTerrain() override;
        

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
        virtual bool                               _AddClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID) override;
        virtual bool                               _RemoveClip(uint64_t clipID) override;
        virtual void                               _SetIsInsertingClips(bool toggleInsertMode) override;
        virtual void                               _ModifyClipMetadata(uint64_t clipId, double importance, int nDimensions) override;
        virtual void                               _GetAllClipsIds(bvector<uint64_t>& allClipIds) override;

        virtual bool                               _ModifySkirt(const bvector<bvector<DPoint3d>>& skirt, uint64_t skirtID) override;
        virtual bool                               _AddSkirt(const bvector<bvector<DPoint3d>>& skirt, uint64_t skirtID) override;
        virtual bool                               _RemoveSkirt(uint64_t skirtID) override;
        virtual int                                _ConvertToCloud(const WString& pi_pOutputDirPath) const override;


        virtual void                               _GetCurrentlyViewedNodes(bvector<IScalableMeshNodePtr>& nodes) override;
        virtual void                               _SetCurrentlyViewedNodes(const bvector<IScalableMeshNodePtr>& nodes) override;
        
#ifdef SCALABLE_MESH_ATP
        virtual int                    _LoadAllNodeHeaders(size_t& nbLoadedNodes, int level) const override;
        virtual int                    _LoadAllNodeData(size_t& nbLoadedNodes, int level) const override;
        virtual int                    _SaveGroupedNodeHeaders(const WString& pi_pOutputDirPath) const override;
#endif

        virtual void                               _SetEditFilesBasePath(const Utf8String& path) override;

        virtual IScalableMeshNodePtr               _GetRootNode() override;

        //Data source synchronization functions.
        virtual bool                   _InSynchWithSources() const override; 
        virtual bool                   _LastSynchronizationCheck(time_t& last) const override;        
        virtual int                    _SynchWithSources() override;           

/*__PUBLISH_SECTION_START__*/

    public:    

        void                ComputeTileBoundaryDuringQuery(bool doCompute);    

        double              GetMinScreenPixelsPerPoint();   
    
        void                SetMinScreenPixelsPerPoint(double pi_minScreenPixelsPerPoint);                       
                                    
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

        virtual void                               _TextureFromRaster(HIMMosaic* mosaicP, Transform unitTransform = Transform::FromIdentity()) override;

        // Inherited from IDTM   
        virtual __int64          _GetPointCount() override;

        virtual bool          _IsTerrain() override;

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
        virtual bool                               _ModifyClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID) override;
        virtual bool                               _AddClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID) override;
        virtual bool                               _RemoveClip(uint64_t clipID) override;
        virtual void                               _SetIsInsertingClips(bool toggleInsertMode) override;
        virtual void                               _ModifyClipMetadata(uint64_t clipId, double importance, int nDimensions) override;
        virtual void                               _GetAllClipsIds(bvector<uint64_t>& allClipIds) override;

        virtual bool                               _ModifySkirt(const bvector<bvector<DPoint3d>>& skirt, uint64_t skirtID) override;
        virtual bool                               _AddSkirt(const bvector<bvector<DPoint3d>>& skirt, uint64_t skirtID) override;
        virtual bool                               _RemoveSkirt(uint64_t skirtID) override;
        
        virtual void                               _GetCurrentlyViewedNodes(bvector<IScalableMeshNodePtr>& nodes) override;
        virtual void                               _SetCurrentlyViewedNodes(const bvector<IScalableMeshNodePtr>& nodes) override;
        //Data source synchronization functions.
        virtual bool                   _InSynchWithSources() const override; 
        virtual bool                   _LastSynchronizationCheck(time_t& last) const override;        
        virtual int                    _SynchWithSources() override;

        virtual int                    _GetRangeInSpecificGCS(DPoint3d& lowPt, DPoint3d& highPt, BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCS) const override;
        virtual int                    _ConvertToCloud(const WString& pi_pOutputDirPath) const override { return ERROR; }

        virtual void                               _SetEditFilesBasePath(const Utf8String& path) override { assert(false); };
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

#ifdef SCALABLE_MESH_ATP
        virtual int                    _LoadAllNodeHeaders(size_t& nbLoadedNodes, int level) const override {return ERROR;}
        virtual int                    _LoadAllNodeData(size_t& nbLoadedNodes, int level) const override { return ERROR; }
        virtual int                    _SaveGroupedNodeHeaders(const WString& pi_pOutputDirPath) const override { return ERROR; }
#endif
           
    };

/*__PUBLISH_SECTION_START__*/
END_BENTLEY_SCALABLEMESH_NAMESPACE
