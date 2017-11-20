/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/GTests/SMUnitTestDisplayQuery.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "SMUnitTestDisplayQuery.h"


struct SmCachedDisplayTexture
    {
    SmCachedDisplayTexture()
        {    
        }    
    };

struct SmCachedDisplayMesh
    {    
    };

struct ScalableMeshDisplayCacheManager : public IScalableMeshDisplayCacheManager
    {   
private:
    
public:

    //Inherited from IScalableMeshDisplayCacheManager
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
        uint64_t                nodeId,
        uint64_t                smId) override;

    virtual BentleyStatus _DestroyCachedMesh(SmCachedDisplayMesh* cachedDisplayMesh) override;

    virtual BentleyStatus _CreateCachedTexture(SmCachedDisplayTexture*& cachedDisplayTexture,
        size_t&                  usedMemInBytes,
        bool&                    isStoredOnGpu,
        int                      xSize,
        int                      ySize,
        int                      enableAlpha,
        int                      format,      // => see QV_*_FORMAT definitions above
        unsigned char const *    texels) override; // => texel image)

    virtual BentleyStatus _DestroyCachedTexture(SmCachedDisplayTexture* cachedDisplayTexture) override;

    virtual BentleyStatus _DeleteFromVideoMemory(SmCachedDisplayMesh* cachedDisplayMesh) override;

    virtual BentleyStatus _DeleteFromVideoMemory(SmCachedDisplayTexture* cachedDisplayTex) override;

    virtual bool _IsUsingVideoMemory() override;

    ScalableMeshDisplayCacheManager(ViewContextR viewContext);

    ~ScalableMeshDisplayCacheManager();

};





//Inherited from IScalableMeshDisplayCacheManager
BentleyStatus ScalableMeshDisplayCacheManager::_CreateCachedMesh(SmCachedDisplayMesh*&   cachedDisplayMesh,
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
                                                                 uint64_t                nodeId,
                                                                 uint64_t                smId)
    {



#if 0
    QvTextureID textureId = 0;

    if (cachedTexture != 0)
    {
        textureId = cachedTexture->m_textureID;
    }

    std::unique_ptr<SmCachedDisplayMesh> qvCachedDisplayMesh(new SmCachedDisplayMesh);

    qvCachedDisplayMesh->m_qvElem = qv_beginElement(m_qvCache, 0, NULL);

    if (nbTriangles > 0) qv_addQuickTriMesh(qvCachedDisplayMesh->m_qvElem, 3 * nbTriangles, indices, (int)nbVertices, positionOrigin, reinterpret_cast <FloatXYZ*> (positions),
        reinterpret_cast <FloatXYZ*> (normals),
        reinterpret_cast <FloatXY*> (params), textureId, QV_QTMESH_GENNORMALS);
    qv_endElement(qvCachedDisplayMesh->m_qvElem);

    if (nbTriangles == 0) qvCachedDisplayMesh->m_qvElem = 0;

    cachedDisplayMesh = qvCachedDisplayMesh.release();

    isStoredOnGpu = false;
    usedMemInBytes = nbVertices * sizeof(float) * 3 + nbTriangles * 3 * sizeof(int32_t) + sizeof(float) * 2 * nbVertices;
#endif

    return SUCCESS;
}

BentleyStatus ScalableMeshDisplayCacheManager::_DestroyCachedMesh(SmCachedDisplayMesh* cachedDisplayMesh)
{
    // shutting down
    if (nullptr == DgnPlatformLib::QueryHost())
        return SUCCESS;

    if (NULL != cachedDisplayMesh->m_qvElem)
    {
        T_HOST.GetGraphicsAdmin()._DeleteQvElem(cachedDisplayMesh->m_qvElem);
        cachedDisplayMesh->m_qvElem = 0;
    }

    delete cachedDisplayMesh;


    return SUCCESS;
}

BentleyStatus ScalableMeshDisplayCacheManager::_CreateCachedTexture(SmCachedDisplayTexture*& cachedDisplayTexture,
    size_t&                  usedMemInBytes,
    bool&                    isStoredOnGpu,
    int                      xSize,
    int                      ySize,
    int                      enableAlpha,
    int                      format,      // => see QV_*_FORMAT definitions above
    unsigned char const *    texels)      // => texel image)
{
    std::unique_ptr<SmCachedDisplayTexture> qvCachedDisplayTexture(new SmCachedDisplayTexture);

    qv_defineTileSection(qvCachedDisplayTexture->m_textureID, nullptr, 0, xSize, ySize, enableAlpha, format, texels);

    //qv_defineTexture(qvCachedDisplayTexture->m_textureID, NULL, (int)xSize, (int)ySize, enableAlpha, format, texels);

    cachedDisplayTexture = qvCachedDisplayTexture.release();

    isStoredOnGpu = false;
    usedMemInBytes = xSize * ySize * 6;

    return SUCCESS;
}

BentleyStatus ScalableMeshDisplayCacheManager::_DestroyCachedTexture(SmCachedDisplayTexture* cachedDisplayTexture)
{
    if (cachedDisplayTexture->m_textureID != 0)
    {
        qv_deleteTexture(cachedDisplayTexture->m_textureID);
        cachedDisplayTexture->m_textureID = 0;
    }

    delete cachedDisplayTexture;

    return SUCCESS;
}


BentleyStatus ScalableMeshDisplayCacheManager::_DeleteFromVideoMemory(SmCachedDisplayMesh* cachedDisplayMesh)
{
    return SUCCESS;
}

BentleyStatus ScalableMeshDisplayCacheManager::_DeleteFromVideoMemory(SmCachedDisplayTexture* cachedDisplayTex)
{
    return SUCCESS;
}

bool ScalableMeshDisplayCacheManager::_IsUsingVideoMemory()
{
    return false;
}

ScalableMeshDisplayCacheManager::ScalableMeshDisplayCacheManager(ViewContextR viewContext)
{
    m_qvCache = T_HOST.GetGraphicsAdmin()._CreateQvCache();
}

ScalableMeshDisplayCacheManager::~ScalableMeshDisplayCacheManager()
{

}




/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.St-Pierre                 11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayQueryTester::DisplayQueryTester()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.St-Pierre                 11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayQueryTester::~DisplayQueryTester()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.St-Pierre                 11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayQueryTester::DoQuery()
    {
    }

