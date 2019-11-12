/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include "IScalableMesh.h"
#include "IScalableMeshQuery.h"
#include <Bentley/bset.h>


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct IScalableMeshDisplayCacheManager;
struct IScalableMeshProgressiveQueryEngine;

typedef RefCountedPtr<IScalableMeshDisplayCacheManager>    IScalableMeshDisplayCacheManagerPtr;
typedef RefCountedPtr<IScalableMeshProgressiveQueryEngine> IScalableMeshProgressiveQueryEnginePtr;

struct SmCachedDisplayMesh;
struct SmCachedDisplayTexture;

struct IScalableMeshDisplayCacheManager :  RefCountedBase
    {
    public:                                         
        
        virtual BentleyStatus _CreateCachedMesh(SmCachedDisplayMesh*&   cachedDisplayMesh,
                                                size_t&                 usedMemInBytes,
                                                bool&                   isStoredOnGpu,
                                                size_t                  nbVertices,
                                                DPoint3d const*         positionOrigin,
                                                float*                  positions,
                                                float*                  normals,
                                                int                     nbTriangles,
                                                int*                    indices,
                                                float*                  params,
                                                SmCachedDisplayTexture* cachedTexture,
                                                uint64_t nodeId,
                                                uint64_t smId) = 0;
        
        virtual BentleyStatus _DestroyCachedMesh(SmCachedDisplayMesh* cachedDisplayMesh) = 0; 

        virtual BentleyStatus _CreateCachedTexture(SmCachedDisplayTexture*& cachedDisplayTexture, 
                                                   size_t&                  usedMemInBytes,
                                                   bool&                    isStoredOnGpu,
                                                   int                      xSize,                       
                                                   int                      ySize,                       
                                                   int                      enableAlpha,                 
                                                   int                      format,      // => see QV_*_FORMAT definitions above
                                                   unsigned char const *    texels) = 0; // => texel image)        

        virtual BentleyStatus _DestroyCachedTexture(SmCachedDisplayTexture* cachedDisplayTexture) = 0; 

        virtual BentleyStatus _DeleteFromVideoMemory(SmCachedDisplayMesh* cachedDisplayMesh) = 0;

        virtual BentleyStatus _DeleteFromVideoMemory(SmCachedDisplayTexture* cachedDisplayTex) = 0;

        virtual bool _IsUsingVideoMemory() = 0;        

        virtual bool _HasCompatibleSettings(SmCachedDisplayMesh* cachedDisplayMesh) = 0;

		//called when a resource was deleted but dependent resources (which may or may not be in use) need to be regenerated
		//virtual void _SetCacheDirty(bool isDirty) = 0;
    };


struct IScalableMeshProgressiveQueryEngine: RefCountedBase
    {
    /*__PUBLISH_SECTION_END__*/
    /*__PUBLISH_CLASS_VIRTUAL__*/

    private:  

    protected:                                                

        virtual BentleyStatus _StartQuery(int                                                                      queryId, 
                                          IScalableMeshViewDependentMeshQueryParamsPtr                             queryParam, 
                                          const bvector<BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshCachedDisplayNodePtr>& startingNodes, 
                                          bool                                                                     loadTexture, 
                                          const bvector<bool>&                                                     clipVisibilities,
                                          IScalableMeshPtr&                                                        smPtr) = 0;

        virtual BentleyStatus _GetOverviewNodes(bvector<BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshCachedDisplayNodePtr>& meshNodes, 
                                                int                                                                queryId) const = 0;

        virtual BentleyStatus _GetRequiredNodes(bvector<BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshCachedDisplayNodePtr>& meshNodes, 
                                               int                                                                queryId) const = 0;


        virtual BentleyStatus _GetRequiredTextureTiles(bvector<BENTLEY_NAMESPACE_NAME::ScalableMesh::SMRasterTile>& rasterTiles,
                                                       int                                                          queryId) const = 0;
        
        virtual BentleyStatus _StopQuery(int queryId) = 0; 

        virtual void          _SetActiveClips(const bset<uint64_t>& activeClips, const IScalableMeshPtr& scalableMeshPtr) = 0;

        virtual void          _GetActiveClips(bset<uint64_t>& activeClips, const IScalableMeshPtr& scalableMeshPtr) = 0;

        virtual bool          _IsQueryComplete(int queryId) = 0; 

        virtual void _ClearOverviews(IScalableMesh* scalableMeshP) = 0;

        virtual void _InitScalableMesh(IScalableMeshPtr& scalableMeshPtr) = 0;
        
    /*__PUBLISH_SECTION_START__*/
    public:        
                                
        BENTLEY_SM_EXPORT BentleyStatus StartQuery(int                                                                      queryId, 
                                                   IScalableMeshViewDependentMeshQueryParamsPtr                             queryParam, 
                                                   const bvector<BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshCachedDisplayNodePtr>& startingNodes, 
                                                   bool                                                                     loadTexture, 
                                                   const bvector<bool>&                                                     clipVisibilities,
                                                   IScalableMeshPtr&                                                        smPtr);

        BENTLEY_SM_EXPORT BentleyStatus GetOverviewNodes(bvector<BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshCachedDisplayNodePtr>& meshNodes, 
                                                         int                                                   queryId);

        BENTLEY_SM_EXPORT BentleyStatus GetRequiredNodes(bvector<BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshCachedDisplayNodePtr>& meshNodes, 
                                                        int                                                   queryId);

        BENTLEY_SM_EXPORT BentleyStatus GetRequiredTextureTiles(bvector<BENTLEY_NAMESPACE_NAME::ScalableMesh::SMRasterTile>& rasterTiles,
                                                                int                                                   queryId);        

        BENTLEY_SM_EXPORT BentleyStatus StopQuery(int queryId); 

        BENTLEY_SM_EXPORT bool IsQueryComplete(int queryId); 

        BENTLEY_SM_EXPORT void GetActiveClips(bset<uint64_t>& activeClips, const IScalableMeshPtr& scalableMeshPtr);

        BENTLEY_SM_EXPORT void SetActiveClips(const bset<uint64_t>& activeClips, const IScalableMeshPtr& scalableMeshPtr);

        BENTLEY_SM_EXPORT static IScalableMeshProgressiveQueryEnginePtr Create(IScalableMeshPtr& scalableMeshPtr, IScalableMeshDisplayCacheManagerPtr& displayCacheManagerPtr, bool loadTexture = true);

        BENTLEY_SM_EXPORT void InitScalableMesh(IScalableMeshPtr& scalableMeshPtr);

        BENTLEY_SM_EXPORT void ClearOverviews(IScalableMesh* scalableMeshP);

        BENTLEY_SM_EXPORT static void CancelAllQueries();
    };


void BENTLEY_SM_EXPORT InitializeProgressiveQueries();
void BENTLEY_SM_EXPORT ClearProgressiveQueriesInfo();

void TerminateProgressiveQueries();

END_BENTLEY_SCALABLEMESH_NAMESPACE
