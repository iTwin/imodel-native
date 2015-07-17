/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/STM/MrDTMQuery.h $
|    $RCSfile: MrDTMQuery.h,v $
|   $Revision: 1.20 $
|       $Date: 2012/06/27 14:07:12 $
|     $Author: Chantal.Poulin $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
|																		|
|	MrDTMNewFileCreator.h    		  	    		(C) Copyright 2001.		|
|												BCIVIL Corporation.		|
|												All Rights Reserved.	|
|                                                                       |
+----------------------------------------------------------------------*/

#pragma once

#include <ScalableTerrainModel/GeoCoords/GCS.h>
#include <ScalableTerrainModel/GeoCoords/Reprojection.h>

#include <ImagePP/all/h/HCPGCoordModel.h>

#include <ScalableTerrainModel/IMrDTMClipContainer.h>
#include <ScalableTerrainModel/IMrDTM.h>
#include <hash_map>

//Only way found to deactivate warning C4250 since the pragma warning(disable... doesn't work
#pragma warning( push, 0 )



BEGIN_BENTLEY_MRDTM_NAMESPACE

struct MrDTMViewDependentQueryParams;
class MrDTMViewDependentQuery;

typedef IDTMFile::Extent3d64f YProtPtExtentType;
typedef HGF3DExtent<double> YProtFeatureExtentType;

struct MrDTMExtentQuery;
typedef RefCountedPtr<MrDTMExtentQuery> MrDTMExtentQueryPtr;


/*==================================================================*/
/*        QUERY PARAMETERS IMPLEMENTATION SECTION - START           */
/*==================================================================*/
struct MrDTMQueryParameters : public virtual IMrDTMQueryParameters                              
    {    
    private: 
       
        bool                                m_isReturnedDataTriangulated;
        Bentley::GeoCoordinates::BaseGCSPtr m_sourceGCSPtr;
        Bentley::GeoCoordinates::BaseGCSPtr m_targetGCSPtr;
        long                                m_edgeOption;      
        double                              m_maxSideLength;   

    protected:                  
        
        virtual bool _GetTriangulationState() override
            {
            return m_isReturnedDataTriangulated;
            }
        
        virtual void _SetTriangulationState(bool pi_isReturnedDataTriangulated) override
            {
            m_isReturnedDataTriangulated = pi_isReturnedDataTriangulated;
            }

        virtual Bentley::GeoCoordinates::BaseGCSPtr _GetSourceGCS() override
            {
            return m_sourceGCSPtr;
            }

        virtual Bentley::GeoCoordinates::BaseGCSPtr _GetTargetGCS() override
            {
            return m_targetGCSPtr;
            }

        virtual void _SetGCS(Bentley::GeoCoordinates::BaseGCSPtr& sourceGCSPtr, 
                             Bentley::GeoCoordinates::BaseGCSPtr& targetGCSPtr) override
            {
            m_sourceGCSPtr = sourceGCSPtr;
            m_targetGCSPtr = targetGCSPtr;
            }

        virtual long _GetEdgeOptionTriangulationParam() override
            {
            return m_edgeOption;
            }

        virtual double _GetMaxSideLengthTriangulationParam() override
            {
            return m_maxSideLength;
            }
                            
        virtual void _SetEdgeOptionTriangulationParam(long edgeOption) override
            {
            m_edgeOption = edgeOption;
            }

        virtual void _SetMaxSideLengthTriangulationParam(double maxSideLength) override
            {
            m_maxSideLength = maxSideLength;
            }
                            
    public:

        MrDTMQueryParameters()
        : m_isReturnedDataTriangulated(true),
          m_edgeOption(2),
          m_maxSideLength(500)
            {                 
            }    
    };

struct MrDTMFullResolutionQueryParams :  public virtual MrDTMQueryParameters,
                                         public virtual IMrDTMFullResolutionQueryParams
                                        
    {    
    private : 
        bool   m_returnAllPtsForLowestLevel;
        size_t m_maximumNumberOfPoints;

    protected :
        
        virtual size_t _GetMaximumNumberOfPoints() //override
            {    
            return m_maximumNumberOfPoints;
            }

        virtual void _SetMaximumNumberOfPoints(size_t maximumNumberOfPoints) //override
            {    
            m_maximumNumberOfPoints = maximumNumberOfPoints;
            }
        
        virtual bool _GetReturnAllPtsForLowestLevel() //override
            {
            return m_returnAllPtsForLowestLevel;
            }

        virtual void _SetReturnAllPtsForLowestLevel(bool returnAllPts) //override
            {
            m_returnAllPtsForLowestLevel = returnAllPts;
            }

    public : 

        MrDTMFullResolutionQueryParams()
            {
            m_returnAllPtsForLowestLevel = true;
            m_maximumNumberOfPoints = UINT_MAX;
            }

        virtual ~MrDTMFullResolutionQueryParams() {}
    };

struct MrDTMFullResolutionLinearQueryParams : public virtual IMrDTMFullResolutionLinearQueryParams,
                                              public virtual MrDTMFullResolutionQueryParams
    {    
    private : 

        size_t           m_maximumNumberOfPointsForLinear;
        bool             m_useDecimation;
        bool             m_cutLinears;          
        bool             m_addLinears;
        std::vector<int> m_filteringFeatureTypes;
        bool             m_doIncludeFilteringFeatureTypes;        
                 
    protected :        

        virtual size_t _GetMaximumNumberOfPointsForLinear() override
            {
            return m_maximumNumberOfPointsForLinear;
            }

        virtual int _SetMaximumNumberOfPointsForLinear(size_t maximumNumberOfPointsForLinear) override
            {
            int status;

            if (maximumNumberOfPointsForLinear > 10)
                {
                m_maximumNumberOfPointsForLinear = maximumNumberOfPointsForLinear;

                status = SUCCESS;
                }
            else
                {
                status = ERROR;
                }

            return status;
            }
                
        virtual void _SetUseDecimation(bool useDecimation) override
            {
            m_useDecimation = useDecimation;
            }
               
        virtual bool _GetUseDecimation() override
            {
            return m_useDecimation;
            }
        
        virtual void _SetCutLinears(bool cutLinears) override
            {
            m_cutLinears = cutLinears;
            }

        virtual bool _GetCutLinears() override
            {
            return m_cutLinears;
            }

        virtual void _SetAddLinears(const bool addLinears) override
            {
            m_addLinears = addLinears;
            }

        virtual bool _GetAddLinears() override
            {
            return m_addLinears;
            }

        virtual const std::vector<int>& _GetFilteringFeatureTypes(bool& doIncludeFilteringFeatureTypes) override
            {
            doIncludeFilteringFeatureTypes = m_doIncludeFilteringFeatureTypes;

            return m_filteringFeatureTypes;
            }

        //When no feature type is specified all feature types are returned.
        virtual int _SetFilteringFeatureTypes(const std::vector<int>& filteringFeatureTypes, bool doIncludeFilteringFeatures) override
            {
            m_filteringFeatureTypes.clear();

            if (filteringFeatureTypes.size() > 0)
                {
                m_filteringFeatureTypes.insert(m_filteringFeatureTypes.end(), filteringFeatureTypes.begin(), filteringFeatureTypes.end());
                }    

            m_doIncludeFilteringFeatureTypes = doIncludeFilteringFeatures;
            return SUCCESS;
            }

        virtual void _SetIncludeFilteringFeatureTypes(const bool& doIncludeFilteringFeatures) override
            {
            m_doIncludeFilteringFeatureTypes = doIncludeFilteringFeatures;
            }           
       
    public : 

        MrDTMFullResolutionLinearQueryParams()
            {
            m_maximumNumberOfPointsForLinear = UINT64_MAX;
            m_useDecimation = true;
            m_cutLinears = false;
            m_addLinears = true;
            m_doIncludeFilteringFeatureTypes = false;
            }

        virtual ~MrDTMFullResolutionLinearQueryParams()
            {
            }
    };
                                                  
struct SrDTMViewDependentQueryParams : public virtual ISrDTMViewDependentQueryParams, 
                                       public virtual MrDTMQueryParameters
    {                
    protected :

        DPoint3d m_viewBox[8];                    

        virtual const DPoint3d* _GetViewBox() const override
            {
            return m_viewBox;
            }        

        virtual void _SetViewBox(const DPoint3d viewBox[]) override
            {
            memcpy(m_viewBox, viewBox, sizeof(m_viewBox));
            }        

    public : 
                            
    
        SrDTMViewDependentQueryParams()
            {
            }

        virtual ~SrDTMViewDependentQueryParams()
            {
            }
    };


struct MrDTMViewDependentQueryParams : public virtual IMrDTMViewDependentQueryParams, 
                                       public virtual SrDTMViewDependentQueryParams         
    {    
    protected :

        double   m_minScreenPixelsPerPoint;
        double   m_rootToViewMatrix[4][4];

        //Determine if the same or different resolution method is used when the camera is off
        bool     m_useSameResolutionWhenCameraIsOff;        

        //Same resolution method's parameters 
        bool     m_useSplitThresholdForLevelSelection;

        //Different resolution method's parameters
        bool     m_useSplitThresholdForTileSelection;

        virtual double        _GetMinScreenPixelsPerPoint() const override
            {
            return m_minScreenPixelsPerPoint;
            } 

        virtual bool          _GetUseSameResolutionWhenCameraIsOff() const override
            {
            return m_useSameResolutionWhenCameraIsOff;
            }

        virtual bool          _GetUseSplitThresholdForLevelSelection() const override
            {
            return m_useSplitThresholdForLevelSelection;
            } 

        virtual bool          _GetUseSplitThresholdForTileSelection() const override
            {
            return m_useSplitThresholdForTileSelection;
            }

        virtual const double* _GetRootToViewMatrix() const override
            {
            return (double*)m_rootToViewMatrix;
            }

        virtual void          _SetMinScreenPixelsPerPoint(double minScreenPixelsPerPoint) override
            {
            m_minScreenPixelsPerPoint = minScreenPixelsPerPoint;
            }

        virtual void          _SetRootToViewMatrix(const double rootToViewMatrix[][4]) override
            {
            memcpy(m_rootToViewMatrix, rootToViewMatrix, sizeof(m_rootToViewMatrix));
            }

        virtual void          _SetUseSameResolutionWhenCameraIsOff(bool useSameResolution) override
            {
            m_useSameResolutionWhenCameraIsOff = useSameResolution;
            }

        virtual void          _SetUseSplitThresholdForLevelSelection(bool useSplitThreshold) override
            {
            m_useSplitThresholdForLevelSelection = useSplitThreshold;
            }

        virtual void          _SetUseSplitThresholdForTileSelection(bool useSplitThreshold) override
            {
            m_useSplitThresholdForTileSelection = useSplitThreshold;
            }                          
        
    public : 

        MrDTMViewDependentQueryParams()
            {
            m_useSameResolutionWhenCameraIsOff = false;                
            m_useSplitThresholdForLevelSelection = true;        
            m_useSplitThresholdForTileSelection = false;    
            m_minScreenPixelsPerPoint = 100;                        
            }

        virtual ~MrDTMViewDependentQueryParams()
            {
            }
    };



template<class POINT> class MrDTMFixResolutionViewPointQuery;



struct MrDTMFixResolutionIndexQueryParams : public virtual IMrDTMFixResolutionIndexQueryParams, 
                                            public virtual MrDTMQueryParameters
    {
    
    protected :

        int      m_resolutionIndex;
        
        virtual int  _GetResolutionIndex() const override
            {
            return m_resolutionIndex;
            }

        virtual void _SetResolutionIndex(int resolutionIndex) override
            {
            m_resolutionIndex = resolutionIndex;
            }

    public : 

        MrDTMFixResolutionIndexQueryParams()
            {
            m_resolutionIndex = 0;            
            }

        virtual ~MrDTMFixResolutionIndexQueryParams()
            {
            }        
    };

