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
    
SMMemoryPoolItemId SMMemoryPool::s_UndefinedPoolItemId = std::numeric_limits<SMMemoryPoolItemId>::max();
SMMemoryPoolPtr    SMMemoryPool::s_memoryPool; 

SMMemoryPoolItemBase::SMMemoryPoolItemBase()
    {
    m_dataType = SMPoolDataTypeDesc::Unknown;
    m_dirty = false;
    m_nodeId = std::numeric_limits<uint64_t>::max();
    m_smId = std::numeric_limits<uint64_t>::max();
    m_poolItemId = SMMemoryPool::s_UndefinedPoolItemId;
    }

SMMemoryPoolItemBase::SMMemoryPoolItemBase(Byte* data, uint64_t size, uint64_t nodeId, SMPoolDataTypeDesc& dataType, uint64_t smId)
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

bool SMMemoryPoolItemBase::IsCorrect(uint64_t nodeId, SMPoolDataTypeDesc& dataType)
    {
    if (nodeId == m_nodeId && dataType == m_dataType)
        return true;

    return false;
    }

bool SMMemoryPoolItemBase::IsCorrect(uint64_t nodeId, SMPoolDataTypeDesc& dataType, uint64_t smId)
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

void SMMemoryPoolItemBase::NotifySizeChangePoolItem(int64_t sizeDelta)
    {
    SMMemoryPool::GetInstance()->NotifySizeChangePoolItem(this, sizeDelta);
    }

static uint64_t s_initSize = 3000;

SMMemoryPool::SMMemoryPool()                        
    {
    m_currentPoolSizeInBytes = 0;
    m_maxPoolSizeInBytes = 0;            
    m_memPoolItems.resize(s_initSize);
    m_lastAccessTime.resize(s_initSize);
    m_memPoolItemMutex.resize(s_initSize);

    for (size_t itemId = 0; itemId < m_memPoolItemMutex.size(); itemId++)
        {
        m_memPoolItemMutex[itemId] = new mutex;                
        }
    }

SMMemoryPool::~SMMemoryPool()
    {
    for (size_t itemId = 0; itemId < m_memPoolItemMutex.size(); itemId++)
        {                
        delete m_memPoolItemMutex[itemId];
        }                                                
    }
    
bool SMMemoryPool::GetItem(SMMemoryPoolItemBasePtr& memItemPtr, SMMemoryPoolItemId id)
    {     
    if (id == SMMemoryPool::s_UndefinedPoolItemId)
        return false; 

    std::lock_guard<std::mutex> lock(*m_memPoolItemMutex[id]);
    m_lastAccessTime[id] = clock();
    memItemPtr = m_memPoolItems[id];
    return memItemPtr.IsValid();
    }

bool SMMemoryPool::RemoveItem(SMMemoryPoolItemId id, uint64_t nodeId, SMPoolDataTypeDesc dataType, uint64_t smId)
    {     
    if (id == SMMemoryPool::s_UndefinedPoolItemId)
        return false; 

    std::lock_guard<std::mutex> lock(*m_memPoolItemMutex[id]);            
    SMMemoryPoolItemBasePtr memItemPtr(m_memPoolItems[id]);

    if (memItemPtr.IsValid() && memItemPtr->IsCorrect(nodeId, dataType, smId))
        {
        m_currentPoolSizeInBytes -= memItemPtr->GetSize();        
        memItemPtr = 0;
        //assert(m_memPoolItems[id]->GetRefCount() == 1);
        m_memPoolItems[id] = 0;
        return true;
        }

    return false;
    }

void SMMemoryPool::ReplaceItem(SMMemoryPoolItemBasePtr& poolItem, SMMemoryPoolItemId id, uint64_t nodeId, SMPoolDataTypeDesc dataType, uint64_t smId)
    {
    if (id == SMMemoryPool::s_UndefinedPoolItemId)
        return;
    std::lock_guard<std::mutex> lock(*m_memPoolItemMutex[id]);
    SMMemoryPoolItemBasePtr memItemPtr(m_memPoolItems[id]);

    if (memItemPtr.IsValid() && memItemPtr->IsCorrect(nodeId, dataType,smId))
        {
        m_currentPoolSizeInBytes -= memItemPtr->GetSize();
        memItemPtr = 0;
        m_memPoolItems[id] = poolItem;
        m_currentPoolSizeInBytes += m_memPoolItems[id]->GetSize();
        m_lastAccessTime[id] = clock();
        return;
        }
    }

SMMemoryPoolItemId SMMemoryPool::AddItem(SMMemoryPoolItemBasePtr& poolItem)
    {    
    uint64_t itemInd = 0;            
    clock_t oldestTime = numeric_limits<clock_t>::max();
    uint64_t oldestInd = 0; 

    m_currentPoolSizeInBytes += poolItem->GetSize();
    
    bool needToFlush = false;

    if (m_currentPoolSizeInBytes > m_maxPoolSizeInBytes)
        {
        needToFlush = true;
        }

    //clock_t currentTime = clock(); 

    for (; itemInd < (uint64_t)m_memPoolItems.size(); itemInd++)
        {
            {   
            std::lock_guard<std::mutex> lock(*m_memPoolItemMutex[itemInd]);

            if (!m_memPoolItems[itemInd].IsValid())
                {
                break;
                }
            }

            /*
        if ((needToFlush && (currentTime - m_lastAccessTime[itemInd] > s_timeDiff)))                
            {
            break;
            } */               

        if (oldestTime > m_lastAccessTime[itemInd])
            {
            oldestTime = m_lastAccessTime[itemInd];
            oldestInd = itemInd;
            }
        }               

    if (m_currentPoolSizeInBytes > m_maxPoolSizeInBytes)
        {
        double flushTimeThreshold = (clock() + oldestTime) / 2.0;

        for (size_t itemIndToDelete = 0; itemIndToDelete < m_memPoolItems.size() && m_currentPoolSizeInBytes > m_maxPoolSizeInBytes; itemIndToDelete++)
            {     
            std::lock_guard<std::mutex> lock(*m_memPoolItemMutex[itemIndToDelete]);

            if (m_memPoolItems[itemIndToDelete].IsValid() && m_lastAccessTime[itemIndToDelete] < flushTimeThreshold && m_memPoolItems[itemIndToDelete]->GetRefCount() == 1)                    
                {
                m_currentPoolSizeInBytes -= m_memPoolItems[itemIndToDelete]->GetSize();                                        
                m_memPoolItems[itemIndToDelete] = 0; 
                itemInd = itemIndToDelete;
                }                                        
            }
        }

    if (m_currentPoolSizeInBytes > m_maxPoolSizeInBytes * s_maxMemBeforeFlushing)
        {
        for (size_t itemIndToDelete = 0; itemIndToDelete < m_memPoolItems.size() && m_currentPoolSizeInBytes > m_maxPoolSizeInBytes; itemIndToDelete++)
            {  
            std::lock_guard<std::mutex> lock(*m_memPoolItemMutex[itemIndToDelete]);                    

            if (m_memPoolItems[itemIndToDelete].IsValid() && m_memPoolItems[itemIndToDelete]->GetRefCount() == 1)
                {
                m_currentPoolSizeInBytes -= m_memPoolItems[itemIndToDelete]->GetSize();                
                m_memPoolItems[itemIndToDelete] = 0; 
                itemInd = itemIndToDelete;
                }                    
            }
        }            

    
    if (itemInd == m_memPoolItems.size())
        {
        //NEEDS_WORK_SM : Replace by vector of vector
        if (m_currentPoolSizeInBytes < m_maxPoolSizeInBytes)
            {                      
            for (size_t itemId = 0; itemId < m_memPoolItemMutex.size(); itemId++)
                {
                m_memPoolItemMutex[itemId]->lock();                
                }

            m_memPoolItems.resize((size_t)(m_memPoolItems.size() * 1.5));
            m_lastAccessTime.resize(m_memPoolItems.size());

            size_t oldItemCount = m_memPoolItemMutex.size();
            m_memPoolItemMutex.resize(m_memPoolItems.size());

            for (size_t itemId = oldItemCount; itemId < m_memPoolItemMutex.size(); itemId++)
                {
                m_memPoolItemMutex[itemId] = new mutex;                
                }

            for (size_t itemId = 0; itemId < oldItemCount; itemId++)
                {
                m_memPoolItemMutex[itemId]->unlock();                
                }
            }
        else
            {
            itemInd = oldestInd;                    
            }                
        }
    
    m_memPoolItemMutex[itemInd]->lock();

    if (m_memPoolItems[itemInd].IsValid())            
        {                
        m_currentPoolSizeInBytes -= m_memPoolItems[itemInd]->GetSize();
        }

    m_memPoolItems[itemInd] = poolItem;
    m_lastAccessTime[itemInd] = clock();

    poolItem->SetPoolItemId(itemInd);            

    m_memPoolItemMutex[itemInd]->unlock();

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

    return itemInd;
    }

void SMMemoryPool::NotifySizeChangePoolItem(SMMemoryPoolItemBase* poolItem, int64_t sizeDelta)
    {
    assert(poolItem->GetPoolItemId() != SMMemoryPool::s_UndefinedPoolItemId);

    std::lock_guard<std::mutex> lock(*m_memPoolItemMutex[poolItem->GetPoolItemId()]);    

    if (m_memPoolItems[poolItem->GetPoolItemId()].get() == poolItem)
        {
        m_currentPoolSizeInBytes += sizeDelta;
        }
    }

END_BENTLEY_SCALABLEMESH_NAMESPACE