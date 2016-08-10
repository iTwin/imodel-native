#include "ScalableMeshSchemaPCH.h"
#include <ScalableMeshSchema\ScalableMeshHandler.h>
#include "ScalableMeshDisplayCacheManager.h"


USING_NAMESPACE_BENTLEY_SCALABLEMESH_SCHEMA


//Inherited from IScalableMeshDisplayCacheManager
BentleyStatus ScalableMeshDisplayCacheManager::_CreateCachedMesh(SmCachedDisplayMesh*&   cachedDisplayMesh,
size_t                  nbVertices,
DPoint3d const*         positionOrigin,
float*                  positions,
float*                  normals,
int                     nbTriangles,
int*                    indices,
float*                  params,
SmCachedDisplayTexture* cachedTexture)
    {
    QvTextureID textureId = 0;

    if (cachedTexture != 0)
        {
        textureId = cachedTexture->m_textureID;
        }

    std::unique_ptr<SmCachedDisplayMesh> qvCachedDisplayMesh(new SmCachedDisplayMesh);
    qvCachedDisplayMesh->m_qvElem = qv_beginElement(m_qvCache, 0, NULL);
    if(nbTriangles > 0) qv_addQuickTriMesh(qvCachedDisplayMesh->m_qvElem, 3 * nbTriangles, indices, (int)nbVertices, positionOrigin, reinterpret_cast <FloatXYZ*> (positions),
                       reinterpret_cast <FloatXYZ*> (normals),
                       reinterpret_cast <FloatXY*> (params), textureId,/*QV_QTMESH_GENNORMALS*/0);
    qv_endElement(qvCachedDisplayMesh->m_qvElem);

    cachedDisplayMesh = qvCachedDisplayMesh.release();

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
                                                                    int                      xSize,
                                                                    int                      ySize,
                                                                    int                      enableAlpha,
                                                                    int                      format,      // => see QV_*_FORMAT definitions above
                                                                    unsigned char const *    texels)      // => texel image)
    {
    std::unique_ptr<SmCachedDisplayTexture> qvCachedDisplayTexture(new SmCachedDisplayTexture);

    qv_defineTexture(qvCachedDisplayTexture->m_textureID, NULL, (int)xSize, (int)ySize, enableAlpha, format, texels);

    cachedDisplayTexture = qvCachedDisplayTexture.release();

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

ScalableMeshDisplayCacheManager::ScalableMeshDisplayCacheManager(DgnDbCR dgbDb)
    {
    m_qvCache = dgbDb.Models().GetQvCache();
    }

ScalableMeshDisplayCacheManager::~ScalableMeshDisplayCacheManager()
    {

    }

