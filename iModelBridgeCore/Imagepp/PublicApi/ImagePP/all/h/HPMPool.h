//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HPMPool.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HPMPool
//-----------------------------------------------------------------------------
#pragma once

#include <Imagepp/all/h/HFCExclusiveKey.h>
#include "HFCPtr.h"

/**

    This class is used to define "object pools".  These are companions
    objects for HPMLoader instances and are used for memory management
    of objects managed by these automatic loaders.  Together with
    HPMObjectStore, these three classes implement automatic memory
    management where objects that share limited memory resources may be
    swapped to another storage media (i.e. database or file) when they are
    not used.  In this mechanism, an instance of HPMPool has the
    responsibility to:

    @list{know which objects are in physical memory, not swapped to an
          object store;}

    @list{determine when objects need to be swapped and which objects
          will be.}

    @end

    A pool is informed of every use of all loaders that are associated to
    it. In return, the pool discards (swaps to a store) objects when
    required.  This way, a HPMPool instance may be considered as managing a
    pool of memory that is shared by many objects accessed through virtual
    pointers.

    This class is a functional one.  Its implementation uses a smart MRU
    list to help keep in memory objects recently and frequently used while
    allowing more "sleepy" objects to be swapped and retrieved when needed.
    This class may be easily derived to implement a different algorithm.

    There is a global pool object, labeled @k{g_DefaultPool}, that can be
    used in cases where a pool is required but the user does not want to
    define a pool with specific limits.  It is a pool that defines an
    infinite memory space, so it does not cause any discarding to occur.

    @see HPMOBjectStore

*/

class HPMPoolItem;
class HPMMemoryMgr;
class HPMMemoryMgrExport;
class HPMPool : public HFCShareableObject<HPMPool>,
    public HFCExclusiveKey
    {
    HDECLARE_BASECLASS_ID(1115)

public:

    friend class HPMMemoryMgrExport;

    // Public types

    typedef list<HFCPtr<HPMPoolItem> > PoolList;
    typedef PoolList::iterator Iterator;

    enum MemoryMgrType
        {
        ExportRaster,
        KeepLastBlock,
        None
        };

    //:> Primary methods

    _HDLLu                     HPMPool(size_t pi_PoolLimit=0, MemoryMgrType pi_MemoryMgrType=None);
    _HDLLu virtual             ~HPMPool();

    //:> Limit management

    _HDLLu void         ChangeLimit(size_t pi_NewPoolLimit);
    size_t              GetLimit() const;
    size_t              GetActualCount() const;

    //:> Operation methods.  Used by loaders.

    _HDLLu void         NotifyAccess(const HFCPtr<HPMPoolItem>& pi_rpItem);
    void                RemoveItem(const HFCPtr<HPMPoolItem>&   pi_rpItem);

    //:> Memory manager...

    _HDLLu void         NeedMemory(size_t pi_DataSize, size_t pi_ObjectSize);
    _HDLLu Byte*      AllocMemory(size_t pi_MemorySize);
    _HDLLu void         FreeMemory(Byte* pi_MemPtr, size_t pi_MemorySize);
    bool               IsMemoryMgrEnabled() const;

protected:

private:

    HPMPool(const HPMPool&);
    HPMPool& operator=(const HPMPool&);

    // The pool itself.  It is a list of pointers to loaders,
    // sorted in MRU order.

    PoolList            m_Pool;

    // Pool parameters

    size_t              m_SizeLimit;
    size_t              m_SizeCount;
//        HFCExclusiveKey     m_Key;

    // Memory manager
    HAutoPtr<HPMMemoryMgr>  m_MemMgr;
    };

class HPMPoolItem : public HFCShareableObject<HPMPoolItem>
    {
    HDECLARE_BASECLASS_ID(1333)

public:
    _HDLLu virtual                 ~HPMPoolItem();

    _HDLLu void                    Discard();

    _HDLLu void                    MoveToPool(HPMPool* pi_pPool);
    _HDLLu void                    RemoveFromPool();
    _HDLLu void                    NotifyPool();
    _HDLLu HPMPool*                GetPool() const;
    _HDLLu bool                   HasInPool() const;

protected:
    friend class HPMPool;
    friend class HPMMemoryMgrExport;

    _HDLLu                        HPMPoolItem(HPMPool* pi_pPool = 0);
    virtual void                  UpdateCachedSize() = 0;

    virtual HFCExclusiveKey&      GetExclusiveKey() = 0;

    bool               m_Discarded;
    size_t              m_ObjectSize;
    HPMPool::Iterator   m_Itr;
    HPMPool*            m_pPool;


private:
    };




class HPMMemoryMgr
    {

public:

    enum MemState
        {
        MemFree=0,
        MemAllocated,
        SysFree,
        SysAllocated
        };

    struct MemEntry
        {
        Byte*     Offset;
        size_t      Size;
        MemState    State;
        };

//                        HPMMemoryMgr();
    virtual             ~HPMMemoryMgr() {};
    _HDLLu virtual Byte*     AllocMemory(size_t pi_MemorySize)=0;
    _HDLLu virtual void        FreeMemory(Byte* pi_MemPtr, size_t pi_MemorySize)=0;
    _HDLLu virtual bool       NeedMemory(size_t pi_DataSize, size_t pi_ObjectSize);

    // This method is virtually identical to the one parameter AllocMemory() except
    // that the second parameter receives the actual memory size allocated
    // and can then use it. The default
    _HDLLu virtual Byte*     AllocMemoryExt(size_t pi_MemorySize, size_t& po_ActualMemory);


    };


//***********************************************************************************************************************
// HPMBaseMemoryMgr
//
// This base memory manager uses the normaly used new and delete operations. The can be used anywhere a memory manager
// is required but no specific memory management is necessary.
// In debug mode, the manager can be used to validate the count of objects that were allocated and deallocated
//***********************************************************************************************************************
class HPMBaseMemoryMgr: public HPMMemoryMgr
    {

public:

#ifdef __HMR_DEBUG
    _HDLLu                     HPMBaseMemoryMgr() {
        m_allocCount = 0;
        m_deallocCount = 0;
        };
    _HDLLu virtual             ~HPMBaseMemoryMgr() {
        HASSERT (m_allocCount == m_deallocCount);
        };

    _HDLLu virtual Byte*     AllocMemory(size_t pi_MemorySize)
        {
        m_allocCount++;
        return new Byte[pi_MemorySize];
        }
    _HDLLu virtual void        FreeMemory(Byte* pi_MemPtr, size_t pi_MemorySize)
        {
        m_deallocCount++;
        delete [] pi_MemPtr;
        }
    size_t m_allocCount;
    size_t m_deallocCount;

#else
    _HDLLu                     HPMBaseMemoryMgr() {};
    _HDLLu virtual             ~HPMBaseMemoryMgr() {};

    _HDLLu virtual Byte*     AllocMemory(size_t pi_MemorySize) {
        return new Byte[pi_MemorySize];
        }
    _HDLLu virtual void        FreeMemory(Byte* pi_MemPtr, size_t pi_MemorySize) {
        delete [] pi_MemPtr;
        };

#endif

    };


class HPMMemoryMgrExport : public HPMMemoryMgr
    {
public:
#define MgrExportMaxEntry   16

    HPMMemoryMgrExport(size_t pi_MemorySize, HPMPool* pi_pPool);
    ~HPMMemoryMgrExport();

    virtual Byte*     AllocMemory(size_t pi_MemorySize);
    virtual void        FreeMemory(Byte* pi_MemPtr, size_t pi_MemorySize);
    virtual bool       NeedMemory(size_t pi_DataSize, size_t pi_ObjectSize);

    int         FindEntry(size_t pi_MemorySize, bool& po_MemAlreadyAllocated);

private:

    int      m_IndexFreeBlock;
    int      m_NextEmptyBlock;
    MemEntry m_MemMgrList[MgrExportMaxEntry];

    HPMPool* m_pPool;
    };


class HPMMemoryMgrKeepLastEntry : public HPMMemoryMgr
    {
public:
#define MgrKeepLastEntryMaxEntry 8

    HPMMemoryMgrKeepLastEntry(size_t pi_MemorySize, HPMPool* pi_pPool);
    ~HPMMemoryMgrKeepLastEntry();

    virtual Byte*     AllocMemory(size_t pi_MemorySize);
    virtual void        FreeMemory(Byte* pi_MemPtr, size_t pi_MemorySize);

private:

    MemEntry    m_MemMgrList[MgrKeepLastEntryMaxEntry];
    int         m_Indexfree;
    HPMPool*    m_pPool;
    };



