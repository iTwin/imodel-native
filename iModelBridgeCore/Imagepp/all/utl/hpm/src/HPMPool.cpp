//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hpm/src/HPMPool.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HPMPool.h>

//Debug Trace
#ifdef __HMR_DEBUG
//NOT_NOW #define __HPMMEMORY_KEEPLAST_TRACE
//NOT_NOW #define __HPMMEMORY_EXPORT_TRACE
#endif

// Global variable initialization

//-----------------------------------------------------------------------------
// class HPMPoolItem
//-----------------------------------------------------------------------------
HPMPoolItem::~HPMPoolItem()
    {
    // In theory, this should have been performed before as the
    // Discard method is virtual, calling it in the destructor prevents the
    // call of the proper override

//    if (!IsDiscarded())
//        {
//        HASSERT(!Pinned() || !"Pool item cannot be pinned upon destruction");
//        HASSERT(!"Discard should have been called prior to reaching this point.");
//        Discard();
//        }
    }

//-----------------------------------------------------------------------------
// SetDirty
//-----------------------------------------------------------------------------
// void HPMPoolItem::SetDirty(bool dirty) const 
//    {
//    m_Dirty = dirty;
//    }
    

//-----------------------------------------------------------------------------
// Discard
// Normally this method is overriden by descendant.
//-----------------------------------------------------------------------------
//bool HPMPoolItem::Discard()
//    {
//    HPRECONDITION(m_pPool != 0 && !m_Discarded);
//    m_pPool->RemoveItem(this);
//    return true;
//    } 







//-----------------------------------------------------------------------------
// class HPMPool
//-----------------------------------------------------------------------------
/**----------------------------------------------------------------------------
 Constructor for this class.  Require limit parameters for memory management.
 Default: unlimited objects (value is zero).

 @param pi_PoolLimit Specifies the limit, in kilobytes, of the memory to be
                     consumed by objects managed by loaders associated to this
                     pool.
-----------------------------------------------------------------------------*/
HPMPool::HPMPool(size_t pi_PoolLimit, MemoryMgrType pi_MemoryMgrType)
: HFCExclusiveKey()
{
    HPRECONDITION(pi_PoolLimit < (SIZE_MAX / 1024));  
    m_SizeLimit = pi_PoolLimit * 1024;
    m_SizeCount = 0;
    
    switch (pi_MemoryMgrType)
    {
        case ExportRaster: // Export
            m_MemoryManager = new HPMMemoryMgrExport(m_SizeLimit, this);
            break;

        case KeepLastBlock: // Normal pool            
            m_MemoryManager = new HPMMemoryMgrKeepLastEntry(m_SizeLimit, this);
            break;
        
        case None:
        default:
            m_MemoryManager = 0;
            break;
    }
}


HPMPool::HPMPool(size_t pi_PoolLimit, IHPMMemoryManager* memoryManager)
    : HFCExclusiveKey()
    {
    HPRECONDITION(pi_PoolLimit < (SIZE_MAX / 1024));
    m_SizeLimit = pi_PoolLimit * 1024;
    m_SizeCount = 0;

    m_MemoryManager = memoryManager;


    HINVARIANTS;
    }

/**----------------------------------------------------------------------------
 Destructor for this class.
-----------------------------------------------------------------------------*/
HPMPool::~HPMPool()
    {
    HASSERT(m_Pool.size() == 0);
//    HASSERT(m_SizeCount == 0);

//    while (m_Pool.size() > 0)
//        {
//        m_Pool.front()->Discard(); 
//        }
    
    }


/**----------------------------------------------------------------------------
 Called by loader when it has created a smart pointer.
-----------------------------------------------------------------------------*/
void HPMPool::ValidateInvariants() const
    {
    // Validation of invariants is mainly validating the invariants of the
    // managed pool items.
//    size_t totalSize = 0L;
//    PoolList::const_iterator itr = m_Pool.begin();
//    for ( ; itr != m_Pool.end() ; itr++)
//        {
//        HASSERT(!(*itr)->IsDiscarded());
//        (*itr)->ValidateInvariants();
//        totalSize += (*itr)->GetMemorySize();
//        }
//    HASSERT(m_SizeCount == totalSize);

    }



/**----------------------------------------------------------------------------
 Called by loader when it has created a smart pointer.
-----------------------------------------------------------------------------*/
void HPMPool::NotifyAccess(const HFCPtr<HPMPoolItem>& pi_rpItem)
    {
    HPRECONDITION(pi_rpItem != 0);
    HFCMonitor Monitor(this);

    // If there is no size limitations to the pool then MRU list is not maintained.
    if (!m_SizeLimit)
        return;

    // When entry is not already in list, size count is incremented;
    // otherwise it is refreshed in case size has changed.
    if (pi_rpItem->GetPoolManaged()) 
        {
        m_SizeCount -= pi_rpItem->GetMemorySize();
        m_Pool.erase(pi_rpItem->GetPoolIterator());  // removing to replace to front of the list
        }

    pi_rpItem->UpdateCachedSize();
    m_SizeCount += pi_rpItem->GetMemorySize();

    // Updating the list
    m_Pool.push_front(pi_rpItem);
    pi_rpItem->SetPoolIterator(m_Pool.begin());
    pi_rpItem->SetDiscarded(false);


    // Because we just updated the size we may be busting the limit.
    // Removing entries from the end of the list while the size counts exceeds
    // the limit.  Discard notifications are sent.  List is kept to a minimum
    // of one entry to avoid infinite looping when limit is too low.


    HFCPtr<HPMPoolItem> pItem;
    while ((m_SizeCount > m_SizeLimit) && (m_Pool.size() > 1))
        {
        pItem = m_Pool.back();
        m_Pool.pop_back();  // remove object from pool
        m_SizeCount -= pItem->GetMemorySize();

        pItem->SetDiscarded(true);
        pItem->m_ObjectSize = 0;
        pItem = 0;  // release the object
        }
        
    HINVARIANTS;

    }

/**----------------------------------------------------------------------------
 Changes the limit of memory resource consumption of objects managed by
 this pool.  If the limit is decreased, objects will be discarded from
 memory until the new limit has been reached.

 @param pi_PoolLimit Specifies the limit, in kilobytes, of the memory to be
                     consumed by objects managed by loaders associated to this
                     pool.
-----------------------------------------------------------------------------*/
bool HPMPool::ChangeLimit(size_t pi_NewPoolLimit)
    {
    HPRECONDITION(pi_NewPoolLimit < (SIZE_MAX / 1024));
    HFCMonitor Monitor(this);
    m_SizeLimit = pi_NewPoolLimit * 1024;

    // Removing entries from the end of the list while the size counts exceeds
    // the limit.  Discard notifications are sent.  List is kept to a minimum
    // of one entry to avoid infinite looping when limit is too low.
    HFCPtr<HPMPoolItem> pItem;
    while ((m_SizeCount > m_SizeLimit) && (m_Pool.size() > 1))
        {
        pItem = m_Pool.back();
        m_Pool.pop_back();  // remove object from pool
        m_SizeCount -= pItem->GetMemorySize();
        pItem->SetDiscarded(true);
        pItem->m_ObjectSize = 0;
        pItem = 0;
        }

    HINVARIANTS;

    return true;
    }



bool HPMPool::NeedMemory(size_t pi_DataSize, size_t pi_ObjectSize)
    {
    HINVARIANTS;

#if (1)
    if (m_MemoryManager != 0)
        m_MemoryManager->NeedMemory(pi_DataSize, pi_ObjectSize);

    return true;
#else
    HFCMonitor Monitor(this);

    if (m_SizeLimit != 0 && m_SizeCount > 0)
        {
        // Check if memory limit attained
        while (m_SizeCount + pi_DataSize > m_SizeLimit)
            {

            HFCPtr<HPMPoolItem> pItem = m_Pool.back();
            m_Pool.erase(pItem->GetPoolIterator());

            m_SizeCount -= pItem->GetMemorySize();
            Monitor.ReleaseKey();
            pItem->SetDiscarded(true);
            pItem->m_ObjectSize = 0;
            pItem = 0;  // release the object
            }

        return true;
        }
    else
        {
        return false;
        }    
#endif
    }

/**----------------------------------------------------------------------------
 Default pool MRU based pool allocation
-----------------------------------------------------------------------------*/
// TBD RENAME ALLOCATE
Byte* HPMPool::AllocMemory(size_t pi_MemorySize)
    {
    HFCMonitor Monitor(this);

    if (m_MemoryManager != 0)
        return m_MemoryManager->AllocMemory(pi_MemorySize);
    else
        {
        HASSERT(false);
        return new Byte[pi_MemorySize];
        }
    }

void HPMPool::FreeMemory(Byte* pi_MemPtr, size_t pi_MemorySize)
    {
    HFCMonitor Monitor(this);

    if (m_MemoryManager != 0)
        m_MemoryManager->FreeMemory(pi_MemPtr, pi_MemorySize);
    else
        {
        HASSERT(false);
        delete[] pi_MemPtr;
        }
    }


/**----------------------------------------------------------------------------
 Unpins the item ... The memory will be immediately deleted if the object has been
 discarded while pinned.
-----------------------------------------------------------------------------*/
//void HPMPoolItem::UnPin() const
//    {
//    if (Pinned())
//        {
//        m_Pinned = false;
        // Delayed discard
//        if (IsDiscarded())
//            {
//            if (m_Memory != NULL)
//                m_pPool->FreeRemovedItemMemory(*this);
//            }
//        }
//    }

/**----------------------------------------------------------------------------
Try to free memory if possible...
If return false: the default implementation will be called
if return true: the default implementation is not called.
-----------------------------------------------------------------------------*/
bool IHPMMemoryManager::NeedMemory(size_t pi_DataSize, size_t pi_ObjectSize)
    {
    return false;
    }



/**----------------------------------------------------------------------------
 Constructor for this class.  Require limit parameters for memory management.
 Default: unlimited objects (value is zero).

 @param pi_PoolLimit Specifies the limit, in kilobytes, of the memory to be
                     consumed by objects managed by loaders associated to this
                     pool.
-----------------------------------------------------------------------------*/
HPMKeepLastBlockPool::HPMKeepLastBlockPool(size_t pi_PoolLimit)
    : HPMPool(pi_PoolLimit, NULL)
    {
    // Although the ancester constructor specified a NULL for memory manager we
    // instead want one that unfortunately required this which implies construction
    // had to be started to obtain.
    m_MemoryManager = new HPMMemoryMgrReuseAlreadyAllocatedBlocks(20);

    }

/**----------------------------------------------------------------------------
 Destructor for this class.
-----------------------------------------------------------------------------*/
HPMKeepLastBlockPool::~HPMKeepLastBlockPool()
    {
    }



/**----------------------------------------------------------------------------
 Constructor for this class.  Require limit parameters for memory management.
 Default: unlimited objects (value is zero).

 @param pi_PoolLimit Specifies the limit, in kilobytes, of the memory to be
                     consumed by objects managed by loaders associated to this
                     pool.
-----------------------------------------------------------------------------*/
HPMExportPool::HPMExportPool(size_t pi_PoolLimit)
    : HPMPool(pi_PoolLimit, NULL)
    {
    // Although the ancester constructor specified a NULL for memory manager we
    // instead want one that unfortunately required this which implies construction
    // had to be started to obtain.
    m_MemoryManager = new HPMMemoryMgrExport(pi_PoolLimit, this);
    }

/**----------------------------------------------------------------------------
 Destructor for this class.
-----------------------------------------------------------------------------*/
HPMExportPool::~HPMExportPool()
    {
    }

bool HPMExportPool::NeedMemory(size_t pi_DataSize, size_t pi_ObjectSize)
    {
    HINVARIANTS;

    if (m_MemoryManager != 0)
        m_MemoryManager->NeedMemory(pi_DataSize, pi_ObjectSize);

    return true;
    }

#if (0)
/**----------------------------------------------------------------------------
 Allocator for this export oriented pool
-----------------------------------------------------------------------------*/
bool HPMExportPool::AllocMemory(size_t pi_MemorySize, const HPMPoolItem& poolItem)
    {
    // The pool item may not be currently pool managed
    HPRECONDITION (!poolItem.GetPoolManaged());

    HFCMonitor Monitor(this);

    // The main characteristic of this pool is that it expects to have a single
    // object of any given size managed and allocated at the same time. From this
    // principle, allocation requires, if the memory limit is attained that
    // the memory block of the same size which is requested be freed upon allocation of
    // the memory required by the item
    
    // First check if it is possible to allocate this new block (is the limit already attained)
    // Check if memory limit attained
    if (m_SizeCount + pi_MemorySize > m_SizeLimit)
        {
        // Some meory must be freed ... first try to find an item which uses the same memory size
        PoolList::reverse_iterator foundItem = m_Pool.rend();
        PoolList::reverse_iterator itrItem;
        for (itrItem = m_Pool.rbegin() ; itrItem != m_Pool.rend() ; itrItem--)
            {
            if ((*itrItem)->GetMemorySize() == pi_MemorySize)
                foundItem = itrItem;
            }

        if (foundItem == m_Pool.rend())
            {
            // No block of same size found ... we instead try to locate the block that is the closest in size
            // but is still greater than requested memory
            for (itrItem = m_Pool.rbegin() ; itrItem != m_Pool.rend() ; itrItem--)
                {
                if ((*itrItem)->GetMemorySize() >= pi_MemorySize)
                    {
                    // It is greater than required ... 
                    if (foundItem == m_Pool.rend())
                        {
                        // No current item ... 
                        }
                    else if ((*itrItem)->GetMemorySize() < (*foundItem)->GetMemorySize())
                        {
                        // Both are greater but new is smaller than current .. it becomes out new candidate
                        foundItem = itrItem;
                        }
                    }
                }
            }

        if (foundItem == m_Pool.rend())
            {
            // No item found ... we will use brute force and used the default strategy of the ancester which
            // is deallocate according to MRU
            return HPMPool::AllocMemory (pi_MemorySize, poolItem);
            }

        
        // At this point ... we have an adequate item ready to be discarded which we do
        if (!(*foundItem)->Discard())
            {
            HASSERT("A problem occured trying to discard an item from pool");
            return false;
            }
        }

    // We allocated memory
    Byte* pMemory;
    size_t actualAllocatedSize = pi_MemorySize;
    size_t actualAllocatedMemory = 0;
    if (m_MemoryManager != NULL)
        {
        pMemory = m_MemoryManager->AllocMemoryExt (pi_MemorySize, actualAllocatedMemory);
        actualAllocatedSize = actualAllocatedMemory; // Trunction of remainder is intended
        HASSERT (actualAllocatedSize >= pi_MemorySize);
        }
    else
        pMemory = new Byte[pi_MemorySize];

    HASSERT(pMemory != NULL);

    poolItem.SetMemory(pMemory, actualAllocatedSize);
    m_SizeCount += actualAllocatedSize;

    m_Pool.push_front(const_cast<HPMPoolItem*>(&poolItem));
    poolItem.SetPoolIterator (m_Pool.begin());
    poolItem.SetPoolManaged(true);
    return true;
    }

#endif


/**----------------------------------------------------------------------------
 Allocates memory ... indicates the actual size allocated if greater.
-----------------------------------------------------------------------------*/
Byte* IHPMMemoryManager::AllocMemoryExt(size_t pi_MemorySize, size_t& po_ActualMemory)
    {
    Byte* temp = AllocMemory(pi_MemorySize);
    po_ActualMemory = pi_MemorySize;
    return temp;
    }


//-----------------------------------------------------------------------------
// class HPMMemoryMgrExport
//-----------------------------------------------------------------------------
#ifdef __HPMMEMORY_EXPORT_TRACE
static FILE* sFileTraceExport=0;
#endif

HPMMemoryMgrExport::HPMMemoryMgrExport(size_t pi_MemorySize, HPMPool* pi_pPool)
    : IHPMMemoryManager()
    {
    HPRECONDITION(pi_pPool != 0);

    m_pPool = pi_pPool;

#ifdef __HPMMEMORY_EXPORT_TRACE
    sFileTraceExport = fopen("k:\\HPMMEM_Export.txt","a");
#endif

    memset(m_MemMgrList, 0, sizeof(m_MemMgrList));
    Byte* MemPtr;
    try
        {
        MemPtr = new Byte[pi_MemorySize];
        }
    catch(...)
        {
        MemPtr = 0;
        }
    if (MemPtr == 0)
        {
        // the MemMgr will use system memory call
        m_IndexFreeBlock = -1;
        m_NextEmptyBlock = 0;

#ifdef __HPMMEMORY_EXPORT_TRACE
        fprintf(sFileTraceExport, "Constructor, %ld, 0, 0\n", pi_MemorySize);
#endif
        HASSERT(false);
        }
    else
        {
        m_IndexFreeBlock = 0;
        m_NextEmptyBlock = 1;

        m_MemMgrList[0].Offset = MemPtr;
        m_MemMgrList[0].Size   = pi_MemorySize;
        m_MemMgrList[0].State = MemFree;

#ifdef __HPMMEMORY_EXPORT_TRACE
        fprintf(sFileTraceExport, "Constructor, %ld, 0, 1\n", pi_MemorySize);
#endif
        }
    }

HPMMemoryMgrExport::~HPMMemoryMgrExport()
    {
#ifdef __HPMMEMORY_EXPORT_TRACE
    fprintf(sFileTraceExport, "Destructor\n");
#endif

    if (m_IndexFreeBlock != -1)
        delete[] m_MemMgrList[0].Offset;

    for (int i=0; i<MgrExportMaxEntry; ++i)
        {
        if (m_MemMgrList[i].State == SysAllocated ||
            m_MemMgrList[i].State == SysFree)
            {
#ifdef __HPMMEMORY_EXPORT_TRACE
            fprintf(sFileTraceExport, "Free, 0, %ld, %ld\n", i, m_MemMgrList[i].Size);
#endif
            delete[] m_MemMgrList[i].Offset;
            }
        }
#ifdef __HPMMEMORY_EXPORT_TRACE
    fclose (sFileTraceExport);
#endif
    }

bool HPMMemoryMgrExport::NeedMemory(size_t pi_DataSize, size_t pi_ObjectSize)
    {
    bool MemAlreadyAllocated;
    HFCMonitor Monitor(m_pPool);

    FindEntry(pi_DataSize, MemAlreadyAllocated);

    // If already allocated, try to free it, normally we should need only one block of each size here.
    if (MemAlreadyAllocated)
        {
        HPMPool::Iterator Itr;

        for (HPMPool::Iterator Itr=m_pPool->m_Pool.begin(); Itr != m_pPool->m_Pool.end(); Itr++)
            {
            if (pi_ObjectSize == (*Itr)->m_ObjectSize)
                {
                HFCPtr<HPMPoolItem> pItem = *Itr;
                m_pPool->m_Pool.erase(Itr);

                m_pPool->m_SizeCount -= pItem->m_ObjectSize;
                Monitor.ReleaseKey();
                pItem->m_Discarded = true;
                pItem->m_ObjectSize = 0;
                pItem = 0;  // release the object
                break;
                }
            }

#ifdef __HPMMEMORY_EXPORT_TRACE
        fprintf(sFileTraceExport, "NeedMem, %ld, 0, 1\n", pi_DataSize);
#endif
        return true;
        }
    else
        {
#ifdef __HPMMEMORY_EXPORT_TRACE
        fprintf(sFileTraceExport, "NeedMem, %ld, 0, 0\n", pi_DataSize);
#endif
        return false;
        }
    }

Byte* HPMMemoryMgrExport::AllocMemory(size_t pi_MemorySize)
    {
    Byte* pMemory = 0;
    bool    MemAlreadyAllocated;

    int BestBlockToAlloc = FindEntry(pi_MemorySize, MemAlreadyAllocated);

    // No memory available, try to alloc system memory
    if (BestBlockToAlloc == -1)
        {
        pMemory = new Byte[pi_MemorySize];

        if (MemAlreadyAllocated)
            {
            // Normally we must have only one block of each size here...
            //  except for some cases with a model with many decimal, in this case the double error causes to
            //  reload some tiles already treated.
            // In this case, the block will not be put in the list
#ifdef __HPMMEMORY_EXPORT_TRACE
            fprintf(sFileTraceExport, "Alloc, %ld, %0Ix, 2\n", pi_MemorySize, pMemory);
#endif
            }
        else
            {
            m_MemMgrList[m_NextEmptyBlock].State        = SysAllocated;
            m_MemMgrList[m_NextEmptyBlock].Size         = pi_MemorySize;
            m_MemMgrList[m_NextEmptyBlock].Offset       = pMemory;

            ++m_NextEmptyBlock;
            HASSERT(m_NextEmptyBlock < MgrExportMaxEntry);

#ifdef __HPMMEMORY_KEEPLAST_TRACE
            fprintf(sFileTraceExport, "Alloc, %ld, 0, 0\n", pi_MemorySize);
#endif
            }

        goto ExitMethod;
        }

    // Alloc in our memory block
    if (m_MemMgrList[BestBlockToAlloc].Size == pi_MemorySize)
        {
        m_MemMgrList[BestBlockToAlloc].State = (m_MemMgrList[BestBlockToAlloc].State == MemFree ? MemAllocated : SysAllocated);
        pMemory = m_MemMgrList[BestBlockToAlloc].Offset;
        }
    else
        {
        size_t MemSize = (pi_MemorySize + 7) & ~7;      // 8 byte boundary

        // Create a new empty block
        m_MemMgrList[m_NextEmptyBlock].Offset = m_MemMgrList[BestBlockToAlloc].Offset + MemSize;
        m_MemMgrList[m_NextEmptyBlock].Size   = m_MemMgrList[BestBlockToAlloc].Size - MemSize;
        m_IndexFreeBlock = m_NextEmptyBlock;
        ++m_NextEmptyBlock;
        HASSERT(m_NextEmptyBlock < MgrExportMaxEntry);

        m_MemMgrList[BestBlockToAlloc].State        = MemAllocated;
        m_MemMgrList[BestBlockToAlloc].Size         = pi_MemorySize;

        pMemory = m_MemMgrList[BestBlockToAlloc].Offset;
        }
#ifdef __HPMMEMORY_KEEPLAST_TRACE
    fprintf(sFileTraceExport, "Alloc, %ld, 0, 1\n", pi_MemorySize);
#endif

ExitMethod:
    return pMemory;
    }

void HPMMemoryMgrExport::FreeMemory(Byte* pi_MemPtr, size_t pi_MemorySize)
    {
    if (pi_MemPtr != 0)
        {
        int i=0;
        for(; i<MgrExportMaxEntry; ++i)
            {
            if (m_MemMgrList[i].Offset == pi_MemPtr)
                {
                HASSERT(pi_MemorySize == m_MemMgrList[i].Size);

                m_MemMgrList[i].State = (m_MemMgrList[i].State == MemAllocated ? MemFree : SysFree);
#ifdef __HPMMEMORY_KEEPLAST_TRACE
                fprintf(sFileTraceExport, "Free , %ld, %ld, 1\n", pi_MemorySize, i);
#endif
                break;
                }
            }

        // If we didn't find the block in the list, certainly allocated by us but not put in the list.
        // See the special case in AllocMemory
        if (i >= MgrExportMaxEntry)
            {
#ifdef __HPMMEMORY_KEEPLAST_TRACE
            fprintf(sFileTraceExport, "Free , %ld, %0Ix, 2\n", pi_MemorySize, pi_MemPtr);
#endif
            delete[] pi_MemPtr;
            }
        }
    }

int HPMMemoryMgrExport::FindEntry(size_t pi_MemorySize, bool& po_MemAlreadyAllocated)
    {
    int BestBlockToAlloc = -1;

    po_MemAlreadyAllocated = false;

    // Check if the block already allocated
    for (int i=0; i<MgrExportMaxEntry; ++i)
        {
        if (m_MemMgrList[i].Size == pi_MemorySize)
            {
            if (m_MemMgrList[i].State == MemAllocated || m_MemMgrList[i].State == SysAllocated)
                {
                po_MemAlreadyAllocated = true;
                break;
                }
            else
                {
                // Free block same size OK
                BestBlockToAlloc = i;
                break;
                }
            }
        }

    // If not already allocated and our free block is large enough
    if(!po_MemAlreadyAllocated && BestBlockToAlloc == -1 && m_IndexFreeBlock != -1 && m_MemMgrList[m_IndexFreeBlock].Size >= pi_MemorySize)
        BestBlockToAlloc = m_IndexFreeBlock;

    return BestBlockToAlloc;
    }


//-----------------------------------------------------------------------------
// class HPMMemoryMgrKeepLastEntry
//-----------------------------------------------------------------------------

