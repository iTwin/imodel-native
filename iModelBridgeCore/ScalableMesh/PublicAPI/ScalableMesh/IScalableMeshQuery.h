/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/IScalableMeshQuery.h $
|    $RCSfile: IScalableMeshQuery.h,v $
|   $Revision: 1.17 $
|       $Date: 2012/11/29 17:30:53 $
|     $Author: Mathieu.St-Pierre $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

//#include <GeoCoord/BaseGeoCoord.h>
#include <TerrainModel/TerrainModel.h>
#include <TerrainModel/Core/IDTM.h>
#include <ScalableMesh/ScalableMeshDefs.h>
#include <ScalableMesh/IScalableMeshFeature.h>
#include <Mtg\MtgStructs.h>
#include <Geom\Polyface.h>

//#include <Bentley/RefCounted.h>

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct IScalableMeshMesh;
struct IScalableMeshNode;
struct IScalableMeshNodeEdit;
struct IScalableMeshMeshQuery;
struct IScalableMeshMeshQueryParams;
struct IScalableMeshViewDependentMeshQueryParams;
struct IScalableMeshNodeRayQuery;
struct IScalableMeshNodeQueryParams;
struct IScalableMeshNodePlaneQueryParams;
struct IScalableMeshMeshRayQueryParams;
struct IScalableMeshQuery;
struct IScalableMeshQueryParameters;
struct IScalableMeshFixResolutionIndexQueryParams;
struct IScalableMeshFixResolutionMaxPointsQueryParams;
struct IScalableMeshFullResolutionQueryParams;
struct IScalableMeshFullResolutionLinearQueryParams;
struct IScalableMeshViewDependentQueryParams;
struct IScalableMeshQueryAllLinearsQueryParams;

typedef RefCountedPtr<IScalableMeshMesh>                              IScalableMeshMeshPtr;
typedef RefCountedPtr<IScalableMeshMeshQuery>                         IScalableMeshMeshQueryPtr;
typedef RefCountedPtr<IScalableMeshMeshQueryParams>                    IScalableMeshMeshQueryParamsPtr;
typedef RefCountedPtr<IScalableMeshViewDependentMeshQueryParams>      IScalableMeshViewDependentMeshQueryParamsPtr;
typedef RefCountedPtr<IScalableMeshNode>                                IScalableMeshNodePtr;
typedef RefCountedPtr<IScalableMeshNodeEdit>                                IScalableMeshNodeEditPtr;
typedef RefCountedPtr<IScalableMeshNodeRayQuery>                         IScalableMeshNodeRayQueryPtr;
typedef RefCountedPtr<IScalableMeshNodeQueryParams>                   IScalableMeshNodeQueryParamsPtr;
typedef RefCountedPtr<IScalableMeshNodePlaneQueryParams>                   IScalableMeshNodePlaneQueryParamsPtr;
typedef RefCountedPtr<IScalableMeshMeshRayQueryParams>                IScalableMeshMeshRayQueryParamsPtr;
typedef RefCountedPtr<IScalableMeshQuery>                             IScalableMeshQueryPtr;
typedef RefCountedPtr<IScalableMeshQueryParameters>                   IScalableMeshQueryParametersPtr;
typedef RefCountedPtr<IScalableMeshFixResolutionIndexQueryParams>     IScalableMeshFixResolutionIndexQueryParamsPtr;
typedef RefCountedPtr<IScalableMeshFixResolutionMaxPointsQueryParams> IScalableMeshFixResolutionMaxPointsQueryParamsPtr;
typedef RefCountedPtr<IScalableMeshFullResolutionQueryParams>         IScalableMeshFullResolutionQueryParamsPtr;
typedef RefCountedPtr<IScalableMeshFullResolutionLinearQueryParams>   IScalableMeshFullResolutionLinearQueryParamsPtr;
typedef RefCountedPtr<IScalableMeshViewDependentQueryParams>          IScalableMeshViewDependentQueryParamsPtr;
typedef RefCountedPtr<IScalableMeshQueryAllLinearsQueryParams>        IScalableMeshQueryAllLinearsQueryParamsPtr;


struct ScalableMeshExtentQuery;
typedef RefCountedPtr<ScalableMeshExtentQuery> ScalableMeshExtentQueryPtr;


/*=================================================================================**//**
* Interface implemented by MRDTM engines.
* @bsiclass                                                     Bentley Systems
+===============+===============+===============+===============+===============+======*/
struct IScalableMeshQueryParameters abstract: virtual public RefCountedBase
    {            
    protected: 
        
                 IScalableMeshQueryParameters();                    
        virtual ~IScalableMeshQueryParameters();

        virtual bool _GetTriangulationState() = 0;        
        virtual void _SetTriangulationState(bool pi_isReturnedDataTriangulated) = 0;                

        virtual Bentley::GeoCoordinates::BaseGCSPtr _GetSourceGCS() = 0;

        virtual Bentley::GeoCoordinates::BaseGCSPtr _GetTargetGCS() = 0;

        virtual void _SetGCS(Bentley::GeoCoordinates::BaseGCSPtr& sourceGCSPtr, 
                             Bentley::GeoCoordinates::BaseGCSPtr& targetGCSPtr) = 0;

        virtual long _GetEdgeOptionTriangulationParam() = 0;            

        virtual double _GetMaxSideLengthTriangulationParam() = 0;
                            
        virtual void _SetEdgeOptionTriangulationParam(long edgeOption) = 0;            

        virtual void _SetMaxSideLengthTriangulationParam(double maxSideLength) = 0;
                               
    public:    
            
        BENTLEYSTM_EXPORT bool GetTriangulationState();        
        BENTLEYSTM_EXPORT void SetTriangulationState(bool pi_isReturnedDataTriangulated);                


        BENTLEYSTM_EXPORT Bentley::GeoCoordinates::BaseGCSPtr GetSourceGCS();
        BENTLEYSTM_EXPORT Bentley::GeoCoordinates::BaseGCSPtr GetTargetGCS();

        BENTLEYSTM_EXPORT void SetGCS(Bentley::GeoCoordinates::BaseGCSPtr& sourceGCSPtr, 
                                      Bentley::GeoCoordinates::BaseGCSPtr& targetGCSPtr);
                
        BENTLEYSTM_EXPORT long GetEdgeOptionTriangulationParam();
            
        BENTLEYSTM_EXPORT double GetMaxSideLengthTriangulationParam();
            
        BENTLEYSTM_EXPORT void SetEdgeOptionTriangulationParam(long edgeOption);
            
        BENTLEYSTM_EXPORT void SetMaxSideLengthTriangulationParam(double maxSideLength);
            
        BENTLEYSTM_EXPORT static double GetToleranceTriangulationParam();
    };

struct IScalableMeshFullResolutionQueryParams : public virtual IScalableMeshQueryParameters
    {    

    protected :

        IScalableMeshFullResolutionQueryParams();
        virtual ~IScalableMeshFullResolutionQueryParams();
                
        virtual size_t _GetMaximumNumberOfPoints() = 0;            

        virtual void _SetMaximumNumberOfPoints(size_t maximumNumberOfPoints) = 0;            
        
        virtual bool _GetReturnAllPtsForLowestLevel() = 0;

        virtual void _SetReturnAllPtsForLowestLevel(bool returnAllPts) = 0;                               

    
    public : 
               
        BENTLEYSTM_EXPORT static IScalableMeshFullResolutionQueryParamsPtr CreateParams();  
        
        BENTLEYSTM_EXPORT size_t GetMaximumNumberOfPoints();            

        BENTLEYSTM_EXPORT void SetMaximumNumberOfPoints(size_t maximumNumberOfPoints);            
        
        BENTLEYSTM_EXPORT bool GetReturnAllPtsForLowestLevel();

        BENTLEYSTM_EXPORT void SetReturnAllPtsForLowestLevel(bool returnAllPts);                               
    };

struct IScalableMeshFullResolutionLinearQueryParams : public virtual IScalableMeshFullResolutionQueryParams
    {                 
    protected :

        IScalableMeshFullResolutionLinearQueryParams();
        virtual ~IScalableMeshFullResolutionLinearQueryParams();


        virtual size_t _GetMaximumNumberOfPointsForLinear() = 0;            

        virtual int _SetMaximumNumberOfPointsForLinear(size_t maximumNumberOfPointsForLinear) = 0;                   
                
        virtual void _SetUseDecimation(bool useDecimation) = 0;
           
        virtual bool _GetUseDecimation() = 0;
        
        virtual void _SetCutLinears(bool cutLinears) = 0;

        virtual bool _GetCutLinears() = 0;        

        virtual void _SetAddLinears(const bool addLinears) = 0;        

        virtual bool _GetAddLinears() = 0;        

        virtual const std::vector<int>& _GetFilteringFeatureTypes(bool& doIncludeFilteringFeatureTypes) = 0;      

        //When no feature type is specified all feature types are returned.
        virtual int                     _SetFilteringFeatureTypes(const std::vector<int>& filteringFeatureTypes, bool doIncludeFilteringFeatures) = 0;      

        virtual void                    _SetIncludeFilteringFeatureTypes(const bool& doIncludeFilteringFeatures) = 0;         
       
    public : 

        BENTLEYSTM_EXPORT static IScalableMeshFullResolutionLinearQueryParamsPtr CreateParams();    

        BENTLEYSTM_EXPORT size_t GetMaximumNumberOfPointsForLinear();            

        BENTLEYSTM_EXPORT int SetMaximumNumberOfPointsForLinear(size_t maximumNumberOfPointsForLinear);                   

        BENTLEYSTM_EXPORT void SetUseDecimation(bool useDecimation);
           
        BENTLEYSTM_EXPORT bool GetUseDecimation();
        
        BENTLEYSTM_EXPORT void SetCutLinears(bool cutLinears);

        BENTLEYSTM_EXPORT bool GetCutLinears();        

        BENTLEYSTM_EXPORT void SetAddLinears(const bool addLinears);        

        BENTLEYSTM_EXPORT bool GetAddLinears();        

        BENTLEYSTM_EXPORT const std::vector<int>& GetFilteringFeatureTypes(bool& doIncludeFilteringFeatureTypes);      

        //When no feature type is specified all feature types are returned.
        BENTLEYSTM_EXPORT int                     SetFilteringFeatureTypes(const std::vector<int>& filteringFeatureTypes, bool doIncludeFilteringFeatures);      

        BENTLEYSTM_EXPORT void                    SetIncludeFilteringFeatureTypes(const bool& doIncludeFilteringFeatures);      
    };

//MS Should probably be in ScalableMeshQuery.h
struct ISrDTMViewDependentQueryParams : public virtual IScalableMeshQueryParameters
    {                
    protected :        

        ISrDTMViewDependentQueryParams();
        virtual ~ISrDTMViewDependentQueryParams();

        virtual const DPoint3d* _GetViewBox() const = 0;                    
                            
        virtual void _SetViewBox(const DPoint3d viewBox[]) = 0;         

    public : 

        const DPoint3d* GetViewBox() const;
                            
        BENTLEYSTM_EXPORT void SetViewBox(const DPoint3d viewBox[]);        
    };

struct IScalableMeshViewDependentQueryParams : public virtual ISrDTMViewDependentQueryParams 
    {
    protected :
     
        IScalableMeshViewDependentQueryParams();
        virtual ~IScalableMeshViewDependentQueryParams();

        virtual double        _GetMinScreenPixelsPerPoint() const = 0;

        virtual bool          _GetUseSameResolutionWhenCameraIsOff() const = 0;

        virtual bool          _GetUseSplitThresholdForLevelSelection() const = 0;

        virtual bool          _GetUseSplitThresholdForTileSelection() const = 0;

        virtual const double* _GetRootToViewMatrix() const = 0;

        virtual void          _SetMinScreenPixelsPerPoint(double minScreenPixelsPerPoint) = 0;

        virtual void          _SetRootToViewMatrix(const double rootToViewMatrix[][4]) = 0;    

        virtual void          _SetUseSameResolutionWhenCameraIsOff(bool useSameResolution) = 0;        

        virtual void          _SetUseSplitThresholdForLevelSelection(bool useSplitThreshold) = 0;        

        virtual void          _SetUseSplitThresholdForTileSelection(bool useSplitThreshold) = 0;                        

       
               
    public : 

        double        GetMinScreenPixelsPerPoint() const;

        bool          GetUseSameResolutionWhenCameraIsOff() const;

        bool          GetUseSplitThresholdForLevelSelection() const;

        bool          GetUseSplitThresholdForTileSelection() const;

        const double* GetRootToViewMatrix() const;
           
                                
        BENTLEYSTM_EXPORT void   SetMinScreenPixelsPerPoint(double minScreenPixelsPerPoint);

        BENTLEYSTM_EXPORT void   SetRootToViewMatrix(const double rootToViewMatrix[][4]);    

        BENTLEYSTM_EXPORT void   SetUseSameResolutionWhenCameraIsOff(bool useSameResolution);        

        BENTLEYSTM_EXPORT void   SetUseSplitThresholdForLevelSelection(bool useSplitThreshold);        

        BENTLEYSTM_EXPORT void   SetUseSplitThresholdForTileSelection(bool useSplitThreshold);             
        
        BENTLEYSTM_EXPORT static IScalableMeshViewDependentQueryParamsPtr CreateParams();       
    };

template<class POINT> class ScalableMeshFixResolutionViewPointQuery;

struct IScalableMeshFixResolutionIndexQueryParams : public virtual IScalableMeshQueryParameters
    {    
    protected :

        IScalableMeshFixResolutionIndexQueryParams();
        virtual ~IScalableMeshFixResolutionIndexQueryParams();

        virtual int  _GetResolutionIndex() const = 0;

        virtual void _SetResolutionIndex(int resolutionIndex) = 0;                                    

    public : 

        int GetResolutionIndex() const;
                                    
        BENTLEYSTM_EXPORT void SetResolutionIndex(int resolutionIndex);                            

        BENTLEYSTM_EXPORT static IScalableMeshFixResolutionIndexQueryParamsPtr CreateParams();     
    };


struct IScalableMeshFixResolutionMaxPointsQueryParams : public virtual IScalableMeshQueryParameters
    {
    template<class POINT>
    friend class ScalableMeshFixResolutionViewPointQuery;
    
    protected :

        IScalableMeshFixResolutionMaxPointsQueryParams();
        virtual ~IScalableMeshFixResolutionMaxPointsQueryParams();

        virtual __int64 _GetMaxNumberPoints() = 0;

        virtual void    _SetMaximumNumberPoints(__int64 maxNumberPoints) = 0;            
                
    public :                 

        __int64 GetMaxNumberPoints();

        BENTLEYSTM_EXPORT void SetMaximumNumberPoints(__int64 maxNumberPoints);    

        BENTLEYSTM_EXPORT static IScalableMeshFixResolutionMaxPointsQueryParamsPtr CreateParams();  
    };

struct IScalableMeshQueryAllLinearsQueryParams : public virtual IScalableMeshFullResolutionLinearQueryParams
    {    
    protected :

        IScalableMeshQueryAllLinearsQueryParams();
        virtual ~IScalableMeshQueryAllLinearsQueryParams();

        virtual std::list<IScalableMeshFeaturePtr> _GetFeatures() = 0;        

#ifdef SCALABLE_TERRAIN_MODEL_PRIVATE_SECTION
        virtual void                   _SetFeatures (const list<HFCPtr<HVEDTMLinearFeature>>& features) = 0;
#endif

    public :                        

#ifdef SCALABLE_TERRAIN_MODEL_PRIVATE_SECTION
        void SetFeatures (const list<HFCPtr<HVEDTMLinearFeature>>& features);
#endif
                
        BENTLEYSTM_EXPORT std::list<IScalableMeshFeaturePtr> GetFeatures();            

        BENTLEYSTM_EXPORT static IScalableMeshQueryAllLinearsQueryParamsPtr CreateParams();  
    };

/*============================================================================**//**
* Interface implemented by MRDTM engines.
* @bsiclass                                                     Bentley Systems
+===============+===============+===============+===============+===============+======*/
struct IScalableMeshQuery abstract: RefCountedBase
    {
    /*__PUBLISH_SECTION_END__*/
    /*__PUBLISH_CLASS_VIRTUAL__*/

    private:  

    protected: 
                
        //NEEDS_WORK_SM: remove clip shape from query interface??
        virtual int _Query(Bentley::TerrainModel::DTMPtr&   dtmPtr, 
                           const DPoint3d*                  pClipShapePts, 
                           int                              nbClipShapePts, 
                           const IScalableMeshQueryParametersPtr&  scmQueryParamsPtr) const = 0;
        /*
        virtual int _Query(IScalableMeshMeshPtr                    meshPtr, 
                           const DPoint3d*                  pClipShapePts, 
                           int                              nbClipShapePts, 
                           const IScalableMeshQueryParametersPtr&  mrDTMQueryParamsPtr) const = 0;
                           */

        virtual int _GetNbClip() const = 0;
        virtual int _AddClip(DPoint3d* clipPointsP,
                             int       numberOfPoints, 
                             bool      isClipMask) = 0;    

        virtual int _RemoveAllClip() = 0;    

    /*__PUBLISH_SECTION_START__*/
    public:

    /* 
    * Warning: Descartes depends on these status indexes. Do not try to play with those when backward compatibility
    *          is required.
    */          
    enum Status
        {
        S_SUCCESS,
        S_ERROR,
        S_NBPTSEXCEEDMAX, 
        S_QTY,
        };
        
        //! Gets the number of points of the DTM.
        //! @return The number of points of the DTM..
        BENTLEYSTM_EXPORT int Query(Bentley::TerrainModel::DTMPtr&    dtmPtr,                            
                                    const DPoint3d*                   pClipShapePts, 
                                    int                               nbClipShapePts, 
                                    const IScalableMeshQueryParametersPtr&   scmQueryParamsPtr) const;         
        
        BENTLEYSTM_EXPORT int GetNbClip() const;
            
        BENTLEYSTM_EXPORT int AddClip(DPoint3d* clipPointsP,
                                      int       numberOfPoints, 
                                      bool      isClipMask);    

        BENTLEYSTM_EXPORT int RemoveAllClip();    
    };

