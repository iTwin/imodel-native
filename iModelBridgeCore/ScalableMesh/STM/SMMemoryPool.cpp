//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include <ScalableMeshPCH.h>

#include <limits>   
#include "SMMemoryPool.h"
#include "Tracer.h"

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

#define PoolItemId2IndexBin(poolItemId)  (poolItemId / s_binSize)
#define PoolItemId2IndexItem(poolItemId) (poolItemId % s_binSize)
#define Index2PoolItemId(bin, item)      (bin*s_binSize + item)


void GetAllDataTypesInCompositeDataType(bvector<SMStoreDataType>& dataTypes, SMStoreDataType compositeDataType) 
    {
    switch (compositeDataType)
        {
        case SMStoreDataType::Cesium3DTiles:
            {
            dataTypes.push_back(SMStoreDataType::UvCoords);
            dataTypes.push_back(SMStoreDataType::Texture);
            }
        case SMStoreDataType::PointAndTriPtIndices:
            {
            dataTypes.push_back(SMStoreDataType::Points);
            dataTypes.push_back(SMStoreDataType::TriPtIndices);
            break;
            }
        case SMStoreDataType::TextureCompressed:
            {
            dataTypes.push_back(SMStoreDataType::TextureCompressed);
            break;
            }
        default:
            {
            assert(!"Unknown composite data type");
            break;
            }
        };
    }

SMStoreDataType GetStoreDataType(SMStoreDataType poolDataType)
    {
    switch (poolDataType)
        {
        case SMStoreDataType::Points :
            return SMStoreDataType::Points;
        case SMStoreDataType::TriPtIndices :
            return SMStoreDataType::TriPtIndices;
        default:
            return SMStoreDataType::Unknown;
        }
    }
    
SMMemoryPoolItemId SMMemoryPool::s_UndefinedPoolItemId = std::numeric_limits<SMMemoryPoolItemId>::max();
SMMemoryPoolPtr    SMMemoryPool::s_memoryPool; 
SMMemoryPoolPtr    SMMemoryPool::s_videoMemoryPool;

SMMemoryPoolItemBase::SMMemoryPoolItemBase()
    {
    m_dataType = SMStoreDataType::Unknown;
    m_dirty = false;
    m_nodeId = std::numeric_limits<uint64_t>::max();
    m_smId = std::numeric_limits<uint64_t>::max();
    m_poolItemId = SMMemoryPool::s_UndefinedPoolItemId;
    }

SMMemoryPoolItemBase::SMMemoryPoolItemBase(Byte* data, uint64_t size, uint64_t nodeId, SMStoreDataType& dataType, uint64_t smId)
    {
    m_data = data;
    m_size = size;   
    m_nodeId = nodeId;
    m_smId = smId;
    m_dataType = dataType;
    m_data = data;
    m_dirty = false;
    }

uint32_t SMMemoryPoolItemBase::AddRef() const
{
    if (m_dataType == SMStoreDataType::DisplayMesh || m_dataType == SMStoreDataType::DisplayTexture)
    {
        TRACEPOINT(m_dataType == SMStoreDataType::DisplayMesh ? EventType::CACHED_MESH_ACQUIRE : EventType::CACHED_TEX_ACQUIRE, m_nodeId, (uint64_t)-1, m_dataType == SMStoreDataType::DisplayTexture ? m_nodeId : -1, m_poolItemId, (uint64_t)this, GetRefCount())
    }
        return RefCounted <IRefCounted>::AddRef();
}

uint32_t SMMemoryPoolItemBase::Release() const
{
    if (m_dataType == SMStoreDataType::DisplayMesh || m_dataType == SMStoreDataType::DisplayTexture)
    {
        TRACEPOINT(m_dataType == SMStoreDataType::DisplayMesh ? EventType::CACHED_MESH_RELEASE : EventType::CACHED_TEX_RELEASE, m_nodeId, (uint64_t)-1, m_dataType == SMStoreDataType::DisplayTexture ? m_nodeId : -1, m_poolItemId, (uint64_t)this, GetRefCount())
    }
        return RefCounted <IRefCounted>::Release();
}
   
SMMemoryPoolItemBase::~SMMemoryPoolItemBase()
    {
    if (m_data != 0)
        {
        delete[] m_data;
        m_data = 0;
        }
    }   

uint64_t SMMemoryPoolItemBase::GetSize()
    {
    return m_size;
    }        

