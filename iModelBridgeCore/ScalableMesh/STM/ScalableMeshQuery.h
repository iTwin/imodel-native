/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshQuery.h $
|    $RCSfile: ScalableMeshQuery.h,v $
|   $Revision: 1.20 $
|       $Date: 2012/06/27 14:07:12 $
|     $Author: Chantal.Poulin $
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
#pragma warning(disable:4250) //yes visual studio I am aware of virtual inheritance thank you
#include <GeoCoord\BaseGeoCoord.h>
#include <ScalableMesh/GeoCoords/GCS.h>
#include <ScalableMesh/GeoCoords/Reprojection.h>


#include <ImagePP/all/h/HCPGCoordModel.h>
//#include <ImagePP/all/h/HRPPixelTypeV24R8G8B8.h>
//#include <ImagePP/all/h/HRPPixelTypeV24B8G8R8.h>
//#include <ImagePP/all/h/HRFBmpFile.h>

#include <ScalableMesh/IScalableMesh.h>
#include <ScalableMesh/IScalableMeshClipContainer.h>
#include <ScalableMesh\IScalableMeshProgressiveQuery.h>

#include "SMMeshIndex.h"
#include "./ScalableMesh/ScalableMeshGraph.h"
#include "Edits/DifferenceSet.h"
#ifdef SCALABLE_MESH_ATP
#include <ScalableMesh/IScalableMeshATP.h>
#endif
//#include <hash_map>


//Only way found to deactivate warning C4250 since the pragma warning(disable... doesn't work
#pragma warning( push, 0 )


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct ScalableMeshViewDependentQueryParams;
class ScalableMeshViewDependentQuery;

//typedef ISMStore::Extent3d64f Extent3dType;
typedef DRange3d Extent3dType;
typedef HGF3DExtent<double> YProtFeatureExtentType;

struct ScalableMeshExtentQuery;
typedef RefCountedPtr<ScalableMeshExtentQuery> ScalableMeshExtentQueryPtr;


/*==================================================================*/
/*        QUERY PARAMETERS IMPLEMENTATION SECTION - START           */
/*==================================================================*/
struct ScalableMeshQueryParameters : public virtual IScalableMeshQueryParameters                              
    {    
    private: 
               
        BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr m_sourceGCSPtr;
        BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr m_targetGCSPtr;
                
    protected:                  
                
        virtual BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr _GetSourceGCS() override
            {
            return m_sourceGCSPtr;
            }

        virtual BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr _GetTargetGCS() override
            {
            return m_targetGCSPtr;
            }

        virtual void _SetGCS(BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& sourceGCSPtr, 
                             BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCSPtr) override
            {
            m_sourceGCSPtr = sourceGCSPtr;
            m_targetGCSPtr = targetGCSPtr;
            }
                                   
    public:

        ScalableMeshQueryParameters()        
            {                 
            }    
    };

struct ScalableMeshFullResolutionQueryParams :  public virtual ScalableMeshQueryParameters,
                                                public virtual IScalableMeshFullResolutionQueryParams
                                        
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

        ScalableMeshFullResolutionQueryParams()
            {
            m_returnAllPtsForLowestLevel = true;
            m_maximumNumberOfPoints = UINT_MAX;
            }

        virtual ~ScalableMeshFullResolutionQueryParams() {}
    };

struct ScalableMeshFullResolutionLinearQueryParams : public virtual IScalableMeshFullResolutionLinearQueryParams,
                                              public virtual ScalableMeshFullResolutionQueryParams
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

        ScalableMeshFullResolutionLinearQueryParams()
            {
            m_maximumNumberOfPointsForLinear = UINT64_MAX;
            m_useDecimation = true;
            m_cutLinears = false;
            m_addLinears = true;
            m_doIncludeFilteringFeatureTypes = false;
            }

        virtual ~ScalableMeshFullResolutionLinearQueryParams()
            {
            }
    };
                                                  
struct SrDTMViewDependentQueryParams : public virtual ISrDTMViewDependentQueryParams, 
                                       public virtual ScalableMeshQueryParameters
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


struct ScalableMeshViewDependentQueryParams : public virtual IScalableMeshViewDependentQueryParams, 
                                       public virtual SrDTMViewDependentQueryParams         
    {    
    protected :

        double   m_minScreenPixelsPerPoint;
        double   m_rootToViewMatrix[4][4];
                                
        virtual double        _GetMinScreenPixelsPerPoint() const override
            {
            return m_minScreenPixelsPerPoint;
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
                      
    public : 

        ScalableMeshViewDependentQueryParams()
            {
            m_minScreenPixelsPerPoint = 100;                        
            }

        virtual ~ScalableMeshViewDependentQueryParams()
            {
            }
    };



template<class POINT> class ScalableMeshFixResolutionViewPointQuery;



struct ScalableMeshFixResolutionIndexQueryParams : public virtual IScalableMeshFixResolutionIndexQueryParams, 
                                            public virtual ScalableMeshQueryParameters
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

        ScalableMeshFixResolutionIndexQueryParams()
            {
            m_resolutionIndex = 0;            
            }

        virtual ~ScalableMeshFixResolutionIndexQueryParams()
            {
            }        
    };

struct ScalableMeshFixResolutionMaxPointsQueryParams : public virtual IScalableMeshFixResolutionMaxPointsQueryParams, 
                                                public virtual ScalableMeshQueryParameters
    {

    template<class POINT>
    friend class ScalableMeshFixResolutionViewPointQuery;

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

        ScalableMeshFixResolutionMaxPointsQueryParams()
            {
            m_maxNumberPoints = 0;
            }

        virtual ~ScalableMeshFixResolutionMaxPointsQueryParams()
            {
            }
    };
