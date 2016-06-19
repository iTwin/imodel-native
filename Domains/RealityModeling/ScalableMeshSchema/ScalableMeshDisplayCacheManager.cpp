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
    assert(m_renderSys != 0);

    Render::IGraphicBuilder::TriMeshArgs trimesh;
    trimesh.m_numPoints  = (int32_t)nbVertices;
    trimesh.m_points     = (FPoint3d*)positions;
    trimesh.m_normals    = (FPoint3d*)normals;
    trimesh.m_numIndices = 3 * nbTriangles;
    trimesh.m_vertIndex  = indices;
    trimesh.m_textureUV  = (FPoint2d*)params;
    
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
    qvCachedDisplayMesh->m_graphic->SetSymbology(ColorDef::Green(), ColorDef::Green(), 0);
    qvCachedDisplayMesh->m_graphic->AddTriMesh(trimesh);
    qvCachedDisplayMesh->m_graphic->Close();

    cachedDisplayMesh = qvCachedDisplayMesh.release();

    return SUCCESS;
    }

BentleyStatus ScalableMeshDisplayCacheManager::_DestroyCachedMesh(SmCachedDisplayMesh* cachedDisplayMesh)
    {
    // shutting down
    if (nullptr == DgnPlatformLib::QueryHost())
        return SUCCESS;
       
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
    assert(m_renderSys != 0);
            
    std::unique_ptr<SmCachedDisplayTexture> qvCachedDisplayTexture(new SmCachedDisplayTexture);

    ByteStream stream(texels, xSize * ySize * 3);
    Render::Image textureImage(xSize, ySize, std::move(stream), Render::Image::Format::Rgb);        

    qvCachedDisplayTexture->m_texturePtr = m_renderSys->_CreateTexture(textureImage);
    
    cachedDisplayTexture = qvCachedDisplayTexture.release();

    return SUCCESS;
    }

BentleyStatus ScalableMeshDisplayCacheManager::_DestroyCachedTexture(SmCachedDisplayTexture* cachedDisplayTexture)
    {    
    delete cachedDisplayTexture;

    return SUCCESS;
    }

ScalableMeshDisplayCacheManager::ScalableMeshDisplayCacheManager()
    {
    m_renderSys = 0;    
    }

ScalableMeshDisplayCacheManager::~ScalableMeshDisplayCacheManager()
    {

    }

void ScalableMeshDisplayCacheManager::SetRenderSys(Dgn::Render::SystemP renderSys)
    {
    m_renderSys = renderSys;
    }

