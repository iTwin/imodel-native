#pragma once
#include <ScalableMeshSchema/ScalableMeshSchemaCommon.h>
#include <ScalableMeshSchema/ExportMacros.h>
#include <ScalableMesh/IScalableMesh.h>
#include <ScalableMesh\IScalableMeshProgressiveQuery.h>
#define QV_NO_MSTN_TYPES
#include <QuickVision\qvision.h>

USING_NAMESPACE_BENTLEY_SCALABLEMESH
USING_NAMESPACE_BENTLEY_DGNPLATFORM

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct SmCachedDisplayTexture
    {
    SmCachedDisplayTexture()
        {
        m_textureID = (QvTextureID)this;
        }

    QvTextureID m_textureID;
    };

struct SmCachedDisplayMesh
    {
    QvElem* m_qvElem;
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE
BEGIN_BENTLEY_SCALABLEMESH_SCHEMA_NAMESPACE
struct ScalableMeshDisplayCacheManager : public IScalableMeshDisplayCacheManager
    {
    private:

        QvCache* m_qvCache;

    public:


        //Inherited from IScalableMeshDisplayCacheManager
        virtual BentleyStatus _CreateCachedMesh(SmCachedDisplayMesh*&   cachedDisplayMesh,
                                                size_t                  nbVertices,
                                                DPoint3d const*         positionOrigin,
                                                float*                  positions,
                                                float*                  normals,
                                                int                     nbTriangles,
                                                int*                    indices,
                                                float*                  params,
                                                SmCachedDisplayTexture* cachedTexture) override;

        virtual BentleyStatus _DestroyCachedMesh(SmCachedDisplayMesh* cachedDisplayMesh) override;

        virtual BentleyStatus _CreateCachedTexture(SmCachedDisplayTexture*& cachedDisplayTexture,
                                                   int                      xSize,
                                                   int                      ySize,
                                                   int                      enableAlpha,
                                                   int                      format,      // => see QV_*_FORMAT definitions above
                                                   unsigned char const *    texels) override; // => texel image)

        virtual BentleyStatus _DestroyCachedTexture(SmCachedDisplayTexture* cachedDisplayTexture) override;

        ScalableMeshDisplayCacheManager(DgnDbCR dgbDb);

        ~ScalableMeshDisplayCacheManager();

    };

END_BENTLEY_SCALABLEMESH_SCHEMA_NAMESPACE