#if 0
struct ScalableMeshQueryAllLinearsQueryParams : public virtual IScalableMeshQueryAllLinearsQueryParams,
                                         public virtual ScalableMeshFullResolutionLinearQueryParams
    {    
#ifdef SCALABLE_TERRAIN_MODEL_PRIVATE_SECTION
    private : 

        typedef HFCPtr<HVEDTMLinearFeature> Feature;
        typedef list<Feature>::iterator FeatureIterator;
        list<Feature> m_features;
#endif 

    protected :

        virtual list<IScalableMeshFeaturePtr> _GetFeatures()
            {
            // convert to publishable list (does not depend on ImagePP objects)
            list<IScalableMeshFeaturePtr> newList;
            for (FeatureIterator it = m_features.begin(); it != m_features.end(); it++)
                {
                IScalableMeshFeaturePtr feature = IScalableMeshFeature::Create();
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
#if 0
        virtual void _SetFeatures (const list<HFCPtr<HVEDTMLinearFeature>>& features)
            {
            m_features = features;
            }
#endif
               
    public : 

        ScalableMeshQueryAllLinearsQueryParams()
            {
            }
               
        virtual ~ScalableMeshQueryAllLinearsQueryParams()
            {
            }               
    };
#endif
#pragma warning( pop)

/*==================================================================*/
/*        QUERY PARAMETERS IMPLEMENTATION SECTION - END             */
/*==================================================================*/ 
class ScalableMeshPointQuery : public IScalableMeshPointQuery
    {
                                        
    protected :
        IScalableMeshClipContainerPtr m_clips;


        HFCPtr<HVEShape> m_clipShapePtr;        
        
        ScalableMeshPointQuery();
        ~ScalableMeshPointQuery();
        

        HFCPtr<HVEShape>  CreateClipShape(DRange3d& spatialIndexRange) const;
        HFCPtr<HVEShape>  CreateClipShape(HFCPtr<HVEShape> areaShape) const;
               
                
        //Inherited from IScalableMeshPointQuery
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
       
        //NEEDS_WORK_SM - TEMP in public
        template <class POINT> static int AddPoints(bvector<DPoint3d>&                   points, 
                                                    const HPMMemoryManagedVector<POINT>& pointList) /*const*/;

        template<class EXTENT> static EXTENT GetExtentFromClipShape(const DPoint3d* pClipShapePts, 
                                                                   int             nbClipShapePts, 
                                                                   double          zMin, 
                                                                   double          zMax) /*const*/;
        //NEEDS_WORK_SM - TEMP in public END
#if 0
        static int AddLinears(const DTMPtr&               dtmPtr,
                       list<HFCPtr<HVEDTMLinearFeature>>& linearList, 
                       size_t                             maxNumberOfPoints,
                       bool                               useDecimation);
#endif
        static IScalableMeshPointQueryPtr GetReprojectionQueryInterface(IScalableMeshPtr        scmToQueryPtr,
                                                                        ScalableMeshQueryType   queryType,                                                             
                                                                        const GeoCoords::GCS&   sourceGCStr,
                                                                        const GeoCoords::GCS&   targetGCStr,
                                                                        const DRange3d&         extentInTargetGCS);

        int _Query(bvector<DPoint3d>&               points, 
                   const DPoint3d*                  pClipShapePts, 
                   int                              nbClipShapePts, 
                   const IScalableMeshQueryParametersPtr&  scmQueryParamsPtr) const ;

};

template <class POINT> class ScalableMeshFullResolutionPointQuery : public ScalableMeshPointQuery
    {   
    public:  // OPERATOR_NEW_KLUDGE  >>> BEIJING_WIP_STM add a static create method
        void*   operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
        void    operator delete(void* rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
        void*   operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
        void    operator delete [] (void* rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

    private : 
                
        HFCPtr<SMPointIndex<POINT, Extent3dType>> m_scmIndexPtr;
        int                                             m_resolutionIndex;

    protected :

        // Inherited from IScalableMeshPointQuery
        virtual int _Query(bvector<DPoint3d>&               points,
                           const DPoint3d*                  pQueryShapePts, 
                           int                              nbQueryShapePts, 
                           const IScalableMeshQueryParametersPtr&  scmQueryParamsPtr) const;
                           
    public :
        
        ScalableMeshFullResolutionPointQuery(const HFCPtr<SMPointIndex<POINT, Extent3dType>>& pointIndexPtr, 
                                      int                                              resolutionIndex);
        
        virtual ~ScalableMeshFullResolutionPointQuery();     
    }; 

template <class POINT> class ScalableMeshViewDependentPointQuery : public ScalableMeshPointQuery
    {   
    public:  // OPERATOR_NEW_KLUDGE  >>> BEIJING_WIP_STM add a static create method
        void*   operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
        void    operator delete(void* rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
        void*   operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
        void    operator delete [] (void* rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

    private : 
                
        HFCPtr<SMPointIndex<POINT, Extent3dType>> m_scmIndexPtr;
        int                                             m_resolutionIndex;

    protected :

        // Inherited from IScalableMeshPointQuery
        virtual int _Query(bvector<DPoint3d>&               points,
                           const DPoint3d*                  pQueryShapePts, 
                           int                              nbQueryShapePts, 
                           const IScalableMeshQueryParametersPtr&  scmQueryParamsPtr) const;
                           
    public :

        ScalableMeshViewDependentPointQuery(const HFCPtr<SMPointIndex<POINT, Extent3dType>>& pointIndexPtr);
        
        virtual ~ScalableMeshViewDependentPointQuery();       

        void SetResolutionIndex(int resolutionIndex);
    };


template <class POINT> class ScalableMeshFixResolutionViewPointQuery : public ScalableMeshPointQuery 
    {  
    public:  // OPERATOR_NEW_KLUDGE  >>> BEIJING_WIP_STM add a static create method
        void*   operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
        void    operator delete(void* rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
        void*   operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
        void    operator delete [] (void* rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

    private : 
        
        HFCPtr<SMPointIndex<POINT, Extent3dType>> m_scmIndexPtr;
        GeoCoords::GCS                                  m_pointIndexGCS;

    protected :

        // Inherited from IScalableMeshPointQuery
        virtual int _Query(bvector<DPoint3d>&               points,
                           const DPoint3d*                  pQueryShapePts, 
                           int                              nbQueryShapePts, 
                           const IScalableMeshQueryParametersPtr&  scmQueryParamsPtr) const;
                           
    public :

        ScalableMeshFixResolutionViewPointQuery(const HFCPtr<SMPointIndex<POINT, Extent3dType>>& pointIndexPtr, 
                                         const GeoCoords::GCS&                                  pointIndexGCS);
        
        virtual ~ScalableMeshFixResolutionViewPointQuery();       
    };


class ScalableMeshReprojectionQuery : public ScalableMeshPointQuery 
    {
    public:  // OPERATOR_NEW_KLUDGE  >>> BEIJING_WIP_STM add a static create method
        void*   operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
        void    operator delete(void* rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
        void*   operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
        void    operator delete [] (void* rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

    private : 
        
        IScalableMeshPointQueryPtr         m_originalQueryPtr;

        GeoCoords::GCS          m_sourceGCS;
        GeoCoords::GCS          m_targetGCS;
        GeoCoords::Reprojection m_sourceToTargetReproj;
        GeoCoords::Reprojection m_targetToSourceReproj;
        DRange3d                m_extentInTargetGCS;
                   
    protected :

        // Inherited from IScalableMeshPointQuery
        /*NEEDS_WORK_SM : need mesh reprojection interface.
        virtual int _Query(bvector<DPoint3d>&               points,
                           const DPoint3d*                  pQueryShapePts, 
                           int                              nbQueryShapePts, 
                           const IScalableMeshQueryParametersPtr&  scmQueryParamsPtr) const;
                           */

        virtual int _AddClip(DPoint3d* clipPointsP,
                             int   numberOfPoints, 
                             bool  isClipMask);    

                           
    public :
        
        ScalableMeshReprojectionQuery(IScalableMeshPointQueryPtr         originalQueryPtr, 
                               const GeoCoords::GCS&      sourceGCS, 
                               const GeoCoords::GCS&      targetGCS,
                               const DRange3d&        extentInTargetGCS);

        
        virtual ~ScalableMeshReprojectionQuery(); 
    };

/*==================================================================*/
/*        3D MESH RELATED CODE - START                              */
/*==================================================================*/
class ScalableMeshMesh;

typedef RefCountedPtr<ScalableMeshMesh> ScalableMeshMeshPtr;

class ScalableMeshMesh : public IScalableMeshMesh
    {
    friend IScalableMeshMesh; 

    private : 
        DVec3d m_viewNormal;
        mutable size_t    m_nbFaceIndexes;
        int32_t*    m_faceIndexes;
        size_t    m_normalCount;
        DVec3d*   m_pNormal;
        int32_t*    m_pNormalIndex;
        mutable DVec3d*   m_pNormalAuto;
        DPoint2d*     m_pUv;
        int32_t*    m_pUvIndex;
        size_t      m_uvCount;

        mutable PolyfaceQueryCarrier* m_polyfaceQueryCarrier; 

    protected : 
        DPoint3d* m_points;
        size_t    m_nbPoints;

        virtual const BENTLEY_NAMESPACE_NAME::PolyfaceQuery* _GetPolyfaceQuery() const override;

        virtual size_t _GetNbPoints() const override;

        virtual size_t _GetNbFaces() const override;

        virtual DPoint3d* _EditPoints() override;

        virtual DTMStatusInt _GetAsBcDTM(BcDTMPtr& bcdtm)override;

        virtual DTMStatusInt _GetBoundary(bvector<DPoint3d>& boundary) override;

        virtual bool _FindTriangleForProjectedPoint(int* outTriangle, DPoint3d& point, bool use2d = false) const override;
        virtual bool _FindTriangleForProjectedPoint(MTGNodeId& outTriangle, DPoint3d& point, bool use2d = false) const override;

        virtual int _ProjectPolyLineOnMesh(DPoint3d& endPt, bvector<bvector<DPoint3d>>& projectedPoints, const DPoint3d* points, int nPts, int* segment, const MTGNodeId triangleEdge, DPoint3d startPt) const override;

        virtual int _ProjectPolyLineOnMesh(DPoint3d& endPt, bvector<bvector<DPoint3d>>& projectedPoints, const DPoint3d* points, int nPts, int* segment, const int* triangle, DPoint3d startPt, MTGNodeId& lastEdge) const override;
       
        virtual bool _FindTriangleAlongRay(int* outTriangle, DRay3d& ray, MTGNodeId edge = -1) const override;
        virtual bool _FindTriangleAlongRay(MTGNodeId& outTriangle, DRay3d& ray) const override;

        virtual bool _CutWithPlane(bvector<DSegment3d>& segmentList, DPlane3d& cuttingPlane) const override;


        ScalableMeshMesh(size_t nbPoints, DPoint3d* points, size_t nbFaceIndexes, const int32_t* faceIndexes, size_t normalCount, DVec3d* pNormal, int32_t* pNormalIndex, size_t uvCount, DVec2d* pUv, int32_t* pUvIndex);

        ScalableMeshMesh(DVec3d viewNormal);

        void CalcNormals () const;
        virtual ~ScalableMeshMesh();

    public :        

        //NEEDS_WORK_SM - Put in CPP.
              size_t    GetNbPoints() const {return m_nbPoints;}
        const DPoint3d* GetPoints() const {return m_points;}

              size_t GetNbFaceIndexes() const {return m_nbFaceIndexes;}
        const int32_t* GetFaceIndexes() const {return m_faceIndexes;}

        int AppendMesh(size_t nbPoints, DPoint3d* points, size_t nbFaceIndexes, const int32_t* faceIndexes, size_t normalCount, DVec3d* pNormal, int32_t* pNormalIndex, size_t uvCount, const DPoint2d* pUv, const int32_t* pUvIndex);

        void ApplyDifferenceSet(const DifferenceSet& d);

        void ApplyClipMesh(const DifferenceSet& d);

        void RecalculateUVs(DRange3d& nodeRange);

        static ScalableMeshMeshPtr Create (DVec3d viewNormal);
        static ScalableMeshMeshPtr Create ();
    };

class ScalableMeshTexture;
typedef RefCountedPtr<ScalableMeshTexture> ScalableMeshTexturePtr;


class ScalableMeshTexture : public IScalableMeshTexture
{
    friend IScalableMeshTexture;    

private:
    
    const Byte*                               m_textureData;
    size_t                                    m_dataSize;    
    Point2d                                   m_dimension;    
    int                                       m_nbChannels;
    RefCountedPtr<SMMemoryPoolBlobItem<Byte>> m_texturePtr;

protected:
    virtual size_t      _GetSize() const override;
    virtual Point2d     _GetDimension() const override;
    virtual const Byte* _GetData() const override;   

    ScalableMeshTexture(RefCountedPtr<SMMemoryPoolBlobItem<Byte>>& pTextureData);
    virtual ~ScalableMeshTexture();

public:    

    static ScalableMeshTexturePtr Create(RefCountedPtr<SMMemoryPoolBlobItem<Byte>>& pTextureData);
};



class ScalableMeshMeshWithGraph;

typedef RefCountedPtr<ScalableMeshMeshWithGraph> ScalableMeshMeshWithGraphPtr;

class ScalableMeshMeshWithGraph : public ScalableMeshMesh
    {

    private:
        MTGGraph* m_graphData;
        bool m_is3d; //helps with traversal/holes assumptions

    protected:
        virtual DTMStatusInt _GetBoundary(bvector<DPoint3d>& boundary) override;
        virtual int _ProjectPolyLineOnMesh(DPoint3d& endPt, bvector<bvector<DPoint3d>>& projectedPoints, const DPoint3d* points, int nPts, int* segment, const MTGNodeId triangleEdge, DPoint3d startPt) const override;
        virtual int _ProjectPolyLineOnMesh(DPoint3d& endPt, bvector<bvector<DPoint3d>>& projectedPoints, const DPoint3d* points, int nPts, int* segment, const int* triangle, DPoint3d startPt, MTGNodeId& lastEdge) const override;
        virtual bool _FindTriangleAlongRay(int* outTriangle, DRay3d& ray, MTGNodeId edge = -1) const override;
        virtual bool _FindTriangleAlongRay(MTGNodeId& outTriangle, DRay3d& ray) const override;
        virtual bool _FindTriangleForProjectedPoint(MTGNodeId& outTriangle, DPoint3d& point, bool use2d = false) const override;
        ScalableMeshMeshWithGraph(size_t nbPoints, DPoint3d* points, size_t nbFaceIndexes, int32_t* faceIndexes, size_t normalCount, DVec3d* pNormal, int32_t* pNormalIndex, MTGGraph* pGraph, bool is3d, size_t uvCount, DVec2d* pUv, int32_t* pUvIndex);
        ScalableMeshMeshWithGraph(MTGGraph* pGraph, bool is3d);

        virtual ~ScalableMeshMeshWithGraph();

    public:

        static ScalableMeshMeshWithGraphPtr Create(size_t nbPoints, DPoint3d* points, size_t nbFaceIndexes, int32_t* faceIndexes, size_t normalCount, DVec3d* pNormal, int32_t* pNormalIndex, MTGGraph* pGraph, bool is3d, size_t uvCount, DVec2d* pUv, int32_t* pUvIndex);
        static ScalableMeshMeshWithGraphPtr Create(MTGGraph* pGraph, bool is3d);
    };


/*struct ScalableMeshTexture : RefCountedBase
{
    bvector<byte>   m_data;
    bvector<byte>   m_compressedData;
    Point2d         m_size;
    MaterialPtr     m_material;

    ScalableMeshTexture(byte const* pData, size_t dataSize);
    ~ScalableMeshTexture();

    void Initialize(size_t nodeID, size_t elementID, ViewContextR viewContext);
    void Activate(ViewContextR viewContext);
    size_t GetMemorySize() const;
    bool IsInitialized() const;

    static ScalableMeshTexturePtr Create(byte const* pData, size_t dataSize);
};*/

struct ScalableMeshViewDependentMeshQueryParams : public IScalableMeshViewDependentMeshQueryParams 
    {    
    protected :

        BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr m_sourceGCSPtr;
        BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr m_targetGCSPtr;

        bool     m_isProgressiveDisplay;
        double   m_minScreenPixelsPerPoint;
        double   m_rootToViewMatrix[4][4];
        

        //NEEDS_WORK_SM : Only one of those is likely required
        DPoint3d m_viewBox[8];                 
        ClipVectorPtr m_viewClipVector;

        StopQueryCallbackFP m_stopQueryCallbackFP;

        virtual size_t _GetLevel() override { return 0; }
        virtual void _SetLevel(size_t depth) override {};
        
        virtual const DPoint3d* _GetViewBox() const override
            {
            return m_viewBox;
            }        

        virtual void _SetViewBox(const DPoint3d viewBox[]) override
            {
            memcpy(m_viewBox, viewBox, sizeof(m_viewBox));
            }      

        virtual double        _GetMinScreenPixelsPerPoint() const override
            {
            return m_minScreenPixelsPerPoint;
            } 

        virtual StopQueryCallbackFP _GetStopQueryCallback() const
            {
            return m_stopQueryCallbackFP;
            }
      
        virtual const double* _GetRootToViewMatrix() const override
            {
            return (double*)m_rootToViewMatrix;
            }

        virtual const ClipVectorPtr _GetViewClipVector() const override
            {
            return m_viewClipVector;
            }

        virtual bool _IsProgressiveDisplay() const override
            {
            return m_isProgressiveDisplay;
            }

        virtual void          _SetMinScreenPixelsPerPoint(double minScreenPixelsPerPoint) override
            {
            m_minScreenPixelsPerPoint = minScreenPixelsPerPoint;
            }

        virtual void          _SetProgressiveDisplay(bool isProgressiveDisplay) override
            {
            m_isProgressiveDisplay = isProgressiveDisplay;
            }

        virtual void          _SetRootToViewMatrix(const double rootToViewMatrix[][4]) override
            {
            memcpy(m_rootToViewMatrix, rootToViewMatrix, sizeof(m_rootToViewMatrix));
            }

        virtual StatusInt     _SetStopQueryCallback(StopQueryCallbackFP stopQueryCallbackFP) override
            {
            m_stopQueryCallbackFP = stopQueryCallbackFP;
            return SUCCESS;
            }                 

        virtual void          _SetViewClipVector(ClipVectorPtr& viewClipVector) override
            {
            m_viewClipVector = viewClipVector;           
            }        
     
        virtual BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr _GetSourceGCS() override
            {
            return m_sourceGCSPtr;
            }

        virtual BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr _GetTargetGCS() override
            {
            return m_targetGCSPtr;
            }

        virtual void _SetGCS(BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& sourceGCSPtr,
                             BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCSPtr) override
            {
            m_sourceGCSPtr = sourceGCSPtr;
            m_targetGCSPtr = targetGCSPtr;
            }        
        
    public : 

        ScalableMeshViewDependentMeshQueryParams()
            {            
            m_minScreenPixelsPerPoint = 100;      
            m_isProgressiveDisplay = false;
            m_stopQueryCallbackFP = 0;
            }

        virtual ~ScalableMeshViewDependentMeshQueryParams()
            {
            }
    };

struct ScalableMeshMeshQueryParams : public IScalableMeshMeshQueryParams
    {
    protected:
        BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr m_sourceGCSPtr;
        BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr m_targetGCSPtr;
        size_t m_depth;
        virtual BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr _GetSourceGCS() override
            {
            return m_sourceGCSPtr;
            }

        virtual BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr _GetTargetGCS() override
            {
            return m_targetGCSPtr;
            }

        virtual size_t _GetLevel() override
            {
            return m_depth;
            }

        virtual void _SetLevel(size_t depth) override
            {
            m_depth = depth;
            }

        virtual void _SetGCS(BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& sourceGCSPtr,
                             BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCSPtr) override
            {
            m_sourceGCSPtr = sourceGCSPtr;
            m_targetGCSPtr = targetGCSPtr;
            }
    public:

        ScalableMeshMeshQueryParams()
            {
            m_depth = (size_t)-1;
            }

        virtual ~ScalableMeshMeshQueryParams()
            {}
    };

int draw(DTMFeatureType dtmFeatureType,int numTriangles, int numMeshPts,DPoint3d *meshPtsP,DPoint3d *meshVectorsP,int numMeshFaces, long *meshFacesP,void *userP);

template <class POINT> class ScalableMeshFullResolutionMeshQuery : public IScalableMeshMeshQuery
    {   
    public:  
        void*   operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
        void    operator delete(void* rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
        void*   operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
        void    operator delete [] (void* rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

    private : 
                
        HFCPtr<SMPointIndex<POINT, Extent3dType>> m_scmIndexPtr;
        
    protected :

        // Inherited from IScalableMeshMeshQuery
        virtual int _Query(IScalableMeshMeshPtr&                                meshPtr,
                           const DPoint3d*                               pQueryExtentPts, 
                           int                                           nbQueryExtentPts, 
                           const IScalableMeshMeshQueryParamsPtr&  scmQueryParamsPtr) const override;

        virtual int _Query(bvector<IScalableMeshNodePtr>&                       meshNodes,
                           const DPoint3d*                               pQueryExtentPts,
                           int                                           nbQueryExtentPts,
                           const IScalableMeshMeshQueryParamsPtr&  scmQueryParamsPtr) const override;
                           
    public :

        ScalableMeshFullResolutionMeshQuery(const HFCPtr<SMPointIndex<POINT, Extent3dType>>& pointIndexPtr);
        
        virtual ~ScalableMeshFullResolutionMeshQuery();
    };


template <class POINT> class ScalableMeshViewDependentMeshQuery : public IScalableMeshMeshQuery
    {
    public:
        void*   operator new(size_t size){ return bentleyAllocator_allocateRefCounted(size); }
        void    operator delete(void* rawMemory, size_t size) { bentleyAllocator_deleteRefCounted(rawMemory, size); }
        void*   operator new [] (size_t size) { return bentleyAllocator_allocateArrayRefCounted(size); }
        void    operator delete [] (void* rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted(rawMemory, size); }

    private:

        HFCPtr<SMPointIndex<POINT, Extent3dType>> m_scmIndexPtr;

    protected:

        // Inherited from IScalableMeshMeshQuery
        virtual int _Query(IScalableMeshMeshPtr&                                meshPtr,
                           const DPoint3d*                               pQueryExtentPts,
                           int                                           nbQueryExtentPts,
                           const IScalableMeshMeshQueryParamsPtr&  scmQueryParamsPtr) const override;

        virtual int _Query(bvector<IScalableMeshNodePtr>&                       meshNodes,
                           const DPoint3d*                               pQueryExtentPts,
                           int                                           nbQueryExtentPts,
                           const IScalableMeshMeshQueryParamsPtr&  scmQueryParamsPtr) const override;

    public:

        ScalableMeshViewDependentMeshQuery(const HFCPtr<SMPointIndex<POINT, Extent3dType>>& pointIndexPtr);

        virtual ~ScalableMeshViewDependentMeshQuery();
    };

template <class POINT> class ScalableMeshReprojectionMeshQuery : public IScalableMeshMeshQuery
    {
    public: 
        void*   operator new(size_t size){ return bentleyAllocator_allocateRefCounted(size); }
        void    operator delete(void* rawMemory, size_t size) { bentleyAllocator_deleteRefCounted(rawMemory, size); }
        void*   operator new [] (size_t size) { return bentleyAllocator_allocateArrayRefCounted(size); }
        void    operator delete [] (void* rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted(rawMemory, size); }

    private:

        IScalableMeshMeshQueryPtr         m_originalQueryPtr;
        HFCPtr<SMMeshIndex<POINT, Extent3dType>> m_scmIndexPtr;

        GeoCoords::GCS          m_sourceGCS;
        GeoCoords::GCS          m_targetGCS;
        GeoCoords::Reprojection m_sourceToTargetReproj;
        GeoCoords::Reprojection m_targetToSourceReproj;
        DRange3d                m_extentInTargetGCS;

    protected:

        // Inherited from IScalableMeshMeshQuery
        virtual int _Query(IScalableMeshMeshPtr&                                meshPtr,
                           const DPoint3d*                               pQueryExtentPts,
                           int                                           nbQueryExtentPts,
                           const IScalableMeshMeshQueryParamsPtr&  scmQueryParamsPtr) const override;

        virtual int _Query(bvector<IScalableMeshNodePtr>&                       meshNodes,
                           const DPoint3d*                               pQueryExtentPts,
                           int                                           nbQueryExtentPts,
                           const IScalableMeshMeshQueryParamsPtr&  scmQueryParamsPtr) const override;


    public:

        ScalableMeshReprojectionMeshQuery(IScalableMeshMeshQueryPtr         originalQueryPtr,
                                   const HFCPtr<SMMeshIndex<POINT, Extent3dType>>& indexPtr,
                               const GeoCoords::GCS&      sourceGCS,
                               const GeoCoords::GCS&      targetGCS,
                               const DRange3d&        extentInTargetGCS);


        virtual ~ScalableMeshReprojectionMeshQuery() {};
    };

struct ScalableMeshNodeRayQueryParams : public IScalableMeshNodeQueryParams
    {
    protected:
        DVec3d m_rayDirection;
        double m_depth;
        bool m_is2d;
        size_t m_level;
        bool m_useUnboundedRay;

        virtual void _SetDirection(DVec3d direction) override
            {
            m_rayDirection = direction;
            }
        virtual DVec3d _GetDirection() const override
            {
            return m_rayDirection;
            }
        virtual void _SetDepth(double depth) override
            {
            m_depth = depth;
            }
        virtual double _GetDepth() const override
            {
            return m_depth;
            }

        virtual void _Set2d(bool is2d) override
            {
            m_is2d = is2d;
            }
        virtual bool _Get2d() const override
            {
            return m_is2d;
            }

        virtual void _SetLevel(size_t level) override
            {
            m_level = level;
            }
        virtual size_t _GetLevel() const override
            {
            return m_level;
            }

        virtual void _SetUseUnboundedRay(bool useUnboundedRay) override
            {
            m_useUnboundedRay = useUnboundedRay;
            }
        virtual bool _GetUseUnboundedRay() const override
            {
            return m_useUnboundedRay;
            }
    public:
        static const double INFINITE_DEPTH;
        ScalableMeshNodeRayQueryParams()
            {
            m_rayDirection = DVec3d::From(0, 0, -1);
            m_depth = INFINITE_DEPTH; 
            m_is2d = false;
            m_level = (size_t)-1;
            m_useUnboundedRay = true;
            }

        virtual ~ScalableMeshNodeRayQueryParams()
            {}
    };

template <class POINT> class ScalableMeshNodeRayQuery : public IScalableMeshNodeRayQuery
    {
    public:
        void*   operator new(size_t size){ return bentleyAllocator_allocateRefCounted(size); }
        void    operator delete(void* rawMemory, size_t size) { bentleyAllocator_deleteRefCounted(rawMemory, size); }
        void*   operator new [] (size_t size) { return bentleyAllocator_allocateArrayRefCounted(size); }
        void    operator delete [] (void* rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted(rawMemory, size); }

    private:

        HFCPtr<SMPointIndex<POINT, Extent3dType>> m_scmIndexPtr;

    protected:

        // Inherited from IScalableMeshNodeRayQuery
        virtual int _Query(IScalableMeshNodePtr&                                nodePtr,
                           const DPoint3d*                               pTestPt,
                           const DPoint3d*                               pClipShapePts,
                           int                                           nbClipShapePts,
                           const IScalableMeshNodeQueryParamsPtr&  scmQueryParamsPtr) const override;

        virtual int _Query(bvector<IScalableMeshNodePtr>&                                nodePtr,
                            const DPoint3d*                               pTestPt,
                            const DPoint3d*                               pClipShapePts,
                            int                                           nbClipShapePts,
                            const IScalableMeshNodeQueryParamsPtr&  scmQueryParamsPtr) const override;

    public:

        ScalableMeshNodeRayQuery(const HFCPtr<SMPointIndex<POINT, Extent3dType>>& pointIndexPtr);

        virtual ~ScalableMeshNodeRayQuery();
    };


struct ScalableMeshNodePlaneQueryParams : public IScalableMeshNodePlaneQueryParams
    {
    protected:
        DPlane3d m_plane;
        double m_depth;

        virtual void _SetPlane(DPlane3d plane) override
            {
            m_plane = plane;
            }
        virtual DPlane3d _GetPlane() const override
            {
            return m_plane;
            }
        virtual void _SetDepth(double depth) override
            {
            m_depth = depth;
            }
        virtual double _GetDepth() const override
            {
            return m_depth;
            }

        virtual size_t _GetLevel() override { return 0; }
        virtual void _SetLevel(size_t depth) override {};
        BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr m_sourceGCSPtr;
        BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr m_targetGCSPtr;
        virtual BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr _GetSourceGCS() override
            {
            return m_sourceGCSPtr;
            }

        virtual BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr _GetTargetGCS() override
            {
            return m_targetGCSPtr;
            }

        virtual void _SetGCS(BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& sourceGCSPtr,
                             BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCSPtr) override
            {
            m_sourceGCSPtr = sourceGCSPtr;
            m_targetGCSPtr = targetGCSPtr;
            }
    public:
        static const double INFINITE_DEPTH;
        ScalableMeshNodePlaneQueryParams()
            {
            m_depth = INFINITE_DEPTH;
            }

        virtual ~ScalableMeshNodePlaneQueryParams()
            {}
    };

template <class POINT> class ScalableMeshNodePlaneQuery : public IScalableMeshMeshQuery
    {
    public:
        void*   operator new(size_t size){ return bentleyAllocator_allocateRefCounted(size); }
        void    operator delete(void* rawMemory, size_t size) { bentleyAllocator_deleteRefCounted(rawMemory, size); }
        void*   operator new [] (size_t size) { return bentleyAllocator_allocateArrayRefCounted(size); }
        void    operator delete [] (void* rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted(rawMemory, size); }

    private:

        HFCPtr<SMPointIndex<POINT, Extent3dType>> m_scmIndexPtr;

    protected:

        virtual int _Query(IScalableMeshMeshPtr&                                meshPtr,
                           const DPoint3d*                               pQueryExtentPts,
                           int                                           nbQueryExtentPts,
                           const IScalableMeshMeshQueryParamsPtr&  scmQueryParamsPtr) const override;

        virtual int _Query(bvector<IScalableMeshNodePtr>&                       meshNodesPtr,
                           const DPoint3d*                               pQueryExtentPts,
                           int                                           nbQueryExtentPts,
                           const IScalableMeshMeshQueryParamsPtr&  scmQueryParamsPtr) const override;

    public:

        ScalableMeshNodePlaneQuery(const HFCPtr<SMPointIndex<POINT, Extent3dType>>& pointIndexPtr);

        virtual ~ScalableMeshNodePlaneQuery();
    };

class ScalableMeshMeshFlags : public virtual IScalableMeshMeshFlags
    {
    protected:

        bool m_loadGraph;
        bool m_loadTexture;

        virtual bool _ShouldLoadTexture() const override;
        virtual bool _ShouldLoadGraph() const override;

        virtual void _SetLoadTexture(bool loadTexture) override;
        virtual void _SetLoadGraph(bool loadGraph) override;

    public:
        ScalableMeshMeshFlags()
            {
            m_loadGraph = false;
            m_loadTexture = false;
            }

        virtual ~ScalableMeshMeshFlags() {}
    };

template<class POINT> class ScalableMeshNode : public virtual IScalableMeshNode

    {                
    protected:
        HFCPtr<SMPointIndexNode<POINT, Extent3dType>> m_node;        

        void ComputeDiffSet(DifferenceSet& diffs, const bvector<bool>& clipsToShow, bool applyAllClips = false) const;

        void ComputeDiffSet(DifferenceSet& diffs, const bset<uint64_t>& clipsToShow) const;

        virtual BcDTMPtr   _GetBcDTM() const override;

        virtual bool    _ArePoints3d() const override;

        virtual bool    _ArePointsFullResolution() const override;

        virtual IScalableMeshMeshPtr _GetMesh(IScalableMeshMeshFlagsPtr& flags, bvector<bool>& clipsToShow) const override;

        virtual IScalableMeshMeshPtr _GetMeshUnderClip(IScalableMeshMeshFlagsPtr& flags, uint64_t clip) const override;

        virtual IScalableMeshMeshPtr _GetMeshByParts(const bvector<bool>& clipsToShow) const override;

        virtual IScalableMeshMeshPtr _GetMeshByParts(const bset<uint64_t>& clipsToShow) const override;

        virtual void   _ApplyAllExistingClips() const override;

        virtual void   _RefreshMergedClip() const override;

        virtual bool   _AddClip(uint64_t id, bool isVisible) const override;

        virtual bool   _AddClipAsync(uint64_t id, bool isVisible) const override;

        virtual bool   _ModifyClip(uint64_t id,  bool isVisible) const override;

        virtual bool   _DeleteClip(uint64_t id, bool isVisible) const override;

        virtual IScalableMeshTexturePtr _GetTexture() const override;

        virtual bool                    _IsTextured() const override;
                
        virtual bvector<IScalableMeshNodePtr> _GetNeighborAt( char relativePosX, char relativePosY, char relativePosZ) const override;

        virtual bvector<IScalableMeshNodePtr> _GetChildrenNodes() const override;

        virtual DRange3d _GetNodeExtent() const override;

        virtual DRange3d _GetContentExtent() const override;

        virtual __int64 _GetNodeId() const override;

        virtual size_t _GetLevel() const override;

        virtual size_t _GetPointCount() const override;                

        virtual bool _IsHeaderLoaded() const override;

        virtual bool _IsMeshLoaded() const override;

        virtual void _LoadHeader() const override;

        virtual bool _HasClip(uint64_t clip) const override;

        virtual bool _IsClippingUpToDate() const override;

        virtual void _GetSkirtMeshes(bvector<PolyfaceHeaderPtr>& meshes) const override;

        virtual bool _RunQuery(ISMPointIndexQuery<DPoint3d, DRange3d>& query, bvector<IScalableMeshNodePtr>& nodes) const override;

        virtual bool _RunQuery(ISMPointIndexQuery<DPoint3d, DRange3d>& query) const override;

        
    public:         
        ScalableMeshNode(HFCPtr<SMPointIndexNode<POINT, Extent3dType>>& nodePtr);
        ScalableMeshNode() {};

        HFCPtr<SMPointIndexNode<POINT, Extent3dType>> GetNodePtr()
            {
            return m_node;
            }
    };


template<class POINT> class ScalableMeshCachedMeshNode : public virtual IScalableMeshCachedDisplayNode, 
                                                         public virtual ScalableMeshNode<POINT>
    {    

    private: 
            //NEEDS_WORK_TEXTURE
            IScalableMeshMeshPtr    m_loadedMesh;
            IScalableMeshTexturePtr m_loadedTexture;            
            bool                    m_loadTexture;             

    protected: 

        virtual IScalableMeshMeshPtr _GetMesh(IScalableMeshMeshFlagsPtr& flags, bvector<bool>& clipsToShow) const override;

            virtual IScalableMeshMeshPtr _GetMeshByParts(const bvector<bool>& clipsToShow) const override;

            virtual IScalableMeshMeshPtr _GetMeshByParts(const bset<uint64_t>& clipsToShow) const override;

            virtual IScalableMeshTexturePtr _GetTexture() const override;

    public:             

            ScalableMeshCachedMeshNode(HFCPtr<SMPointIndexNode<POINT, Extent3dType>>& nodePtr, bool loadTexture)
            : ScalableMeshNode(nodePtr)
                {           
                auto meshNode = dynamic_pcast<SMMeshIndexNode<POINT, Extent3dType>, SMPointIndexNode<POINT, Extent3dType>>(m_node);

                m_loadTexture = loadTexture;
                }

            ScalableMeshCachedMeshNode() {};

            void LoadMesh(bool loadGraph, const bset<uint64_t>& clipsToShow);

            virtual StatusInt _GetCachedMesh(SmCachedDisplayMesh*& cachedMesh) const override {return ERROR;}

            virtual StatusInt _GetCachedTexture(SmCachedDisplayTexture*& cachedTexture) const override {return ERROR;}            

            static ScalableMeshCachedMeshNode* Create(HFCPtr<SMPointIndexNode<POINT, Extent3dType>>& nodePtr, bool loadTexture) 
                {
                return new ScalableMeshCachedMeshNode(nodePtr, loadTexture);
                }
    };


template<class POINT> class ScalableMeshCachedDisplayNode : public virtual IScalableMeshCachedDisplayNode, 
                                                            public virtual ScalableMeshNode<POINT>
    {    

    private: 

            RefCountedPtr<SMMemoryPoolGenericBlobItem<SmCachedDisplayData>> m_cachedDisplayData;                        

    protected:         
                                    
            virtual StatusInt _GetCachedMesh(SmCachedDisplayMesh*& cachedMesh) const override 
                {                            
                if (m_cachedDisplayData.IsValid())
                    {
                    cachedMesh = m_cachedDisplayData->GetData()->GetCachedDisplayMesh();
                    return SUCCESS;                
                    }

                return ERROR;                
                }

            virtual StatusInt _GetCachedTexture(SmCachedDisplayTexture*& cachedTexture) const override                 
                {
                if (m_cachedDisplayData.IsValid())
                    {
                    cachedTexture = m_cachedDisplayData->GetData()->GetCachedDisplayTexture();
                    return SUCCESS;                
                    }

                return ERROR;                                    
                }            
          
    public:             
            
            ScalableMeshCachedDisplayNode(HFCPtr<SMPointIndexNode<POINT, Extent3dType>>& nodePtr);

            virtual ~ScalableMeshCachedDisplayNode();
                
            void LoadMesh(bool loadGraph, const bset<uint64_t>& clipsToShow, IScalableMeshDisplayCacheManagerPtr& displayCacheManagerPtr, bool loadTexture);

            bool IsLoaded() const;
                
            bool HasCorrectClipping(const bset<uint64_t>& clipsToShow) const;

            void RemoveDisplayDataFromCache();
                
            static ScalableMeshCachedDisplayNode<POINT>* Create(HFCPtr<SMPointIndexNode<POINT, Extent3dType>>& nodePtr)
                {
                return new ScalableMeshCachedDisplayNode<POINT>(nodePtr);
                }

            typedef RefCountedPtr<ScalableMeshCachedDisplayNode<POINT>> Ptr;
    };


       


template<class POINT> class ScalableMeshNodeEdit : public IScalableMeshNodeEdit, public ScalableMeshNode<POINT>

    {
    protected:

    protected:

        virtual StatusInt _AddMesh(DPoint3d* vertices, size_t nVertices, int32_t* indices, size_t nIndices) override;

        // The binary buffer for the texture starts with three int numbers representing texture width, texture height and number of color channels
        virtual StatusInt _AddTextures(bvector<Byte>& data, bool sibling = false) override;

        virtual StatusInt _AddTexturedMesh(bvector<DPoint3d>& vertices, bvector<bvector<int32_t>>& ptsIndices, bvector<DPoint2d>& uv, bvector<bvector<int32_t>>& uvIndices, size_t nTexture) override;
        
        virtual StatusInt _SetNodeExtent(DRange3d& extent) override;

        virtual StatusInt _SetContentExtent(DRange3d& extent) override;

        virtual StatusInt _SetArePoints3d(bool arePoints3d) override;

        virtual bool _IsHeaderLoaded() const override;

        virtual bool _IsMeshLoaded() const override;

        virtual void _LoadHeader() const override;

    public:
        ScalableMeshNodeEdit(HFCPtr<SMPointIndexNode<POINT, Extent3dType>>& nodePtr);
        ScalableMeshNodeEdit() {};

        HFCPtr<SMPointIndexNode<POINT, Extent3dType>> GetNodePtr()
            {
            return m_node;
            }

    };


template<class POINT> class ScalableMeshNodeWithReprojection : public ScalableMeshNode<POINT>

    {
    private:
        GeoCoords::Reprojection  m_reprojectFunction;
    protected:
        virtual IScalableMeshMeshPtr _GetMesh(IScalableMeshMeshFlagsPtr& flags, bvector<bool>& clipsToShow) const override;

        virtual IScalableMeshMeshPtr _GetMeshByParts(const bvector<bool>& clipsToShow) const override;

        virtual IScalableMeshMeshPtr _GetMeshByParts(const bset<uint64_t>& clipsToShow) const override;
    public:
        ScalableMeshNodeWithReprojection(IScalableMeshNodePtr nodeInfo, const GeoCoords::Reprojection& reproject);
        ScalableMeshNodeWithReprojection(HFCPtr<SMPointIndexNode<POINT, Extent3dType>>& nodePtr, const GeoCoords::Reprojection& reproject);
    };

/*==================================================================*/
/*        3D MESH RELATED CODE - END                                */
/*==================================================================*/


//#include "ScalableMeshPointQuery.hpp"

END_BENTLEY_SCALABLEMESH_NAMESPACE