bool SMMemoryPoolItemBase::IsCorrect(uint64_t nodeId, SMStoreDataType& dataType)
    {
    if (nodeId == m_nodeId && dataType == m_dataType)
        return true;

    return false;
    }

bool SMMemoryPoolItemBase::IsCorrect(uint64_t nodeId, SMStoreDataType& dataType, uint64_t smId)
    {
    if (nodeId == m_nodeId && dataType == m_dataType && smId == m_smId)
        return true;

    return false;
    }

SMMemoryPoolItemId SMMemoryPoolItemBase::GetPoolItemId() const
    {
    return m_poolItemId;
    }

void SMMemoryPoolItemBase::SetPoolItemId(SMMemoryPoolItemId poolItemId)
    {                
    m_poolItemId = poolItemId;
    }

void SMMemoryPoolItemBase::NotifySizeChangePoolItem(int64_t sizeDelta, int64_t nbItemDelta)
    {
    if(m_poolId == SMMemoryPoolType::CPU)
        SMMemoryPool::GetInstance()->NotifySizeChangePoolItem(this, sizeDelta);
    else
        SMMemoryPool::GetInstanceVideo()->NotifySizeChangePoolItem(this, sizeDelta);

    std::lock_guard<std::mutex> lock(m_listenerMutex);

    for (auto& listener : m_listeners)
        {
        listener->NotifyListener(this, sizeDelta, nbItemDelta);
        }
    }  

/*
SMMemoryPoolItemBasePtr PointAndTriPtIndices::GetNodeDataStore(size_t nbItems, uint64_t nodeId, SMStoreDataType dataType, uint64_t smId)
    {
    SMMemoryPoolItemBasePtr item; 

    if (dataType == SMStoreDataType::Points)
        {
        item = new SMMemoryPoolVectorItem<DPoint3d>(nbItems, nodeId, dataType, smId);            
        m_pointData = (DPoint3d *)item->m_data;
        }
    else
    if (dataType == SMStoreDataType::TriPtIndices)
        {
        item = new SMMemoryPoolVectorItem<int32_t>(nbItems, nodeId, dataType, smId);            
        m_indicesData = (int32_t*)item->m_data;        
        }   

    return item;
    }
    */

static uint64_t s_binSize = 3000;
static uint64_t s_maxBins = 1000;

SMMemoryPool::SMMemoryPool(SMMemoryPoolType type)                        
    {
    m_currentPoolSizeInBytes = 0;
    m_maxPoolSizeInBytes = 0;            
    m_nbBins = 1;
    m_id = type;
    m_memPoolItems.resize(s_maxBins);
    m_memPoolItems[0].resize(s_binSize);
    m_lastAccessTime.resize(s_maxBins);
    m_lastAccessTime[0].resize(s_binSize);
    m_memPoolItemMutex.resize(s_maxBins);
    m_memPoolItemMutex[0].resize(s_binSize);

    m_nextEntryAvailable = 0;

    for (size_t itemId = 0; itemId < m_memPoolItemMutex[0].size(); itemId++)
        {
        m_memPoolItemMutex[0][itemId] = new Spinlock;
        }
    }

SMMemoryPool::~SMMemoryPool()
    {
    for (size_t binId = 0; binId < m_nbBins; binId++)
        {
        for (size_t itemId = 0; itemId < m_memPoolItemMutex.size(); itemId++)
            {                
            delete m_memPoolItemMutex[binId][itemId];
            }                                                
        }
    }
    
bool SMMemoryPool::GetItem(SMMemoryPoolItemBasePtr& memItemPtr, SMMemoryPoolItemId id)
    {     
    if (id == SMMemoryPool::s_UndefinedPoolItemId)
        return false; 

    size_t binId = PoolItemId2IndexBin(id);
    size_t itemId = PoolItemId2IndexItem(id);
    //assert(binId < m_nbBins);

    if (binId >= m_nbBins) return false;
    std::lock_guard<Spinlock> lock(*m_memPoolItemMutex[binId][itemId]);
    m_lastAccessTime[binId][itemId] = clock();
    memItemPtr = m_memPoolItems[binId][itemId];
    return memItemPtr.IsValid();
    }



bool SMMemoryPool::RemoveItem(SMMemoryPoolItemId id, uint64_t nodeId, SMStoreDataType dataType, uint64_t smId)
    {     
    if (id == SMMemoryPool::s_UndefinedPoolItemId)
        return false; 

    size_t binId = PoolItemId2IndexBin(id);
    size_t itemId = PoolItemId2IndexItem(id);
    assert(binId < m_nbBins);

    if (binId >= m_nbBins) return false;

    std::lock_guard<Spinlock> lock(*m_memPoolItemMutex[binId][itemId]);
    SMMemoryPoolItemBasePtr memItemPtr(m_memPoolItems[binId][itemId]);

    if (memItemPtr.IsValid() && memItemPtr->IsCorrect(nodeId, dataType, smId))
        {
                
        TRACEPOINTSize(EventType::POOL_REMOVEITEM, nodeId, (uint64_t)-1, dataType == SMStoreDataType::DisplayTexture ? nodeId : -1, 
                        memItemPtr->GetPoolItemId(), (uint64_t)&m_memPoolItems[binId][itemId], 
                        memItemPtr->GetRefCount(), -1 * (int64_t)(memItemPtr->GetSizeAllocated()))

        DecreaseCurrentPool(memItemPtr->GetSizeAllocated());
        memItemPtr = 0;        
        m_memPoolItems[binId][itemId] = 0;

        std::lock_guard<std::mutex> lockFree(m_freeEntryMutex);
        m_freeEntries.push(Index2PoolItemId(binId, itemId));

        return true;
        }

    return false;
    }

uint64_t SMMemoryPool::RemoveAllItemsOfType(SMStoreDataType dataType, uint64_t smId)
    {
    uint64_t nRemoved = 0;
    for (size_t binIndToDelete = 0; binIndToDelete < m_nbBins; binIndToDelete++)
        {
        for (size_t itemIndToDelete = 0; itemIndToDelete < m_memPoolItems[binIndToDelete].size(); itemIndToDelete++)
            {
            std::lock_guard<Spinlock> lock(*m_memPoolItemMutex[binIndToDelete][itemIndToDelete]);

            if (m_memPoolItems[binIndToDelete][itemIndToDelete].IsValid() && m_memPoolItems[binIndToDelete][itemIndToDelete]->GetRefCount() == 1 && m_memPoolItems[binIndToDelete][itemIndToDelete]->IsCorrect(m_memPoolItems[binIndToDelete][itemIndToDelete]->GetNodeId(), dataType, smId))
                {
                TRACEPOINTSize(EventType::POOL_REMOVEITEM, m_memPoolItems[binIndToDelete][itemIndToDelete]->GetNodeId(), (uint64_t)-1,
                    dataType == SMStoreDataType::DisplayTexture ? m_memPoolItems[binIndToDelete][itemIndToDelete]->GetNodeId() : -1,
                    m_memPoolItems[binIndToDelete][itemIndToDelete]->GetPoolItemId(), (uint64_t)&m_memPoolItems[binIndToDelete][itemIndToDelete],
                    m_memPoolItems[binIndToDelete][itemIndToDelete]->GetRefCount(), -1 * (int64_t)(m_memPoolItems[binIndToDelete][itemIndToDelete]->GetSizeAllocated()))

                DecreaseCurrentPool(m_memPoolItems[binIndToDelete][itemIndToDelete]->GetSizeAllocated());

                m_memPoolItems[binIndToDelete][itemIndToDelete] = 0;

                std::lock_guard<std::mutex> lockFree(m_freeEntryMutex);
                m_freeEntries.push(Index2PoolItemId(binIndToDelete, itemIndToDelete));

                nRemoved++;
                }
            }
        }
    return nRemoved;
    }

void SMMemoryPool::ReplaceItem(SMMemoryPoolItemBasePtr& poolItem, SMMemoryPoolItemId id, uint64_t nodeId, SMStoreDataType dataType, uint64_t smId)
    {
    if (id == SMMemoryPool::s_UndefinedPoolItemId)
        return;

    size_t binId = PoolItemId2IndexBin(id);
    size_t itemId = PoolItemId2IndexItem(id);
    assert(binId < m_nbBins); 

    std::lock_guard<Spinlock> lock(*m_memPoolItemMutex[binId][itemId]);
    SMMemoryPoolItemBasePtr memItemPtr(m_memPoolItems[binId][itemId]);

    if (memItemPtr.IsValid() && memItemPtr->IsCorrect(nodeId, dataType,smId))
        {
        TRACEPOINTSize(EventType::POOL_REMOVEITEM, memItemPtr->GetNodeId(), (uint64_t)-1,
            dataType == SMStoreDataType::DisplayTexture ? memItemPtr->GetNodeId() : -1,
            memItemPtr->GetPoolItemId(), (uint64_t)&memItemPtr,
            memItemPtr->GetRefCount(), -1 * (int64_t)(memItemPtr->GetSizeAllocated()))

        DecreaseCurrentPool(memItemPtr->GetSizeAllocated());
        //Ensure the replaced item is not saved to file.
        memItemPtr->SetDirty(false);
        memItemPtr = 0;        
        m_memPoolItems[binId][itemId] = poolItem;
        m_memPoolItems[binId][itemId]->SetPoolItemId(id);

        TRACEPOINTSize(EventType::POOL_ADDITEM, m_memPoolItems[binId][itemId]->GetNodeId(), (uint64_t)-1,
            dataType == SMStoreDataType::DisplayTexture ? m_memPoolItems[binId][itemId]->GetNodeId() : -1,
            m_memPoolItems[binId][itemId]->GetPoolItemId(), (uint64_t)&m_memPoolItems[binId][itemId],
            m_memPoolItems[binId][itemId]->GetRefCount(), m_memPoolItems[binId][itemId]->GetSizeAllocated())

        IncreaseCurrentPool(m_memPoolItems[binId][itemId]->GetSizeAllocated());
        m_lastAccessTime[binId][itemId] = clock();
        return;
        }
    }

SMMemoryPoolItemId SMMemoryPool::AddItem(SMMemoryPoolItemBasePtr& poolItem)
    {    
    uint64_t itemInd = 0;
    uint64_t binInd = 0;

    IncreaseCurrentPool(poolItem->GetSizeAllocated());

    poolItem->m_poolId = m_id;

    // Possibly a better way to do that.
    clock_t oldestTime = numeric_limits<clock_t>::max();
    
    if (m_currentPoolSizeInBytes > m_maxPoolSizeInBytes)
        {
        for (binInd = 0; binInd < m_nbBins; binInd++)
            {
            for (itemInd = 0; itemInd < (uint64_t)m_memPoolItems[binInd].size(); itemInd++)
                {
                if (m_memPoolItems[binInd][itemInd].IsValid() && oldestTime > m_lastAccessTime[binInd][itemInd])
                    {
                    oldestTime = m_lastAccessTime[binInd][itemInd];
                    }
                }

            if (itemInd < (uint64_t)m_memPoolItems[binInd].size())
                {
                break;
                }
            }
        }

    if (m_currentPoolSizeInBytes > m_maxPoolSizeInBytes)
        {
        double flushTimeThreshold = (clock() + oldestTime) / 2.0;

        for (size_t binIndToDelete = 0; binIndToDelete < m_nbBins; binIndToDelete++)
            {
            for (size_t itemIndToDelete = 0; itemIndToDelete < m_memPoolItems[binIndToDelete].size() && m_currentPoolSizeInBytes > m_maxPoolSizeInBytes; itemIndToDelete++)
                {     
                std::lock_guard<Spinlock> lock(*m_memPoolItemMutex[binIndToDelete][itemIndToDelete]);

                if (m_memPoolItems[binIndToDelete][itemIndToDelete].IsValid() && m_lastAccessTime[binIndToDelete][itemIndToDelete] < flushTimeThreshold && m_memPoolItems[binIndToDelete][itemIndToDelete]->GetRefCount() == 1)                    
                    {
                    TRACEPOINTSize(EventType::POOL_DELETEITEM, m_memPoolItems[binIndToDelete][itemIndToDelete]->GetNodeId(), 
                                    m_memPoolItems[binIndToDelete][itemIndToDelete]->GetDataType() == SMStoreDataType::DisplayMesh ? m_memPoolItems[binIndToDelete][itemIndToDelete]->GetNodeId() : -1, 
                                    m_memPoolItems[binIndToDelete][itemIndToDelete]->GetDataType() == SMStoreDataType::DisplayTexture ? m_memPoolItems[binIndToDelete][itemIndToDelete]->GetNodeId() : -1, 
                                    m_memPoolItems[binIndToDelete][itemIndToDelete]->GetPoolItemId(), (uint64_t)&m_memPoolItems[binIndToDelete][itemIndToDelete], 
                                    m_memPoolItems[binIndToDelete][itemIndToDelete]->GetRefCount(), -1 * (int64_t)(m_memPoolItems[binIndToDelete][itemIndToDelete]->GetSizeAllocated()))

                    DecreaseCurrentPool(m_memPoolItems[binIndToDelete][itemIndToDelete]->GetSizeAllocated());
                    m_memPoolItems[binIndToDelete][itemIndToDelete] = 0; 

                    std::lock_guard<std::mutex> lockFree(m_freeEntryMutex);
                    m_freeEntries.push(Index2PoolItemId(binIndToDelete, itemIndToDelete));
                    }                                        
                }
            }
        }

    if (m_currentPoolSizeInBytes > m_maxPoolSizeInBytes * s_maxMemBeforeFlushing)
        {
        for (size_t binIndToDelete = 0; binIndToDelete < m_nbBins; binIndToDelete++)
            {
            for (size_t itemIndToDelete = 0; itemIndToDelete < m_memPoolItems[binIndToDelete].size() && m_currentPoolSizeInBytes > m_maxPoolSizeInBytes; itemIndToDelete++)
                {  
                std::lock_guard<Spinlock> lock(*m_memPoolItemMutex[binIndToDelete][itemIndToDelete]);

                if (m_memPoolItems[binIndToDelete][itemIndToDelete].IsValid() && m_memPoolItems[binIndToDelete][itemIndToDelete]->GetRefCount() == 1)
                    {
                    TRACEPOINTSize(EventType::POOL_DELETEITEM, m_memPoolItems[binIndToDelete][itemIndToDelete]->GetNodeId(),
                        m_memPoolItems[binIndToDelete][itemIndToDelete]->GetDataType() == SMStoreDataType::DisplayMesh ? m_memPoolItems[binIndToDelete][itemIndToDelete]->GetNodeId() : -1,
                        m_memPoolItems[binIndToDelete][itemIndToDelete]->GetDataType() == SMStoreDataType::DisplayTexture ? m_memPoolItems[binIndToDelete][itemIndToDelete]->GetNodeId() : -1,
                        m_memPoolItems[binIndToDelete][itemIndToDelete]->GetPoolItemId(), (uint64_t)&m_memPoolItems[binIndToDelete][itemIndToDelete],
                        m_memPoolItems[binIndToDelete][itemIndToDelete]->GetRefCount(), -1 * (int64_t)(m_memPoolItems[binIndToDelete][itemIndToDelete]->GetSizeAllocated()))

                    DecreaseCurrentPool(m_memPoolItems[binIndToDelete][itemIndToDelete]->GetSizeAllocated());
                    m_memPoolItems[binIndToDelete][itemIndToDelete] = 0; 

                    std::lock_guard<std::mutex> lockFree(m_freeEntryMutex);
                    m_freeEntries.push(Index2PoolItemId(binIndToDelete, itemIndToDelete));
                    }                    
                }
            }
        }            

    
    // Select an entry
    m_freeEntryMutex.lock();
    if (!m_freeEntries.empty())
    {
        auto data = m_freeEntries.front();
        m_freeEntries.pop();

        itemInd = PoolItemId2IndexItem(data);
        binInd = PoolItemId2IndexBin(data);

        TRACEPOINTSize(EventType::POOL_ADDITEM, poolItem->GetNodeId(), poolItem->GetDataType() == SMStoreDataType::DisplayMesh ? poolItem->GetNodeId() : -1,
            poolItem->GetDataType() == SMStoreDataType::DisplayTexture ? poolItem->GetNodeId() : -1, data, (uint64_t)&m_memPoolItems[binInd][itemInd],
            880, m_freeEntries.size())

    }
    else if ((binInd = PoolItemId2IndexBin(m_nextEntryAvailable)) < m_nbBins)
    {
        // Useful to fill a new bin for the first time
        itemInd = PoolItemId2IndexItem(m_nextEntryAvailable);
        ++m_nextEntryAvailable;

        TRACEPOINTSize(EventType::POOL_ADDITEM, poolItem->GetNodeId(), poolItem->GetDataType() == SMStoreDataType::DisplayMesh ? poolItem->GetNodeId() : -1,
            poolItem->GetDataType() == SMStoreDataType::DisplayTexture ? poolItem->GetNodeId() : -1, m_nextEntryAvailable-1, (uint64_t)&m_memPoolItems[binInd][itemInd],
            881, m_nextEntryAvailable)
    }
    else
    {
        // full, add a new bin 
        assert(m_nbBins + 1 < s_maxBins);
        
        m_memPoolItems[m_nbBins].resize(s_binSize);            
        m_lastAccessTime[m_nbBins].resize(s_binSize);            
        m_memPoolItemMutex[m_nbBins].resize(s_binSize);

        for (size_t itemId = 0; itemId < m_memPoolItemMutex[m_nbBins].size(); itemId++)
            {
            m_memPoolItemMutex[m_nbBins][itemId] = new Spinlock;
            }
             
        binInd = m_nbBins;
        m_nbBins++;
        itemInd = 0;
        m_nextEntryAvailable = Index2PoolItemId(binInd, itemInd) + 1;

        TRACEPOINTSize(EventType::POOL_ADDITEM, poolItem->GetNodeId(), poolItem->GetDataType() == SMStoreDataType::DisplayMesh ? poolItem->GetNodeId() : -1,
            poolItem->GetDataType() == SMStoreDataType::DisplayTexture ? poolItem->GetNodeId() : -1, Index2PoolItemId(binInd, itemInd), (uint64_t)&m_memPoolItems[binInd][itemInd],
            882, m_nbBins)
        }       
    m_memPoolItemMutex[binInd][itemInd]->lock();
    m_freeEntryMutex.unlock();

    BeAssert(!m_memPoolItems[binInd][itemInd].IsValid() || m_memPoolItems[binInd][itemInd]->GetRefCount() <= 1);

    m_memPoolItems[binInd][itemInd] = poolItem;
    m_lastAccessTime[binInd][itemInd] = clock();

    poolItem->SetPoolItemId(Index2PoolItemId(binInd, itemInd));

    TRACEPOINTSize(EventType::POOL_ADDITEM, poolItem->GetNodeId(), poolItem->GetDataType() == SMStoreDataType::DisplayMesh ? poolItem->GetNodeId() : -1,
        poolItem->GetDataType() == SMStoreDataType::DisplayTexture ? poolItem->GetNodeId() : -1, poolItem->GetPoolItemId(), (uint64_t)&m_memPoolItems[binInd][itemInd],
        poolItem->GetRefCount(), poolItem->GetSizeAllocated())

    m_memPoolItemMutex[binInd][itemInd]->unlock();

#ifndef NDEBUG
    /*
    uint64_t totalSize = 0;

    for (auto poolItem : m_memPoolItems)
        {
        if (poolItem.IsValid())
            {    
            totalSize += poolItem->GetSizeAllocated();
            }
        }

    assert(totalSize == m_currentPoolSizeInBytes);*/

#endif

    return Index2PoolItemId (binInd, itemInd);
    }