#ifdef __HPMMEMORY_KEEPLAST_TRACE
static FILE* sFileTraceKeepLast=0;
#endif
HPMMemoryMgrKeepLastEntry::HPMMemoryMgrKeepLastEntry(size_t pi_MemorySize, HPMPool* pi_pPool)
    : IHPMMemoryManager()
    {
    HPRECONDITION(pi_pPool != 0);

    m_pPool         = pi_pPool;
    m_Indexfree     = 0;
    memset(m_MemMgrList, 0, sizeof(m_MemMgrList));

#ifdef __HPMMEMORY_KEEPLAST_TRACE
    sFileTraceKeepLast = fopen("k:\\HPMMEM_KeepLast.txt","a");
#endif
    }

HPMMemoryMgrKeepLastEntry::~HPMMemoryMgrKeepLastEntry()
    {
#ifdef __HPMMEMORY_KEEPLAST_TRACE
    fprintf(sFileTraceKeepLast, "Destructor\n");
#endif

    // cleanup the memory cached by the MemoryManager
    for (int i=0; i<MgrKeepLastEntryMaxEntry; ++i)
        {
        if (m_MemMgrList[i].Offset != 0)
            {
#ifdef __HPMMEMORY_KEEPLAST_TRACE
            fprintf(sFileTraceKeepLast, "D-Free, 0, %ld, %ld\n", i, m_MemMgrList[i].Size);
#endif
            delete[] m_MemMgrList[i].Offset;
            }
        }

#ifdef __HPMMEMORY_KEEPLAST_TRACE
    fclose(sFileTraceKeepLast);
#endif
    }

Byte* HPMMemoryMgrKeepLastEntry::AllocMemory(size_t pi_MemorySize)
    {
    int     Entry = -1;
    for (int i=0; i<MgrKeepLastEntryMaxEntry; ++i)
        {
        if (m_MemMgrList[i].Size == pi_MemorySize)
            {
            Entry = i;
            break;
            }
        }

    if (Entry == -1)
        {
#ifdef __HPMMEMORY_KEEPLAST_TRACE
        fprintf(sFileTraceKeepLast, "Alloc, %ld, 0, 0\n", pi_MemorySize);
#endif
        return 0;
        }
    else
        {
        Byte* MemBlock = m_MemMgrList[Entry].Offset;
        m_MemMgrList[Entry].Offset = 0;
        m_MemMgrList[Entry].Size = 0;

#ifdef __HPMMEMORY_KEEPLAST_TRACE
        fprintf(sFileTraceKeepLast, "Alloc, %ld, 0, 1\n", pi_MemorySize);
#endif
        return MemBlock;
        }
    }

void HPMMemoryMgrKeepLastEntry::FreeMemory(Byte* pi_MemPtr, size_t pi_MemorySize)
    {
    if (pi_MemPtr != 0)
        {
        int     Entry = -1;
        for (int i=0; i<MgrKeepLastEntryMaxEntry; ++i)
            {
            if (m_MemMgrList[i].Offset == 0)
                {
                Entry = i;
                break;
                }
            }

        // If no free entry found, used the default one
        if (Entry == -1)
            {
            Entry = m_Indexfree;

            if (m_MemMgrList[Entry].Offset != 0)
                delete[] m_MemMgrList[Entry].Offset;

#ifdef __HPMMEMORY_KEEPLAST_TRACE
            fprintf(sFileTraceKeepLast, "Free, %ld, %ld, %ld\n", pi_MemorySize, Entry, m_MemMgrList[Entry].Size);
#endif
            }
#ifdef __HPMMEMORY_KEEPLAST_TRACE
        else
            fprintf(sFileTraceKeepLast, "Free, %ld, 0, 0\n", pi_MemorySize);
#endif

        m_Indexfree = (Entry+1) % MgrKeepLastEntryMaxEntry;
        m_MemMgrList[Entry].Offset  = pi_MemPtr;
        m_MemMgrList[Entry].Size = pi_MemorySize;
        }
    }



HPMMemoryMgrReuseAlreadyAllocatedBlocks::HPMMemoryMgrReuseAlreadyAllocatedBlocks(size_t maxFreeBlocks)
    : IHPMMemoryManager()
    {
    m_maxFreeBlocks = maxFreeBlocks;

#ifdef __HMR_DEBUG
    m_allocCount = 0;
    m_deallocCount = 0;
    m_realAllocCount = 0;
    m_realDeallocCount = 0;
#endif

    }

HPMMemoryMgrReuseAlreadyAllocatedBlocks::~HPMMemoryMgrReuseAlreadyAllocatedBlocks()
    {
    HASSERT (m_UsedMemMgrList.size() == 0);

    for (size_t i = 0 ; i < m_UsedMemMgrList.size() ; i++)
        {
        delete [] m_UsedMemMgrList[i].Offset;
#ifdef __HMR_DEBUG
        m_deallocCount++;
        m_realDeallocCount++;
#endif
        }

    for (size_t i = 0 ; i < m_FreeMemMgrList.size() ; i++)
        {
        delete [] m_FreeMemMgrList[i].Offset;
#ifdef __HMR_DEBUG
        m_realDeallocCount++;
#endif
        }

#ifdef __HMR_DEBUG
    HASSERT(m_realDeallocCount == m_realAllocCount);
    HASSERT(m_deallocCount == m_allocCount);
#endif
    }


Byte* HPMMemoryMgrReuseAlreadyAllocatedBlocks::AllocMemory(size_t pi_MemorySize)
    {
    size_t dummy;
    return AllocMemoryExt (pi_MemorySize, dummy);
    }


Byte* HPMMemoryMgrReuseAlreadyAllocatedBlocks::AllocMemoryExt(size_t pi_MemorySize, size_t& po_ActualSize)
    {

    if (pi_MemorySize == 0)
        return NULL;

#ifdef __HMR_DEBUG
    m_allocCount++;

    // For the purpose of debugging we increase the requested size by 4 bytes
    pi_MemorySize += 2;
#endif

    Byte* pMemory = 0;
    // Search through free blocks
    vector<MemEntry>::iterator itr;
    vector<MemEntry>::iterator itrFound = m_FreeMemMgrList.end();
    bool oneFound = false;
    // We search for the smallest free block that can contain the requested memory (of course we stop is memory size is exactly the size requested)
    for(itr = m_FreeMemMgrList.begin() ; ((!(oneFound && itrFound->Size == pi_MemorySize)) && (itr != m_FreeMemMgrList.end())); ++itr)
        {
        // We find the smallest free block that can contain new memory
        if ((pi_MemorySize <= itr->Size) && ((!oneFound) || (itr->Size < itrFound->Size)))
            {
            itrFound = itr;
            oneFound = true;
            }
        }

    if (oneFound)
        {
        // A valid free block was found ... reactivate it
        itrFound->State = SysAllocated;
        pMemory = itrFound->Offset;

        // Note that in debug mode, the node size contains the 4 additional bytes.
        po_ActualSize = itrFound->Size;
        m_UsedMemMgrList.push_back(*itrFound);
        m_FreeMemMgrList.erase(itrFound);


        }
    else
        {
        // No free block found
        pMemory = new Byte[pi_MemorySize];

#ifdef __HMR_DEBUG
        m_realAllocCount++;
#endif
        MemEntry newEntry;
        newEntry.State = SysAllocated;
        newEntry.Size = pi_MemorySize;
        newEntry.Offset = pMemory;
        po_ActualSize = pi_MemorySize;
        m_UsedMemMgrList.push_back(newEntry);

        }


    HASSERT(po_ActualSize >= pi_MemorySize);
#ifdef __HMR_DEBUG
    m_reuseRatio = (m_allocCount - m_realAllocCount) / (double)m_allocCount;

    // Since in debug we actually allocated 4 additional bytes we diminish the reported size
    po_ActualSize -= 2;
    pMemory += 2;
#endif

    return pMemory;
    }

void HPMMemoryMgrReuseAlreadyAllocatedBlocks::FreeMemory(Byte* pi_MemPtr, size_t pi_MemorySize)
    {
    // For this implementation the second parameter is ignored
    if (pi_MemPtr != 0)
        {
        Byte* pDeallocMemory = pi_MemPtr;
#ifdef __HMR_DEBUG
        m_deallocCount++;

        
        // We adjust the location of the memory to be freed in debug
        pDeallocMemory = pDeallocMemory - 2;
        bool locatedBlock = false;;
#endif
        vector<MemEntry>::iterator itr;
        for(itr = m_UsedMemMgrList.begin() ; itr != m_UsedMemMgrList.end(); ++itr)
            {
            if (itr->Offset == pDeallocMemory)
                {
                itr->State = SysFree;
                m_FreeMemMgrList.push_back(*itr);
                m_UsedMemMgrList.erase(itr);

#ifdef __HMR_DEBUG
                locatedBlock = true;
#endif
                break;
                }
            }
#ifdef __HMR_DEBUG
        HASSERT(locatedBlock);
#endif
        }

    

    // Check if the maximum number of free blocks attained
    if (m_FreeMemMgrList.size() > m_maxFreeBlocks)
        {
        HASSERT ((m_FreeMemMgrList.size() - 1) == m_maxFreeBlocks);

        // Find the smallest block
        vector<MemEntry>::iterator itr;
        vector<MemEntry>::iterator itrFound = m_FreeMemMgrList.begin();
        for(itr = m_FreeMemMgrList.begin() ; itr != m_FreeMemMgrList.end(); ++itr)
            {
            if (itr->Size < itrFound->Size)
                {
                itrFound = itr;
                }
            }

        // Release memory then remove entry from list
        delete [] (itrFound->Offset);
#ifdef __HMR_DEBUG
        m_realDeallocCount++;
#endif
        m_FreeMemMgrList.erase(itrFound);
        }
    }


void HPMMemoryMgrReuseAlreadyAllocatedBlocks::FreeAll()
    {
    vector<MemEntry>::iterator itr;
    while (m_UsedMemMgrList.size() != 0)
        {
        FreeMemory(m_UsedMemMgrList.begin()->Offset, m_UsedMemMgrList.begin()->Size);
        }
    }



HPMMemoryMgrReuseAlreadyAllocatedBlocksWithAlignment::HPMMemoryMgrReuseAlreadyAllocatedBlocksWithAlignment(size_t maxFreeBlocks, size_t m_AlignmentSize)
    : HPMMemoryMgrReuseAlreadyAllocatedBlocks(maxFreeBlocks)
    {
    m_alignment = m_AlignmentSize;

    // The alignement must be aligned on a 4096 bytes frontier as it is the OS internal alignment
    // Compute alignment size
    if (m_alignment != 0)
        m_alignment = (((m_AlignmentSize - 1) / 4096) + 1) * 4096;

    }

HPMMemoryMgrReuseAlreadyAllocatedBlocksWithAlignment::~HPMMemoryMgrReuseAlreadyAllocatedBlocksWithAlignment()
    {
    FreeAll();
    }


// Same as ancester function except it will only reuse excactly equal block sizes.
Byte* HPMMemoryMgrReuseAlreadyAllocatedBlocksWithAlignment::AllocMemoryExt(size_t pi_MemorySize, size_t& po_ActualSize)
    {

    if (pi_MemorySize == 0)
        return NULL;

#ifdef __HMR_DEBUG
    m_allocCount++;
    pi_MemorySize += 2;
#endif


    // Compute alignment size
    if (m_alignment != 0)
        pi_MemorySize = (((pi_MemorySize - 1) / m_alignment) + 1) * m_alignment;


    Byte* pMemory = 0;
    // Search through free blocks
    vector<MemEntry>::iterator itr;
    vector<MemEntry>::iterator itrFound = m_FreeMemMgrList.end();
    bool oneFound = false;
    // We search for the smallest free block that can contain the requested memory (of course we stop is memory size is exactly the size requested)
    for(itr = m_FreeMemMgrList.begin() ; ((!oneFound) && (itr != m_FreeMemMgrList.end())); ++itr)
        {
        // We find the smallest free block that can contain new memory
        if (pi_MemorySize == itr->Size)
            {
            itrFound = itr;
            oneFound = true;
            }
        }

    if (oneFound)
        {
        // A valid free block was found ... reactivate it
        itrFound->State = SysAllocated;
        pMemory = itrFound->Offset;
        po_ActualSize = itrFound->Size;
        m_UsedMemMgrList.push_back(*itrFound);
        m_FreeMemMgrList.erase(itrFound);


        }
    else
        {
        // No free block found
        pMemory = new Byte[pi_MemorySize];

#ifdef __HMR_DEBUG
        m_realAllocCount++;
#endif
        MemEntry newEntry;
        newEntry.State = SysAllocated;
        newEntry.Size = pi_MemorySize;
        newEntry.Offset = pMemory;
        po_ActualSize = pi_MemorySize;
        m_UsedMemMgrList.push_back(newEntry);

        }
    HASSERT(po_ActualSize >= pi_MemorySize);
#ifdef __HMR_DEBUG
    m_reuseRatio = (m_allocCount - m_realAllocCount) / (double)m_allocCount;
    po_ActualSize -= 2;
    pMemory += 2;

#endif


    return pMemory;
    }

void HPMMemoryMgrReuseAlreadyAllocatedBlocksWithAlignment::FreeMemory(Byte* pi_MemPtr, size_t pi_MemorySize)
    {
    // For this implementation the second parameter is ignored
    if (pi_MemPtr != 0)
        {
        Byte* pDeallocMemory = pi_MemPtr;
#ifdef __HMR_DEBUG
        m_deallocCount++;

        // We adjust the location of the memory to be freed in debug
        pDeallocMemory = pDeallocMemory - 2;
        bool locatedBlock = false;;

#endif
        vector<MemEntry>::iterator itr;
        for(itr = m_UsedMemMgrList.begin() ; itr != m_UsedMemMgrList.end(); ++itr)
            {
            if (itr->Offset == pDeallocMemory)
                {
                itr->State = SysFree;
                m_FreeMemMgrList.push_back(*itr);
                m_UsedMemMgrList.erase(itr);
#ifdef __HMR_DEBUG
                locatedBlock = true;
#endif
                break;
                }
            }
#ifdef __HMR_DEBUG
        HASSERT(locatedBlock);
#endif

        }

    // Check if the maximum number of free blocks attained
    if (m_FreeMemMgrList.size() > m_maxFreeBlocks)
        {
        HASSERT ((m_FreeMemMgrList.size() - 1) == m_maxFreeBlocks);


        // We erase the first block (which has not been reused for a while
        vector<MemEntry>::iterator itrFound = m_FreeMemMgrList.begin();

        // Release memory then remove entry from list
        delete [] (itrFound->Offset);
#ifdef __HMR_DEBUG
        m_realDeallocCount++;
#endif
        m_FreeMemMgrList.erase(itrFound);
        }
    }