/**

    This class is used to maximize reuse of big blocks typically
    to prevent fragmentation of the memory space.

    This memory manager will allocate blocks upon the heap yet not release
    them immediately. It will instead keep these allocated memory chunks
    for a future request of memory. Any request for memory will return the smallest
    chunk of free memory that contains at least the requested size.
    If the requested memory is greater than any free chunks then a new chunk
    will be allocated. If a new memory chunk is allocated then some sort of
    chunk releasal will be performed. If there are too many chunks of
    allocated free memory then the smallest of them will be released.

    The purpose of this memory manager is simply to minimise the chances
    of memory heap fragmentation which may result in effective memory
    exhaustion when large chunks of memroy are iteratively allocated/deallocated at the
    same time smaller portions of memory are allocated/deallocated on the same heap.

    Since the present manager also uses the normal heap, the large memory chunks will still
    make use of the same memory space but the manager will minimize the allocate/deallocate
    events upon managed memory blocks thus diminishing fragmentation.

    The present manager is not meant to manage small portions of memory!!

*/
class HPMMemoryMgrReuseAlreadyAllocatedBlocks : public HPMMemoryMgr
    {
public:

    _HDLLu HPMMemoryMgrReuseAlreadyAllocatedBlocks(size_t numberOfAllowedFreeBlocks);
    _HDLLu ~HPMMemoryMgrReuseAlreadyAllocatedBlocks();

    virtual Byte*     AllocMemoryExt(size_t pi_MemorySize, size_t& po_ActualMemory);
    virtual Byte*     AllocMemory(size_t pi_MemorySize);
    virtual void        FreeMemory(Byte* pi_MemPtr, size_t pi_MemorySize);
    virtual void        FreeAll();

#ifdef __HMR_DEBUG
    // The following fields are maintained for memory debugging purposes.
    size_t m_allocCount;
    size_t m_deallocCount;
    size_t m_realAllocCount;
    size_t m_realDeallocCount;
    double m_reuseRatio;
#endif
protected:

    vector<MemEntry> m_UsedMemMgrList;
    vector<MemEntry> m_FreeMemMgrList;

    size_t m_maxFreeBlocks;
    };


/**

    This class is used to maximize reuse of big blocks typically
    to prevent fragmentation of the memory space.

    This memory manager will allocate blocks upon the heap yet not release
    them immediately. It will instead keep these allocated memory chunks
    for a future request of memory. Any request for memory will return the smallest
    chunk of free memory that contains at least the requested size.
    If the requested memory is greater than any free chunks then a new chunk
    will be allocated. If a new memory chunk is allocated then some sort of
    chunk releasal will be performed. If there are too many chunks of
    allocated free memory then the smallest of them will be released.

    The purpose of this memory manager is simply to minimise the chances
    of memory heap fragmentation which may result in effective memory
    exhaustion when large chunks of memroy are iteratively allocated/deallocated at the
    same time smaller portions of memory are allocated/deallocated on the same heap.

    Since the present manager also uses the normal heap, the large memory chunks will still
    make use of the same memory space but the manager will minimize the allocate/deallocate
    events upon managed memory blocks thus diminishing fragmentation.

    The present manager is not meant to manage small portions of memory!!

*/
class HPMMemoryMgrReuseAlreadyAllocatedBlocksWithAlignment : public HPMMemoryMgrReuseAlreadyAllocatedBlocks
    {
public:

    _HDLLu HPMMemoryMgrReuseAlreadyAllocatedBlocksWithAlignment(size_t numberOfAllowedFreeBlocks, size_t pi_alignmentSize);
    _HDLLu ~HPMMemoryMgrReuseAlreadyAllocatedBlocksWithAlignment();

    _HDLLu virtual Byte*     AllocMemoryExt(size_t pi_MemorySize, size_t& po_ActualMemory);
    _HDLLu virtual void        FreeMemory(Byte* pi_MemPtr, size_t pi_MemorySize);
private:
    size_t m_alignment;

    };



extern HPMPool g_DefaultPool;





#include "HPMPool.hpp"

