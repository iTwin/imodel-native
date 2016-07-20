//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/SMMemoryPool.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include <ScalableMeshPCH.h>

#include <limits>   
#include "SMMemoryPool.h"


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

void GetAllDataTypesInCompositeDataType(bvector<SMStoreDataType>& dataTypes, SMStoreDataType compositeDataType) 
    {
    if (compositeDataType == SMStoreDataType::PointAndTriPtIndices)
        {
        dataTypes.push_back(SMStoreDataType::Points);
        dataTypes.push_back(SMStoreDataType::TriPtIndices);
        return; 
        }

    assert(!"Unknown composite data type");
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
    SMMemoryPool::GetInstance()->NotifySizeChangePoolItem(this, sizeDelta);

    std::lock_guard<std::mutex> lock(m_listenerMutex);

    for (auto& listener : m_listeners)
        {
        listener->NotifyListener(this, sizeDelta, nbItemDelta);
        }
    }  


SMMemoryPoolItemBasePtr PointAndPtIndicesData::GetNodeDataStore(size_t nbItems, uint64_t nodeId, SMStoreDataType dataType, uint64_t smId)
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


static uint64_t s_binSize = 3000;
static uint64_t s_maxBins = 1000;

SMMemoryPool::SMMemoryPool()                        
    {
    m_currentPoolSizeInBytes = 0;
    m_maxPoolSizeInBytes = 0;            
    m_nbBins = 1;
    m_memPoolItems.resize(s_maxBins);
    m_memPoolItems[0].resize(s_binSize);
    m_lastAccessTime.resize(s_maxBins);
    m_lastAccessTime[0].resize(s_binSize);
    m_memPoolItemMutex.resize(s_maxBins);
    m_memPoolItemMutex[0].resize(s_binSize);

    for (size_t itemId = 0; itemId < m_memPoolItemMutex[0].size(); itemId++)
        {
        m_memPoolItemMutex[0][itemId] = new mutex;                
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

    size_t binId = id / s_binSize;
    size_t itemId = id % s_binSize;
    assert(binId < m_nbBins);

    std::lock_guard<std::mutex> lock(*m_memPoolItemMutex[binId][itemId]);
    m_lastAccessTime[binId][itemId] = clock();
    memItemPtr = m_memPoolItems[binId][itemId];
    return memItemPtr.IsValid();
    }

bool SMMemoryPool::RemoveItem(SMMemoryPoolItemId id, uint64_t nodeId, SMStoreDataType dataType, uint64_t smId)
    {     
    if (id == SMMemoryPool::s_UndefinedPoolItemId)
        return false; 

    size_t binId = id / s_binSize;
    size_t itemId = id % s_binSize;
    assert(binId < m_nbBins);

    std::lock_guard<std::mutex> lock(*m_memPoolItemMutex[binId][itemId]);            
    SMMemoryPoolItemBasePtr memItemPtr(m_memPoolItems[binId][itemId]);

    if (memItemPtr.IsValid() && memItemPtr->IsCorrect(nodeId, dataType, smId))
        {
        m_currentPoolSizeInBytes -= memItemPtr->GetSize();        
        memItemPtr = 0;        
        m_memPoolItems[binId][itemId] = 0;
        return true;
        }

    return false;
    }

void SMMemoryPool::ReplaceItem(SMMemoryPoolItemBasePtr& poolItem, SMMemoryPoolItemId id, uint64_t nodeId, SMStoreDataType dataType, uint64_t smId)
    {
    if (id == SMMemoryPool::s_UndefinedPoolItemId)
        return;

    size_t binId = id / s_binSize;
    size_t itemId = id % s_binSize;
    assert(binId < m_nbBins);

    std::lock_guard<std::mutex> lock(*m_memPoolItemMutex[binId][itemId]);
    SMMemoryPoolItemBasePtr memItemPtr(m_memPoolItems[binId][itemId]);

    if (memItemPtr.IsValid() && memItemPtr->IsCorrect(nodeId, dataType,smId))
        {
        m_currentPoolSizeInBytes -= memItemPtr->GetSize();
        memItemPtr = 0;
        m_memPoolItems[binId][itemId] = poolItem;
        m_currentPoolSizeInBytes += m_memPoolItems[binId][itemId]->GetSize();
        m_lastAccessTime[binId][itemId] = clock();
        return;
        }
    }

SMMemoryPoolItemId SMMemoryPool::AddItem(SMMemoryPoolItemBasePtr& poolItem)
    {    
    uint64_t itemInd = 0;
    uint64_t binInd = 0;

    clock_t oldestTime = numeric_limits<clock_t>::max();
    uint64_t oldestInd = 0; 

    m_currentPoolSizeInBytes += poolItem->GetSize();
    
    bool needToFlush = false;

    if (m_currentPoolSizeInBytes > m_maxPoolSizeInBytes)
        {
        needToFlush = true;
        }

    //clock_t currentTime = clock(); 

    for (; binInd < m_nbBins; binInd++)
        {        
        for (itemInd = 0; itemInd < (uint64_t)m_memPoolItems[binInd].size(); itemInd++)
            {
                {   
                std::lock_guard<std::mutex> lock(*m_memPoolItemMutex[binInd][itemInd]);

                if (!m_memPoolItems[binInd][itemInd].IsValid())
                    {
                    break;
                    }
                }

                /*
            if ((needToFlush && (currentTime - m_lastAccessTime[itemInd] > s_timeDiff)))                
                {
                break;
                } */               

            if (oldestTime > m_lastAccessTime[binInd][itemInd])
                {
                oldestTime = m_lastAccessTime[binInd][itemInd];
                oldestInd = itemInd;
                }
            }

        if (itemInd < (uint64_t)m_memPoolItems[binInd].size())
            break;
        }

    if (m_currentPoolSizeInBytes > m_maxPoolSizeInBytes)
        {
        double flushTimeThreshold = (clock() + oldestTime) / 2.0;

        for (size_t binIndToDelete = 0; binIndToDelete < m_nbBins; binIndToDelete++)
            {
            for (size_t itemIndToDelete = 0; itemIndToDelete < m_memPoolItems[binIndToDelete].size() && m_currentPoolSizeInBytes > m_maxPoolSizeInBytes; itemIndToDelete++)
                {     
                std::lock_guard<std::mutex> lock(*m_memPoolItemMutex[binIndToDelete][itemIndToDelete]);

                if (m_memPoolItems[binIndToDelete][itemIndToDelete].IsValid() && m_lastAccessTime[binIndToDelete][itemIndToDelete] < flushTimeThreshold && m_memPoolItems[binIndToDelete][itemIndToDelete]->GetRefCount() == 1)                    
                    {
                    m_currentPoolSizeInBytes -= m_memPoolItems[binIndToDelete][itemIndToDelete]->GetSize();                                        
                    m_memPoolItems[binIndToDelete][itemIndToDelete] = 0; 
                    itemInd = itemIndToDelete;
                    binInd = binIndToDelete;
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
                std::lock_guard<std::mutex> lock(*m_memPoolItemMutex[binIndToDelete][itemIndToDelete]);                    

                if (m_memPoolItems[binIndToDelete][itemIndToDelete].IsValid() && m_memPoolItems[binIndToDelete][itemIndToDelete]->GetRefCount() == 1)
                    {
                    m_currentPoolSizeInBytes -= m_memPoolItems[binIndToDelete][itemIndToDelete]->GetSize();                
                    m_memPoolItems[binIndToDelete][itemIndToDelete] = 0; 
                    itemInd = itemIndToDelete;
                    binInd = binIndToDelete;
                    }                    
                }
            }
        }            

    
    if (itemInd == s_binSize)
        {                
        m_increaseBinMutex.lock();

        if (binInd == m_nbBins)
            {                    
            assert(m_nbBins + 1 < s_maxBins);
            
            m_memPoolItems[m_nbBins].resize(s_binSize);            
            m_lastAccessTime[m_nbBins].resize(s_binSize);            
            m_memPoolItemMutex[m_nbBins].resize(s_binSize);

            for (size_t itemId = 0; itemId < m_memPoolItemMutex[m_nbBins].size(); itemId++)
                {
                m_memPoolItemMutex[m_nbBins][itemId] = new mutex;                
                }
                         
            m_nbBins++;
            }

        m_increaseBinMutex.unlock();

        itemInd = 0;

        for (; itemInd < (uint64_t)m_memPoolItems[binInd].size(); itemInd++)
            {                   
            std::lock_guard<std::mutex> lock(*m_memPoolItemMutex[binInd][itemInd]);

            if (!m_memPoolItems[binInd][itemInd].IsValid())
                {
                break;                
                }
            }

        assert(itemInd < (uint64_t)m_memPoolItems[binInd].size());
        }        
    
    m_memPoolItemMutex[binInd][itemInd]->lock();

    if (m_memPoolItems[binInd][itemInd].IsValid())            
        {                
        m_currentPoolSizeInBytes -= m_memPoolItems[binInd][itemInd]->GetSize();
        }

    m_memPoolItems[binInd][itemInd] = poolItem;
    m_lastAccessTime[binInd][itemInd] = clock();

    poolItem->SetPoolItemId(binInd*s_binSize + itemInd);

    m_memPoolItemMutex[binInd][itemInd]->unlock();

#ifndef NDEBUG
    /*
    uint64_t totalSize = 0;

    for (auto poolItem : m_memPoolItems)
        {
        if (poolItem.IsValid())
            {    
            totalSize += poolItem->GetSize();
            }
        }

    assert(totalSize == m_currentPoolSizeInBytes);*/

#endif

    return itemInd + binInd * s_binSize;
    }

void SMMemoryPool::NotifySizeChangePoolItem(SMMemoryPoolItemBase* poolItem, int64_t sizeDelta)
    {
    assert(poolItem->GetPoolItemId() != SMMemoryPool::s_UndefinedPoolItemId);

    size_t binId = poolItem->GetPoolItemId() / s_binSize;
    size_t itemId = poolItem->GetPoolItemId() % s_binSize;
    assert(binId < m_nbBins);
    
    std::lock_guard<std::mutex> lock(*m_memPoolItemMutex[binId][itemId]);    

    if (m_memPoolItems[binId][itemId].get() == poolItem)
        {
        m_currentPoolSizeInBytes += sizeDelta;
        }
    }

END_BENTLEY_SCALABLEMESH_NAMESPACE