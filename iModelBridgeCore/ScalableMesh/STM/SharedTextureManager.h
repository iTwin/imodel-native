#pragma once
#include <Bentley/bmap.h>
#include "SMMemoryPool.h"

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE
struct SharedTextureManager
    {
    bmap<uint64_t, SMMemoryPoolItemId> m_texMap;
    bmap<uint64_t, SMMemoryPoolItemId> m_texDataMap;
    std::mutex m_texMutex;


    SharedTextureManager() {}

    SMMemoryPoolItemId GetPoolIdForTexture(uint64_t texID)
        {
        std::lock_guard<std::mutex> lock(m_texMutex);
        if (m_texMap.count(texID) == 0) return SMMemoryPool::s_UndefinedPoolItemId;
        return m_texMap[texID];
        }

    SMMemoryPoolItemId GetPoolIdForTextureData(uint64_t texID)
        {
        std::lock_guard<std::mutex> lock(m_texMutex);
        if (m_texDataMap.count(texID) == 0) return SMMemoryPool::s_UndefinedPoolItemId;
        return m_texDataMap[texID];
        }

    void SetPoolIdForTexture(uint64_t texID, SMMemoryPoolItemId id)
        {
        std::lock_guard<std::mutex> lock(m_texMutex);
        m_texMap[texID] = id;
        }

    void SetPoolIdForTextureData(uint64_t texID, SMMemoryPoolItemId id)
        {
        std::lock_guard<std::mutex> lock(m_texMutex);
        m_texDataMap[texID] = id;
        }
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE