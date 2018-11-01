#pragma once
#include <ScalableMeshSchema/ScalableMeshSchemaCommon.h>
#include <ScalableMeshSchema/ExportMacros.h>
#include <ScalableMesh/IScalableMesh.h>
#include <ScalableMesh\IScalableMeshProgressiveQuery.h>
#define QV_NO_MSTN_TYPES

#include <DgnPlatform/Render.h>


USING_NAMESPACE_BENTLEY_SCALABLEMESH
USING_NAMESPACE_BENTLEY_DGN

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct SmCachedDisplayTexture
    {
    SmCachedDisplayTexture()
        {        
        }

    Render::TexturePtr m_texturePtr;    
    };

struct SmCachedDisplayMesh
    {
    Render::GraphicBuilderPtr m_graphic;
    
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE


BEGIN_BENTLEY_SCALABLEMESH_SCHEMA_NAMESPACE
struct ScalableMeshDisplayCacheManager : public IScalableMeshDisplayCacheManager
    {
    private:
        
        Dgn::Render::SystemP m_renderSys;
        bool m_resourcesOutOfDate;

        virtual uint32_t _GetExcessiveRefCountThreshold() const { return 4000000000; }

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
                                                uint64_t nodeId,
                                                uint64_t smId) override;

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

        virtual BentleyStatus _DeleteFromVideoMemory(SmCachedDisplayMesh* cachedDisplayMesh) override { return SUCCESS; }

        virtual BentleyStatus _DeleteFromVideoMemory(SmCachedDisplayTexture* cachedDisplayTex) override { return SUCCESS; }

        virtual bool _IsUsingVideoMemory() { return false;  }

        //virtual void _SetCacheDirty(bool isDirty) override;
        void SetRenderSys(Dgn::Render::SystemP renderSys);
        bool IsDirty();

        //bool IsValid(QvElem* elem);

        //bool IsValidForId(QvElem* elem, uint64_t id);

        bool CanDisplay()
            {
            return true;
            //return m_renderSys != nullptr;
            }

        ScalableMeshDisplayCacheManager();

        ~ScalableMeshDisplayCacheManager();

    };

END_BENTLEY_SCALABLEMESH_SCHEMA_NAMESPACE
