/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/IScalableMeshQuery.h $
|    $RCSfile: IScalableMeshPointQuery.h,v $
|   $Revision: 1.17 $
|       $Date: 2012/11/29 17:30:53 $
|     $Author: Mathieu.St-Pierre $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

//#include <GeoCoord/BaseGeoCoord.h>
#include <TerrainModel/TerrainModel.h>
#include <TerrainModel/Core/IDTM.h>
#include <TerrainModel/Core/bcDTMClass.h>
#include <ScalableMesh/ScalableMeshDefs.h>
#include <ScalableMesh/IScalableMeshFeature.h>
#include <Mtg\MtgStructs.h>
#include <Geom\Polyface.h>
#include <list>
#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/ClipVector.h>
#include <Bentley\bset.h>
USING_NAMESPACE_BENTLEY_DGNPLATFORM
//#include <Bentley/RefCounted.h>
template<class POINT, class EXTENT> class ISMPointIndexQuery;
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct IScalableMeshMesh;
struct IScalableMeshTexture;
struct IScalableMeshNode;
struct IScalableMeshNodeEdit;
struct IScalableMeshCachedDisplayNode;
struct IScalableMeshPointQuery;
struct IScalableMeshMeshQuery;
struct IScalableMeshMeshFlags;
struct IScalableMeshMeshQueryParams;
struct IScalableMeshViewDependentMeshQueryParams;
struct IScalableMeshNodeRayQuery;
struct IScalableMeshNodeQueryParams;
struct IScalableMeshNodePlaneQueryParams;
struct IScalableMeshMeshRayQueryParams;
struct IScalableMeshQueryParameters;
struct IScalableMeshFixResolutionIndexQueryParams;
struct IScalableMeshFixResolutionMaxPointsQueryParams;
struct IScalableMeshFullResolutionQueryParams;
struct IScalableMeshViewDependentQueryParams;


typedef RefCountedPtr<IScalableMeshMesh>                              IScalableMeshMeshPtr;
typedef RefCountedPtr<IScalableMeshMeshFlags>                         IScalableMeshMeshFlagsPtr;
typedef RefCountedPtr<IScalableMeshTexture>                           IScalableMeshTexturePtr;
typedef RefCountedPtr<IScalableMeshMeshQuery>                         IScalableMeshMeshQueryPtr;
typedef RefCountedPtr<IScalableMeshMeshQueryParams>                   IScalableMeshMeshQueryParamsPtr;
typedef RefCountedPtr<IScalableMeshViewDependentMeshQueryParams>      IScalableMeshViewDependentMeshQueryParamsPtr;
typedef RefCountedPtr<IScalableMeshNode>                              IScalableMeshNodePtr;
typedef RefCountedPtr<IScalableMeshNodeEdit>                          IScalableMeshNodeEditPtr;
typedef RefCountedPtr<IScalableMeshCachedDisplayNode>                 IScalableMeshCachedDisplayNodePtr;
typedef RefCountedPtr<IScalableMeshNodeRayQuery>                      IScalableMeshNodeRayQueryPtr;
typedef RefCountedPtr<IScalableMeshNodeQueryParams>                   IScalableMeshNodeQueryParamsPtr;
typedef RefCountedPtr<IScalableMeshNodePlaneQueryParams>              IScalableMeshNodePlaneQueryParamsPtr;
typedef RefCountedPtr<IScalableMeshMeshRayQueryParams>                IScalableMeshMeshRayQueryParamsPtr;
typedef RefCountedPtr<IScalableMeshPointQuery>                        IScalableMeshPointQueryPtr;
typedef RefCountedPtr<IScalableMeshQueryParameters>                   IScalableMeshQueryParametersPtr;
typedef RefCountedPtr<IScalableMeshFixResolutionIndexQueryParams>     IScalableMeshFixResolutionIndexQueryParamsPtr;
typedef RefCountedPtr<IScalableMeshFixResolutionMaxPointsQueryParams> IScalableMeshFixResolutionMaxPointsQueryParamsPtr;
typedef RefCountedPtr<IScalableMeshFullResolutionQueryParams>         IScalableMeshFullResolutionQueryParamsPtr;
typedef RefCountedPtr<IScalableMeshViewDependentQueryParams>          IScalableMeshViewDependentQueryParamsPtr;


