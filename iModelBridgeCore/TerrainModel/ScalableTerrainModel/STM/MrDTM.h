/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/STM/MrDTM.h $
|    $RCSfile: MrDTM.h,v $
|   $Revision: 1.54 $
|       $Date: 2012/01/06 16:30:13 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
|                                                                        |
|    MrDTMNewFileCreator.h                              (C) Copyright 2001.        |
|                                                BCIVIL Corporation.        |
|                                                All Rights Reserved.    |
|                                                                       |
+----------------------------------------------------------------------*/

#pragma once

/*__PUBLISH_SECTION_START__*/
#include <ScalableTerrainModel/IMrDTM.h>
#include <ScalableTerrainModel/IMrDTMClipContainer.h>

/*----------------------------------------------------------------------+
| This template class must be exported, so we instanciate and export it |
+----------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
| CLASS definitions                                                     |
+----------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
| Core DTM Methods                                                      |
+----------------------------------------------------------------------*/

#include "MrDTMCoreDefs.h"
#include "MrDTMCoreFns.h"


USING_NAMESPACE_BENTLEY_TERRAINMODEL

/*__PUBLISH_SECTION_END__*/

#include "HGFPointIndex.h"
#include "HGFPointTileStore.h"
#include "HGFFeatureIndex.h"
#include <ScalableTerrainModel/GeoCoords/Reprojection.h>
#include <ScalableTerrainModel/GeoCoords/GCS.h>



/*__PUBLISH_SECTION_START__*/
using namespace Bentley::GeoCoordinates;

BEGIN_BENTLEY_MRDTM_NAMESPACE

typedef IDTMFile::Extent3d64f        YProtPtExtentType;
typedef HGF3DExtent<double> YProtFeatureExtentType;

/*----------------------------------------------------------------------------+
|Class MrDTMBase
+----------------------------------------------------------------------------*/
struct MrDTMBase : public RefCounted<IMrDTM>
    {
protected:

/*__PUBLISH_SECTION_END__*/
    IDTMFile::File::Ptr                 m_iDTMFilePtr;  
    size_t                              m_workingLayer;

    GeoCoords::GCS                      m_sourceGCS;
    DRange3d                            m_contentExtent;

    // NOTE: Stored in order to make it possible for the creator to use this. Remove when creator does not depends on
    // this interface anymore (take only a path).
    const WString                  m_path; 

    explicit                            MrDTMBase                  (IDTMFile::File::Ptr&            filePtr, 
                                                                    const WString&             filePath);

/*__PUBLISH_SECTION_START__*/

    virtual                             ~MrDTMBase                 () = 0;


    bool                                LoadGCSFrom                (const IDTMFile::LayerDir&       layerDir);

public:
    const IDTMFile::File::Ptr&          GetIDTMFile                () const;
    const WChar*                      GetPath                    () const;

    };

/*----------------------------------------------------------------------------+
|Class MrDTM
+----------------------------------------------------------------------------*/
template <class INDEXPOINT> class MrDTM : public MrDTMBase
    {
/*__PUBLISH_SECTION_END__*/
    private : 
             
        friend struct IMrDTM;        
        friend class auto_ptr<MrDTM>;

        typedef DTMTaggedTileStore<HFCPtr<HVEDTMLinearFeature>, INDEXPOINT, YProtPtExtentType, HGF3DCoord<double>, YProtFeatureExtentType >
                                        TileStoreType;

        typedef HGFPointIndex<INDEXPOINT, YProtPtExtentType>
                                        PointIndexType;
        typedef HGFFeatureIndex<HFCPtr<HVEDTMLinearFeature>, HGF3DCoord<double>, YProtFeatureExtentType >
                                        LinearIndexType;


        bool                            m_areDataCompressed; 
        bool                            m_computeTileBoundary;
        double                          m_minScreenPixelsPerPoint;            

        HFCPtr<PointIndexType>          m_mrDTMPointIndexPtr;        
        HFCPtr<LinearIndexType>         m_mrDTMLinearIndexPtr;                                              
                        
        explicit                        MrDTM                          (IDTMFile::File::Ptr&            iDTMFilePtr, 
                                                                        const WString&             path);

        virtual                         ~MrDTM                         ();


        static MrDTM<INDEXPOINT>*       Open                           (IDTMFile::File::Ptr&            filePtr, 
                                                                        const WString&             filePath, 
                                                                        StatusInt&                      status);

        int                             Open                           ();                
                    
        int                             Close                          ();

  
        unsigned __int64                CountPointsInExtent(YProtPtExtentType&  extent, 
                                                            HFCPtr<HGFPointIndexNode<INDEXPOINT, YProtPtExtentType>> nodePtr,
                                                            const unsigned __int64& maxNumberCountedPoints) const;

        unsigned __int64                CountLinearsInExtent(YProtFeatureExtentType& extent, unsigned __int64 maxFeatures) const;

        static DRange3d                 ComputeTotalExtentFor          (const PointIndexType*           pointIndexP,
                                                                        const LinearIndexType*          linearIndexP);

        bool                            AreDataCompressed              ();

        void                            AddBreaklineSet                (list<HFCPtr<HVEDTMLinearFeature> >& breaklineList, 
                                                                        BC_DTM_OBJ*                         po_ppBcDtmObj);

        IHGFPointIndexFilter<INDEXPOINT, YProtPtExtentType>* CreatePointIndexFilter(IDTMFile::UniformFeatureDir* pointDirPtr) const;

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
        virtual IDTMVolumeP     _GetDTMVolume() override;

        virtual DTMStatusInt     _GetRange(DRange3dR range) override;
        virtual BcDTMP           _GetBcDTM() override;
        virtual DTMStatusInt     _GetBoundary(BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPointArray& boundary) override;
        virtual DTMStatusInt     _CalculateSlopeArea(double&, double&, const DPoint3d*, int) override;
        virtual DTMStatusInt     _CalculateSlopeArea(double& flatArea, double& slopeArea, DPoint3dCP pts, int numPoints, DTMAreaValuesCallback progressiveCallback, DTMCancelProcessCallback isCancelledCallback) override;
        virtual bool             _GetTransformation(TransformR) override;
        virtual DTMStatusInt     _GetTransformDTM(BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr& transformedDTM, TransformCR) override;

    // Inherited from IMRDTM                   
        virtual Count                  _GetCountInRange (const DRange2d& range, const CountType& type, const unsigned __int64& maxNumberCountedPoints) const;
        virtual int                    _GenerateSubResolutions() override;
        virtual __int64                _GetBreaklineCount() const override;
        virtual MrDTMCompressionType   _GetCompressionType() const override;          
        virtual int                    _GetNbResolutions(DTMQueryDataType queryDataType) const override;        
        virtual IMrDTMQueryPtr         _GetQueryInterface(DTMQueryType     queryType, 
                                                          DTMQueryDataType queryDataType) const override;
        virtual IMrDTMQueryPtr         _GetQueryInterface(DTMQueryType                         queryType, 
                                                          DTMQueryDataType                     queryDataType, 
                                                          Bentley::GeoCoordinates::BaseGCSPtr& targetGCS,
                                                          const DRange3d&                      extentInTargetGCS) const override;        
        virtual const GeoCoords::GCS&  _GetGCS() const override;
        virtual StatusInt              _SetGCS(const GeoCoords::GCS& sourceGCS) override;        
        virtual MrDTMState             _GetState() const override;     
        virtual bool                   _IsProgressive() const override;    
        virtual bool                   _IsReadOnly() const override;
        virtual bool                   _IsShareable() const override;
        virtual int                    _GetRangeInSpecificGCS(DPoint3d& lowPt, DPoint3d& highPt, Bentley::GeoCoordinates::BaseGCSPtr& targetGCS) const;
        


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

template<class POINT> class MrDTMFixResolutionViewPointQuery;

//MS It should inherit from IDTM but the query interface is not implemented at that level. 
template <class POINT> class MrDTMSingleResolutionPointIndexView : public RefCounted<IMrDTM>
    {        
    friend class MrDTMFixResolutionViewPointQuery<POINT>;

    private : 
                
        HFCPtr<HGFPointIndex<POINT, YProtPtExtentType>> m_mrDTMPointIndexPtr;
        int                                             m_resolutionIndex;
        GeoCoords::GCS                                  m_sourceGCS;
        
    protected : 

        MrDTMSingleResolutionPointIndexView(HFCPtr<HGFPointIndex<POINT, YProtPtExtentType>> mrDTMPointIndexPtr, 
                                            int                                             resolutionIndex, 
                                            GeoCoords::GCS                             sourceGCS);

        virtual ~MrDTMSingleResolutionPointIndexView();

        // Inherited from IDTM   
        virtual __int64          _GetPointCount() override;
        virtual IDTMDrapingP     _GetDTMDraping() override;
        virtual IDTMDrainageP    _GetDTMDrainage() override;
        virtual IDTMContouringP  _GetDTMContouring() override;
        virtual IDTMVolumeP     _GetDTMVolume() override;

        virtual DTMStatusInt     _GetRange(DRange3dR range) override;
        virtual BcDTMP           _GetBcDTM() override;
        virtual DTMStatusInt     _GetBoundary(BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPointArray& boundary) override;
        virtual DTMStatusInt     _CalculateSlopeArea(double&, double&, const DPoint3d*, int) override;
        virtual DTMStatusInt    _CalculateSlopeArea(double& flatArea, double& slopeArea, DPoint3dCP pts, int numPoints, DTMAreaValuesCallback progressiveCallback, DTMCancelProcessCallback isCancelledCallback) override;
        virtual bool             _GetTransformation(TransformR) override;
        virtual DTMStatusInt     _GetTransformDTM(BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr& transformedDTM, TransformCR) override;

        // Inherited from IMRDTM             
        virtual Count                  _GetCountInRange (const DRange2d& range, const CountType& type, const unsigned __int64& maxNumberCountedPoints) const override;
        virtual int                    _GenerateSubResolutions() override;
        virtual __int64                _GetBreaklineCount() const override;
        virtual MrDTMCompressionType   _GetCompressionType() const override;           
        virtual int                    _GetNbResolutions(DTMQueryDataType queryDataType) const override;        
        virtual IMrDTMQueryPtr         _GetQueryInterface(DTMQueryType     queryType, 
                                                          DTMQueryDataType queryDataType) const override;
        virtual IMrDTMQueryPtr         _GetQueryInterface(DTMQueryType                         queryType, 
                                                          DTMQueryDataType                     queryDataType, 
                                                          Bentley::GeoCoordinates::BaseGCSPtr& targetGCS,
                                                          const DRange3d&                      extentInTargetGCS) const override;                
        const GeoCoords::GCS&          _GetGCS() const override;
        StatusInt                      _SetGCS(const GeoCoords::GCS& sourceGCS) override;
        virtual MrDTMState             _GetState() const override;   
        virtual bool                   _IsProgressive() const override;       
        virtual bool                   _IsReadOnly() const override;
        virtual bool                   _IsShareable() const override;
        
        //Data source synchronization functions.
        virtual bool                   _InSynchWithSources() const override; 
        virtual bool                   _LastSynchronizationCheck(time_t& last) const override;        
        virtual int                    _SynchWithSources() override;

        virtual int                    _GetRangeInSpecificGCS(DPoint3d& lowPt, DPoint3d& highPt, Bentley::GeoCoordinates::BaseGCSPtr& targetGCS) const override;
           
    };

/*__PUBLISH_SECTION_START__*/
END_BENTLEY_MRDTM_NAMESPACE