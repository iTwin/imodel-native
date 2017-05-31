#include "ScalableMeshSchemaPCH.h"
#include <ScalableMeshSchema\ScalableMeshHandler.h>
#include "ScalableMeshDisplayCacheManager.h"
#include <mutex>

USING_NAMESPACE_BENTLEY_SCALABLEMESH_SCHEMA

/*struct QvElData
{
    QvElem* handle;
    size_t nVerts;
    size_t nIndices;
};
bmap<uint64_t, bvector<QvElData>> elemsCreatedForNode;
bmap< QvElem*, bool> state;*/
std::mutex elemMutex;

//Inherited from IScalableMeshDisplayCacheManager
BentleyStatus ScalableMeshDisplayCacheManager::_CreateCachedMesh(SmCachedDisplayMesh*&   cachedDisplayMesh,
                                                                    size_t                  nbVertices,
                                                                    DPoint3d const*         positionOrigin,
                                                                    float*                  positions,
                                                                    float*                  normals,
                                                                    int                     nbTriangles,
                                                                    int*                    indices,
                                                                    float*                  params,
                                                                    SmCachedDisplayTexture* cachedTexture,
                                                                    uint64_t nodeId,
                                                                    uint64_t smId)
    {
    QvTextureID textureId = 0;

    if (cachedTexture != 0)
        {
        textureId = cachedTexture->m_textureID;
        }

    std::unique_ptr<SmCachedDisplayMesh> qvCachedDisplayMesh(new SmCachedDisplayMesh);
    if (m_qvCache == nullptr)
        {
        return SUCCESS;
        }
    {
        std::lock_guard<std::mutex> l(elemMutex);
        qvCachedDisplayMesh->m_qvElem = qv_beginElement(m_qvCache, 0, NULL);
    }
    if(nbTriangles > 0) qv_addQuickTriMesh(qvCachedDisplayMesh->m_qvElem, 3 * nbTriangles, indices, (int)nbVertices, positionOrigin, reinterpret_cast <FloatXYZ*> (positions),
                       reinterpret_cast <FloatXYZ*> (normals),
                       reinterpret_cast <FloatXY*> (params), textureId,/*QV_QTMESH_GENNORMALS*/0);

    {
        std::lock_guard<std::mutex> l(elemMutex);
        qv_endElement(qvCachedDisplayMesh->m_qvElem);
    }

    if (nbTriangles == 0) qvCachedDisplayMesh->m_qvElem = 0;

  /*  {
        std::lock_guard<std::mutex> l(elemMutex);
        QvElData data;
        data.handle = qvCachedDisplayMesh->m_qvElem;
        data.nVerts = nbVertices;
        data.nIndices = 3 * nbTriangles;
        elemsCreatedForNode[nodeId].push_back(data);
        //listOfElemsCreatedAndDestroyed.push_back(make_bpair(true, qvCachedDisplayMesh->m_qvElem));
        state[qvCachedDisplayMesh->m_qvElem] = true;
    }*/

    cachedDisplayMesh = qvCachedDisplayMesh.release();

    return SUCCESS;
    }

BentleyStatus ScalableMeshDisplayCacheManager::_DestroyCachedMesh(SmCachedDisplayMesh* cachedDisplayMesh)
    {
    // shutting down
    if (nullptr == DgnPlatformLib::QueryHost())
        { 
        assert(!"WARNING : Possible memory leak, missing AdoptHost in thread?");
        return SUCCESS;
        }

    if (NULL != cachedDisplayMesh->m_qvElem)
        {
         /*   {
                std::lock_guard<std::mutex> l(elemMutex);
                //listOfElemsCreatedAndDestroyed.push_back(make_bpair(false, cachedDisplayMesh->m_qvElem));
                state[cachedDisplayMesh->m_qvElem] = false;
            }*/
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
    if (nullptr == DgnPlatformLib::QueryHost())
        { 
        assert(!"WARNING : Possible memory leak, missing AdoptHost in thread?");
        return SUCCESS;
        }

    if (cachedDisplayTexture->m_textureID != 0)
        {
        T_HOST.GetGraphicsAdmin()._DeleteTexture(cachedDisplayTexture->m_textureID);
       // qv_deleteTexture(cachedDisplayTexture->m_textureID);
        cachedDisplayTexture->m_textureID = 0;
        }

    delete cachedDisplayTexture;

    return SUCCESS;
    }

/*void ScalableMeshDisplayCacheManager::_SetCacheDirty(bool isDirty)
    {
    m_resourcesOutOfDate = isDirty;
    }*/

bool ScalableMeshDisplayCacheManager::IsDirty()
    {
    return m_resourcesOutOfDate;
    }

bool ScalableMeshDisplayCacheManager::IsValid(QvElem* elem)
{
   // std::lock_guard<std::mutex> l(elemMutex);
   // return state.count(elem) > 0 && state[elem] == true;
    return true;
}

bool ScalableMeshDisplayCacheManager::IsValidForId(QvElem* elem, uint64_t id)
{
 /*   bvector<QvElData> allElements;
    {
        std::lock_guard<std::mutex> l(elemMutex);
        allElements = elemsCreatedForNode[id];
    }
    for (auto& elemData : allElements)
        if (elemData.handle == elem) return true;
    return false;*/
    return true;
}

ScalableMeshDisplayCacheManager::ScalableMeshDisplayCacheManager(DgnDbCR dgbDb)
    {
    m_qvCache = dgbDb.Models().GetQvCache();
    m_resourcesOutOfDate = false;
    }

ScalableMeshDisplayCacheManager::~ScalableMeshDisplayCacheManager()
    {

    }

