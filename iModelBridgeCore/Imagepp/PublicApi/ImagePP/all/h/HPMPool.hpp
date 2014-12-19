//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HPMPool.hpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HPMPool
//-----------------------------------------------------------------------------

#include "HFCMonitor.h"

/**----------------------------------------------------------------------------
Returns the actual size limit of the memory pool  being managed, which
is the limit of space to be consumed by objects (in kilobytes).
-----------------------------------------------------------------------------*/
inline size_t HPMPool::GetLimit() const
    {
    return m_SizeLimit / 1024;
    }

/**----------------------------------------------------------------------------
 Returns the actual memory, in bytes, being used by undiscarded objects
 managed by this pool.
-----------------------------------------------------------------------------*/
inline size_t HPMPool::GetActualCount() const
    {
    HFCMonitor Monitor((HPMPool*)this);
    size_t Result = m_SizeCount;
    return Result;
    }

/**----------------------------------------------------------------------------
Called when an item must be remove from the pool
-----------------------------------------------------------------------------*/
inline void HPMPool::RemoveItem(const HFCPtr<HPMPoolItem>& pi_rpItem)
    {
    HPRECONDITION(pi_rpItem != 0);
    HFCMonitor Monitor(this);

    if (pi_rpItem->m_ObjectSize > 1)  // value is 0 or 1 when not in list
        {
        m_SizeCount -= pi_rpItem->m_ObjectSize;
        pi_rpItem->m_ObjectSize = 0;
        m_Pool.erase(pi_rpItem->m_Itr);
        pi_rpItem->m_Discarded = true;
        }
    }

/**----------------------------------------------------------------------------
Verify if the memory manager is enabled
see methods AllocMemory, FreeMemory
-----------------------------------------------------------------------------*/
inline bool HPMPool::IsMemoryMgrEnabled() const
    {
    return m_MemMgr != 0;
    }

/**----------------------------------------------------------------------------
Try to free memory if possible...
If return false: the default implementation will be called
if return true: the default implementation is not called.
-----------------------------------------------------------------------------*/
inline bool HPMMemoryMgr::NeedMemory(size_t pi_DataSize, size_t pi_ObjectSize)
    {
    return false;
    }




//-----------------------------------------------------------------------------
// Inline methods for class HPMPoolItem
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// MoveToPool
//-----------------------------------------------------------------------------
inline void HPMPoolItem::MoveToPool(HPMPool* pi_pPool)
    {
    HPMPool* pPool = 0;

    HFCMonitor Monitor(GetExclusiveKey());
    if (m_pPool != pi_pPool)
        {
        if (m_pPool)
            m_pPool->RemoveItem(this);
        m_pPool = pi_pPool;
        }
    if (m_pPool)
        pPool = m_pPool;

    Monitor.ReleaseKey();

    if (pPool)
        pPool->NotifyAccess(this);
    }

//-----------------------------------------------------------------------------
// RemoveFromPool
//-----------------------------------------------------------------------------
inline void HPMPoolItem::RemoveFromPool()
    {
    HFCMonitor Monitor(GetExclusiveKey());
    if (m_pPool)
        m_pPool->RemoveItem(this);

    m_pPool = 0;
    }

//-----------------------------------------------------------------------------
// NotifyPool
//-----------------------------------------------------------------------------
inline void HPMPoolItem::NotifyPool()
    {
    HPMPool* pPool = 0;

    HFCMonitor Monitor(GetExclusiveKey());
    if (m_pPool)
        pPool = m_pPool;
    Monitor.ReleaseKey();

    if (pPool)
        pPool->NotifyAccess(this);
    }

//-----------------------------------------------------------------------------
// GetPool
//-----------------------------------------------------------------------------
inline HPMPool* HPMPoolItem::GetPool() const
    {
    return m_pPool;
    }

//-----------------------------------------------------------------------------
// Discard
//-----------------------------------------------------------------------------
inline void HPMPoolItem::Discard()
    {
    HPRECONDITION(m_pPool != 0 && !m_Discarded);
    m_pPool->RemoveItem(this);
    }

//-----------------------------------------------------------------------------
// HasInPool
//-----------------------------------------------------------------------------
inline bool HPMPoolItem::HasInPool() const
    {
    return (m_ObjectSize > 1);
    }

//-----------------------------------------------------------------------------
// protected section
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// constructor
//-----------------------------------------------------------------------------
inline HPMPoolItem::HPMPoolItem(HPMPool* pi_pPool)
    : m_pPool(pi_pPool),
      m_ObjectSize(0),
      m_Discarded(false)
    {
    }
