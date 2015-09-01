//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HPMPool.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HPMPool
//-----------------------------------------------------------------------------

#include "HFCMonitor.h"

BEGIN_IMAGEPP_NAMESPACE

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

    if (pi_rpItem->GetPoolManaged())  // value is 0 or 1 when not in list
        {
        m_SizeCount -= pi_rpItem->GetMemorySize();
        pi_rpItem->m_ObjectSize = 0;
        m_Pool.erase(pi_rpItem->GetPoolIterator());
        pi_rpItem->SetDiscarded(true);
        }
    else if (pi_rpItem->m_ObjectSize == 1)      // Tile voluntary created outside of the pool.
        pi_rpItem->SetDiscarded(true);
    }

/**----------------------------------------------------------------------------
Verify if the memory manager is enabled
see methods AllocMemory, FreeMemory
-----------------------------------------------------------------------------*/
inline bool HPMPool::IsMemoryMgrEnabled() const
    {
    return m_MemoryManager != 0;
    }


//-----------------------------------------------------------------------------
// Inline methods for class HPMPoolItem
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ValidateInvariants
//-----------------------------------------------------------------------------
inline void HPMPoolItem::ValidateInvariants() const
    {
    // If the item is pool managed but discarded then it must be pinned
    // HASSERT( !(m_PoolManaged && m_Discarded) || m_Pinned);
    }

//-----------------------------------------------------------------------------
// MoveToPool
//-----------------------------------------------------------------------------
inline void HPMPoolItem::MoveToPool(HPMPool* pi_pPool)
    {
    HINVARIANTS;

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
    HINVARIANTS;

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
    HINVARIANTS;

    HPMPool* pPool = 0;

    HFCMonitor Monitor(GetExclusiveKey());
    if (m_pPool)
        pPool = m_pPool;
    Monitor.ReleaseKey();

    // This check allows to call notify pool even if no pool is set
    // If no pool set then the present call does nothing.
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
inline bool HPMPoolItem::Discard()
    {
    HPRECONDITION(m_pPool != 0 && !m_Discarded);
    m_pPool->RemoveItem(this);
    return true;
    } 
    
//-----------------------------------------------------------------------------
// GetPoolManaged
//-----------------------------------------------------------------------------
inline bool HPMPoolItem::GetPoolManaged() const
    {
    return (m_ObjectSize > 1);
    }    



//-----------------------------------------------------------------------------
// SetPoolManaged
//-----------------------------------------------------------------------------
inline void HPMPoolItem::SetPoolManaged(bool pi_Managed) const
    {
   	// Currently we can only set unmanaged ...
    HASSERT(!pi_Managed);
    
    if (!pi_Managed)
    	m_ObjectSize = 0;
    	
    // m_PoolManaged = pi_Managed;
    // Do not call invariants as we are likely in an inconsisent state
    }
    
//-----------------------------------------------------------------------------
// HasInPool
//-----------------------------------------------------------------------------
inline bool HPMPoolItem::HasInPool() const
    {
    return (GetPoolManaged());    	

    }





//-----------------------------------------------------------------------------
// SetDiscarded
//-----------------------------------------------------------------------------
inline void HPMPoolItem::SetDiscarded(bool pi_Discarded) const
    {
    m_Discarded = pi_Discarded;
    }

//-----------------------------------------------------------------------------
// IsDiscarded
//-----------------------------------------------------------------------------
inline bool HPMPoolItem::IsDiscarded() const
    {
    return m_Discarded;
    }


/**----------------------------------------------------------------------------
 Returns the allocated count as the number of bytes used by memory
-----------------------------------------------------------------------------*/
inline size_t HPMPoolItem::GetMemorySize() const
    {
    return m_ObjectSize;
    }

/**----------------------------------------------------------------------------
 The Pin method inflates object into memory if required and prevents it from being
 discarded in actuality (the memory s not freed till unpinned).
-----------------------------------------------------------------------------*/
//inline bool HPMPoolItem::Pin() const // Intentionaly const as only mutable members get modified
//    {
//    HINVARIANTS;
//
//    HPRECONDITION(!IsDiscarded());
//
//    if (m_Pinned)
//        return false;
//
//    m_Pinned = true;
//    return true;
//    }

/**----------------------------------------------------------------------------
 The Pinned method indicates if the item is pinned
-----------------------------------------------------------------------------*/
//inline bool HPMPoolItem::Pinned() const
//    {
//    return m_Pinned;
//    }



/**----------------------------------------------------------------------------
 Sets the pool item iterator in list
-----------------------------------------------------------------------------*/
inline void HPMPoolItem::SetPoolIterator(HPMPool::Iterator poolIterator) const
    {
    m_PoolIterator = poolIterator;
    }

/**----------------------------------------------------------------------------
 Gets the pool item iterator in list
-----------------------------------------------------------------------------*/
inline HPMPool::Iterator HPMPoolItem::GetPoolIterator() const
    {
    return m_PoolIterator;
    }




//-----------------------------------------------------------------------------
// IsDirty
//-----------------------------------------------------------------------------
//inline bool HPMPoolItem::IsDirty() const
//    {
//    HINVARIANTS;
//
//    return m_Dirty;
//    }


//-----------------------------------------------------------------------------
// protected section
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// constructor
//-----------------------------------------------------------------------------
inline HPMPoolItem::HPMPoolItem()
    : m_pPool(NULL),
      m_ObjectSize(0),
      m_Discarded(true)
    {
    HINVARIANTS;
    }
//-----------------------------------------------------------------------------
// constructor
//-----------------------------------------------------------------------------
inline HPMPoolItem::HPMPoolItem(HPMPool* pi_pPool)
    : m_pPool(pi_pPool),
      m_ObjectSize(0),
      m_Discarded(false)
    {
    HINVARIANTS;
    }

END_IMAGEPP_NAMESPACE