/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/IScalableMeshQuery.h $
|    $RCSfile: IScalableMeshPointQuery.h,v $
|   $Revision: 1.17 $
|       $Date: 2012/11/29 17:30:53 $
|     $Author: Mathieu.St-Pierre $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

//#include <GeoCoord/BaseGeoCoord.h>
#include <ScalableMesh/GeoCoords/GCS.h>
#include <TerrainModel/TerrainModel.h>
#include <TerrainModel/Core/IDTM.h>
#include <TerrainModel/Core/bcDTMClass.h>
#include <ScalableMesh/ScalableMeshDefs.h>
#include <ScalableMesh/IScalableMeshFeature.h>
#include <Mtg/MtgStructs.h>
#include <Geom/Polyface.h>
#include <list>
#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/ClipVector.h>
#include <Bentley/bset.h>

#include <BeJsonCpp/BeJsonUtilities.h>

#ifdef VANCOUVER_API
    USING_NAMESPACE_BENTLEY_DGNPLATFORM
#define CLIP_VECTOR_NAMESPACE BENTLEY_NAMESPACE_NAME
#else
    USING_NAMESPACE_BENTLEY_DGN
#define CLIP_VECTOR_NAMESPACE BENTLEY_NAMESPACE_NAME::Dgn
#endif



//#include <Bentley/RefCounted.h>
template<class POINT, class EXTENT> class ISMPointIndexQuery;
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct IScalableMeshMesh;
struct IScalableMesh;
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
struct IScalableMeshQueryParameters : virtual public RefCountedBase
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

        virtual int64_t _GetMaxNumberPoints() = 0;

        virtual void    _SetMaximumNumberPoints(int64_t maxNumberPoints) = 0;            
                
    public :                 

        int64_t GetMaxNumberPoints();

        BENTLEY_SM_EXPORT void SetMaximumNumberPoints(int64_t maxNumberPoints);    

        BENTLEY_SM_EXPORT static IScalableMeshFixResolutionMaxPointsQueryParamsPtr CreateParams();  
    };


