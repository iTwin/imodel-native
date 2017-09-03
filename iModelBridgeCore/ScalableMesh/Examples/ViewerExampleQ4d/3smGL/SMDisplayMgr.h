#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>  // for MS Windows
#include <GL/gl.h>  // GLUT, include glu.h and gl.h
#include <GL/glu.h>  // GLUT, include glu.h and gl.h

#include <DgnPlatform/DgnPlatform.h>

#define VANCOUVER_API
//#include <ScalableMesh/ScalableMeshDefs.h>
#include <ScalableMesh/IScalableMesh.h>
#include <ScalableMesh/IScalableMeshNodeCreator.h>
#include <ScalableMesh/IScalableMeshQuery.h>
#include <ScalableMesh/ScalableMeshLib.h>

#include <ScalableMesh/IScalableMeshProgressiveQuery.h>

//USING_NAMESPACE_BENTLEY_SCALABLEMESH
//USING_NAMESPACE_BENTLEY_DGNPLATFORM

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

extern bool s_dontkeepIntermediateDisplayData;
extern bool s_useVBO;

struct SmCachedDisplayTexture
    {
    //This serves as the reference Scalable Mesh will hold to your texture data for this tile
    //Allows to store IDs so that already loaded textures can be reused between queries.
    int m_textureId;
    int width;
    int height;
    unsigned char * texels;

    SmCachedDisplayTexture() { m_textureId = -1;  width = 0; height = 0; texels = nullptr;  };
    };

struct SmCachedDisplayMesh
    {
    //This serves as the reference Scalable Mesh will hold to your display data for this tile. 
    //Allows to store IDs so that already loaded meshes can be reused between queries.
    uint64_t                meshId;
    size_t                  nbPoints;
    DPoint3d *              points;
    int                     nbTriangles;         // => size of index array (numTri * 3)
    int*                    indices;
    float*                  positions;
    float*                  uvs;
    SmCachedDisplayTexture* cachedTexture;
    
    // display list info - filled at first drawing
    int                     displayIndex;
    DPoint3d                localCenter;
	uint64_t nodeId;
	int                     displayIndexIBO;

    SmCachedDisplayMesh() { cachedTexture = nullptr; displayIndex = -1; }
    };

struct SMDisplayMgr : public IScalableMeshDisplayCacheManager
    {
    friend struct DisplayCacheHandler;
    public:
        //Called when a new tile is available
        //Inherited from IScalableMeshDisplayCacheManager
        virtual BentleyStatus _CreateCachedMesh(SmCachedDisplayMesh*&   cachedDisplayMesh,
            size_t                  nbVertices,
            DPoint3d const*         positionOrigin,     //vertex coords are given relative to this origin
            float*                  positions,
            float*                  normals,
            int                     nbTriangles,
            int*                    indices,
            float*                  params,             //UVs, or 0 if untextured
            SmCachedDisplayTexture* cachedTexture,
            uint64_t nodeId,
            uint64_t smId) override;

        //Called when node is unloaded or data becomes invalidated
        virtual BentleyStatus _DestroyCachedMesh(SmCachedDisplayMesh* cachedDisplayMesh) override;

        virtual BentleyStatus _CreateCachedTexture(SmCachedDisplayTexture*& cachedDisplayTexture,
            int                      xSize,
            int                      ySize,
            int                      enableAlpha,
            int                      format,
            unsigned char const *    texels) override; // => texel image)

        virtual BentleyStatus _DestroyCachedTexture(SmCachedDisplayTexture* cachedDisplayTexture) override;

        virtual BentleyStatus _DeleteFromVideoMemory(SmCachedDisplayMesh* cachedDisplayMesh) override { return SUCCESS; }

        virtual BentleyStatus _DeleteFromVideoMemory(SmCachedDisplayTexture* cachedDisplayTex) override { return SUCCESS; }

        virtual bool _IsUsingVideoMemory() { return false; }

        SMDisplayMgr();
        ~SMDisplayMgr();

    public:

        DPoint3d m_Center; // keep the 3SM center for large coordinate handling

    };

typedef RefCountedPtr<SMDisplayMgr> SMDisplayMgrPtr;

struct DisplayCacheHandler
    {
    public:
        SMDisplayMgrPtr m_DisplayMgrPtr;

        void Init()
            {
            m_DisplayMgrPtr = new SMDisplayMgr();
            }

    };

END_BENTLEY_SCALABLEMESH_NAMESPACE