/*============================================================================**//**
* Interface implemented by MRDTM engines.
* @bsiclass                                                     Bentley Systems
+===============+===============+===============+===============+===============+======*/
struct IScalableMeshMesh : public RefCountedBase
                    
    {
    private:  

    protected:         

        virtual const Bentley::PolyfaceQuery* _GetPolyfaceQuery() const = 0;

        virtual DPoint3d* _EditPoints() = 0;

        virtual size_t _GetNbPoints() const = 0;

        virtual bool _FindTriangleForProjectedPoint(int* outTriangle, DPoint3d& point, bool use2d = false) const = 0;
        virtual bool _FindTriangleForProjectedPoint(MTGNodeId& outTriangle, DPoint3d& point, bool use2d = false) const = 0;

        virtual int _ProjectPolyLineOnMesh(DPoint3d& endPt, bvector<bvector<DPoint3d>>& projectedPoints, const DPoint3d* points, int nPts, int* segment, const int* triangle, DPoint3d startPt, MTGNodeId& lastEdge) const = 0;
        virtual int _ProjectPolyLineOnMesh(DPoint3d& endPt, bvector<bvector<DPoint3d>>& projectedPoints, const DPoint3d* points, int nPts, int* segment, const MTGNodeId triangleEdge, DPoint3d startPt) const = 0;

        virtual bool _FindTriangleAlongRay(int* outTriangle, DRay3d& ray, MTGNodeId edge = -1) const = 0;
        virtual bool _FindTriangleAlongRay(MTGNodeId& outTriangle, DRay3d& ray) const = 0;

        virtual bool _CutWithPlane(bvector<DSegment3d>& segmentList, DPlane3d& cuttingPlane) const = 0;

    public: 

        BENTLEYSTM_EXPORT const Bentley::PolyfaceQuery* GetPolyfaceQuery() const;

        BENTLEYSTM_EXPORT size_t GetNbPoints() const;

        DPoint3d* EditPoints();

        //NEEDS_WORK_SM: maybe move all geometry-related functions to util interface
        
        BENTLEYSTM_EXPORT bool FindTriangleForProjectedPoint(int* outTriangle, DPoint3d& point, bool use2d = false) const;
        BENTLEYSTM_EXPORT bool FindTriangleForProjectedPoint(MTGNodeId& outTriangle, DPoint3d& point, bool use2d = false) const;

        BENTLEYSTM_EXPORT int ProjectPolyLineOnMesh(DPoint3d& endPt, bvector<bvector<DPoint3d>>& projectedPoints, const DPoint3d* points, int nPts, int* segment, const MTGNodeId triangleEdge, DPoint3d startPt) const;

        BENTLEYSTM_EXPORT int ProjectPolyLineOnMesh(DPoint3d& endPt, bvector<bvector<DPoint3d>>& projectedPoints, const DPoint3d* points, int nPts, int* segment, const int* triangle, DPoint3d startPt, MTGNodeId& lastEdge) const;

        BENTLEYSTM_EXPORT bool FindTriangleAlongRay(int* outTriangle, DRay3d& ray, MTGNodeId edge =-1) const;
        BENTLEYSTM_EXPORT bool FindTriangleAlongRay(MTGNodeId& outTriangle, DRay3d& ray) const;

        BENTLEYSTM_EXPORT bool CutWithPlane(bvector<DSegment3d>& segmentList, DPlane3d& cuttingPlane) const;
        
        BENTLEYSTM_EXPORT static IScalableMeshMeshPtr Create(size_t    nbPoints, 
                                                      DPoint3d* points, 
                                                      size_t    nbFaceIndexes, 
                                                      int32_t*    faceInd, 
                                                      size_t    normalCount,
                                                      DVec3d*   pNormal,
                                                      int32_t*    pNormalIndex);
    
    };