struct ScalableMeshExtentQuery;
typedef RefCountedPtr<ScalableMeshExtentQuery> ScalableMeshExtentQueryPtr;


/*
* Warning: Descartes depends on these status indexes. Do not try to play with those when backward compatibility
*          is required.
*/
enum class SMQueryStatus
    {
    S_SUCCESS,
    S_ERROR,
    S_NBPTSEXCEEDMAX,
    S_SUCCESS_INCOMPLETE,
    S_QTY,
    };

/*=================================================================================**//**
* Interface implemented by MRDTM engines.
* @bsiclass                                                     Bentley Systems
+===============+===============+===============+===============+===============+======*/
struct IScalableMeshQueryParameters abstract: virtual public RefCountedBase
    {            
    protected: 
        
                 IScalableMeshQueryParameters();                    
        virtual ~IScalableMeshQueryParameters();                

        virtual BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr _GetSourceGCS() = 0;

        virtual BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr _GetTargetGCS() = 0;

        virtual void _SetGCS(BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& sourceGCSPtr, 
                             BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCSPtr) = 0;
                                       
    public:    
                    
        BENTLEY_SM_EXPORT BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr GetSourceGCS();
        BENTLEY_SM_EXPORT BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr GetTargetGCS();

        BENTLEY_SM_EXPORT void SetGCS(BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& sourceGCSPtr, 
                                      BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCSPtr);                        
            
        BENTLEY_SM_EXPORT static double GetToleranceTriangulationParam();
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
               
        BENTLEY_SM_EXPORT static IScalableMeshFullResolutionQueryParamsPtr CreateParams();  
        
        BENTLEY_SM_EXPORT size_t GetMaximumNumberOfPoints();            

        BENTLEY_SM_EXPORT void SetMaximumNumberOfPoints(size_t maximumNumberOfPoints);            
        
        BENTLEY_SM_EXPORT bool GetReturnAllPtsForLowestLevel();

        BENTLEY_SM_EXPORT void SetReturnAllPtsForLowestLevel(bool returnAllPts);                               
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
                            
        BENTLEY_SM_EXPORT void SetViewBox(const DPoint3d viewBox[]);        
    };

struct IScalableMeshViewDependentQueryParams : public virtual ISrDTMViewDependentQueryParams 
    {
    protected :
     
        IScalableMeshViewDependentQueryParams();
        virtual ~IScalableMeshViewDependentQueryParams();

        virtual double        _GetMinScreenPixelsPerPoint() const = 0;

        virtual const double* _GetRootToViewMatrix() const = 0;

        virtual void          _SetMinScreenPixelsPerPoint(double minScreenPixelsPerPoint) = 0;

        virtual void          _SetRootToViewMatrix(const double rootToViewMatrix[][4]) = 0;    
               
               
    public : 

        double        GetMinScreenPixelsPerPoint() const;
       
        const double* GetRootToViewMatrix() const;
           
                                
        BENTLEY_SM_EXPORT void   SetMinScreenPixelsPerPoint(double minScreenPixelsPerPoint);

        BENTLEY_SM_EXPORT void   SetRootToViewMatrix(const double rootToViewMatrix[][4]);            
        
        BENTLEY_SM_EXPORT static IScalableMeshViewDependentQueryParamsPtr CreateParams();       
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
                                    
        BENTLEY_SM_EXPORT void SetResolutionIndex(int resolutionIndex);                            

        BENTLEY_SM_EXPORT static IScalableMeshFixResolutionIndexQueryParamsPtr CreateParams();     
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

        BENTLEY_SM_EXPORT void SetMaximumNumberPoints(__int64 maxNumberPoints);    

        BENTLEY_SM_EXPORT static IScalableMeshFixResolutionMaxPointsQueryParamsPtr CreateParams();  
    };


/*============================================================================**//**
* Interface implemented by MRDTM engines.
* @bsiclass                                                     Bentley Systems
+===============+===============+===============+===============+===============+======*/
struct IScalableMeshPointQuery abstract: RefCountedBase
    {
    /*__PUBLISH_SECTION_END__*/
    /*__PUBLISH_CLASS_VIRTUAL__*/

    private:  

    protected: 
                
        //NEEDS_WORK_SM: remove clip shape from query interface??
        virtual int _Query(bvector<DPoint3d>&               points, 
                           const DPoint3d*                  pClipShapePts, 
                           int                              nbClipShapePts, 
                           const IScalableMeshQueryParametersPtr&  scmQueryParamsPtr) const = 0;
        
        virtual int _GetNbClip() const = 0;
        virtual int _AddClip(DPoint3d* clipPointsP,
                             int       numberOfPoints, 
                             bool      isClipMask) = 0;    

        virtual int _RemoveAllClip() = 0;    

    /*__PUBLISH_SECTION_START__*/
    public:

        
        //! Gets the number of points of the DTM.
        //! @return The number of points of the DTM..
        BENTLEY_SM_EXPORT int Query(bvector<DPoint3d>&                     points,    
                                    const DPoint3d*                        pClipShapePts, 
                                    int                                    nbClipShapePts, 
                                    const IScalableMeshQueryParametersPtr& scmQueryParamsPtr) const;         
        
        BENTLEY_SM_EXPORT int GetNbClip() const;
            
        BENTLEY_SM_EXPORT int AddClip(DPoint3d* clipPointsP,
                                      int       numberOfPoints, 
                                      bool      isClipMask);    

        BENTLEY_SM_EXPORT int RemoveAllClip();    
    };

/*============================================================================**//**
* Interface implemented by MRDTM engines.
* @bsiclass                                                     Bentley Systems
+===============+===============+===============+===============+===============+======*/
struct IScalableMeshMesh : public RefCountedBase
                    
    {
    private:  

    protected:         

        virtual const BENTLEY_NAMESPACE_NAME::PolyfaceQuery* _GetPolyfaceQuery() const = 0;

        virtual DPoint3d* _EditPoints() = 0;

        virtual size_t _GetNbPoints() const = 0;

        virtual size_t _GetNbFaces() const = 0;

        virtual DTMStatusInt _GetAsBcDTM(BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTMPtr& bcdtm) = 0;

        virtual DTMStatusInt _GetBoundary(bvector<DPoint3d>& boundary) = 0;

        virtual bool _FindTriangleForProjectedPoint(int* outTriangle, DPoint3d& point, bool use2d = false) const = 0;
        virtual bool _FindTriangleForProjectedPoint(MTGNodeId& outTriangle, DPoint3d& point, bool use2d = false) const = 0;

        virtual int _ProjectPolyLineOnMesh(DPoint3d& endPt, bvector<bvector<DPoint3d>>& projectedPoints, const DPoint3d* points, int nPts, int* segment, const int* triangle, DPoint3d startPt, MTGNodeId& lastEdge) const = 0;
        virtual int _ProjectPolyLineOnMesh(DPoint3d& endPt, bvector<bvector<DPoint3d>>& projectedPoints, const DPoint3d* points, int nPts, int* segment, const MTGNodeId triangleEdge, DPoint3d startPt) const = 0;

        virtual bool _FindTriangleAlongRay(int* outTriangle, DRay3d& ray, MTGNodeId edge = -1) const = 0;
        virtual bool _FindTriangleAlongRay(MTGNodeId& outTriangle, DRay3d& ray) const = 0;

        virtual bool _CutWithPlane(bvector<DSegment3d>& segmentList, DPlane3d& cuttingPlane) const = 0;

    public: 

        BENTLEY_SM_EXPORT const BENTLEY_NAMESPACE_NAME::PolyfaceQuery* GetPolyfaceQuery() const;

        BENTLEY_SM_EXPORT size_t GetNbPoints() const;

        BENTLEY_SM_EXPORT size_t GetNbFaces() const;

        DPoint3d* EditPoints();

        BENTLEY_SM_EXPORT DTMStatusInt GetAsBcDTM(BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTMPtr& bcdtm);

        BENTLEY_SM_EXPORT DTMStatusInt GetBoundary(bvector<DPoint3d>& boundary);

        //NEEDS_WORK_SM: maybe move all geometry-related functions to util interface
        
        BENTLEY_SM_EXPORT bool FindTriangleForProjectedPoint(int* outTriangle, DPoint3d& point, bool use2d = false) const;
        BENTLEY_SM_EXPORT bool FindTriangleForProjectedPoint(MTGNodeId& outTriangle, DPoint3d& point, bool use2d = false) const;

        BENTLEY_SM_EXPORT int ProjectPolyLineOnMesh(DPoint3d& endPt, bvector<bvector<DPoint3d>>& projectedPoints, const DPoint3d* points, int nPts, int* segment, const MTGNodeId triangleEdge, DPoint3d startPt) const;

        BENTLEY_SM_EXPORT int ProjectPolyLineOnMesh(DPoint3d& endPt, bvector<bvector<DPoint3d>>& projectedPoints, const DPoint3d* points, int nPts, int* segment, const int* triangle, DPoint3d startPt, MTGNodeId& lastEdge) const;

        BENTLEY_SM_EXPORT bool FindTriangleAlongRay(int* outTriangle, DRay3d& ray, MTGNodeId edge =-1) const;
        BENTLEY_SM_EXPORT bool FindTriangleAlongRay(MTGNodeId& outTriangle, DRay3d& ray) const;

        BENTLEY_SM_EXPORT bool CutWithPlane(bvector<DSegment3d>& segmentList, DPlane3d& cuttingPlane) const;
        
        BENTLEY_SM_EXPORT static IScalableMeshMeshPtr Create(size_t         nbPoints, 
                                                             DPoint3d*      points, 
                                                             size_t         nbFaceIndexes, 
                                                             const int32_t* faceInd, 
                                                             size_t         normalCount,
                                                             DVec3d*        pNormal,
                                                             int32_t*       pNormalIndex,
                                                             size_t         uvCount, 
                                                             DVec2d*        pUv,
                                                             int32_t*       pUvIndex);
    
    };


struct IScalableMeshTexture : public RefCountedBase
    {
    
protected:
    virtual const Byte* _GetData() const = 0;
    virtual size_t      _GetSize() const = 0;
    virtual Point2d     _GetDimension() const = 0;
    
public:
//    BENTLEY_SM_EXPORT void Initialize(node, host, viewContext);
    BENTLEY_SM_EXPORT const Byte* GetData() const;
    BENTLEY_SM_EXPORT size_t      GetSize() const;
    BENTLEY_SM_EXPORT size_t      GetNOfChannels() const;
    BENTLEY_SM_EXPORT Point2d     GetDimension() const;    
    };

typedef size_t ScalableMeshTextureID;

struct IScalableMeshMeshFlags abstract: public RefCountedBase
    {
    protected:
        virtual bool _ShouldLoadTexture() const = 0;
        virtual bool _ShouldLoadGraph() const = 0;

        virtual void _SetLoadTexture(bool loadTexture) = 0;
        virtual void _SetLoadGraph(bool loadGraph) = 0;

    public:
        BENTLEY_SM_EXPORT bool ShouldLoadTexture() const;
        BENTLEY_SM_EXPORT bool ShouldLoadGraph() const;

        BENTLEY_SM_EXPORT void SetLoadTexture(bool loadTexture);
        BENTLEY_SM_EXPORT void SetLoadGraph(bool loadGraph);

        BENTLEY_SM_EXPORT static IScalableMeshMeshFlagsPtr Create();

        BENTLEY_SM_EXPORT static IScalableMeshMeshFlagsPtr Create(bool shouldLoadTexture, bool shouldLoadGraph);
    };


struct IScalableMeshNode abstract: virtual public RefCountedBase

    {
    private:
    protected:
        virtual BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTMPtr  _GetBcDTM() const = 0;

        virtual bool    _ArePoints3d() const = 0;

        virtual bool    _ArePointsFullResolution() const = 0;

        virtual IScalableMeshMeshPtr _GetMesh(IScalableMeshMeshFlagsPtr& flags) const = 0;

        virtual IScalableMeshMeshPtr _GetMeshUnderClip(IScalableMeshMeshFlagsPtr& flags, uint64_t clip) const = 0;

        virtual IScalableMeshMeshPtr _GetMeshByParts(const bset<uint64_t>& clipsToShow) const = 0;

        virtual IScalableMeshTexturePtr _GetTexture() const = 0;    

        virtual bool                    _IsTextured() const = 0;
        
        virtual bvector<IScalableMeshNodePtr> _GetNeighborAt(char relativePosX, char relativePosY, char relativePosZ) const = 0;

        virtual bvector<IScalableMeshNodePtr> _GetChildrenNodes() const = 0;
        
        virtual void     _ApplyAllExistingClips() const = 0;

        virtual void     _RefreshMergedClip() const = 0;

        virtual bool     _AddClip(uint64_t id, bool isVisible) const = 0;

        virtual bool     _ModifyClip(uint64_t id, bool isVisible) const = 0;

        virtual bool     _DeleteClip(uint64_t id, bool isVisible) const = 0;
        
        virtual DRange3d _GetNodeExtent() const = 0;

        virtual DRange3d _GetContentExtent() const = 0;

        virtual __int64 _GetNodeId() const = 0;

        virtual size_t _GetLevel() const = 0;

        virtual size_t  _GetPointCount() const = 0;                

        virtual bool _IsHeaderLoaded() const = 0;

        virtual bool _IsMeshLoaded() const = 0;        

        virtual void _LoadHeader() const = 0;

        virtual bool _HasClip(uint64_t id) const = 0;

        virtual bool _IsClippingUpToDate() const = 0;

        virtual void _GetSkirtMeshes(bvector<PolyfaceHeaderPtr>& meshes) const = 0;

        virtual bool _RunQuery(ISMPointIndexQuery<DPoint3d, DRange3d>& query, bvector<IScalableMeshNodePtr>& nodes) const = 0;

        virtual bool _RunQuery(ISMPointIndexQuery<DPoint3d, DRange3d>& query) const = 0;

                
    public:
        static const BENTLEY_SM_EXPORT ScalableMeshTextureID UNTEXTURED_PART = 0;

        BENTLEY_SM_EXPORT BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTMPtr          GetBcDTM() const;
        
        BENTLEY_SM_EXPORT bool          ArePoints3d() const;

        BENTLEY_SM_EXPORT bool          ArePointsFullResolution() const;
        
        BENTLEY_SM_EXPORT IScalableMeshMeshPtr GetMesh(IScalableMeshMeshFlagsPtr& flags) const;

        BENTLEY_SM_EXPORT IScalableMeshMeshPtr GetMeshUnderClip(IScalableMeshMeshFlagsPtr& flags,uint64_t clip) const;

        BENTLEY_SM_EXPORT IScalableMeshMeshPtr GetMeshByParts(bset<uint64_t>& clipsToShow) const;

        BENTLEY_SM_EXPORT IScalableMeshTexturePtr GetTexture() const;   

        BENTLEY_SM_EXPORT bool                    IsTextured() const;           
                
        //Gets neighbors by relative position. For example, neighbor (-1, 0, 0) shares the node's left face. (1,1,0) shares the node's top-right diagonal. 
        BENTLEY_SM_EXPORT bvector<IScalableMeshNodePtr> GetNeighborAt(char relativePosX,  char relativePosY,  char relativePosZ) const;

        BENTLEY_SM_EXPORT bvector<IScalableMeshNodePtr> GetChildrenNodes() const;

        BENTLEY_SM_EXPORT void     ApplyAllExistingClips() const;

        BENTLEY_SM_EXPORT void     RefreshMergedClip() const;

        BENTLEY_SM_EXPORT bool     AddClip(uint64_t id, bool isVisible=true) const;

        BENTLEY_SM_EXPORT bool     AddClipAsync(uint64_t id, bool isVisible = true) const;

        BENTLEY_SM_EXPORT bool     ModifyClip(uint64_t id,bool isVisible=true) const;

        BENTLEY_SM_EXPORT bool     DeleteClip(uint64_t id, bool isVisible=true) const;

        BENTLEY_SM_EXPORT DRange3d GetNodeExtent() const;

        BENTLEY_SM_EXPORT DRange3d GetContentExtent() const;

        BENTLEY_SM_EXPORT __int64 GetNodeId() const;

        BENTLEY_SM_EXPORT size_t GetLevel() const;

        BENTLEY_SM_EXPORT size_t GetPointCount() const;        

        BENTLEY_SM_EXPORT bool IsHeaderLoaded() const;

        BENTLEY_SM_EXPORT bool IsMeshLoaded() const;

        BENTLEY_SM_EXPORT void LoadNodeHeader() const;

        BENTLEY_SM_EXPORT bool HasClip(uint64_t id) const;

        BENTLEY_SM_EXPORT bool IsClippingUpToDate() const;

        BENTLEY_SM_EXPORT void GetSkirtMeshes(bvector<PolyfaceHeaderPtr>& meshes) const;

        BENTLEY_SM_EXPORT bool RunQuery(ISMPointIndexQuery<DPoint3d, DRange3d>& query, bvector<IScalableMeshNodePtr>& nodes) const;

        BENTLEY_SM_EXPORT bool RunQuery(ISMPointIndexQuery<DPoint3d, DRange3d>& query) const;
    };

struct SmCachedDisplayMesh;
struct SmCachedDisplayTexture;

struct IScalableMeshCachedDisplayNode : public virtual IScalableMeshNode
    {
    protected:

        virtual StatusInt _GetCachedMesh(SmCachedDisplayMesh*& cachedMesh) const = 0;

        virtual StatusInt _GetCachedTexture(SmCachedDisplayTexture*& cachedTexture) const = 0;       

    public : 

        BENTLEY_SM_EXPORT StatusInt GetCachedMesh(SmCachedDisplayMesh*& cachedMesh) const;

        BENTLEY_SM_EXPORT StatusInt GetCachedTexture(SmCachedDisplayTexture*& cachedTexture) const;        
    };


struct IScalableMeshNodeEdit : public virtual IScalableMeshNode
    {
    protected:
        virtual StatusInt _AddMesh(DPoint3d* vertices, size_t nVertices, int32_t* indices, size_t nIndices) = 0;
        virtual StatusInt _AddTexturedMesh(bvector<DPoint3d>& vertices, bvector<bvector<int32_t>>& ptsIndices, bvector<DPoint2d>& uv, bvector<bvector<int32_t>>& uvIndices, size_t idTexture) = 0;
        virtual StatusInt _SetNodeExtent(DRange3d& extent) = 0;
        virtual StatusInt _SetContentExtent(DRange3d& extent) = 0;
        virtual StatusInt _SetArePoints3d(bool arePoints3d) = 0;
        virtual StatusInt _AddTextures(bvector<Byte>& data, bool sibling) = 0;
    public:
        BENTLEY_SM_EXPORT StatusInt AddMesh(DPoint3d* vertices, size_t nVertices, int32_t* indices, size_t nIndices);
        BENTLEY_SM_EXPORT StatusInt AddTexturedMesh(bvector<DPoint3d>& vertices, bvector<bvector<int32_t>>& ptsIndices, bvector<DPoint2d>& uv, bvector<bvector<int32_t>>& uvIndices, size_t nTexture);
        BENTLEY_SM_EXPORT StatusInt AddTextures(bvector<Byte>& data, bool sibling = false);
        BENTLEY_SM_EXPORT StatusInt SetNodeExtent(DRange3d& extent);
        BENTLEY_SM_EXPORT StatusInt SetContentExtent(DRange3d& extent);
        BENTLEY_SM_EXPORT StatusInt SetArePoints3d(bool arePoints3d);
    };


struct IScalableMeshMeshQueryParams abstract : virtual public RefCountedBase
    {
    protected:
        IScalableMeshMeshQueryParams();
        virtual ~IScalableMeshMeshQueryParams();

        virtual BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr _GetSourceGCS() = 0;

        virtual BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr _GetTargetGCS() = 0;

        virtual size_t _GetLevel() = 0;

        virtual void _SetGCS(BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& sourceGCSPtr,
                             BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCSPtr) = 0;

        virtual void _SetLevel(size_t depth) = 0;
    public:
        BENTLEY_SM_EXPORT static IScalableMeshMeshQueryParamsPtr CreateParams();

        BENTLEY_SM_EXPORT BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr GetSourceGCS();
        BENTLEY_SM_EXPORT BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr GetTargetGCS();

        BENTLEY_SM_EXPORT size_t GetLevel();

        BENTLEY_SM_EXPORT void SetGCS(BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& sourceGCSPtr,
                                      BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCSPtr);

        BENTLEY_SM_EXPORT void SetLevel(size_t depth);
    };


typedef bool (*StopQueryCallbackFP)();

struct IScalableMeshViewDependentMeshQueryParams abstract: virtual public IScalableMeshMeshQueryParams
    {
    protected :
     
        IScalableMeshViewDependentMeshQueryParams();
        virtual ~IScalableMeshViewDependentMeshQueryParams();

        virtual const DPoint3d*     _GetViewBox() const = 0;                    
                            
        virtual void                _SetViewBox(const DPoint3d viewBox[]) = 0;         

        virtual double              _GetMinScreenPixelsPerPoint() const = 0;        

        virtual StopQueryCallbackFP _GetStopQueryCallback() const = 0;
               
        virtual const double*       _GetRootToViewMatrix() const = 0;

        virtual const ClipVectorPtr _GetViewClipVector() const = 0;

        virtual bool                _IsProgressiveDisplay() const = 0;

        virtual void            _SetMinScreenPixelsPerPoint(double minScreenPixelsPerPoint) = 0;

        virtual void            _SetProgressiveDisplay(bool isProgressiveDisplay) = 0;

        virtual void            _SetRootToViewMatrix(const double rootToViewMatrix[][4]) = 0;    

        virtual StatusInt       _SetStopQueryCallback(StopQueryCallbackFP stopQueryCallbackFP) = 0;
        
        virtual void            _SetViewClipVector(ClipVectorPtr& viewClipVector) = 0;            
                                                  
    public : 

        
        const DPoint3d*     GetViewBox() const;
                                        
        double              GetMinScreenPixelsPerPoint() const;

        StopQueryCallbackFP GetStopQueryCallback() const;

        bool                GetUseSameResolutionWhenCameraIsOff() const;

        bool                GetUseSplitThresholdForLevelSelection() const;

        bool                GetUseSplitThresholdForTileSelection() const;

        const double*       GetRootToViewMatrix() const;

        const ClipVectorPtr GetViewClipVector() const;

        bool                IsProgressiveDisplay() const;
    
        
        BENTLEY_SM_EXPORT void      SetViewBox(const DPoint3d viewBox[]);        

        BENTLEY_SM_EXPORT void      SetMinScreenPixelsPerPoint(double minScreenPixelsPerPoint);

        BENTLEY_SM_EXPORT void      SetProgressiveDisplay(bool isProgressiveDisplay);

        BENTLEY_SM_EXPORT void      SetRootToViewMatrix(const double rootToViewMatrix[][4]);    

        BENTLEY_SM_EXPORT StatusInt SetStopQueryCallback(StopQueryCallbackFP stopQueryCallbackFP);

        BENTLEY_SM_EXPORT void      SetUseSameResolutionWhenCameraIsOff(bool useSameResolution);        

        BENTLEY_SM_EXPORT void      SetUseSplitThresholdForLevelSelection(bool useSplitThreshold);        

        BENTLEY_SM_EXPORT void      SetUseSplitThresholdForTileSelection(bool useSplitThreshold);    

        BENTLEY_SM_EXPORT void      SetViewClipVector(ClipVectorPtr& viewClipVector);
        
        BENTLEY_SM_EXPORT static    IScalableMeshViewDependentMeshQueryParamsPtr CreateParams();       
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
    
        //! Gets the number of points of the DTM.
        //! @return The number of points of the DTM..
        BENTLEY_SM_EXPORT int Query(IScalableMeshMeshPtr&                               meshPtr,  
                                    const DPoint3d*                              pQueryExtentPts,
                                    int                                          nbQueryExtentPts,
                                    const IScalableMeshMeshQueryParamsPtr& scmQueryParamsPtr) const;

         BENTLEY_SM_EXPORT int Query(bvector<IScalableMeshNodePtr>&                      meshNodesPtr,    
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

        virtual size_t _GetLevel() const = 0;
        virtual void _SetLevel(size_t level) = 0;

        virtual bool _GetUseUnboundedRay() const = 0;
        virtual void _SetUseUnboundedRay(bool useUnboundedRay) = 0;
    public:
        BENTLEY_SM_EXPORT void SetDirection(DVec3d direction) { return _SetDirection(direction); }
        BENTLEY_SM_EXPORT DVec3d GetDirection() { return _GetDirection(); }

        BENTLEY_SM_EXPORT  void SetDepth(double depth) { return _SetDepth(depth); }
        BENTLEY_SM_EXPORT  double GetDepth() { return _GetDepth(); }

        BENTLEY_SM_EXPORT  void Set2d(bool is2d) { return _Set2d(is2d); }
        BENTLEY_SM_EXPORT bool Get2d() { return _Get2d(); }

        BENTLEY_SM_EXPORT void SetLevel(size_t level) { return _SetLevel(level); }
        BENTLEY_SM_EXPORT size_t GetLevel() { return _GetLevel(); }

        BENTLEY_SM_EXPORT void SetUseUnboundedRay(bool useUnboundedRay) { return _SetUseUnboundedRay(useUnboundedRay); }
        BENTLEY_SM_EXPORT bool GetUseUnboundedRay() { return _GetUseUnboundedRay(); }

        BENTLEY_SM_EXPORT static IScalableMeshNodeQueryParamsPtr CreateParams();
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

       virtual int _Query(bvector<IScalableMeshNodePtr>&                                nodesPtr,
                            const DPoint3d*                               pTestPt,
                            const DPoint3d*                               pClipShapePts,
                            int                                           nbClipShapePts,
                            const IScalableMeshNodeQueryParamsPtr& scmQueryParamsPtr) const = 0;
        

        /*__PUBLISH_SECTION_START__*/
    public:


        BENTLEY_SM_EXPORT int Query(IScalableMeshNodePtr&                                nodePtr,
                                    const DPoint3d*                              pTestPt,
                                    const DPoint3d*                              pClipShapePts,
                                    int                                          nbClipShapePts,
                                    const IScalableMeshNodeQueryParamsPtr& scmQueryParamsPtr) const;

        BENTLEY_SM_EXPORT int Query(bvector<IScalableMeshNodePtr>&                                nodesPtr,
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
        BENTLEY_SM_EXPORT void SetPlane(DPlane3d plane) { return _SetPlane(plane); }
        BENTLEY_SM_EXPORT DPlane3d GetPlane() { return _GetPlane(); }
        BENTLEY_SM_EXPORT  void SetDepth(double depth) { return _SetDepth(depth); }
        BENTLEY_SM_EXPORT  double GetDepth() { return _GetDepth(); }


        BENTLEY_SM_EXPORT static IScalableMeshNodePlaneQueryParamsPtr CreateParams();
    };



END_BENTLEY_SCALABLEMESH_NAMESPACE
