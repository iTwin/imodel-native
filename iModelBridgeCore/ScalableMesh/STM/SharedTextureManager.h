#pragma once
#include <Bentley/bmap.h>
#include "SMMemoryPool.h"

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE
struct SharedTextureManager
    {
    bmap<uint64_t, SMMemoryPoolItemId> m_texMap;
    bmap<uint64_t, SMMemoryPoolItemId> m_texVideoMap;
    bmap<uint64_t, SMMemoryPoolItemId> m_texDataMap;
        
    std::mutex m_texMutex;


    SharedTextureManager() {}

    ~SharedTextureManager()
        {
        assert(m_texMap.size() == 0);
        assert(m_texDataMap.size() == 0);
        assert(m_texVideoMap.size() == 0);
        
        /*
        std::lock_guard<std::mutex> lock(m_texMutex);
        for (auto& data : m_texMap)
            {
            SMMemoryPool::GetInstance()->RemoveItem(data.second, data.first, SMStoreDataType::Texture, (uint64_t)this);
            }
        for (auto& data : m_texDataMap)
            {
            SMMemoryPool::GetInstance()->RemoveItem(data.second, data.first, SMStoreDataType::DisplayTexture, (uint64_t)this);
            }
            */
        }

    SMMemoryPoolItemId GetPoolIdForTexture(uint64_t texID)
        {
        std::lock_guard<std::mutex> lock(m_texMutex);
        if (m_texMap.count(texID) == 0) return SMMemoryPool::s_UndefinedPoolItemId;
        return m_texMap[texID];
        }

    SMMemoryPoolItemId GetPoolIdForTextureVideo(uint64_t texID)
        {
        std::lock_guard<std::mutex> lock(m_texMutex);
        if (m_texVideoMap.count(texID) == 0) return SMMemoryPool::s_UndefinedPoolItemId;
        return m_texVideoMap[texID];
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

    void SetPoolIdForTextureVideo(uint64_t texID, SMMemoryPoolItemId id)
        {
        std::lock_guard<std::mutex> lock(m_texMutex);
        m_texVideoMap[texID] = id;
        }

    void SetPoolIdForTextureData(uint64_t texID, SMMemoryPoolItemId id)
        {
        std::lock_guard<std::mutex> lock(m_texMutex);
        m_texDataMap[texID] = id;
        }    

    void RemovePoolIdForTextureData(uint64_t texID)
        {
        std::lock_guard<std::mutex> lock(m_texMutex);
        m_texDataMap.erase(texID);
        }

    void RemovePoolIdForTexture(uint64_t texID)
        {
        std::lock_guard<std::mutex> lock(m_texMutex);        
        m_texMap.erase(texID);
        }

    void RemovePoolIdForTextureVideo(uint64_t texID)
        {
        std::lock_guard<std::mutex> lock(m_texMutex);
        m_texVideoMap.erase(texID);
        }

    void RemoveAllPoolIdForTexture()
        {
        std::lock_guard<std::mutex> lock(m_texMutex);
        m_texMap.clear();
        }

    void RemoveAllPoolIdForTextureVideo()
        {
        std::lock_guard<std::mutex> lock(m_texMutex);        
        m_texVideoMap.clear();
        }
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE