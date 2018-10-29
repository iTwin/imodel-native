#include "ScalableMeshSchemaPCH.h"
#include <ScalableMeshSchema/ScalableMeshHandler.h>
#include "ScalableMeshDisplayCacheManager.h"
#include <mutex>
//#include <QuickVision/qvision.h>

USING_NAMESPACE_BENTLEY_SCALABLEMESH_SCHEMA

std::mutex elemMutex;

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
                                                                 uint64_t nodeId,
                                                                 uint64_t smId)
    {

    //assert(!"MST_TODO");
#if 0
    assert(m_renderSys != 0);

    Render::IGraphicBuilder::TriMeshArgs trimesh;
    trimesh.m_numPoints  = (int32_t)nbVertices;
    trimesh.m_points     = (FPoint3d*)positions;
    trimesh.m_normals    = (FPoint3d*)normals;
    trimesh.m_numIndices = 3 * nbTriangles;
    trimesh.m_vertIndex  = indices;       

            
    std::unique_ptr<SmCachedDisplayMesh> qvCachedDisplayMesh(new SmCachedDisplayMesh);

    Transform placement;

    if (positionOrigin != 0)
        {
        placement = Transform::From(*positionOrigin);
        }
    else
        {
        placement = Transform::FromIdentity();
        }
    
    Render::Graphic::CreateParams createParams(nullptr, placement);
    qvCachedDisplayMesh->m_graphic = m_renderSys->_CreateGraphic(createParams);

    if (cachedTexture != nullptr)
        {
        trimesh.m_texture = cachedTexture->m_texturePtr;
        trimesh.m_textureUV = (FPoint2d*)params;

        qvCachedDisplayMesh->m_graphic->SetSymbology(ColorDef::White(), ColorDef::White(), 0);        
        }
    else
        {   
        qvCachedDisplayMesh->m_graphic->SetSymbology(ColorDef::Green(), ColorDef::Green(), 0);
        }
    
    //trimesh.m_flags |= QV_QTMESH_GENNORMALS;    
    trimesh.m_flags = 1;
        
    qvCachedDisplayMesh->m_graphic->AddTriMesh(trimesh);
    qvCachedDisplayMesh->m_graphic->Close();

    cachedDisplayMesh = qvCachedDisplayMesh.release();

    isStoredOnGpu = false;
    usedMemInBytes = nbVertices * sizeof(float) * 3 + nbTriangles * 3 * sizeof(int32_t) + sizeof(float) * 2 * nbVertices;

#endif

    cachedDisplayMesh = nullptr;
    isStoredOnGpu = false;
    usedMemInBytes = 0;

    return SUCCESS;
    }

BentleyStatus ScalableMeshDisplayCacheManager::_DestroyCachedMesh(SmCachedDisplayMesh* cachedDisplayMesh)
    {
    // shutting down
/*
    if (nullptr == DgnPlatformLib::QueryHost())
        return SUCCESS;
       
    delete cachedDisplayMesh;
*/

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
    //assert(!"MST_TODO");
#if 0
    assert(m_renderSys != 0);
            
    std::unique_ptr<SmCachedDisplayTexture> qvCachedDisplayTexture(new SmCachedDisplayTexture);

    ByteStream stream(texels, xSize * ySize * 3);
    Render::Image textureImage(xSize, ySize, std::move(stream), Render::Image::Format::Rgb);        

    qvCachedDisplayTexture->m_texturePtr = m_renderSys->_CreateTexture(textureImage);
    
    cachedDisplayTexture = qvCachedDisplayTexture.release();

    isStoredOnGpu = false;
    usedMemInBytes = xSize * ySize * 6;

#endif

    cachedDisplayTexture = nullptr;
    isStoredOnGpu = false;
    usedMemInBytes = 0;
    return SUCCESS;
    }

BentleyStatus ScalableMeshDisplayCacheManager::_DestroyCachedTexture(SmCachedDisplayTexture* cachedDisplayTexture)
    {    

    //delete cachedDisplayTexture;

    return SUCCESS;
    }

bool ScalableMeshDisplayCacheManager::IsDirty()
    {
    return m_resourcesOutOfDate;
    }

#if 0 
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
#endif

ScalableMeshDisplayCacheManager::ScalableMeshDisplayCacheManager()
    {
    m_renderSys = 0;    
    m_resourcesOutOfDate = false;
    }

ScalableMeshDisplayCacheManager::~ScalableMeshDisplayCacheManager()
    {

    }

void ScalableMeshDisplayCacheManager::SetRenderSys(Dgn::Render::SystemP renderSys)
    {
    assert(renderSys != nullptr);
    m_renderSys = renderSys;
    }