struct IScalableMeshNode : public RefCountedBase

    {
    private:

    protected:

        virtual bool    _ArePoints3d() const = 0;

        virtual bool    _ArePointsFullResolution() const = 0;

        virtual IScalableMeshMeshPtr _GetMesh(bool loadGraph) const = 0;

        virtual bvector<IScalableMeshNodePtr> _GetNeighborAt(char relativePosX, char relativePosY, char relativePosZ) const = 0;
        
        virtual DRange3d _GetNodeExtent() const = 0;

        virtual DRange3d _GetContentExtent() const = 0;

        virtual __int64 _GetNodeId() const = 0;

        virtual size_t _GetLevel() const = 0;

        virtual size_t  _GetPointCount() const = 0;                

        virtual bool _IsHeaderLoaded() const = 0;

        virtual bool _IsMeshLoaded() const = 0;        

        virtual void _LoadHeader() const = 0;

                
    public:

        //NEEDS_WORK_SM : Not sure required except in debug
        BENTLEYSTM_EXPORT bool          ArePoints3d() const;

        BENTLEYSTM_EXPORT bool          ArePointsFullResolution() const;
        //NEEDS_WORK_SM : Not sure required except in debug - END

        BENTLEYSTM_EXPORT IScalableMeshMeshPtr GetMesh(bool loadGraph) const;
        
        //Gets neighbors by relative position. For example, neighbor (-1, 0, 0) shares the node's left face. (1,1,0) shares the node's top-right diagonal. 
        BENTLEYSTM_EXPORT bvector<IScalableMeshNodePtr> GetNeighborAt(char relativePosX,  char relativePosY,  char relativePosZ) const;

        BENTLEYSTM_EXPORT DRange3d GetNodeExtent() const;

        BENTLEYSTM_EXPORT DRange3d GetContentExtent() const;

        BENTLEYSTM_EXPORT __int64 GetNodeId() const;

        BENTLEYSTM_EXPORT size_t GetLevel() const;

        BENTLEYSTM_EXPORT size_t GetPointCount() const;        

        BENTLEYSTM_EXPORT bool IsHeaderLoaded() const;

        BENTLEYSTM_EXPORT bool IsMeshLoaded() const;

        BENTLEYSTM_EXPORT void LoadHeader() const;
    };


struct IScalableMeshNodeEdit : public IScalableMeshNode
    {
    protected:
        virtual StatusInt _AddMesh(DPoint3d* vertices, size_t nVertices, int32_t* indices, size_t nIndices) = 0;
        virtual StatusInt _SetNodeExtent(DRange3d& extent) = 0;
        virtual StatusInt _SetContentExtent(DRange3d& extent) = 0;
    public:
        BENTLEYSTM_EXPORT StatusInt AddMesh(DPoint3d* vertices, size_t nVertices, int32_t* indices, size_t nIndices);
        BENTLEYSTM_EXPORT StatusInt SetNodeExtent(DRange3d& extent);
        BENTLEYSTM_EXPORT StatusInt SetContentExtent(DRange3d& extent);
    };


struct IScalableMeshMeshQueryParams abstract : virtual public RefCountedBase
    {
    protected:
        IScalableMeshMeshQueryParams();
        virtual ~IScalableMeshMeshQueryParams();

        virtual Bentley::GeoCoordinates::BaseGCSPtr _GetSourceGCS() = 0;

        virtual Bentley::GeoCoordinates::BaseGCSPtr _GetTargetGCS() = 0;

        virtual void _SetGCS(Bentley::GeoCoordinates::BaseGCSPtr& sourceGCSPtr,
                             Bentley::GeoCoordinates::BaseGCSPtr& targetGCSPtr) = 0;
    public:
        BENTLEYSTM_EXPORT static IScalableMeshMeshQueryParamsPtr CreateParams();

        BENTLEYSTM_EXPORT Bentley::GeoCoordinates::BaseGCSPtr GetSourceGCS();
        BENTLEYSTM_EXPORT Bentley::GeoCoordinates::BaseGCSPtr GetTargetGCS();

        BENTLEYSTM_EXPORT void SetGCS(Bentley::GeoCoordinates::BaseGCSPtr& sourceGCSPtr,
                                      Bentley::GeoCoordinates::BaseGCSPtr& targetGCSPtr);
    };

struct IScalableMeshViewDependentMeshQueryParams abstract: virtual public IScalableMeshMeshQueryParams
    {
    protected :
     
        IScalableMeshViewDependentMeshQueryParams();
        virtual ~IScalableMeshViewDependentMeshQueryParams();

        virtual const DPoint3d*     _GetViewBox() const = 0;                    
                            
        virtual void                _SetViewBox(const DPoint3d viewBox[]) = 0;         

        virtual double              _GetMinScreenPixelsPerPoint() const = 0;

        virtual bool                _GetUseSameResolutionWhenCameraIsOff() const = 0;

        virtual bool                _GetUseSplitThresholdForLevelSelection() const = 0;

        virtual bool                _GetUseSplitThresholdForTileSelection() const = 0;

        virtual const double*       _GetRootToViewMatrix() const = 0;

        virtual const ClipVectorPtr _GetViewClipVector() const = 0;

        virtual void            _SetMinScreenPixelsPerPoint(double minScreenPixelsPerPoint) = 0;

        virtual void            _SetRootToViewMatrix(const double rootToViewMatrix[][4]) = 0;    

        virtual void            _SetUseSameResolutionWhenCameraIsOff(bool useSameResolution) = 0;        

        virtual void            _SetUseSplitThresholdForLevelSelection(bool useSplitThreshold) = 0;        

        virtual void            _SetUseSplitThresholdForTileSelection(bool useSplitThreshold) = 0;   

        virtual void            _SetViewClipVector(ClipVectorPtr& viewClipVector) = 0;            

         //NEEDS_WORK_SM : Maybe should be activate only on a specialized define
        virtual bool            _GetGatherQueriedNodeBoundaries() const = 0;                                            

        virtual void            _SetGatherQueriedNodeBoundaries(bool gatherNodeBoundaries) = 0;                                            

        virtual bool            _GetQueriedNodeBoundaries(bvector<DSegment3d>& nodeBoundaries) const = 0;

        virtual void            _SetQueriedNodeBoundaries(bvector<DSegment3d>& nodeBoundaries) = 0;   
        //NEEDS_WORK_SM : Maybe should be activate only on a specialized define END
                    
               
    public : 

         //NEEDS_WORK_SM : Maybe should be activate only on a specialized define
        bool          GetGatherQueriedNodeBoundaries() const;

        BENTLEYSTM_EXPORT void  SetGatherQueriedNodeBoundaries(bool gatherNodeBoundaries);

        BENTLEYSTM_EXPORT bool  GetQueriedNodeBoundaries(bvector<DSegment3d>& nodeBoundaries) const;
            
        void  SetQueriedNodeBoundaries(bvector<DSegment3d>& nodeBoundaries);
        //NEEDS_WORK_SM : Maybe should be activate only on a specialized define END


        const DPoint3d*     GetViewBox() const;
                                        
        double              GetMinScreenPixelsPerPoint() const;

        bool                GetUseSameResolutionWhenCameraIsOff() const;

        bool                GetUseSplitThresholdForLevelSelection() const;

        bool                GetUseSplitThresholdForTileSelection() const;

        const double*       GetRootToViewMatrix() const;

        const ClipVectorPtr GetViewClipVector() const;
    
        
        BENTLEYSTM_EXPORT void   SetViewBox(const DPoint3d viewBox[]);        

        BENTLEYSTM_EXPORT void   SetMinScreenPixelsPerPoint(double minScreenPixelsPerPoint);

        BENTLEYSTM_EXPORT void   SetRootToViewMatrix(const double rootToViewMatrix[][4]);    

        BENTLEYSTM_EXPORT void   SetUseSameResolutionWhenCameraIsOff(bool useSameResolution);        

        BENTLEYSTM_EXPORT void   SetUseSplitThresholdForLevelSelection(bool useSplitThreshold);        

        BENTLEYSTM_EXPORT void   SetUseSplitThresholdForTileSelection(bool useSplitThreshold);    

        BENTLEYSTM_EXPORT void   SetViewClipVector(ClipVectorPtr& viewClipVector);
        
        BENTLEYSTM_EXPORT static IScalableMeshViewDependentMeshQueryParamsPtr CreateParams();       
    };


struct IScalableMeshMeshQuery abstract: RefCountedBase
    {
    /*__PUBLISH_SECTION_END__*/
    /*__PUBLISH_CLASS_VIRTUAL__*/

    private:  

    protected: 
                                
        virtual int _Query(IScalableMeshMeshPtr&                                meshPtr,    
                           const DPoint3d*                               pQueryExtentPts, 
                           int                                           nbQueryExtentPts,
                           const IScalableMeshMeshQueryParamsPtr&  scmQueryParamsPtr) const = 0;

        virtual int _Query(bvector<IScalableMeshNodePtr>&                       meshNodesPtr,    
                           const DPoint3d*                               pQueryExtentPts,
                           int                                           nbQueryExtentPts,
                           const IScalableMeshMeshQueryParamsPtr&  scmQueryParamsPtr) const = 0;
        
    /*__PUBLISH_SECTION_START__*/
    public:

    /* 
    * Warning: Descartes depends on these status indexes. Do not try to play with those when backward compatibility
    *          is required.
    */          
    enum Status
        {
        S_SUCCESS,
        S_ERROR,
        S_NBPTSEXCEEDMAX, 
        S_QTY,
        };
        
        //! Gets the number of points of the DTM.
        //! @return The number of points of the DTM..
        BENTLEYSTM_EXPORT int Query(IScalableMeshMeshPtr&                               meshPtr,  
                                    const DPoint3d*                              pQueryExtentPts,
                                    int                                          nbQueryExtentPts,
                                    const IScalableMeshMeshQueryParamsPtr& scmQueryParamsPtr) const;

         BENTLEYSTM_EXPORT int Query(bvector<IScalableMeshNodePtr>&                      meshNodesPtr,    
                                     const DPoint3d*                              pQueryExtentPts,
                                     int                                          nbQueryExtentPts,
                                     const IScalableMeshMeshQueryParamsPtr& scmQueryParamsPtr) const;
    };


struct IScalableMeshNodeQueryParams abstract: RefCountedBase
    {
    protected:
        virtual void _SetDirection(DVec3d direction) = 0;
        virtual DVec3d _GetDirection() const = 0;
        virtual void _SetDepth(double depth) = 0;
        virtual double _GetDepth() const = 0;
        virtual void _Set2d(bool is2d) = 0;
        virtual bool _Get2d() const = 0;
    public:
        BENTLEYSTM_EXPORT void SetDirection(DVec3d direction) { return _SetDirection(direction); }
        BENTLEYSTM_EXPORT DVec3d GetDirection() { return _GetDirection(); }
        BENTLEYSTM_EXPORT  void SetDepth(double depth) { return _SetDepth(depth); }
        BENTLEYSTM_EXPORT  double GetDepth() { return _GetDepth(); }
        BENTLEYSTM_EXPORT  void Set2d(bool is2d) { return _Set2d(is2d); }
        BENTLEYSTM_EXPORT bool Get2d() { return _Get2d(); }

        BENTLEYSTM_EXPORT static IScalableMeshNodeQueryParamsPtr CreateParams();
    };

struct IScalableMeshNodeRayQuery abstract : RefCountedBase
    {
    /*__PUBLISH_SECTION_END__*/
    /*__PUBLISH_CLASS_VIRTUAL__*/

    private:

    protected:

        virtual int _Query(IScalableMeshNodePtr&                                nodePtr,
                           const DPoint3d*                               pTestPt,
                           const DPoint3d*                               pClipShapePts,
                           int                                           nbClipShapePts,
                           const IScalableMeshNodeQueryParamsPtr& scmQueryParamsPtr) const = 0;


        /*__PUBLISH_SECTION_START__*/
    public:

        /*
        * Warning: Descartes depends on these status indexes. Do not try to play with those when backward compatibility
        *          is required.
        */
        enum Status
            {
            S_SUCCESS,
            S_ERROR,
            S_NBPTSEXCEEDMAX,
            S_QTY,
            };


        BENTLEYSTM_EXPORT int Query(IScalableMeshNodePtr&                                nodePtr,
                                    const DPoint3d*                              pTestPt,
                                    const DPoint3d*                              pClipShapePts,
                                    int                                          nbClipShapePts,
                                    const IScalableMeshNodeQueryParamsPtr& scmQueryParamsPtr) const;
    };


struct IScalableMeshNodePlaneQueryParams abstract : IScalableMeshMeshQueryParams
    {
    protected:
        virtual void _SetPlane(DPlane3d plane) = 0;
        virtual DPlane3d _GetPlane() const = 0;
        virtual void _SetDepth(double depth) = 0;
        virtual double _GetDepth() const = 0;

    public:
        BENTLEYSTM_EXPORT void SetPlane(DPlane3d plane) { return _SetPlane(plane); }
        BENTLEYSTM_EXPORT DPlane3d GetPlane() { return _GetPlane(); }
        BENTLEYSTM_EXPORT  void SetDepth(double depth) { return _SetDepth(depth); }
        BENTLEYSTM_EXPORT  double GetDepth() { return _GetDepth(); }


        BENTLEYSTM_EXPORT static IScalableMeshNodePlaneQueryParamsPtr CreateParams();
    };



END_BENTLEY_SCALABLEMESH_NAMESPACE
