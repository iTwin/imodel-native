/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshQuery.h $
|    $RCSfile: ScalableMeshQuery.h,v $
|   $Revision: 1.20 $
|       $Date: 2012/06/27 14:07:12 $
|     $Author: Chantal.Poulin $
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
#pragma warning(disable:4250) //yes visual studio I am aware of virtual inheritance thank you
#include <GeoCoord/BaseGeoCoord.h>
#include <ScalableMesh/GeoCoords/GCS.h>
#include <ScalableMesh/GeoCoords/Reprojection.h>


#include <ImagePP/all/h/HCPGCoordModel.h>
//#include <ImagePP/all/h/HRPPixelTypeV24R8G8B8.h>
//#include <ImagePP/all/h/HRPPixelTypeV24B8G8R8.h>
//#include <ImagePP/all/h/HRFBmpFile.h>

#include <ScalableMesh/IScalableMesh.h>
#include <ScalableMesh/IScalableMeshClipContainer.h>
#include <ScalableMesh/IScalableMeshProgressiveQuery.h>

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
class GeometryGuide;

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

        int64_t m_maxNumberPoints;
        
        virtual int64_t _GetMaxNumberPoints() override 
            {
            return m_maxNumberPoints;
            }

        virtual void    _SetMaximumNumberPoints(int64_t maxNumberPoints) override
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

        template<class EXTENT> static EXTENT GetExtentFromClipShape(const DPoint3d* pClipShapePts, 
                                                                   int             nbClipShapePts, 
                                                                   double          zMin, 
                                                                   double          zMax) /*const*/;
        //NEEDS_WORK_SM - TEMP in public END

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
        size_t    m_normalCount;
        DVec3d*   m_pNormal;
        int32_t*    m_pNormalIndex;
        mutable DVec3d*   m_pNormalAuto;
        DPoint2d*     m_pUv;
        int32_t*    m_pUvIndex;
        size_t      m_uvCount;

        bvector<DRange3d> m_boxes;

        mutable PolyfaceQueryCarrier* m_polyfaceQueryCarrier; 

    protected : 
        DPoint3d* m_points;
        size_t    m_nbPoints;
		Transform m_transform;
        mutable size_t    m_nbFaceIndexes;
        int32_t*    m_faceIndexes;

        virtual const BENTLEY_NAMESPACE_NAME::PolyfaceQuery* _GetPolyfaceQuery() const override;

        virtual size_t _GetNbPoints() const override;

        virtual size_t _GetNbFaces() const override;

        virtual DPoint3d* _EditPoints() override;

        virtual DTMStatusInt _GetAsBcDTM(BcDTMPtr& bcdtm)override;

        virtual DTMStatusInt _GetBoundary(bvector<DPoint3d>& boundary) override;

		virtual void _SetTransform(Transform myTransform);

		virtual void _RemoveSlivers(double edgeLengthRatio) override;

        virtual bool _FindTriangleForProjectedPoint(int* outTriangle, DPoint3d& point, bool use2d = false) const override;
        virtual bool _FindTriangleForProjectedPoint(MTGNodeId& outTriangle, DPoint3d& point, bool use2d = false) const override;

        virtual int _ProjectPolyLineOnMesh(DPoint3d& endPt, bvector<bvector<DPoint3d>>& projectedPoints, const DPoint3d* points, int nPts, int* segment, const MTGNodeId triangleEdge, DPoint3d startPt) const override;

        virtual int _ProjectPolyLineOnMesh(DPoint3d& endPt, bvector<bvector<DPoint3d>>& projectedPoints, const DPoint3d* points, int nPts, int* segment, const int* triangle, DPoint3d startPt, MTGNodeId& lastEdge) const override;
       
        virtual bool _FindTriangleAlongRay(int* outTriangle, DRay3d& ray, MTGNodeId edge = -1) const override;
        virtual bool _FindTriangleAlongRay(MTGNodeId& outTriangle, DRay3d& ray) const override;

        virtual bool _CutWithPlane(bvector<DSegment3d>& segmentList, DPlane3d& cuttingPlane) const override;

        virtual bool _IntersectRay(DPoint3d& pt, const DRay3d& ray) const override;
        virtual bool _IntersectRay(bvector<DTMRayIntersection>& pts, const DRay3d& ray) const override;

        virtual void _WriteToFile(WString& filePath) override;

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

        BENTLEY_SM_EXPORT int AppendMesh(size_t nbPoints, DPoint3d* points, size_t nbFaceIndexes, const int32_t* faceIndexes, size_t normalCount, DVec3d* pNormal, int32_t* pNormalIndex, size_t uvCount, const DPoint2d* pUv, const int32_t* pUvIndex);

        void ApplyDifferenceSet(const DifferenceSet& d);

        void ApplyClipMesh(const DifferenceSet& d);

        void RecalculateUVs(DRange3d& nodeRange);

		void RemoveDuplicates();

        void StoreTriangleBoxes();

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

        void FindTrianglesAroundLabel(bvector<bvector<DPoint3d>>& triangles, int labelValue);

    public:

        void SmoothToGeometry(const GeometryGuide& source, bvector<size_t>& affectedIndices, bvector<DPoint3d>& affectedIndicesCoords, double smoothness);

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
        double   m_maxPixelError;
        double   m_rootToViewMatrix[4][4];
        bool m_loadContours;
        

        //NEEDS_WORK_SM : Only one of those is likely required
        DPoint3d m_viewBox[8];                 
        ClipVectorPtr m_viewClipVector;

        StopQueryCallbackFP m_stopQueryCallbackFP;

        virtual size_t _GetLevel() override { return 0; }
        virtual void _SetLevel(size_t depth) override {};
        virtual void _SetUseAllResolutions(bool useAllResolutions) override {};
        virtual bool _GetUseAllResolutions() override { return false; };
        
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

        virtual double        _GetMaxPixelError() const override
        {
            return m_maxPixelError;
        }

        virtual double _GetTargetPixelTolerance() override
        {
            assert(false && "Not supported by this query");
            return 0.0;
        }

        virtual void _SetTargetPixelTolerance(double pixelTol) override
        {
            assert(false && "Not supported by this query");
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

        virtual bool            _ShouldLoadContours() const override
            {
            return m_loadContours;
            }

        virtual void          _SetMinScreenPixelsPerPoint(double minScreenPixelsPerPoint) override
            {
            m_minScreenPixelsPerPoint = minScreenPixelsPerPoint;
            }

        virtual void          _SetMaxPixelError(double errorInPixels) override
            {
            m_maxPixelError = errorInPixels;
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


        virtual void            _SetLoadContours(bool loadContours) override
            {
            m_loadContours = loadContours;
            }
        
    public : 

        ScalableMeshViewDependentMeshQueryParams()
            {            
            m_minScreenPixelsPerPoint = 100;  
            m_maxPixelError = 1.0;
            m_isProgressiveDisplay = false;
            m_stopQueryCallbackFP = 0;
            m_loadContours = false;
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
        bool m_useAllResolutions;

        double m_pixelTolerance;

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

        virtual bool _GetUseAllResolutions() override
            {
            return m_useAllResolutions;
            }

        virtual double _GetTargetPixelTolerance() override
        {
            return m_pixelTolerance;
        }

        virtual void _SetTargetPixelTolerance(double pixelTol) override
            {
            m_pixelTolerance = pixelTol;
            }

        virtual void _SetLevel(size_t depth) override
            {
            m_depth = depth;
            }

        virtual void _SetUseAllResolutions(bool useAllResolutions) override
            {
            m_useAllResolutions = useAllResolutions;
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
            m_useAllResolutions = false;
            m_pixelTolerance = 0.0;
            }

        virtual ~ScalableMeshMeshQueryParams()
            {}
    };

//int draw(DTMFeatureType dtmFeatureType,int numTriangles, int numMeshPts,DPoint3d *meshPtsP,DPoint3d *meshVectorsP,int numMeshFaces, long *meshFacesP,void *userP);

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

        virtual int _Query(bvector<IScalableMeshNodePtr>&                       meshNodes,
                           ClipVectorCP                                       queryExtent3d,
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


    protected:

        HFCPtr<SMPointIndex<POINT, Extent3dType>> m_scmIndexPtr;


        // Inherited from IScalableMeshMeshQuery
        virtual int _Query(IScalableMeshMeshPtr&                                meshPtr,
                           const DPoint3d*                               pQueryExtentPts,
                           int                                           nbQueryExtentPts,
                           const IScalableMeshMeshQueryParamsPtr&  scmQueryParamsPtr) const override;

        virtual int _Query(bvector<IScalableMeshNodePtr>&                       meshNodes,
                           const DPoint3d*                               pQueryExtentPts,
                           int                                           nbQueryExtentPts,
                           const IScalableMeshMeshQueryParamsPtr&  scmQueryParamsPtr) const override;

        virtual int _Query(bvector<IScalableMeshNodePtr>&                       meshNodes,
                           ClipVectorCP                                       queryExtent3d,
                           const IScalableMeshMeshQueryParamsPtr&  scmQueryParamsPtr) const override;

    public:

        ScalableMeshViewDependentMeshQuery(const HFCPtr<SMPointIndex<POINT, Extent3dType>>& pointIndexPtr);

        virtual ~ScalableMeshViewDependentMeshQuery();
    };

template <class POINT> class ScalableMeshContextMeshQuery : public ScalableMeshViewDependentMeshQuery<POINT>
    {
    public:
        void*   operator new(size_t size){ return bentleyAllocator_allocateRefCounted(size); }
        void    operator delete(void* rawMemory, size_t size) { bentleyAllocator_deleteRefCounted(rawMemory, size); }
        void*   operator new [] (size_t size) { return bentleyAllocator_allocateArrayRefCounted(size); }
        void    operator delete [] (void* rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted(rawMemory, size); }

    

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

        virtual int _Query(bvector<IScalableMeshNodePtr>&                       meshNodes,
                           ClipVectorCP                                       queryExtent3d,
                           const IScalableMeshMeshQueryParamsPtr&  scmQueryParamsPtr) const override;

    public:

        ScalableMeshContextMeshQuery(const HFCPtr<SMPointIndex<POINT, Extent3dType>>& pointIndexPtr);

        virtual ~ScalableMeshContextMeshQuery();
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

        virtual int _Query(bvector<IScalableMeshNodePtr>&                       meshNodes,
                           ClipVectorCP                                       queryExtent3d,
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
		size_t m_level;

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

        virtual double _GetTargetPixelTolerance() override
        {
            assert(false && "Not supported by this query");
            return 0.0;
        }

        virtual void _SetTargetPixelTolerance(double pixelTol) override
        {
            assert(false && "Not supported by this query");
        }

		virtual void _SetLevel(size_t level) override
		{
			m_level = level;
		}
		virtual size_t _GetLevel() override
		{
			return m_level;
		}
        virtual void _SetUseAllResolutions(bool useAllResolutions) override {};
        virtual bool _GetUseAllResolutions() override { return false; };
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

        virtual int _Query(bvector<IScalableMeshNodePtr>&                       meshNodes,
                           ClipVectorCP                                       queryExtent3d,
                           const IScalableMeshMeshQueryParamsPtr&  scmQueryParamsPtr) const override;

    public:

        ScalableMeshNodePlaneQuery(const HFCPtr<SMPointIndex<POINT, Extent3dType>>& pointIndexPtr);

        virtual ~ScalableMeshNodePlaneQuery();
    };

class ScalableMeshMeshFlags : public virtual IScalableMeshMeshFlags
    {
    protected:

        //NEEDS_WORK_SM : Load graph required since removed from file?
        bool m_loadGraph;
        bool m_loadClips;
        bool m_loadIndices;
        bool m_loadTexture;
        bool m_saveToCache;
        bool m_precomputeBoxes;

        bool           m_useClipsToShow; 
        bset<uint64_t> m_clipsToShow;
        bool           m_shouldInvertClips;

        virtual bool _ShouldLoadClips() const override;
        virtual bool _ShouldLoadTexture() const override;
        virtual bool _ShouldLoadIndices() const override;
        virtual bool _ShouldLoadGraph() const override;
        virtual bool _ShouldSaveToCache() const override;
        virtual bool _ShouldPrecomputeBoxes() const override;
        virtual bool _ShouldUseClipsToShow() const override;
        virtual bool _ShouldInvertClips() const override;
        virtual void _GetClipsToShow(bset<uint64_t>& clipsToShow) const override;         
        
        virtual void _SetLoadClips(bool loadClips) override;
        virtual void _SetLoadTexture(bool loadTexture) override;
        virtual void _SetLoadIndices(bool loadIndices) override;
        virtual void _SetLoadGraph(bool loadGraph) override;
        virtual void _SetSaveToCache(bool saveToCache) override;
        virtual void _SetPrecomputeBoxes(bool precomputeBoxes) override;
        virtual void _SetClipsToShow(bset<uint64_t>& clipsToShow, bool shouldInvertClips) override;

    public:
        ScalableMeshMeshFlags()
            {
            m_loadGraph = false;
            m_loadClips = false;
            m_loadTexture = false;
            m_loadIndices = true;
            m_saveToCache = false;
            m_precomputeBoxes = false;
            m_useClipsToShow = false;
            }

        virtual ~ScalableMeshMeshFlags() {}
    };

template<class POINT> class ScalableMeshNode : public virtual IScalableMeshNode

    {                
    protected:
        HFCPtr<SMPointIndexNode<POINT, Extent3dType>> m_node;        

        bool ComputeDiffSet(DifferenceSet& diffs, const bset<uint64_t>& clipsToShow, bool shouldInvertClips =false) const;

        virtual BcDTMPtr   _GetBcDTM() const override;

        virtual bool    _ArePoints3d() const override;

        virtual bool    _ArePointsFullResolution() const override;

        virtual IScalableMeshMeshPtr _GetMesh(IScalableMeshMeshFlagsPtr& flags) const override;
        
        virtual IScalableMeshMeshPtr _GetMeshUnderClip(IScalableMeshMeshFlagsPtr& flags, uint64_t clip) const override;

        virtual IScalableMeshMeshPtr _GetMeshUnderClip2(IScalableMeshMeshFlagsPtr& flags, ClipVectorPtr clips, uint64_t coverageID, bool isClipBoundary) const override;

        virtual IScalableMeshMeshPtr _GetMeshByParts(const bset<uint64_t>& clipsToShow) const override;

        virtual void   _RefreshMergedClip(Transform tr) const override;

        virtual bool   _AddClip(uint64_t id, bool isVisible) const override;

        virtual bool   _ModifyClip(uint64_t id,  bool isVisible) const override;

        virtual bool   _DeleteClip(uint64_t id, bool isVisible) const override;

        virtual IScalableMeshTexturePtr _GetTexture() const override;

        virtual IScalableMeshTexturePtr _GetTextureCompressed() const override;

        virtual bool                    _IsTextured() const override;

        virtual void                    _GetResolutions(float& geometricResolution, float& textureResolution) const override;
                
        virtual bvector<IScalableMeshNodePtr> _GetNeighborAt( char relativePosX, char relativePosY, char relativePosZ) const override;

        virtual bvector<IScalableMeshNodePtr> _GetChildrenNodes() const override;

        virtual IScalableMeshNodePtr _GetParentNode() const override;

        virtual DRange3d _GetNodeExtent() const override;

        virtual DRange3d _GetContentExtent() const override;

        virtual int64_t _GetNodeId() const override;

        virtual size_t _GetLevel() const override;

        virtual size_t _GetPointCount() const override;                

        virtual bool _IsHeaderLoaded() const override;        

        virtual void _LoadHeader() const override;

        virtual bool _HasClip(uint64_t clip) const override;

        virtual bool _IsClippingUpToDate() const override;

        virtual bool _IsDataUpToDate() const override;

        virtual void _UpdateData() override;

        virtual void _GetSkirtMeshes(bvector<PolyfaceHeaderPtr>& meshes, bset<uint64_t>& activeClips) const override;

        virtual bool _RunQuery(ISMPointIndexQuery<DPoint3d, DRange3d>& query, bvector<IScalableMeshNodePtr>& nodes) const override;

        virtual bool _RunQuery(ISMPointIndexQuery<DPoint3d, DRange3d>& query) const override;

		virtual void _ClearCachedData() override;

        SMNodeViewStatus _IsCorrectForView(IScalableMeshViewDependentMeshQueryParamsPtr& viewDependentQueryParams) const override;

        virtual IScalableMeshNodeEditPtr _EditNode() override;

#ifdef WIP_MESH_IMPORT
        virtual bool _IntersectRay(DPoint3d& pt, const DRay3d& ray, Json::Value& retrievedMetadata) override;

        virtual void _GetAllSubMeshes(bvector<IScalableMeshMeshPtr>& meshes, bvector<uint64_t>& texIDs) const override;

        virtual IScalableMeshTexturePtr _GetTexture(uint64_t texID) const override;
#endif

        
    public:   
#ifdef VANCOUVER_API
        static ScalableMeshNode<POINT>* CreateItem(HFCPtr<SMPointIndexNode<POINT, Extent3dType>>& nodePtr)
            {
            return new ScalableMeshNode<POINT>(nodePtr);
            }
#endif
        BENTLEY_SM_EXPORT ScalableMeshNode(HFCPtr<SMPointIndexNode<POINT, Extent3dType>>& nodePtr);
        BENTLEY_SM_EXPORT ScalableMeshNode() {};

        BENTLEY_SM_EXPORT ~ScalableMeshNode() { m_node = nullptr; }

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
            IScalableMeshTexturePtr m_loadedTextureCompressed;
            bool                    m_loadTexture;

    protected: 

        virtual IScalableMeshMeshPtr _GetMesh(IScalableMeshMeshFlagsPtr& flags) const override;

            virtual IScalableMeshMeshPtr _GetMeshByParts(const bset<uint64_t>& clipsToShow) const override;

            virtual IScalableMeshTexturePtr _GetTexture() const override;

            virtual IScalableMeshTexturePtr _GetTextureCompressed() const override;

            virtual void      _SetIsInVideoMemory(bool isInVideoMemory) override {}

            virtual bool      _GetContours(bvector<bvector<DPoint3d>>& contours) override
            {
                return false;
            }

    public:             

            ScalableMeshCachedMeshNode(HFCPtr<SMPointIndexNode<POINT, Extent3dType>>& nodePtr, bool loadTexture)
            : ScalableMeshNode<POINT>(nodePtr)
                {           
                auto meshNode = dynamic_pcast<SMMeshIndexNode<POINT, Extent3dType>, SMPointIndexNode<POINT, Extent3dType>>(m_node);

                m_loadTexture = loadTexture;
                }

            ScalableMeshCachedMeshNode() {};

            void LoadMesh(bool loadGraph, const bset<uint64_t>& clipsToShow);

            virtual StatusInt _GetCachedMeshes(bvector<SmCachedDisplayMesh*>& cachedMesh, bvector<bpair<bool, uint64_t>>& textureIds) const override { return ERROR; }

            virtual StatusInt _GetCachedTextures(bvector<SmCachedDisplayTexture*>& cachedTexture, bvector<uint64_t>& textureIds) const override { return ERROR; }

            virtual StatusInt _GetDisplayClipVectors(bvector<ClipVectorPtr>& clipVectors) const override {return ERROR;}


            static ScalableMeshCachedMeshNode* Create(HFCPtr<SMPointIndexNode<POINT, Extent3dType>>& nodePtr, bool loadTexture) 
                {
                return new ScalableMeshCachedMeshNode(nodePtr, loadTexture);
                }
    };


template<class POINT> class ScalableMeshCachedDisplayNode : public virtual IScalableMeshCachedDisplayNode, 
                                                            public virtual ScalableMeshNode<POINT>
    {    

    private: 

            mutable RefCountedPtr<SMMemoryPoolGenericVectorItem<SmCachedDisplayMeshData>> m_cachedDisplayMeshData;
            bvector< RefCountedPtr<SMMemoryPoolGenericBlobItem<SmCachedDisplayTextureData>>> m_cachedDisplayTextureData;
            bvector<ClipVectorPtr>                                          m_clipVectors;            
            const IScalableMesh* m_scalableMeshP;
			bool m_invertClips;
			bool m_loadTexture;


    protected:         
                                    
            virtual StatusInt _GetCachedMeshes(bvector<SmCachedDisplayMesh*>& cachedMesh, bvector<bpair<bool,uint64_t>>& textureIds) const override 
                {         
                if (m_cachedDisplayMeshData.IsValid() && m_cachedDisplayMeshData->size() > 0)
                    {
                    for (size_t i = 0; i < m_cachedDisplayMeshData->size(); ++i)
                        {
                        cachedMesh.push_back((*m_cachedDisplayMeshData)[i].GetCachedDisplayMesh());
                        uint64_t id = 0;
                        bool ret = (*m_cachedDisplayMeshData)[i].GetTextureInfo(id);                        
                        textureIds.push_back(make_bpair(ret, id));                        
                        }
                    return SUCCESS;
                    }

                return ERROR;                
                }

            virtual StatusInt _GetCachedTextures(bvector<SmCachedDisplayTexture*>& cachedTexture, bvector<uint64_t>& textureIds) const override
                {
                if (!m_cachedDisplayTextureData.empty())
                    {
                    for (size_t i = 0; i < m_cachedDisplayTextureData.size(); ++i)
                        {
                        if (m_cachedDisplayTextureData[i].IsValid())
                            {
                            cachedTexture.push_back(m_cachedDisplayTextureData[i]->GetData()->GetCachedDisplayTexture());
                            textureIds.push_back(m_cachedDisplayTextureData[i]->GetData()->GetTextureID());
                            }
                        else
                            {
                            cachedTexture.push_back(nullptr);
                            textureIds.push_back(0);
                            }
                        }
                    return SUCCESS;                
                    }

                return ERROR;                                    
                }            

            virtual StatusInt _GetDisplayClipVectors(bvector<ClipVectorPtr>& clipVectors) const override 
                {
                clipVectors.insert(clipVectors.end(), m_clipVectors.begin(), m_clipVectors.end());                
                return SUCCESS;                
                }

            virtual void      _SetIsInVideoMemory(bool isInVideoMemory);

            virtual bool      _GetContours(bvector<bvector<DPoint3d>>& contours) override
            {
                return false;
            }
          
    public:             
            
            ScalableMeshCachedDisplayNode(HFCPtr<SMPointIndexNode<POINT, Extent3dType>>& nodePtr);

            ScalableMeshCachedDisplayNode(HFCPtr<SMPointIndexNode<POINT, Extent3dType>>& nodePtr, Transform reprojectionTransform);

            ScalableMeshCachedDisplayNode(HFCPtr<SMPointIndexNode<POINT, Extent3dType>>& nodePtr, const IScalableMesh* scalableMeshP);

            ScalableMeshCachedDisplayNode(ScalableMeshCachedDisplayNode<POINT>& otherPtr);

            virtual ~ScalableMeshCachedDisplayNode();

            void AddClipVector(ClipVectorPtr& clipVector);
                
            void LoadMesh(bool loadGraph, const bset<uint64_t>& clipsToShow, IScalableMeshDisplayCacheManagerPtr& displayCacheManagerPtr, bool loadTexture, bool invertClips = false);

            bool IsLoaded() const;
            bool IsLoaded(IScalableMeshDisplayCacheManager* mgr, bool isTextureRequired) const;
            bool IsLoadedInVRAM(IScalableMeshDisplayCacheManager* mgr, bool isTextureRequired) const;
                
            bool HasCorrectClipping(const bset<uint64_t>& clipsToShow) const;

            void RemoveDisplayDataFromCache();

			bool HasInvertedClips()
			{
				return m_invertClips;
			}

			bool ShouldLoadTexture()
			{
				return m_loadTexture;
			}
          

            SmCachedDisplayTexture* GetCachedDisplayTextureForID(uint64_t textureID)
                {
                for (size_t i = 0; i < m_cachedDisplayTextureData.size(); ++i)
                    {
                    if (m_cachedDisplayTextureData[i]->GetData()->GetTextureID() == textureID)
                        return m_cachedDisplayTextureData[i]->GetData()->GetCachedDisplayTexture();
                    }
                return nullptr;
                }

            bool GetOrLoadAllTextureData(IScalableMeshDisplayCacheManagerPtr& displayCacheManagerPtr);
                
                
            static ScalableMeshCachedDisplayNode<POINT>* Create(HFCPtr<SMPointIndexNode<POINT, Extent3dType>>& nodePtr)
                {
                return new ScalableMeshCachedDisplayNode<POINT>(nodePtr);
                }

            static ScalableMeshCachedDisplayNode<POINT>* Create(HFCPtr<SMPointIndexNode<POINT, Extent3dType>>& nodePtr, Transform reprojectionTransform)
                {
                return new ScalableMeshCachedDisplayNode<POINT>(nodePtr, reprojectionTransform);
                }

            static ScalableMeshCachedDisplayNode<POINT>* Create(HFCPtr<SMPointIndexNode<POINT, Extent3dType>>& nodePtr, const IScalableMesh* scalableMeshP)
            {
                return new ScalableMeshCachedDisplayNode<POINT>(nodePtr, scalableMeshP);
            }

            typedef RefCountedPtr<ScalableMeshCachedDisplayNode<POINT>> Ptr;
    };

    struct ContoursParameters
    {
        float majorContourSpacing;
        float minorContourSpacing;
    };

    template<class POINT> class ScalableMeshContourCachedDisplayNode : public virtual ScalableMeshCachedDisplayNode<POINT>
    {

    private:
        
        bvector<bvector<DPoint3d>> m_contours;
        ContoursParameters m_params;
        std::atomic<bool> m_contoursReady;

        ScalableMeshContourCachedDisplayNode(HFCPtr<SMPointIndexNode<POINT, Extent3dType>>& nodePtr);
        ScalableMeshContourCachedDisplayNode(IScalableMeshCachedDisplayNode* nodePtr);

    protected:
        virtual bool      _GetContours(bvector<bvector<DPoint3d>>& contours) override;

    public:

        void ComputeContours(ContoursParameters params);

        static ScalableMeshContourCachedDisplayNode<POINT>* Create(HFCPtr<SMPointIndexNode<POINT, Extent3dType>>& nodePtr)
        {
            return new ScalableMeshContourCachedDisplayNode<POINT>(nodePtr);
        }

        static ScalableMeshContourCachedDisplayNode<POINT>* Create(IScalableMeshCachedDisplayNode* nodePtr)
        {
            return new ScalableMeshContourCachedDisplayNode<POINT>(nodePtr);
        }
    };


template<class POINT> class ScalableMeshNodeEdit : public IScalableMeshNodeEdit, public ScalableMeshNode<POINT>

    {
    protected:

    protected:

        virtual StatusInt _AddMesh(DPoint3d* vertices, size_t nVertices, int32_t* indices, size_t nIndices) override;

        // The binary buffer for each texture starts with three int numbers representing texture width, texture height and number of color channels
        virtual StatusInt _AddTextures(bvector<Byte>& data) override;

        virtual StatusInt _AddTexturedMesh(bvector<DPoint3d>& vertices, bvector<int32_t>& ptsIndices, bvector<DPoint2d>& uv, bvector<int32_t>& uvIndices, size_t nTexture, int64_t texID) override;

        virtual StatusInt _AddTexturedMesh(bvector<DPoint3d>& vertices, bvector<bvector<int32_t>>& ptsIndices, bvector<DPoint2d>& uv, bvector<bvector<int32_t>>& uvIndices, size_t nTexture, int64_t texID) override;
        
        virtual StatusInt _SetNodeExtent(DRange3d& extent) override;

        virtual StatusInt _SetContentExtent(DRange3d& extent) override;

        virtual StatusInt _SetArePoints3d(bool arePoints3d) override;

        virtual StatusInt _SetResolution(float geometricResolution, float textureResolution) override;

        virtual bool _IsHeaderLoaded() const override;        

        virtual void _LoadHeader() const override;

        virtual bvector<IScalableMeshNodeEditPtr> _EditChildrenNodes() override;
        virtual IScalableMeshNodeEditPtr _EditParentNode() override;

        virtual void   _ReplaceIndices(const bvector<size_t>& posToChange, const bvector<DPoint3d>& newCoordinates) override;

    public:
        BENTLEY_SM_EXPORT ScalableMeshNodeEdit(HFCPtr<SMPointIndexNode<POINT, Extent3dType>>& nodePtr);
        BENTLEY_SM_EXPORT ScalableMeshNodeEdit() {};

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
        virtual IScalableMeshMeshPtr _GetMesh(IScalableMeshMeshFlagsPtr& flags) const override;

        virtual IScalableMeshMeshPtr _GetMeshByParts(const bset<uint64_t>& clipsToShow) const override;
    public:
        ScalableMeshNodeWithReprojection(IScalableMeshNodePtr nodeInfo, const GeoCoords::Reprojection& reproject);
        ScalableMeshNodeWithReprojection(HFCPtr<SMPointIndexNode<POINT, Extent3dType>>& nodePtr, const GeoCoords::Reprojection& reproject);
    };

/*==================================================================*/
/*        3D MESH RELATED CODE - END                                */
/*==================================================================*/


template <class POINT> int BuildQueryObject(
        ISMPointIndexQuery<POINT, Extent3dType>*&                        pQueryObject,
        const DPoint3d*                                                       pQueryExtentPts,
        int                                                                   nbQueryExtentPts,
        IScalableMeshViewDependentMeshQueryParamsPtr                          queryParam,
        IScalableMesh*                                                        smP);

//This class encapsulates various geometry elements to provide a unified interface for e.g. distance.
class GeometryGuide
{
    enum Type
    {
        None = 0,
        Plane,
        Cylinder,
        Qty
    };

    Type m_type;
    DPlane3d m_planeDef;
    DPoint3d m_cylinderCenter;
    DVec3d m_cylinderDir;
    double m_cylinderRadius;
    Transform m_transformToCylinder;

public:
    GeometryGuide(const DPlane3d& plane);

    GeometryGuide(DPoint3d center, DVec3d direction, double radius, double height);

    double DistanceTo(const DPoint3d& pt) const;

    void TransformWith(const Transform& tr);

    DPoint3d Project(const DPoint3d& pt) const;

    DVec3d NormalAt(const DPoint3d& pt) const;
};

//#include "ScalableMeshPointQuery.hpp"

END_BENTLEY_SCALABLEMESH_NAMESPACE
