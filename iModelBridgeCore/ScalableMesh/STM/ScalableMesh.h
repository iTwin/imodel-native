/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMesh.h $
|    $RCSfile: ScalableMesh.h,v $
|   $Revision: 1.54 $
|       $Date: 2012/01/06 16:30:13 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
#include "SMPointTileStore.h"
#ifdef SCALABLE_MESH_DGN
#include <ImagePP/all/h/DgnTileStore.h>
#endif
#include <ScalableMesh/GeoCoords/Reprojection.h>
#include <ScalableMesh/GeoCoords/GCS.h>
#include "ScalableMeshMemoryPools.h"


/*__PUBLISH_SECTION_START__*/
using namespace Bentley::GeoCoordinates;

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

typedef IDTMFile::Extent3d64f        YProtPtExtentType;
typedef HGF3DExtent<double> YProtFeatureExtentType;

/*----------------------------------------------------------------------------+
|Class ScalableMeshBase
+----------------------------------------------------------------------------*/
struct ScalableMeshBase : public RefCounted<IScalableMesh>
    {
protected:

/*__PUBLISH_SECTION_END__*/
    IDTMFile::File::Ptr                 m_iDTMFilePtr; 
#ifdef SCALABLE_MESH_DGN
    IDgnDbScalableMeshPtr               m_dgnScalableMeshPtr;
    bool                                m_isDgnDB;
#endif
    size_t                              m_workingLayer;

    GeoCoords::GCS                      m_sourceGCS;
    DRange3d                            m_contentExtent;

    // NOTE: Stored in order to make it possible for the creator to use this. Remove when creator does not depends on
    // this interface anymore (take only a path).
    const WString                  m_path; 

    explicit                            ScalableMeshBase                  (IDTMFile::File::Ptr&            filePtr, 
                                                                    const WString&             filePath);

/*__PUBLISH_SECTION_START__*/

    virtual                             ~ScalableMeshBase                 () = 0;


    bool                                LoadGCSFrom                (const IDTMFile::LayerDir&       layerDir);

public:
    const IDTMFile::File::Ptr&          GetIDTMFile                () const;
    const WChar*                      GetPath                    () const;

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

        typedef SMPointTaggedTileStore<INDEXPOINT, YProtPtExtentType >
                                        TileStoreType;

#ifdef SCALABLE_MESH_DGN
        typedef DgnTaggedTileStore<HFCPtr<HVEDTMLinearFeature>,
            INDEXPOINT,
            YProtPtExtentType,
            HGF3DCoord<double>,
            YProtFeatureExtentType >     DgnStoreType;
#endif

        typedef SMMeshIndex<INDEXPOINT, YProtPtExtentType>
                                        PointIndexType;


        bool                            m_areDataCompressed; 
        bool                            m_computeTileBoundary;
        double                          m_minScreenPixelsPerPoint;            

        HFCPtr<PointIndexType>          m_scmPointIndexPtr;                                                    
                        
        explicit                        ScalableMesh                          (IDTMFile::File::Ptr&            iDTMFilePtr, 
                                                                        const WString&             path);

        virtual                         ~ScalableMesh                         ();


        static ScalableMesh<INDEXPOINT>*       Open                           (IDTMFile::File::Ptr&            filePtr, 
                                                                        const WString&             filePath, 
                                                                        StatusInt&                      status);

        int                             Open                           ();                
                    
        int                             Close                          ();

  
        unsigned __int64                CountPointsInExtent(YProtPtExtentType&  extent, 
                                                            HFCPtr<SMPointIndexNode<INDEXPOINT, YProtPtExtentType>> nodePtr,
                                                            const unsigned __int64& maxNumberCountedPoints) const;

        unsigned __int64                CountLinearsInExtent(YProtFeatureExtentType& extent, unsigned __int64 maxFeatures) const;


        static DRange3d                 ComputeTotalExtentFor          (const PointIndexType*           pointIndexP);

        bool                            AreDataCompressed              ();

        void                            AddBreaklineSet                (list<HFCPtr<HVEDTMLinearFeature> >& breaklineList, 
                                                                        BC_DTM_OBJ*                         po_ppBcDtmObj);

        ISMPointIndexFilter<INDEXPOINT, YProtPtExtentType>* CreatePointIndexFilter(IDTMFile::UniformFeatureDir* pointDirPtr) const;

        void CreateSpatialIndexFromExtents(list<HVE2DSegment>& pi_rpBreaklineList, 
                                           BC_DTM_OBJ**        po_ppBcDtmObj);
        
        bool                            IsValidInContextOf             (const GeoCoords::GCS&           gcs) const;

        bool                            IsEmpty                        () const;

    protected : 
            
    // Inherited from IDTM   
        virtual __int64          _GetPointCount() override;
        
        virtual IDTMDrapingP     _GetDTMDraping() override;
        virtual IDTMDrainageP    _GetDTMDrainage() override;
        virtual IDTMContouringP  _GetDTMContouring() override;
        virtual IDTMVolumeP  _GetDTMVolume() override;

        virtual DTMStatusInt     _GetRange(DRange3dR range) override;
        virtual BcDTMP           _GetBcDTM() override;
        virtual DTMStatusInt     _GetBoundary(Bentley::TerrainModel::DTMPointArray& boundary) override;
        virtual DTMStatusInt     _CalculateSlopeArea(double&, double&, const DPoint3d*, int) override;
        virtual bool             _GetTransformation(TransformR) override;
        virtual DTMStatusInt     _GetTransformDTM(Bentley::TerrainModel::DTMPtr& transformedDTM, TransformCR) override;

    // Inherited from IMRDTM                   
        virtual Count                  _GetCountInRange (const DRange2d& range, const CountType& type, const unsigned __int64& maxNumberCountedPoints) const;
        virtual int                    _GenerateSubResolutions() override;
        virtual __int64                _GetBreaklineCount() const override;
        virtual ScalableMeshCompressionType   _GetCompressionType() const override;          
        virtual int                    _GetNbResolutions(DTMQueryDataType queryDataType) const override;        
        virtual IScalableMeshQueryPtr         _GetQueryInterface(DTMQueryType     queryType, 
                                                          DTMQueryDataType queryDataType) const override;
        virtual IScalableMeshQueryPtr         _GetQueryInterface(DTMQueryType                         queryType, 
                                                          DTMQueryDataType                     queryDataType, 
                                                          Bentley::GeoCoordinates::BaseGCSPtr& targetGCS,
                                                          const DRange3d&                      extentInTargetGCS) const override;        
        virtual IScalableMeshMeshQueryPtr     _GetMeshQueryInterface(MeshQueryType queryType) const override;
        virtual IScalableMeshMeshQueryPtr     _GetMeshQueryInterface(MeshQueryType queryType,
                                                              Bentley::GeoCoordinates::BaseGCSPtr& targetGCS,
                                                              const DRange3d&                      extentInTargetGCS) const override;
        virtual IScalableMeshNodeRayQueryPtr     _GetNodeQueryInterface() const override;
        virtual const GeoCoords::GCS&  _GetGCS() const override;
        virtual StatusInt              _SetGCS(const GeoCoords::GCS& sourceGCS) override;        
        virtual ScalableMeshState             _GetState() const override;     
        virtual bool                   _IsProgressive() const override;    
        virtual bool                   _IsReadOnly() const override;
        virtual bool                   _IsShareable() const override;
        virtual int                    _GetRangeInSpecificGCS(DPoint3d& lowPt, DPoint3d& highPt, Bentley::GeoCoordinates::BaseGCSPtr& targetGCS) const;
        
#ifdef SCALABLE_MESH_ATP
        virtual int                    _LoadAllNodeHeaders(size_t& nbLoadedNodes) const override; 
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
    };