struct MrDTMFixResolutionMaxPointsQueryParams : public virtual IMrDTMFixResolutionMaxPointsQueryParams, 
                                                public virtual MrDTMQueryParameters
    {

    template<class POINT>
    friend class MrDTMFixResolutionViewPointQuery;

    private : 
    
    protected :

        __int64 m_maxNumberPoints;
        
        virtual __int64 _GetMaxNumberPoints() override 
            {
            return m_maxNumberPoints;
            }

        virtual void    _SetMaximumNumberPoints(__int64 maxNumberPoints) override
            {
            m_maxNumberPoints = maxNumberPoints;
            }

    public :

        MrDTMFixResolutionMaxPointsQueryParams()
            {
            m_maxNumberPoints = 0;
            }

        virtual ~MrDTMFixResolutionMaxPointsQueryParams()
            {
            }
    };

struct MrDTMQueryAllLinearsQueryParams : public virtual IMrDTMQueryAllLinearsQueryParams,
                                         public virtual MrDTMFullResolutionLinearQueryParams
    {    
#ifdef SCALABLE_TERRAIN_MODEL_PRIVATE_SECTION
    private : 

        typedef HFCPtr<HVEDTMLinearFeature> Feature;
        typedef list<Feature>::iterator FeatureIterator;
        list<Feature> m_features;
#endif 

    protected :

        virtual list<IMrDTMFeaturePtr> _GetFeatures()
            {
            // convert to publishable list (does not depend on ImagePP objects)
            list<IMrDTMFeaturePtr> newList;
            for (FeatureIterator it = m_features.begin(); it != m_features.end(); it++)
                {
                IMrDTMFeaturePtr feature = IMrDTMFeature::Create();
                if ((*it)->GetFeatureType() != (int)DTMFeatureType::ContourLine)
                    feature->SetType((*it)->GetFeatureType());
                else
                    feature->SetType ((int)DTMFeatureType::Breakline);
                for (size_t idx = 0; idx < (*it)->GetSize(); idx++)
                    {
                    const HGF3DPoint& point = (*it)->GetPoint(idx);
                    DPoint2d newPoint;
                    newPoint.x = point.GetX();
                    newPoint.y = point.GetY();
                    feature->AppendPoint(newPoint);
                    }
                newList.push_back(feature);
                }
            return newList;
            }

        virtual void _SetFeatures (const list<HFCPtr<HVEDTMLinearFeature>>& features)
            {
            m_features = features;
            }
               
    public : 

        MrDTMQueryAllLinearsQueryParams()
            {
            }
               
        virtual ~MrDTMQueryAllLinearsQueryParams()
            {
            }               
    };

#pragma warning( pop)

/*==================================================================*/
/*        QUERY PARAMETERS IMPLEMENTATION SECTION - END             */
/*==================================================================*/ 
class MrDTMQuery : public IMrDTMQuery
    {
                                        
    protected :
        IMrDTMClipContainerPtr m_clips;


        HFCPtr<HVEShape> m_clipShapePtr;        
        
        MrDTMQuery();
        ~MrDTMQuery();
        

        HFCPtr<HVEShape>  CreateClipShape(DRange3d& spatialIndexRange) const;
        HFCPtr<HVEShape>  CreateClipShape(HFCPtr<HVEShape> areaShape) const;
        
        template<class EXTENT> EXTENT GetExtentFromClipShape(const DPoint3d* pClipShapePts, 
                                                             int             nbClipShapePts, 
                                                             double          zMin, 
                                                             double          zMax) const;

        template <class POINT> int AddPoints(const DTMPtr&          dtmPtr, 
                                             const list<POINT>&     pointList) const;
        template <class POINT> int AddPoints(const DTMPtr&          dtmPtr, 
                                             const HPMMemoryManagedVector<POINT>& pointList) const;

        //Inherited from IMrDTMQuery
        virtual int _GetNbClip() const;
/*
        virtual int _GetClip(DPoint3d*& clipPointsP,
                             int&       numberOfPoints, 
                             bool&      isClipMask,
                             int        clipInd) const;
*/
        virtual int _AddClip(DPoint3d* clipPointsP,
                             int   numberOfPoints, 
                             bool  isClipMask);    

        virtual int _RemoveAllClip();    

    public :
       
        static int AddLinears(const DTMPtr&               dtmPtr,
                       list<HFCPtr<HVEDTMLinearFeature>>& linearList, 
                       size_t                             maxNumberOfPoints,
                       bool                               useDecimation);

		static IMrDTMQueryPtr GetReprojectionQueryInterface(IMrDTMPtr              mrDtmToQueryPtr,
                                                            DTMQueryType           queryType, 
                                                            DTMQueryDataType       queryDataType, 
                                                            const GeoCoords::GCS&   sourceGCStr,
                                                            const GeoCoords::GCS&   targetGCStr,
                                                            const DRange3d&        extentInTargetGCS);

        int _Query(Bentley::TerrainModel::DTMPtr&   dtmPtr, 
                   const DPoint3d*                  pClipShapePts, 
                   int                              nbClipShapePts, 
                   const IMrDTMQueryParametersPtr&  mrDTMQueryParamsPtr) const ;

};

template <class POINT> class MrDTMFullResolutionPointQuery : public MrDTMQuery
    {   
    public:  // OPERATOR_NEW_KLUDGE  >>> BEIJING_WIP_STM add a static create method
        void*   operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
        void    operator delete(void* rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
        void*   operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
        void    operator delete [] (void* rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

    private : 
                
        HFCPtr<HGFPointIndex<POINT, YProtPtExtentType>> m_mrDTMPointIndexPtr;
        int                                             m_resolutionIndex;

    protected :

        // Inherited from IMrDTMQuery
        virtual int _Query(Bentley::TerrainModel::DTMPtr&   dtmPtr, 
                           const DPoint3d*                  pQueryShapePts, 
                           int                              nbQueryShapePts, 
                           const IMrDTMQueryParametersPtr&  mrDTMQueryParamsPtr) const;
                           
    public :
        
        MrDTMFullResolutionPointQuery(const HFCPtr<HGFPointIndex<POINT, YProtPtExtentType>>& pointIndexPtr, 
                                      int                                              resolutionIndex);
        
        virtual ~MrDTMFullResolutionPointQuery();     
    }; 

template <class POINT> class MrDTMViewDependentPointQuery : public MrDTMQuery
    {   
    public:  // OPERATOR_NEW_KLUDGE  >>> BEIJING_WIP_STM add a static create method
        void*   operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
        void    operator delete(void* rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
        void*   operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
        void    operator delete [] (void* rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

    private : 
                
        HFCPtr<HGFPointIndex<POINT, YProtPtExtentType>> m_mrDTMPointIndexPtr;
        int                                             m_resolutionIndex;

    protected :

        // Inherited from IMrDTMQuery
        virtual int _Query(Bentley::TerrainModel::DTMPtr&            dtmPtr, 
                           const DPoint3d*                  pQueryShapePts, 
                           int                              nbQueryShapePts, 
                           const IMrDTMQueryParametersPtr&  mrDTMQueryParamsPtr) const;
                           
    public :

        MrDTMViewDependentPointQuery(const HFCPtr<HGFPointIndex<POINT, YProtPtExtentType>>& pointIndexPtr);
        
        virtual ~MrDTMViewDependentPointQuery();       

        void SetResolutionIndex(int resolutionIndex);
    };

class MrDTMFullResolutionLinearQuery : public MrDTMQuery
    {  
    public:  // OPERATOR_NEW_KLUDGE  >>> BEIJING_WIP_STM add a static create method
        void*   operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
        void    operator delete(void* rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
        void*   operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
        void    operator delete [] (void* rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

    private : 
                       
        HFCPtr<HGFFeatureIndex<HFCPtr<HVEDTMLinearFeature>, 
                               HGF3DCoord<double>, 
                               YProtFeatureExtentType>> m_mrDTMLinearIndexPtr;                

    protected :

        // Inherited from IMrDTMQuery
        virtual int _Query(Bentley::TerrainModel::DTMPtr&            dtmPtr, 
                           const DPoint3d*                  pQueryShapePts, 
                           int                              nbQueryShapePts, 
                           const IMrDTMQueryParametersPtr&  mrDTMQueryParamsPtr) const;
                           
    public :
        
        MrDTMFullResolutionLinearQuery(HFCPtr<HGFFeatureIndex<HFCPtr<HVEDTMLinearFeature>, 
                                       HGF3DCoord<double>, 
                                       YProtFeatureExtentType>> linearIndexPtr);
        
        virtual ~MrDTMFullResolutionLinearQuery();               
    }; 

template <class POINT> class MrDTMFixResolutionViewPointQuery : public MrDTMQuery 
    {  
    public:  // OPERATOR_NEW_KLUDGE  >>> BEIJING_WIP_STM add a static create method
        void*   operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
        void    operator delete(void* rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
        void*   operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
        void    operator delete [] (void* rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

    private : 
        
        HFCPtr<HGFPointIndex<POINT, YProtPtExtentType>> m_mrDTMPointIndexPtr;
        GeoCoords::GCS                                  m_pointIndexGCS;

    protected :

        // Inherited from IMrDTMQuery
        virtual int _Query(Bentley::TerrainModel::DTMPtr&            dtmPtr, 
                           const DPoint3d*                  pQueryShapePts, 
                           int                              nbQueryShapePts, 
                           const IMrDTMQueryParametersPtr&  mrDTMQueryParamsPtr) const;
                           
    public :

        MrDTMFixResolutionViewPointQuery(const HFCPtr<HGFPointIndex<POINT, YProtPtExtentType>>& pointIndexPtr, 
                                         const GeoCoords::GCS&                                  pointIndexGCS);
        
        virtual ~MrDTMFixResolutionViewPointQuery();       
    };


class MrDTMReprojectionQuery : public MrDTMQuery 
    {
    public:  // OPERATOR_NEW_KLUDGE  >>> BEIJING_WIP_STM add a static create method
        void*   operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
        void    operator delete(void* rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
        void*   operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
        void    operator delete [] (void* rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

    private : 
        
        IMrDTMQueryPtr         m_originalQueryPtr;

        GeoCoords::GCS          m_sourceGCS;
        GeoCoords::GCS          m_targetGCS;
        GeoCoords::Reprojection m_sourceToTargetReproj;
        GeoCoords::Reprojection m_targetToSourceReproj;
        DRange3d                m_extentInTargetGCS;
                   
    protected :

        // Inherited from IMrDTMQuery
        virtual int _Query(Bentley::TerrainModel::DTMPtr&            dtmPtr, 
                           const DPoint3d*                  pQueryShapePts, 
                           int                              nbQueryShapePts, 
                           const IMrDTMQueryParametersPtr&  mrDTMQueryParamsPtr) const;

        virtual int _AddClip(DPoint3d* clipPointsP,
                             int   numberOfPoints, 
                             bool  isClipMask);    

                           
    public :
        
        MrDTMReprojectionQuery(IMrDTMQueryPtr         originalQueryPtr, 
                               const GeoCoords::GCS&      sourceGCS, 
                               const GeoCoords::GCS&      targetGCS,
                               const DRange3d&        extentInTargetGCS);

        
        virtual ~MrDTMReprojectionQuery(); 
    };

#include "MrDTMQuery.hpp"

END_BENTLEY_MRDTM_NAMESPACE