void SMMemoryPool::NotifySizeChangePoolItem(SMMemoryPoolItemBase* poolItem, int64_t sizeDelta)
    {
    assert(poolItem->GetPoolItemId() != SMMemoryPool::s_UndefinedPoolItemId);

    size_t binId = PoolItemId2IndexBin(poolItem->GetPoolItemId());
    size_t itemId = PoolItemId2IndexItem(poolItem->GetPoolItemId());
    assert(binId < m_nbBins);
    
    std::lock_guard<Spinlock> lock(*m_memPoolItemMutex[binId][itemId]);

    if (m_memPoolItems[binId][itemId].get() == poolItem) 
        {
        TRACEPOINTSize(EventType::POOL_CHANGESIZEITEM, m_memPoolItems[binId][itemId]->GetNodeId(),
            m_memPoolItems[binId][itemId]->GetDataType() == SMStoreDataType::DisplayMesh ? m_memPoolItems[binId][itemId]->GetNodeId() : -1,
            m_memPoolItems[binId][itemId]->GetDataType() == SMStoreDataType::DisplayTexture ? m_memPoolItems[binId][itemId]->GetNodeId() : -1,
            m_memPoolItems[binId][itemId]->GetPoolItemId(), (uint64_t)&m_memPoolItems[binId][itemId],
            m_memPoolItems[binId][itemId]->GetRefCount(), sizeDelta)
        IncreaseCurrentPool(sizeDelta);
        }
    }


void SMMemoryPool::CleanVideoMemoryPool() 
{
    if (s_videoMemoryPool != 0)
        s_videoMemoryPool = 0;

}

END_BENTLEY_SCALABLEMESH_NAMESPACE