/*============================================================================**//**
* Interface implemented by MRDTM engines.
* @bsiclass                                                     Bentley Systems
+===============+===============+===============+===============+===============+======*/
struct IScalableMeshPointQuery : RefCountedBase
    {
    /*__PUBLISH_SECTION_END__*/
    /*__PUBLISH_CLASS_VIRTUAL__*/

    private:  

    protected: 
                        
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

		virtual void _SetTransform(Transform newTransform) = 0;

		virtual void _RemoveSlivers(double edgeLengthRatio) = 0;

        virtual bool _FindTriangleForProjectedPoint(int* outTriangle, DPoint3d& point, bool use2d = false) const = 0;
        virtual bool _FindTriangleForProjectedPoint(MTGNodeId& outTriangle, DPoint3d& point, bool use2d = false) const = 0;

        virtual int _ProjectPolyLineOnMesh(DPoint3d& endPt, bvector<bvector<DPoint3d>>& projectedPoints, const DPoint3d* points, int nPts, int* segment, const int* triangle, DPoint3d startPt, MTGNodeId& lastEdge) const = 0;
        virtual int _ProjectPolyLineOnMesh(DPoint3d& endPt, bvector<bvector<DPoint3d>>& projectedPoints, const DPoint3d* points, int nPts, int* segment, const MTGNodeId triangleEdge, DPoint3d startPt) const = 0;

        virtual bool _FindTriangleAlongRay(int* outTriangle, DRay3d& ray, MTGNodeId edge = -1) const = 0;
        virtual bool _FindTriangleAlongRay(MTGNodeId& outTriangle, DRay3d& ray) const = 0;

        virtual bool _CutWithPlane(bvector<DSegment3d>& segmentList, DPlane3d& cuttingPlane) const = 0;

        virtual bool _IntersectRay(DPoint3d& pt, const DRay3d& ray) const = 0;
        virtual bool _IntersectRay(bvector<BENTLEY_NAMESPACE_NAME::TerrainModel::DTMRayIntersection>& pt, const DRay3d& ray) const = 0;

        virtual void _WriteToFile(WString& filePath) = 0;

    public: 

        BENTLEY_SM_EXPORT const BENTLEY_NAMESPACE_NAME::PolyfaceQuery* GetPolyfaceQuery() const;

        BENTLEY_SM_EXPORT size_t GetNbPoints() const;

        BENTLEY_SM_EXPORT size_t GetNbFaces() const;

        BENTLEY_SM_EXPORT DPoint3d* EditPoints();

        BENTLEY_SM_EXPORT DTMStatusInt GetAsBcDTM(BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTMPtr& bcdtm);

        BENTLEY_SM_EXPORT DTMStatusInt GetBoundary(bvector<DPoint3d>& boundary);

		BENTLEY_SM_EXPORT void SetTransform(Transform newTransform);

		BENTLEY_SM_EXPORT void RemoveSlivers(double edgeLengthRatio=1e-6);
                
        bool FindTriangleForProjectedPoint(int* outTriangle, DPoint3d& point, bool use2d = false) const;
        bool FindTriangleForProjectedPoint(MTGNodeId& outTriangle, DPoint3d& point, bool use2d = false) const;

        int ProjectPolyLineOnMesh(DPoint3d& endPt, bvector<bvector<DPoint3d>>& projectedPoints, const DPoint3d* points, int nPts, int* segment, const MTGNodeId triangleEdge, DPoint3d startPt) const;

        int ProjectPolyLineOnMesh(DPoint3d& endPt, bvector<bvector<DPoint3d>>& projectedPoints, const DPoint3d* points, int nPts, int* segment, const int* triangle, DPoint3d startPt, MTGNodeId& lastEdge) const;

        bool FindTriangleAlongRay(int* outTriangle, DRay3d& ray, MTGNodeId edge =-1) const;
        bool FindTriangleAlongRay(MTGNodeId& outTriangle, DRay3d& ray) const;

        BENTLEY_SM_EXPORT bool CutWithPlane(bvector<DSegment3d>& segmentList, DPlane3d& cuttingPlane) const;

        BENTLEY_SM_EXPORT bool IntersectRay(DPoint3d& pt, const DRay3d& ray) const;
        BENTLEY_SM_EXPORT bool IntersectRay(bvector<BENTLEY_NAMESPACE_NAME::TerrainModel::DTMRayIntersection>& points, const DRay3d& ray) const;

        BENTLEY_SM_EXPORT void WriteToFile(WString& filePath);

        
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

struct IScalableMeshMeshFlags : public RefCountedBase
    {
    protected:        

        virtual bool _ShouldLoadClips() const = 0;
        virtual bool _ShouldLoadTexture() const = 0;
        virtual bool _ShouldLoadIndices() const = 0;
        virtual bool _ShouldLoadGraph() const = 0;
        virtual bool _ShouldSaveToCache() const = 0;
        virtual bool _ShouldPrecomputeBoxes() const = 0;
        virtual bool _ShouldUseClipsToShow() const = 0;
        virtual bool _ShouldInvertClips() const = 0;
        virtual void _GetClipsToShow(bset<uint64_t>& clipsToShow) const = 0;

        virtual void _SetLoadClips(bool loadClips) = 0;
        virtual void _SetLoadTexture(bool loadTexture) = 0;
        virtual void _SetLoadIndices(bool loadIndices) = 0;        
        virtual void _SetLoadGraph(bool loadGraph) = 0;
        virtual void _SetSaveToCache(bool saveToCache) = 0;
        virtual void _SetPrecomputeBoxes(bool precomputeBoxes) = 0;
        virtual void _SetClipsToShow(bset<uint64_t>& clipsToShow, bool shouldInvertClips) = 0;

    public:
        
        BENTLEY_SM_EXPORT bool ShouldLoadClips() const;
        BENTLEY_SM_EXPORT bool ShouldLoadTexture() const;
        BENTLEY_SM_EXPORT bool ShouldLoadIndices() const;
        BENTLEY_SM_EXPORT bool ShouldLoadGraph() const;
        BENTLEY_SM_EXPORT bool ShouldSaveToCache() const;
        BENTLEY_SM_EXPORT bool ShouldPrecomputeBoxes() const;
        BENTLEY_SM_EXPORT bool ShouldUseClipsToShow() const;
        BENTLEY_SM_EXPORT bool ShouldInvertClips() const;
        BENTLEY_SM_EXPORT void GetClipsToShow(bset<uint64_t>& clipsToShow) const;

        BENTLEY_SM_EXPORT void SetLoadClips(bool loadClips);
        BENTLEY_SM_EXPORT void SetLoadTexture(bool loadTexture);
        BENTLEY_SM_EXPORT void SetLoadIndices(bool loadIndices);
        BENTLEY_SM_EXPORT void SetLoadGraph(bool loadGraph);
        BENTLEY_SM_EXPORT void SetSaveToCache(bool saveToCache);
        BENTLEY_SM_EXPORT void SetPrecomputeBoxes(bool precomputeBoxes);
        BENTLEY_SM_EXPORT void SetClipsToShow(bset<uint64_t>& clipsToShow, bool shouldInvertClips);

        BENTLEY_SM_EXPORT static IScalableMeshMeshFlagsPtr Create();

        BENTLEY_SM_EXPORT static IScalableMeshMeshFlagsPtr Create(bool shouldLoadTexture, bool shouldLoadGraph);
        BENTLEY_SM_EXPORT static IScalableMeshMeshFlagsPtr Create(bool shouldLoadTexture, bool shouldLoadGraph, bool shouldLoadClips);
    };


struct IScalableMeshNode : virtual public RefCountedBase

    {
    private:
    protected:
        virtual BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTMPtr  _GetBcDTM() const = 0;

        virtual bool    _ArePoints3d() const = 0;

        virtual bool    _ArePointsFullResolution() const = 0;

        virtual IScalableMeshMeshPtr _GetMesh(IScalableMeshMeshFlagsPtr& flags) const = 0;

        virtual IScalableMeshMeshPtr _GetMeshUnderClip(IScalableMeshMeshFlagsPtr& flags, uint64_t clip) const = 0;

        virtual IScalableMeshMeshPtr _GetMeshUnderClip2(IScalableMeshMeshFlagsPtr& flags, CLIP_VECTOR_NAMESPACE::ClipVectorPtr clips, uint64_t coverageID, bool isClipBoundary) const = 0;

        virtual IScalableMeshMeshPtr _GetMeshByParts(const bset<uint64_t>& clipsToShow) const = 0;

        virtual IScalableMeshTexturePtr _GetTexture() const = 0;    

        virtual IScalableMeshTexturePtr _GetTextureCompressed() const = 0;

        virtual bool                    _IsTextured() const = 0;        

        virtual void                    _GetResolutions(float& geometricResolution, float& textureResolution) const = 0;
        
        virtual bvector<IScalableMeshNodePtr> _GetNeighborAt(char relativePosX, char relativePosY, char relativePosZ) const = 0;

        virtual bvector<IScalableMeshNodePtr> _GetChildrenNodes() const = 0;

        virtual IScalableMeshNodePtr _GetParentNode() const = 0;
        
        virtual void     _RefreshMergedClip(Transform tr) const = 0;

        virtual bool     _AddClip(uint64_t id, bool isVisible) const = 0;

        virtual bool     _ModifyClip(uint64_t id, bool isVisible) const = 0;

        virtual bool     _DeleteClip(uint64_t id, bool isVisible) const = 0;
        
        virtual DRange3d _GetNodeExtent() const = 0;

        virtual DRange3d _GetContentExtent() const = 0;

        virtual int64_t _GetNodeId() const = 0;

        virtual size_t _GetLevel() const = 0;

        virtual size_t  _GetPointCount() const = 0;                

        virtual bool _IsHeaderLoaded() const = 0;
        
        virtual void _LoadHeader() const = 0;

        virtual bool _HasClip(uint64_t id) const = 0;

        virtual bool _IsClippingUpToDate() const = 0;

        virtual bool _IsDataUpToDate() const = 0;

        virtual void _UpdateData() = 0;

        virtual void _GetSkirtMeshes(bvector<PolyfaceHeaderPtr>& meshes, bset<uint64_t>& activeClips) const = 0;

        virtual bool _RunQuery(ISMPointIndexQuery<DPoint3d, DRange3d>& query, bvector<IScalableMeshNodePtr>& nodes) const = 0;

        virtual bool _RunQuery(ISMPointIndexQuery<DPoint3d, DRange3d>& query) const = 0;

		virtual void _ClearCachedData() = 0;      

        virtual SMNodeViewStatus _IsCorrectForView(IScalableMeshViewDependentMeshQueryParamsPtr& viewDependentQueryParams) const = 0;

        virtual IScalableMeshNodeEditPtr _EditNode() = 0;

#ifdef WIP_MESH_IMPORT
        virtual bool _IntersectRay(DPoint3d& pt, const DRay3d& ray, Json::Value& retrievedMetadata) = 0;

        virtual void _GetAllSubMeshes(bvector<IScalableMeshMeshPtr>& meshes, bvector<uint64_t>& texIDs) const= 0;

        virtual IScalableMeshTexturePtr _GetTexture(uint64_t texID) const= 0;
#endif

                
    public:
        static const BENTLEY_SM_EXPORT ScalableMeshTextureID UNTEXTURED_PART = 0;

        BENTLEY_SM_EXPORT BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTMPtr          GetBcDTM() const;
        
        BENTLEY_SM_EXPORT bool          ArePoints3d() const;

        BENTLEY_SM_EXPORT bool          ArePointsFullResolution() const;
        
        BENTLEY_SM_EXPORT IScalableMeshMeshPtr GetMesh(IScalableMeshMeshFlagsPtr& flags) const;

        BENTLEY_SM_EXPORT IScalableMeshMeshPtr GetMeshUnderClip(IScalableMeshMeshFlagsPtr& flags,uint64_t clip) const;

        BENTLEY_SM_EXPORT IScalableMeshMeshPtr GetMeshUnderClip2(IScalableMeshMeshFlagsPtr& flags, CLIP_VECTOR_NAMESPACE::ClipVectorPtr clips, uint64_t coverageID, bool isClipBoundary) const;

        BENTLEY_SM_EXPORT IScalableMeshMeshPtr GetMeshByParts(bset<uint64_t>& clipsToShow) const;

        BENTLEY_SM_EXPORT IScalableMeshTexturePtr GetTexture() const;   

        BENTLEY_SM_EXPORT IScalableMeshTexturePtr GetTextureCompressed() const;

        BENTLEY_SM_EXPORT bool                    IsTextured() const;           

        BENTLEY_SM_EXPORT void                    GetResolutions(float& geometricResolution, float& textureResolution) const;
                
        //Gets neighbors by relative position. For example, neighbor (-1, 0, 0) shares the node's left face. (1,1,0) shares the node's top-right diagonal. 
        BENTLEY_SM_EXPORT bvector<IScalableMeshNodePtr> GetNeighborAt(char relativePosX,  char relativePosY,  char relativePosZ) const;

        BENTLEY_SM_EXPORT bvector<IScalableMeshNodePtr> GetChildrenNodes() const;        

        BENTLEY_SM_EXPORT IScalableMeshNodePtr GetParentNode() const;

        // Deprecated, use _RefreshMergedClip instead
        //BENTLEY_SM_EXPORT void     ApplyAllExistingClips(Transform tr) const;

        BENTLEY_SM_EXPORT void     RefreshMergedClip(Transform tr) const;

        bool     AddClip(uint64_t id, bool isVisible=true) const;

        bool     AddClipAsync(uint64_t id, bool isVisible = true) const;

        bool     ModifyClip(uint64_t id,bool isVisible=true) const;

        bool     DeleteClip(uint64_t id, bool isVisible=true) const;

        BENTLEY_SM_EXPORT DRange3d GetNodeExtent() const;

        BENTLEY_SM_EXPORT DRange3d GetContentExtent() const;

        BENTLEY_SM_EXPORT int64_t GetNodeId() const;

        BENTLEY_SM_EXPORT size_t GetLevel() const;

        BENTLEY_SM_EXPORT size_t GetPointCount() const;        

        BENTLEY_SM_EXPORT bool IsHeaderLoaded() const;
        
        BENTLEY_SM_EXPORT void LoadNodeHeader() const;

        BENTLEY_SM_EXPORT bool HasClip(uint64_t id) const;

        BENTLEY_SM_EXPORT bool IsClippingUpToDate() const;

        BENTLEY_SM_EXPORT bool IsDataUpToDate() const;

        BENTLEY_SM_EXPORT void UpdateData();

        BENTLEY_SM_EXPORT void GetSkirtMeshes(bvector<PolyfaceHeaderPtr>& meshes, bset<uint64_t>& activeClips) const;

        BENTLEY_SM_EXPORT bool RunQuery(ISMPointIndexQuery<DPoint3d, DRange3d>& query, bvector<IScalableMeshNodePtr>& nodes) const;

        BENTLEY_SM_EXPORT bool RunQuery(ISMPointIndexQuery<DPoint3d, DRange3d>& query) const;

		BENTLEY_SM_EXPORT void ClearCachedData();

        BENTLEY_SM_EXPORT SMNodeViewStatus IsCorrectForView(IScalableMeshViewDependentMeshQueryParamsPtr& viewDependentQueryParams) const;

        BENTLEY_SM_EXPORT IScalableMeshNodeEditPtr EditNode();

#ifdef WIP_MESH_IMPORT
        BENTLEY_SM_EXPORT bool IntersectRay(DPoint3d& pt, const DRay3d& ray, Json::Value& retrievedMetadata);

        BENTLEY_SM_EXPORT void GetAllSubMeshes(bvector<IScalableMeshMeshPtr>& meshes, bvector<uint64_t>& texIDs) const;

        BENTLEY_SM_EXPORT IScalableMeshTexturePtr GetTexture(uint64_t texID) const;
#endif
    };

struct SmCachedDisplayMesh;
struct SmCachedDisplayTexture;

struct IScalableMeshCachedDisplayNode : public virtual IScalableMeshNode
    {
    protected:

        virtual StatusInt _GetCachedMeshes(bvector<SmCachedDisplayMesh*>& cachedMesh, bvector<bpair<bool, uint64_t>>& textureIds) const = 0;

        virtual StatusInt _GetCachedTextures(bvector<SmCachedDisplayTexture*>& cachedTexture, bvector<uint64_t>& textureIds) const = 0;

        virtual StatusInt _GetDisplayClipVectors(bvector<CLIP_VECTOR_NAMESPACE::ClipVectorPtr>& clipVectors) const = 0;

        virtual void      _SetIsInVideoMemory(bool isInVideoMemory) = 0;

        virtual bool      _GetContours(bvector<bvector<DPoint3d>>& contours) = 0;

    public : 


        BENTLEY_SM_EXPORT StatusInt GetCachedMeshes(bvector<SmCachedDisplayMesh*>& cachedMesh, bvector<bpair<bool, uint64_t>>& textureIds) const;

        BENTLEY_SM_EXPORT StatusInt GetCachedTextures(bvector<SmCachedDisplayTexture*>& cachedTexture, bvector<uint64_t>& textureIds) const;

        BENTLEY_SM_EXPORT StatusInt GetDisplayClipVectors(bvector<CLIP_VECTOR_NAMESPACE::ClipVectorPtr>& clipVectors) const;

        BENTLEY_SM_EXPORT void      SetIsInVideoMemory(bool isInVideoMemory);

        BENTLEY_SM_EXPORT bool      GetContours(bvector<bvector<DPoint3d>>& contours);

        BENTLEY_SM_EXPORT static IScalableMeshCachedDisplayNodePtr Create(uint64_t nodeId, IScalableMesh* smP);
    };


struct IScalableMeshNodeEdit : public virtual IScalableMeshNode
    {
    protected:
        virtual StatusInt _AddMesh(DPoint3d* vertices, size_t nVertices, int32_t* indices, size_t nIndices) = 0;
        virtual StatusInt _AddTexturedMesh(bvector<DPoint3d>& vertices, bvector<bvector<int32_t>>& ptsIndices, bvector<DPoint2d>& uv, bvector<bvector<int32_t>>& uvIndices, size_t nTexture, int64_t texID) = 0;
        virtual StatusInt _AddTexturedMesh(bvector<DPoint3d>& vertices, bvector<int32_t>& ptsIndices, bvector<DPoint2d>& uv, bvector<int32_t>& uvIndices, size_t nTexture, int64_t texID) = 0;
        virtual StatusInt _SetNodeExtent(DRange3d& extent) = 0;
        virtual StatusInt _SetContentExtent(DRange3d& extent) = 0;
        virtual StatusInt _SetArePoints3d(bool arePoints3d) = 0;
        virtual StatusInt _AddTextures(bvector<Byte>& data) = 0;
        virtual StatusInt _SetResolution(float geometricResolution, float textureResolution) = 0;

        virtual bvector<IScalableMeshNodeEditPtr> _EditChildrenNodes() = 0;
        virtual IScalableMeshNodeEditPtr _EditParentNode() = 0;
        virtual void   _ReplaceIndices(const bvector<size_t>& posToChange, const bvector<DPoint3d>& newCoordinates) = 0;

    public:
        BENTLEY_SM_EXPORT StatusInt AddMesh(DPoint3d* vertices, size_t nVertices, int32_t* indices, size_t nIndices);
        BENTLEY_SM_EXPORT StatusInt AddTexturedMesh(bvector<DPoint3d>& vertices, bvector<bvector<int32_t>>& ptsIndices, bvector<DPoint2d>& uv, bvector<bvector<int32_t>>& uvIndices, size_t nTexture, int64_t texID = -1);
        BENTLEY_SM_EXPORT StatusInt AddTexturedMesh(bvector<DPoint3d>& vertices, bvector<int32_t>& ptsIndices, bvector<DPoint2d>& uv, bvector<int32_t>& uvIndices, size_t nTexture, int64_t texID = -1);
        BENTLEY_SM_EXPORT StatusInt AddTextures(bvector<Byte>& data);
        BENTLEY_SM_EXPORT StatusInt SetNodeExtent(DRange3d& extent);
        BENTLEY_SM_EXPORT StatusInt SetContentExtent(DRange3d& extent);
        BENTLEY_SM_EXPORT StatusInt SetArePoints3d(bool arePoints3d);

        BENTLEY_SM_EXPORT StatusInt SetResolution(float geometricResolution, float textureResolution);

        BENTLEY_SM_EXPORT bvector<IScalableMeshNodeEditPtr> EditChildrenNodes();
        BENTLEY_SM_EXPORT IScalableMeshNodeEditPtr EditParentNode();

        void                        ReplaceIndices(const bvector<size_t>& posToChange, const bvector<DPoint3d>& newCoordinates);
    };


struct IScalableMeshMeshQueryParams  : virtual public RefCountedBase
    {
    protected:
        IScalableMeshMeshQueryParams();
        virtual ~IScalableMeshMeshQueryParams();

        virtual BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr _GetSourceGCS() = 0;

        virtual BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr _GetTargetGCS() = 0;

        virtual size_t _GetLevel() = 0;

        virtual bool  _GetUseAllResolutions() = 0;

        virtual double  _GetTargetPixelTolerance() = 0;

        virtual void _SetGCS(BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& sourceGCSPtr,
                             BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCSPtr) = 0;

        virtual void _SetLevel(size_t depth) = 0;

        virtual void _SetUseAllResolutions(bool useAllResolutions) = 0;

        virtual void _SetTargetPixelTolerance(double pixelTol) = 0;


        virtual bool _GetReturnNodesWithNoMesh() = 0;

        virtual void _SetReturnNodesWithNoMesh(bool returnEmptyNodes) = 0;

    public:
        BENTLEY_SM_EXPORT static IScalableMeshMeshQueryParamsPtr CreateParams();

        BENTLEY_SM_EXPORT BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr GetSourceGCS();
        BENTLEY_SM_EXPORT BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr GetTargetGCS();

        BENTLEY_SM_EXPORT size_t GetLevel();

        BENTLEY_SM_EXPORT bool GetUseAllResolutions();

        BENTLEY_SM_EXPORT double GetTargetPixelTolerance();

        BENTLEY_SM_EXPORT void SetGCS(BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& sourceGCSPtr,
                                      BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCSPtr);

        BENTLEY_SM_EXPORT void SetLevel(size_t depth);

        BENTLEY_SM_EXPORT void SetUseAllResolutions(bool useAllResolutions);

        BENTLEY_SM_EXPORT void SetTargetPixelTolerance(double pixelTol);

        BENTLEY_SM_EXPORT bool GetReturnNodesWithNoMesh();

        BENTLEY_SM_EXPORT void SetReturnNodesWithNoMesh(bool returnEmptyNodes);
    };


typedef bool (*StopQueryCallbackFP)();

struct IScalableMeshViewDependentMeshQueryParams : virtual public IScalableMeshMeshQueryParams
    {
    protected :
     
        IScalableMeshViewDependentMeshQueryParams();
        virtual ~IScalableMeshViewDependentMeshQueryParams();

        virtual const DPoint3d*     _GetViewBox() const = 0;                    
                            
        virtual void                _SetViewBox(const DPoint3d viewBox[]) = 0;         

        virtual double              _GetMinScreenPixelsPerPoint() const = 0;        

        virtual double              _GetMaxPixelError() const = 0;

        virtual StopQueryCallbackFP _GetStopQueryCallback() const = 0;
               
        virtual const double*       _GetRootToViewMatrix() const = 0;

        virtual const CLIP_VECTOR_NAMESPACE::ClipVectorPtr _GetViewClipVector() const = 0;

        virtual bool                _IsProgressiveDisplay() const = 0;

        virtual void            _SetMinScreenPixelsPerPoint(double minScreenPixelsPerPoint) = 0;

        virtual void            _SetMaxPixelError(double errorInPixels) = 0;

        virtual void            _SetProgressiveDisplay(bool isProgressiveDisplay) = 0;

        virtual void            _SetRootToViewMatrix(const double rootToViewMatrix[][4]) = 0;    

        virtual StatusInt       _SetStopQueryCallback(StopQueryCallbackFP stopQueryCallbackFP) = 0;
        
        virtual void            _SetViewClipVector(CLIP_VECTOR_NAMESPACE::ClipVectorPtr& viewClipVector) = 0;

        virtual bool            _ShouldLoadContours() const = 0;

        virtual double          _GetMajorContourInterval() const = 0;

        virtual double          _GetMinorContourInterval() const = 0;

        virtual void            _SetLoadContours(bool loadContours) = 0;

        virtual void            _SetContourInterval(double major, double minor) = 0;
                                                  
    public : 

        
        const DPoint3d*     GetViewBox() const;
                                        
        double              GetMinScreenPixelsPerPoint() const;

        double              GetMaxPixelError() const;

        StopQueryCallbackFP GetStopQueryCallback() const;        

        const double*       GetRootToViewMatrix() const;

        const CLIP_VECTOR_NAMESPACE::ClipVectorPtr GetViewClipVector() const;

        bool                IsProgressiveDisplay() const;

        bool                ShouldLoadContours() const;

        double              GetMajorContourInterval() const;

        double              GetMinorContourInterval() const;
    
        
        BENTLEY_SM_EXPORT void      SetViewBox(const DPoint3d viewBox[]);        

        BENTLEY_SM_EXPORT void      SetMinScreenPixelsPerPoint(double minScreenPixelsPerPoint);

        BENTLEY_SM_EXPORT void      SetMaxPixelError(double errorInPixels);

        BENTLEY_SM_EXPORT void      SetProgressiveDisplay(bool isProgressiveDisplay);

        BENTLEY_SM_EXPORT void      SetRootToViewMatrix(const double rootToViewMatrix[][4]);    

        BENTLEY_SM_EXPORT StatusInt SetStopQueryCallback(StopQueryCallbackFP stopQueryCallbackFP);

        BENTLEY_SM_EXPORT void      SetViewClipVector(CLIP_VECTOR_NAMESPACE::ClipVectorPtr& viewClipVector);
        
        BENTLEY_SM_EXPORT static    IScalableMeshViewDependentMeshQueryParamsPtr CreateParams();

        BENTLEY_SM_EXPORT void      SetLoadContours(bool loadContours);

        BENTLEY_SM_EXPORT void      SetContourInterval(double major, double minor);
    };


struct IScalableMeshMeshQuery : RefCountedBase
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

        virtual int _Query(bvector<IScalableMeshNodePtr>&                       meshNodesPtr,
			CLIP_VECTOR_NAMESPACE::ClipVectorCP                      queryExtent3d,
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


         BENTLEY_SM_EXPORT int Query(bvector<IScalableMeshNodePtr>&                      meshNodesPtr,
			 CLIP_VECTOR_NAMESPACE::ClipVectorCP                                        queryExtent3d,
                                     const IScalableMeshMeshQueryParamsPtr& scmQueryParamsPtr) const;
    };


struct IScalableMeshNodeQueryParams : RefCountedBase
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

struct IScalableMeshNodeRayQuery  : RefCountedBase
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


struct IScalableMeshNodePlaneQueryParams  : IScalableMeshMeshQueryParams
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