/*__PUBLISH_SECTION_END__*/

template<class POINT> class ScalableMeshFixResolutionViewPointQuery;

//MS It should inherit from IDTM but the query interface is not implemented at that level. 
template <class POINT> class ScalableMeshSingleResolutionPointIndexView : public RefCounted<IScalableMesh>
    {        
    friend class ScalableMeshFixResolutionViewPointQuery<POINT>;

    private : 
                
        HFCPtr<SMPointIndex<POINT, YProtPtExtentType>> m_scmPointIndexPtr;
        int                                             m_resolutionIndex;
        GeoCoords::GCS                                  m_sourceGCS;
        
    protected : 

        ScalableMeshSingleResolutionPointIndexView(HFCPtr<SMPointIndex<POINT, YProtPtExtentType>> scmPointIndexPtr, 
                                            int                                             resolutionIndex, 
                                            GeoCoords::GCS                             sourceGCS);

        virtual ~ScalableMeshSingleResolutionPointIndexView();

        // Inherited from IDTM   
        virtual __int64          _GetPointCount() override;
        virtual IDTMDrapingP     _GetDTMDraping() override;
        virtual IDTMDrainageP    _GetDTMDrainage() override;
        virtual IDTMContouringP  _GetDTMContouring() override;
        virtual IDTMVolumeP  _GetDTMVolume() override;

        virtual DTMStatusInt     _GetRange(DRange3dR range) override;
        virtual BcDTMP           _GetBcDTM() override;
        virtual DTMStatusInt     _GetBoundary(Bentley::TerrainModel::DTMPointArray& boundary) override;
        virtual DTMStatusInt     _CalculateSlopeArea(double&, double&, const DPoint3d*, int) override;
        virtual bool             _GetTransformation(TransformR) override;
        virtual DTMStatusInt     _GetTransformDTM(Bentley::TerrainModel::DTMPtr& transformedDTM, TransformCR) override;

        // Inherited from IMRDTM             
        virtual Count                  _GetCountInRange (const DRange2d& range, const CountType& type, const unsigned __int64& maxNumberCountedPoints) const override;
        virtual int                    _GenerateSubResolutions() override;
        virtual __int64                _GetBreaklineCount() const override;
        virtual ScalableMeshCompressionType   _GetCompressionType() const override;           
        virtual int                    _GetNbResolutions(DTMQueryDataType queryDataType) const override;        
        virtual IScalableMeshQueryPtr         _GetQueryInterface(DTMQueryType     queryType, 
                                                          DTMQueryDataType queryDataType) const override;
        virtual IScalableMeshQueryPtr         _GetQueryInterface(DTMQueryType                         queryType, 
                                                          DTMQueryDataType                     queryDataType, 
                                                          Bentley::GeoCoordinates::BaseGCSPtr& targetGCS,
                                                          const DRange3d&                      extentInTargetGCS) const override;                
        virtual IScalableMeshMeshQueryPtr     _GetMeshQueryInterface(MeshQueryType queryType) const override; 
        virtual IScalableMeshMeshQueryPtr     _GetMeshQueryInterface(MeshQueryType queryType,
                                                              Bentley::GeoCoordinates::BaseGCSPtr& targetGCS,
                                                              const DRange3d&                      extentInTargetGCS) const override;
        virtual IScalableMeshNodeRayQueryPtr     _GetNodeQueryInterface() const override;
        const GeoCoords::GCS&          _GetGCS() const override;
        StatusInt                      _SetGCS(const GeoCoords::GCS& sourceGCS) override;
        virtual ScalableMeshState             _GetState() const override;   
        virtual bool                   _IsProgressive() const override;       
        virtual bool                   _IsReadOnly() const override;
        virtual bool                   _IsShareable() const override;
        
        //Data source synchronization functions.
        virtual bool                   _InSynchWithSources() const override; 
        virtual bool                   _LastSynchronizationCheck(time_t& last) const override;        
        virtual int                    _SynchWithSources() override;

        virtual int                    _GetRangeInSpecificGCS(DPoint3d& lowPt, DPoint3d& highPt, Bentley::GeoCoordinates::BaseGCSPtr& targetGCS) const override;

#ifdef SCALABLE_MESH_ATP
        virtual int                    _LoadAllNodeHeaders(size_t& nbLoadedNodes) const override {return ERROR;}
#endif
           
    };

/*__PUBLISH_SECTION_START__*/
END_BENTLEY_SCALABLEMESH_NAMESPACE
