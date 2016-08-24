/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/IScalableMeshProgressiveQuery.h $
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

#include "IScalableMesh.h"
#include "IScalableMeshQuery.h"
#include <Bentley\bset.h>


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct IScalableMeshDisplayCacheManager;
struct IScalableMeshProgressiveQueryEngine;

typedef RefCountedPtr<IScalableMeshDisplayCacheManager>    IScalableMeshDisplayCacheManagerPtr;
typedef RefCountedPtr<IScalableMeshProgressiveQueryEngine> IScalableMeshProgressiveQueryEnginePtr;

struct SmCachedDisplayMesh;
struct SmCachedDisplayTexture;

struct IScalableMeshDisplayCacheManager abstract: RefCountedBase
    {
    public:                                         
        
        virtual BentleyStatus _CreateCachedMesh(SmCachedDisplayMesh*&   cachedDisplayMesh, 
                                                size_t                  nbVertices,
                                                DPoint3d const*         positionOrigin,
                                                float*                  positions,
                                                float*                  normals,
                                                int                     nbTriangles,
                                                int*                    indices,
                                                float*                  params,                                                
                                                SmCachedDisplayTexture* cachedTexture) = 0; 

        virtual BentleyStatus _DestroyCachedMesh(SmCachedDisplayMesh* cachedDisplayMesh) = 0; 

        virtual BentleyStatus _CreateCachedTexture(SmCachedDisplayTexture*& cachedDisplayTexture, 
                                                   int                      xSize,                       
                                                   int                      ySize,                       
                                                   int                      enableAlpha,                 
                                                   int                      format,      // => see QV_*_FORMAT definitions above
                                                   unsigned char const *    texels) = 0; // => texel image)

        virtual BentleyStatus _DestroyCachedTexture(SmCachedDisplayTexture* cachedDisplayTexture) = 0;       
    };


struct IScalableMeshProgressiveQueryEngine abstract: RefCountedBase
    {
    /*__PUBLISH_SECTION_END__*/
    /*__PUBLISH_CLASS_VIRTUAL__*/

    private:  

    protected:                                        

        virtual BentleyStatus _ClearCaching(const bvector<DRange2d>* clearRanges, const IScalableMeshPtr& scalableMeshPtr) = 0;

        virtual BentleyStatus _ClearCaching(const bvector<uint64_t>& clipIds, const IScalableMeshPtr& scalableMeshPtr) = 0;

        virtual BentleyStatus _StartQuery(int                                                                      queryId, 
                                          IScalableMeshViewDependentMeshQueryParamsPtr                             queryParam, 
                                          const bvector<BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshCachedDisplayNodePtr>& startingNodes, 
                                          bool                                                                     loadTexture, 
                                          const bvector<bool>&                                                     clipVisibilities,
                                          const DMatrix4d*                                                         prevLocalToView, //NEEDS_WORK_SM : prev and new local to view not used anymore.
                                          const DMatrix4d*                                                         newLocalToView) = 0; 

        virtual BentleyStatus _GetOverviewNodes(bvector<BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshCachedDisplayNodePtr>& meshNodes, 
                                                int                                                                queryId) const = 0;

        virtual BentleyStatus _GetRequiredNodes(bvector<BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshCachedDisplayNodePtr>& meshNodes, 
                                               int                                                                queryId) const = 0;

        virtual BentleyStatus _StopQuery(int queryId) = 0; 

        virtual void          _SetActiveClips(const bset<uint64_t>& activeClips, const IScalableMeshPtr& scalableMeshPtr) = 0;

        virtual bool          _IsQueryComplete(int queryId) = 0; 
        
    /*__PUBLISH_SECTION_START__*/
    public:

        BENTLEY_SM_EXPORT BentleyStatus ClearCaching(const bvector<DRange2d>* clearRanges, const IScalableMeshPtr& scalableMeshPtr);

        BENTLEY_SM_EXPORT BentleyStatus ClearCaching(const bvector<uint64_t>& clipIds, const IScalableMeshPtr& scalableMeshPtr);
                                
        BENTLEY_SM_EXPORT BentleyStatus StartQuery(int                                                                      queryId, 
                                                   IScalableMeshViewDependentMeshQueryParamsPtr                             queryParam, 
                                                   const bvector<BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshCachedDisplayNodePtr>& startingNodes, 
                                                   bool                                                                     loadTexture, 
                                                   const bvector<bool>&                                                     clipVisibilities,
                                                   const DMatrix4d*                                                         prevLocalToView,
                                                   const DMatrix4d*                                                         newLocalToView); 

        BENTLEY_SM_EXPORT BentleyStatus GetOverviewNodes(bvector<BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshCachedDisplayNodePtr>& meshNodes, 
                                                         int                                                   queryId);

        BENTLEY_SM_EXPORT BentleyStatus GetRequiredNodes(bvector<BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshCachedDisplayNodePtr>& meshNodes, 
                                                        int                                                   queryId);

        BENTLEY_SM_EXPORT BentleyStatus StopQuery(int queryId); 

        BENTLEY_SM_EXPORT bool IsQueryComplete(int queryId); 

        BENTLEY_SM_EXPORT void SetActiveClips(const bset<uint64_t>& activeClips, const IScalableMeshPtr& scalableMeshPtr);

        BENTLEY_SM_EXPORT static IScalableMeshProgressiveQueryEnginePtr Create(IScalableMeshPtr& scalableMeshPtr, IScalableMeshDisplayCacheManagerPtr& displayCacheManagerPtr);
    };


void BENTLEY_SM_EXPORT InitializeProgressiveQueries();
void BENTLEY_SM_EXPORT ClearProgressiveQueriesInfo();

void TerminateProgressiveQueries();

END_BENTLEY_SCALABLEMESH_NAMESPACE
