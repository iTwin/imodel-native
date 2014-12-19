//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hpm/src/HPMPool.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HPMPool.h>

//Debug Trace
#ifdef __HMR_DEBUG
//NOT_NOW #define __HPMMEMORY_KEEPLAST_TRACE
//NOT_NOW #define __HPMMEMORY_EXPORT_TRACE
#endif

// Global variable initialization
HPMPool g_DefaultPool;

//-----------------------------------------------------------------------------
// class HPMPoolItem
//-----------------------------------------------------------------------------
HPMPoolItem::~HPMPoolItem()
    {
    }



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
            m_MemMgr = new HPMMemoryMgrExport(m_SizeLimit, this);
            break;

        case KeepLastBlock: // Normal pool
            m_MemMgr = new HPMMemoryMgrKeepLastEntry(m_SizeLimit, this);
            break;

        case None:
        default:
            m_MemMgr = 0;
            break;
        }
    }

/**----------------------------------------------------------------------------
 Destructor for this class.
-----------------------------------------------------------------------------*/
HPMPool::~HPMPool()
    {
    HASSERT(m_Pool.size() == 0);
    }


/**----------------------------------------------------------------------------
 Called by loader when it has created a smart pointer.
-----------------------------------------------------------------------------*/
void HPMPool::NotifyAccess(const HFCPtr<HPMPoolItem>& pi_rpItem)
    {
    HPRECONDITION(pi_rpItem != 0);
    HFCMonitor Monitor(this);
    if (!m_SizeLimit)
        return;

    // When entry is not already in list, size count is incremented;
    // otherwise it is refreshed in case size has changed.
    if (pi_rpItem->m_ObjectSize > 1)  // value is 0 or 1 when not in list
        {
        m_SizeCount -= pi_rpItem->m_ObjectSize;
        m_Pool.erase(pi_rpItem->m_Itr);  // removing to replace to front of the list
        }

    pi_rpItem->UpdateCachedSize();
    m_SizeCount += pi_rpItem->m_ObjectSize;

    // Updating the list
    m_Pool.push_front(pi_rpItem);
    pi_rpItem->m_Itr = m_Pool.begin();
    pi_rpItem->m_Discarded = false;


    // Removing entries from the end of the list while the size counts exceeds
    // the limit.  Discard notifications are sent.  List is kept to a minimum
    // of one entry to avoid infinite looping when limit is too low.

    HFCPtr<HPMPoolItem> pItem;
    while ((m_SizeCount > m_SizeLimit) && (m_Pool.size() > 1))
        {
        pItem = m_Pool.back();
        m_Pool.pop_back();  // remove object from pool
        m_SizeCount -= pItem->m_ObjectSize;
        pItem->m_Discarded = true;
        pItem->m_ObjectSize = 0;
        pItem = 0;  // release the object
        }
    }

/**----------------------------------------------------------------------------
 Changes the limit of memory resource consumption of objects managed by
 this pool.  If the limit is decreased, objects will be discarded from
 memory until the new limit has been reached.

 @param pi_PoolLimit Specifies the limit, in kilobytes, of the memory to be
                     consumed by objects managed by loaders associated to this
                     pool.
-----------------------------------------------------------------------------*/
void HPMPool::ChangeLimit(size_t pi_NewPoolLimit)
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
        m_SizeCount -= pItem->m_ObjectSize;
        pItem->m_Discarded = true;
        pItem->m_ObjectSize = 0;
        pItem = 0;
        }
    }


void HPMPool::NeedMemory(size_t pi_DataSize, size_t pi_ObjectSize)
    {
    if (m_MemMgr != 0)
        m_MemMgr->NeedMemory(pi_DataSize, pi_ObjectSize);

    }

Byte* HPMPool::AllocMemory(size_t pi_MemorySize)
    {
    HFCMonitor Monitor(this);

    if (m_MemMgr != 0)
        return m_MemMgr->AllocMemory(pi_MemorySize);
    else
        {
        HASSERT(false);
        return new Byte[pi_MemorySize];
        }
    }

void HPMPool::FreeMemory(Byte* pi_MemPtr, size_t pi_MemorySize)
    {
    HFCMonitor Monitor(this);

    if (m_MemMgr != 0)
        m_MemMgr->FreeMemory(pi_MemPtr, pi_MemorySize);
    else
        {
        HASSERT(false);
        delete[] pi_MemPtr;
        }
    }


/**----------------------------------------------------------------------------
 Allocates memory ... indicates the actual size allocated if greater.
-----------------------------------------------------------------------------*/
Byte* HPMMemoryMgr::AllocMemoryExt(size_t pi_MemorySize, size_t& po_ActualMemory)
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
    : HPMMemoryMgr()
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
    : HPMMemoryMgr()
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

    // cleanup the memory cached by the MemoryMgr
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
    : HPMMemoryMgr()
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

#ifdef __HMR_DEBUG
    m_reuseRatio = (m_allocCount - m_realAllocCount) / (double)m_allocCount;
#endif
    HASSERT(po_ActualSize >= pi_MemorySize);
    return pMemory;
    }

void HPMMemoryMgrReuseAlreadyAllocatedBlocks::FreeMemory(Byte* pi_MemPtr, size_t pi_MemorySize)
    {
    // For this implementation the second parameter is ignored
    if (pi_MemPtr != 0)
        {
#ifdef __HMR_DEBUG
        m_deallocCount++;
#endif
        vector<MemEntry>::iterator itr;
        for(itr = m_UsedMemMgrList.begin() ; itr != m_UsedMemMgrList.end(); ++itr)
            {
            if (itr->Offset == pi_MemPtr)
                {
                itr->State = SysFree;
                m_FreeMemMgrList.push_back(*itr);
                m_UsedMemMgrList.erase(itr);
                break;
                }
            }
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
    }


// Same as ancester function except it will only reuse excactly equal block sizes.
Byte* HPMMemoryMgrReuseAlreadyAllocatedBlocksWithAlignment::AllocMemoryExt(size_t pi_MemorySize, size_t& po_ActualSize)
    {

    if (pi_MemorySize == 0)
        return NULL;

    // Compute alignment size
    if (m_alignment != 0)
        pi_MemorySize = (((pi_MemorySize - 1) / m_alignment) + 1) * m_alignment;

#ifdef __HMR_DEBUG
    m_allocCount++;
#endif

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

#ifdef __HMR_DEBUG
    m_reuseRatio = (m_allocCount - m_realAllocCount) / (double)m_allocCount;
#endif

    HASSERT(po_ActualSize >= pi_MemorySize);
    return pMemory;
    }

void HPMMemoryMgrReuseAlreadyAllocatedBlocksWithAlignment::FreeMemory(Byte* pi_MemPtr, size_t pi_MemorySize)
    {
    // For this implementation the second parameter is ignored
    if (pi_MemPtr != 0)
        {
#ifdef __HMR_DEBUG
        m_deallocCount++;
#endif
        vector<MemEntry>::iterator itr;
        for(itr = m_UsedMemMgrList.begin() ; itr != m_UsedMemMgrList.end(); ++itr)
            {
            if (itr->Offset == pi_MemPtr)
                {
                itr->State = SysFree;
                m_FreeMemMgrList.push_back(*itr);
                m_UsedMemMgrList.erase(itr);
                break;
                }
            }
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