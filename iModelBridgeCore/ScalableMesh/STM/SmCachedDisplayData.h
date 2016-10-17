#pragma once

#include "SMMemoryPool.h"
#include "Stores\SMSQLiteStore.h"

#include <ScalableMesh\IScalableMeshProgressiveQuery.h>

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct SmCachedDisplayData
    {
    private:

        SmCachedDisplayMesh*                m_cachedDisplayMesh;
        SmCachedDisplayTexture*             m_cachedDisplayTexture;
        IScalableMeshDisplayCacheManagerPtr m_displayCacheManagerPtr;
        size_t                              m_memorySize;
        bvector<uint64_t>                   m_appliedClips;

    public:

        SmCachedDisplayData(SmCachedDisplayMesh*                 cachedDisplayMesh,
                            SmCachedDisplayTexture*              cachedDisplayTexture,
                            IScalableMeshDisplayCacheManagerPtr& displayCacheManagerPtr,
                            size_t                               memorySize,
                            const bvector<uint64_t>&             appliedClips)
            {
            m_cachedDisplayMesh = cachedDisplayMesh;
            m_cachedDisplayTexture = cachedDisplayTexture;
            m_displayCacheManagerPtr = displayCacheManagerPtr;
            m_memorySize = memorySize;
            m_appliedClips.insert(m_appliedClips.end(), appliedClips.begin(), appliedClips.end());
            }

        virtual ~SmCachedDisplayData()
            {
            if (m_cachedDisplayMesh != 0)
                {
                BentleyStatus status = m_displayCacheManagerPtr->_DestroyCachedMesh(m_cachedDisplayMesh);
                assert(status == SUCCESS);
                }

            if (m_cachedDisplayTexture != 0)
                {
                BentleyStatus status = m_displayCacheManagerPtr->_DestroyCachedTexture(m_cachedDisplayTexture);
                assert(status == SUCCESS);
                }
            }

        size_t GetMemorySize() const
            {
            return m_memorySize;
            }

        SmCachedDisplayMesh* GetCachedDisplayMesh() const
            {
            return m_cachedDisplayMesh;
            }

        SmCachedDisplayTexture* GetCachedDisplayTexture() const
            {
            return m_cachedDisplayTexture;
            }

        const bvector<uint64_t>& GetAppliedClips()
            {
            return m_appliedClips;
            }
    };

struct SmCachedDisplayMeshData
    {
    private:

        SmCachedDisplayMesh*                m_cachedDisplayMesh;
        uint64_t                             m_textureID;
        bool                                 m_isTextured;
        IScalableMeshDisplayCacheManagerPtr m_displayCacheManagerPtr;
        size_t                              m_memorySize;
        bvector<uint64_t>                   m_appliedClips;

    public:
        SmCachedDisplayMeshData()
            {}

        SmCachedDisplayMeshData(SmCachedDisplayMesh*                 cachedDisplayMesh,
                                IScalableMeshDisplayCacheManagerPtr& displayCacheManagerPtr,
                                uint64_t                             textureID,
                                bool                                isTextured,
                                size_t                               memorySize,
                                const bvector<uint64_t>&             appliedClips)
            {
            m_cachedDisplayMesh = cachedDisplayMesh;
            m_textureID = textureID;
            m_isTextured = isTextured;
            m_displayCacheManagerPtr = displayCacheManagerPtr;
            m_memorySize = memorySize;
            m_appliedClips.insert(m_appliedClips.end(), appliedClips.begin(), appliedClips.end());
            }

        virtual ~SmCachedDisplayMeshData()
            {
            if (m_cachedDisplayMesh != 0)
                {
                BentleyStatus status = m_displayCacheManagerPtr->_DestroyCachedMesh(m_cachedDisplayMesh);
                assert(status == SUCCESS);
                }
            }

        size_t GetMemorySize() const
            {
            return m_memorySize;
            }

        SmCachedDisplayMesh* GetCachedDisplayMesh() const
            {
            return m_cachedDisplayMesh;
            }

        const bvector<uint64_t>& GetAppliedClips()
            {
            return m_appliedClips;
            }

        bool GetTextureInfo(uint64_t& textureID) const
            {
            if (!m_isTextured) return false;
            textureID = m_textureID;
            return true;
            }

        IScalableMeshDisplayCacheManager* GetDisplayCacheManager() const
            {
            return m_displayCacheManagerPtr.get();
            }
    };

struct SmCachedDisplayTextureData
    {

    private:
        SmCachedDisplayTexture*             m_cachedDisplayTexture;
        uint64_t m_textureID;
        IScalableMeshDisplayCacheManagerPtr m_displayCacheManagerPtr;
        size_t                              m_memorySize;


    public:

        SmCachedDisplayTextureData(SmCachedDisplayTexture*             cachedDisplayTexture,
                                   uint64_t textureID,
                                   IScalableMeshDisplayCacheManagerPtr& displayCacheManagerPtr,
                                   size_t                               memorySize)
            {
            m_cachedDisplayTexture = cachedDisplayTexture;
            m_textureID = textureID;
            m_displayCacheManagerPtr = displayCacheManagerPtr;
            m_memorySize = memorySize;
            }

        virtual ~SmCachedDisplayTextureData()
            {
            if (m_cachedDisplayTexture != 0)
                {
                BentleyStatus status = m_displayCacheManagerPtr->_DestroyCachedTexture(m_cachedDisplayTexture);
                assert(status == SUCCESS);
                }
            }

        size_t GetMemorySize() const
            {
            return m_memorySize;
            }

        uint64_t GetTextureID() const
            {
            return m_textureID;
            }

        SmCachedDisplayTexture* GetCachedDisplayTexture() const
            {
            return m_cachedDisplayTexture;
            }

        IScalableMeshDisplayCacheManager* GetDisplayCacheManager() const
            {
            return m_displayCacheManagerPtr.get();
            }
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE