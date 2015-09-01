//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HPMPool.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HPMPool
//-----------------------------------------------------------------------------
#pragma once

#include <Imagepp/all/h/HFCExclusiveKey.h>
#include "HFCPtr.h"

BEGIN_IMAGEPP_NAMESPACE

/**----------------------------------------------------------------------------+
|
|   This class is used to define "object pools".  The pool will manage
|   a set of objects (pool items) according to imposed limitations. These
|   limitations could be based upon the number of items of the amount of 
|   memory allowed. 
|   The pool will advice pool items if these have not been recently used 
|   and the limtations have been reached so these items can be discarded.
|   The discarding proper is not the responsibility of the pool proper
|   but instead is performed after a request to the pool item. The actual
|   way these items are discarded is unknow to the pool but a 
|   typical way is for pool items to be persisted upon a storage.
|   Notice that the HPM class set provides HPMObjectStore amd 
|   HPMLoader which may be used 
|   by implementations of pool items. These loaders will interact with 
|   the pool if required.
|   Together with
|   HPMObjectStore, these classes may implement a system of automatic memory
|   management where objects that share limited memory or count resources may be
|   swapped to another storage media (i.e. database or file) when they are
|   not used.  In this mechanism, an instance of HPMPool has the
|   responsibility to:
|
|   @list{know which objects are in physical memory, not swapped to an
|         object store;}
|
|   @list{determine when objects need to be swapped and which objects
|         will be.}
|
|   @end
|
|   A pool is informed of every use of all items that are associated to
|   it. In return, the pool requests for discards (swaps to a store or whatever 
|   the pool item performs in order to discard) objects when
|   required.  This way, a HPMPool instance may be considered as managing a
|   pool of memory that is shared by many objects.
|
|   This present class is a functional one.  Its implementation uses a smart MRU
|   list to help keep in memory objects recently and frequently used while
|   allowing more "sleepy" objects to be discarded when needed.
|   This class may be easily derived to implement a different algorithm.
|
|   NOTE: The pool will always allow one object whatever the size limit, even if the
|   pool size is smaller than the size of the object.
|
|   @see HPMObjectStore
|
+----------------------------------------------------------------------------*/

class HPMPoolItem;
class IHPMMemoryManager;
class HPMMemoryMgrExport;
class HPMPool : public HFCShareableObject<HPMPool>,
    public HFCExclusiveKey
    {
    HDECLARE_BASECLASS_ID(HPMPoolId_Base)

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

	/* Incorrect constructor ... should be phased out or better yet, HPMPool should be deprecated and replaced */
    IMAGEPP_EXPORT                     HPMPool(size_t pi_PoolLimit=0, MemoryMgrType pi_MemoryMgrType=None);


    /*----------------------------------------------------------------------------+
    | Constructor
    | The pool is created by default by indicating the limit to the number of items 
    | or global item sizes. The memory manager used to allocate the memory to be used
    | by the items is provided. When an item will be re-loaded or created, it will
    | first require the pool to provide the necessary memory then add itslef as
    | part of the list of pool managed items.
    | If NULL is provided as memory manager then a default memory manager will be 
    | used.
    | NOTE: The memory manager is owned by the pool which will be responsible of
    | destroying it at the end.
    +----------------------------------------------------------------------------*/
    IMAGEPP_EXPORT                     HPMPool(size_t pi_PoolLimit, IHPMMemoryManager* memoryManager);

    IMAGEPP_EXPORT virtual void        ValidateInvariants() const;
    IMAGEPP_EXPORT virtual             ~HPMPool();

    //:> Limit management

    /*----------------------------------------------------------------------------+
    | ChangeLimit
    | This method changes the limit imposed to the pool.
    | A limit of 0 indicates there is no limit.
    +----------------------------------------------------------------------------*/
    IMAGEPP_EXPORT bool         ChangeLimit(size_t pi_NewPoolLimit);

    /*----------------------------------------------------------------------------+
    | GetLimit
    | Returns the limit of the pool. The interpretation of the limit depends on the
    | actual implementation of the pool.
    +----------------------------------------------------------------------------*/
    size_t              GetLimit() const;
    /*----------------------------------------------------------------------------+
    | GetActualCount
    | Returns the actual used memory or count depending on the 
    | actual implementation of the pool.
    +----------------------------------------------------------------------------*/
    size_t              GetActualCount() const;

    //:> Operation methods.  Used by loaders.
    /*----------------------------------------------------------------------------+
    | NotifyAccess
    | This method indicates to the pool that the designated item has been used.
    | NOTE: The method may not be called at every access of the item. One strategy
    | for large objects that are accessed frequently would be to notify
    | access at every N accesses, N being significant and judiciously selected 
    | according to item use. For STM 3D points nodes, where each node contains
    | typically 5000 points, the access is notified every 100 effective accesses.
    | For smaller objects, other strategies can be used, keeping in mind that
    | call to the present function takes a little time to execute if the
    | the item is not the last used. The pool caches the last and next to last 
    | used item and subsequent access notifications to these items are 
    | disregarded till other pooled items notify access.
    | This caching will only operate properly if accesses to more than two 
    | individual items are not intermingled as part of the algorithm.
    +----------------------------------------------------------------------------*/
    IMAGEPP_EXPORT void         NotifyAccess(const HFCPtr<HPMPoolItem>& pi_rpItem);

    
    /*----------------------------------------------------------------------------+
    | RemoveItem
    | This method removes an item from the pool management. Typically this occurs
    | as a result of a discard, initiated at the request of the pool itself.
    | Once removed the item is not managed anymore, and the amount of memory it
    | may still use is not accounted for. This does not implyu necessarily
    | that the item does not use memory from the memory manager anymore; it simply
    | indicates that it should not use such memory for a lengthy period.
    | As an example, pool items can be pinned into memory, for multi-threading 
    | purposes. In such case the discard MAY result in removal from the pool
    | but not into release of the resources used. NOTE: the actual behavior 
    | of pinning/unpinning and memory release of ppol items is implementation
    | specific, yet the guidelines mentionned here should be followed as closely 
    | as possible.
    +----------------------------------------------------------------------------*/
    IMAGEPP_EXPORT void                RemoveItem(const HFCPtr<HPMPoolItem>&   pi_rpItem);

    //:> Memory manager...


    /*----------------------------------------------------------------------------+
    | Memory management
    | Although the memory manager of the pool is owned by the pool proper, it is usally 
    | for the pool item to request that memory be allocated. 
    | DESIGN differences. At the moment there is a significant difference between the
    | present pool and the Count limited pool that was implemented aside the present
    | pool. The intent is to merge both designs. 
    | The present pool provides a simple repeat of the memory manager interface:
    |  AllocMemory
    |  FreeMemory
    |  NeedMemory
    | The later is only needed by a specific export intended memory manager.
    |
    | Old design interface:
    |   Pool:
    |     ChangeLimit                - Unused
    |     GetLimit                   - Unused
    |     GetActualCount             - Unused
    |     NotifyAccess               - Used by HPMPoolItem::NotifyPool only
    |     RemoveItem                 - Used by HPMPoolItem::MoveToPool to change pool and HPMPoolItem::RemoveFromPool and HPMPoolItem::Discard
    |     AllocMemory                - Used by HRABitmap only
    |     NeedMemory                 - Used by HRATiledRaster only
    |     FreeMemory                 - Used by HRABitmap only
    |     IsMemoryMgrEnabled         - Used by HRABitmap only
    |
    |   PoolItem (NOTE: Only HRATiledRaster and HIMBufferedImage are pool items)
    |     Discard                    - Used by HIMBufferedImage and HRATiledRaster
    |     MoveToPool                 - Used by HIMBlendCorridor and HIMBufferedImage only
    |     RemoveFromPool             - Unused (The HPMMemoryMgrExport performs the removal all the time)
    |     NotifyPool                 - Used by HRAPyramidRaster only (upon individual tiles)
    |     GetPool                    - Used by HRABitmap, HPS*(?), HPMObjectStore(?)
    |     HasInPool                  - Used by HRAPyramidRaster
    |
    |
    | The other design is different.
    | Upon Allocate, the pool item is provided. The pool will handshake with the pool item
    | to add the item to the pool and allocate memory then provide it to the
    | item.
    | Reallocate can be used. In that case the memcpy will be performed to the new location.
    | Again Free is used to at the same time remove the item from the pool and release the memory.
    | if the item is pinned into memory then memory deallocation is suspended but the item is removed from
    | pool management using RemoveItem. Then when memory can be freed at last, the call to FreeRemovedItemMemory
    | is performed.
    | NOTE: What happens if item is removed without free and pool is destroyed. In that case
    | memory that should normaly be under control of the memory manager of the pool
    | will be stuck in memory. ????? Should the pool request immediate freeing of such item?
    |
    | New Design interface in brief:
    |   Pool:
    |     Allocate                   = AllocMemory
    |     Reallocate                 TO BE ADDED
    |     Free                       = FreeMemory
    |     NotifyAccess               = NotifyAccess
    |     FreeRemovedItemMemory      To BE ADDED
    |     RemoveItem                 = RemoveItem
    |     ChangeLimit                = ChangeLimit
    |     GetLimit                   = GetLimit
    |     GetActualCount             = GetActualCount
    | 
    | This design only works because the pool item has the following
    | interface:
    |
    |   PoolItem: 
    |       member: pool
    |       SetPool                      = MoveToPool / RemoveFromPool
    |       GetPool                      = GetPool
    |       IsDirty                      TO ADD!
    |       SetDirty                     TO ADD!
    |       Discard                      = Discard
    |       Discarded                    TO ADD!
    |       Inflate                      Reverse of Discard. Allows an eventual management system to discard/inflate according to some mechanism
    |       GetPoolManaged               = HasInPool (?)
    |     Protected:
    |       Pin
    |       Pinned
    |       Unpin
    |       SetDiscarded
    |       GetMemory
    |       GetMemoryCount
    |     Private (accessible through pool becasue friendship)
    |       SetMemory
    |       GetPoolIterator
    |       SetPoolIterator
    |       SetPoolManaged  
    | The 
    | This method removes an item from the pool management. Typically this occurs
    | as a result of a discard, initiated at the request of the pool itself.
    | Once removed the item is not managed anymore, and the amount of memory it
    | may still use is not accounted for. This does not implyu necessarily
    | that the item does not use memory from the memory manager anymore; it simply
    | indicates that it should not use such memory for a lengthy period.
    | As an example, pool items can be pinned into memory, for multi-threading 
    | purposes. In such case the discard MAY result in removal from the pool
    | but not into release of the resources used. NOTE: the actual behavior 
    | of pinning/unpinning and memory release of ppol items is implementation
    | specific, yet the guidelines mentionned here should be followed as closely 
    | as possible.
    +----------------------------------------------------------------------------*/

    IMAGEPP_EXPORT virtual bool NeedMemory(size_t pi_DataSize, size_t pi_ObjectSize);
    IMAGEPP_EXPORT Byte*        AllocMemory(size_t pi_MemorySize);
    IMAGEPP_EXPORT void         FreeMemory(Byte* pi_MemPtr, size_t pi_MemorySize);

    /*----------------------------------------------------------------------------+
    | Disable/EnableDiscard
    | These methods allow to suspend temporarily the discard of the pool.
    | A method must call DisableDiscard() AND VERIFY ture is returned. If false 
    | is returned then the pool discard has already been disabled.
    | Only the instance of code or object that has sucessfully disabled
    | discard can enable it back using EnableDiscard()
    +----------------------------------------------------------------------------*/
    // IMAGEPP_EXPORT bool         DisableDiscard();
    // IMAGEPP_EXPORT bool         EnableDiscard();

// TO BE REMOVED
    IHPMMemoryManager& GetMemoryManager() {return *m_MemoryManager;}


    IMAGEPP_EXPORT bool         IsMemoryMgrEnabled() const;

protected:


    HPMPool(const HPMPool&);
    HPMPool& operator=(const HPMPool&);

    // The pool itself.  It is a list of pointers to loaders,
    // sorted in MRU order.

    PoolList            m_Pool;
    // PoolList            m_ToBeDiscardedList; // This list contains the list of items to be discarded
    // bool                m_discardEnabled; // Indicates if discard is enabled. If not then items to be discarded are added to the to be discarded list.

    // Pool parameters

    size_t              m_SizeLimit;
    size_t              m_SizeCount;
//        HFCExclusiveKey     m_Key;

    // Memory manager
    HAutoPtr<IHPMMemoryManager>  m_MemoryManager;
    };


/**----------------------------------------------------------------------------+
|
|   This class is a variant of the ordinary pool. It will maintain one
|   block of any given size. This specific pool is meant for use for export
|   of rasters when large blocks are used.
|
+----------------------------------------------------------------------------*/
class HPMKeepLastBlockPool : public HPMPool
    
    {
    HDECLARE_CLASS_ID(HPMKeepLastBlockPoolId, HPMPool)
     
public:
 
    IMAGEPP_EXPORT                      HPMKeepLastBlockPool(size_t pi_PoolLimit);
    IMAGEPP_EXPORT virtual              ~HPMKeepLastBlockPool();
 
 
    };


/**----------------------------------------------------------------------------+
|
|   This class is a variant of the ordinary pool. It will maintain one
|   block of any given size. This specific pool is meant for use for export
|   of rasters when large blocks are used.
|
+----------------------------------------------------------------------------*/
class HPMExportPool : public HPMPool
    {
    HDECLARE_CLASS_ID(HPMExportPoolId, HPMPool)
     
public:
 
    IMAGEPP_EXPORT                      HPMExportPool(size_t pi_PoolLimit);
    IMAGEPP_EXPORT virtual              ~HPMExportPool();
    IMAGEPP_EXPORT bool                 AllocMemory(size_t pi_MemorySize, const HPMPoolItem& poolItem);
 
    IMAGEPP_EXPORT virtual bool NeedMemory(size_t pi_DataSize, size_t pi_ObjectSize);
 
 
    };





/*----------------------------------------------------------------------------+
|struct HPMPoolItem
| Ancester class for items that must be managed by a pool. 
| IMPORTANT NOTE: The Discard can be called by the pool when the item has already 
| been removed from the pool list. This enables the discard operation to use the pool in
| the discard process. This is for example essential for classes such as pyramid raster 
| for which the discard may trigger the update of the sub-resolution which in turn require
| new pool managed memory. To prevent running conditions the pool item is first 
| removed from pool management and then the discard is called. This implies that during the discard 
| process the pool item may not be pool managed yet the pool must be called in order
| to complete memory deallocation.
| The class will inherit from this Pool Item and implement
| the appropriate Discard() function. The Discard() implementation MUST call the pool Free() method
| to deallocate its memory otherwise the pool memory management administration will be incorrect.
| A pool item cannot refuse to discard. If for some reason, the memory allocated cannot be
| released, then the pool item may call RemoveFromManagement() method. This pool method will remove the
| memory of the item from management without deallocating it. In such case the pool item is responsible
| of releasing the memory used by itself
|
| The memory will always be allocated by the pool and set internally in the Pool Item through the
| SetMemory() function only available to the Pool (through friendship)
|
| NOTE: In order to fucntion properly, the pool must be advised whenever a pool item is accessed
| Thus a call to NotifyAccess() method of the pool should be performed.

+----------------------------------------------------------------------------*/
class HPMPoolItem : public HFCShareableObject<HPMPoolItem>
    {
    HDECLARE_BASECLASS_ID(HPMPoolItemId_Base)

public:
    IMAGEPP_EXPORT virtual                 ~HPMPoolItem();

    IMAGEPP_EXPORT virtual void            ValidateInvariants() const;
    IMAGEPP_EXPORT virtual bool            Discard();

    IMAGEPP_EXPORT void                    MoveToPool(HPMPool* pi_pPool);
    IMAGEPP_EXPORT void                    RemoveFromPool();
    IMAGEPP_EXPORT void                    NotifyPool();
    IMAGEPP_EXPORT HPMPool*                GetPool() const;
    IMAGEPP_EXPORT bool                    HasInPool() const; // TBD RENAME TO SOMETHING MORE MEANINGFULL
    IMAGEPP_EXPORT bool                    GetPoolManaged() const; // TBD RENAME TO SOMETHING MORE MEANINGFULL

    // New API to be evaluated
    /**----------------------------------------------------------------------------
     Returns true if the item is currently discarded
    -----------------------------------------------------------------------------*/
     IMAGEPP_EXPORT bool                    IsDiscarded() const;

    /**----------------------------------------------------------------------------
     The Pin method inflates object into memory if required and prevents it from being
     discarded in actuality (the memory is not freed till unpinned even if pool
     requests discard).
     The method will return false if the item is already pinned
    -----------------------------------------------------------------------------*/
    // IMAGEPP_EXPORT bool Pin() const; // Intentionaly const as only mutable members get modified

    /**----------------------------------------------------------------------------
     The Pinned method indicates if the item is pinned
    -----------------------------------------------------------------------------*/
    // IMAGEPP_EXPORT bool Pinned() const;

    /**----------------------------------------------------------------------------
     Unpins the item ... The memory will be immediately deleted if the object has been
     discarded while pinned.
    -----------------------------------------------------------------------------*/
    // IMAGEPP_EXPORT virtual void UnPin() const;

    /**----------------------------------------------------------------------------
     Returns true if the item is dirty and false otherwise.
    -----------------------------------------------------------------------------*/
    // IMAGEPP_EXPORT bool                    IsDirty() const;

    /**----------------------------------------------------------------------------
     Sets the item dirty flag.
     Against all likelyness this method is const. The dirty flag can be unset
     which would go against the principle that the item has been modified.
     The dirtyness is thus considered mutable and changing it does not change
     its constness.
    -----------------------------------------------------------------------------*/
    // IMAGEPP_EXPORT virtual void            SetDirty (bool dirty) const; // Intentionaly const ... only mutable field changed



    /**----------------------------------------------------------------------------
     Indicates that the item should be inflated and given over to pool management
     This new method implies that a pool item knows to which pool she must give 
     management to.
    -----------------------------------------------------------------------------*/
//    virtual bool Inflate() const; // Intentionaly const ... only mutable members are modified



protected:
    friend class HPMPool;
    friend class HPMMemoryMgrExport;

    /**----------------------------------------------------------------------------
      SetDiscarded()
      Sets the discarded flag.
    -----------------------------------------------------------------------------*/
     IMAGEPP_EXPORT void SetDiscarded(bool pi_Discarded) const;
    /**----------------------------------------------------------------------------
      GetMemorySize()
      Returns the size in bytes of memory used by the pool item
    -----------------------------------------------------------------------------*/
    IMAGEPP_EXPORT size_t GetMemorySize() const;

    /**----------------------------------------------------------------------------
      SetPoolIterator()
      Used privately by pool to handshake with a newly managed pool item.
      This method sets the internal pool item iterator indicating the location
      of the item inside the pool.
    -----------------------------------------------------------------------------*/
    IMAGEPP_EXPORT void SetPoolIterator(HPMPool::Iterator poolIterator) const; // Intentionaly const ... only mutable members are modified


    /**----------------------------------------------------------------------------
      Returns the pool iterator assigned to item
    -----------------------------------------------------------------------------*/
    IMAGEPP_EXPORT HPMPool::Iterator GetPoolIterator() const; // Intentionaly const ... only mutable members are modified



    /**----------------------------------------------------------------------------
     Sets if the pool is managed ... the item may be removed from pool management by the pool itself
    -----------------------------------------------------------------------------*/
    IMAGEPP_EXPORT void SetPoolManaged(bool poolManaged) const; // Intentionaly const ... only mutable members are modified



    IMAGEPP_EXPORT                HPMPoolItem();
    IMAGEPP_EXPORT                HPMPoolItem(HPMPool* pi_pPool);
    virtual void                  UpdateCachedSize() = 0; // This method is intentionaly const as it only modifies mutable control members
    virtual HFCExclusiveKey&      GetExclusiveKey() = 0;

    mutable bool                m_Discarded;
    mutable bool                m_Dirty;
    mutable size_t              m_ObjectSize;
    mutable HPMPool::Iterator   m_PoolIterator;

    HPMPool*                    m_pPool;

    mutable bool                m_Pinned;

private:
    };




/*---------------------------------------------------------------------------------**//**
* IHPMMemoryManager
+---------------+---------------+---------------+---------------+---------------+------*/
class IHPMMemoryManager
{
public:
    enum MemState
        {
        MemFree = 0,
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

    IHPMMemoryManager(){};
    virtual                       ~IHPMMemoryManager() {};
    virtual Byte*                 AllocMemory(size_t pi_MemorySize) = 0;
    virtual void                  FreeMemory(Byte* pi_MemPtr, size_t pi_MemorySize) = 0;
    IMAGEPP_EXPORT virtual bool   NeedMemory(size_t pi_DataSize, size_t pi_ObjectSize);

    // This method is virtually identical to the one parameter AllocMemory() except
    // that the second parameter receives the actual memory size allocated
    // and can then use it. The default implementation will simply call
    // AllocMemory() instead.
    IMAGEPP_EXPORT virtual Byte*     AllocMemoryExt(size_t pi_MemorySize, size_t& po_ActualMemory);

private:
    IHPMMemoryManager(IHPMMemoryManager const&) = delete;
    IHPMMemoryManager& operator=(IHPMMemoryManager const&) = delete;
};


typedef IHPMMemoryManager HPMMemoryMgr;

//***********************************************************************************************************************
// HPMBaseMemoryManager
//
// This base memory manager uses the normaly used new and delete operations. The can be used anywhere a memory manager
// is required but no specific memory management is necessary.
// In debug mode, the manager can be used to validate the count of objects that were allocated and deallocated
//***********************************************************************************************************************
class HPMBaseMemoryManager: public IHPMMemoryManager
    {

public:

#ifdef __HMR_DEBUG
    IMAGEPP_EXPORT HPMBaseMemoryManager() {
        m_allocCount = 0;
        m_deallocCount = 0;
        };
    IMAGEPP_EXPORT virtual             ~HPMBaseMemoryManager() {
        HASSERT (m_allocCount == m_deallocCount);
        };

    IMAGEPP_EXPORT virtual Byte*     AllocMemory(size_t pi_MemorySize)
        {
        m_allocCount++;
        return new Byte[pi_MemorySize];
        }
    IMAGEPP_EXPORT virtual void        FreeMemory(Byte* pi_MemPtr, size_t pi_MemorySize)
        {
        m_deallocCount++;
        delete [] pi_MemPtr;
        }
    size_t m_allocCount;
    size_t m_deallocCount;

#else
    IMAGEPP_EXPORT HPMBaseMemoryManager() {};
    IMAGEPP_EXPORT virtual              ~HPMBaseMemoryManager() {};

    IMAGEPP_EXPORT virtual Byte*        AllocMemory(size_t pi_MemorySize) 
        {
        return new Byte[pi_MemorySize];
        }
    IMAGEPP_EXPORT virtual void         FreeMemory(Byte* pi_MemPtr, size_t pi_MemorySize) 
        {
        delete [] pi_MemPtr;
        };

#endif

    };


/*---------------------------------------------------------------------------------**//**
* HPMMemoryMgrExport
+---------------+---------------+---------------+---------------+---------------+------*/
class HPMMemoryMgrExport : public IHPMMemoryManager
    {
public:
#define MgrExportMaxEntry   16

    HPMMemoryMgrExport(size_t pi_MemorySize, HPMPool* pi_pPool);
    ~HPMMemoryMgrExport();

    virtual Byte*       AllocMemory(size_t pi_MemorySize);
    virtual void        FreeMemory(Byte* pi_MemPtr, size_t pi_MemorySize);
    virtual bool        NeedMemory(size_t pi_DataSize, size_t pi_ObjectSize);

    int         FindEntry(size_t pi_MemorySize, bool& po_MemAlreadyAllocated);

private:

    int      m_IndexFreeBlock;
    int      m_NextEmptyBlock;
    MemEntry m_MemMgrList[MgrExportMaxEntry];

    HPMPool* m_pPool;
    };

/*---------------------------------------------------------------------------------**//**
* HPMMemoryMgrKeepLastEntry
* This memory manager reuses previously allocated memory. It never allocate memory. 
* Instead, it collect previously allocated memory in its FreeMemory method , and then 
* reuse it when requested to alloc memory. It can keep up to MgrKeepLastEntryMaxEntry
* memory buffers. When the maximum number of buffers is attained, it releases an entry 
* to store the current buffer.
+---------------+---------------+---------------+---------------+---------------+------*/
class HPMMemoryMgrKeepLastEntry : public IHPMMemoryManager
{
public:
#define MgrKeepLastEntryMaxEntry 8

    HPMMemoryMgrKeepLastEntry(size_t pi_MemorySize, HPMPool* pi_pPool);
    ~HPMMemoryMgrKeepLastEntry();

    virtual Byte*       AllocMemory(size_t pi_MemorySize);
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
class HPMMemoryMgrReuseAlreadyAllocatedBlocks : public IHPMMemoryManager
    {
public:

    IMAGEPP_EXPORT HPMMemoryMgrReuseAlreadyAllocatedBlocks(size_t numberOfAllowedFreeBlocks);
    IMAGEPP_EXPORT ~HPMMemoryMgrReuseAlreadyAllocatedBlocks();

    virtual Byte*       AllocMemoryExt(size_t pi_MemorySize, size_t& po_ActualMemory) override;
    virtual Byte*       AllocMemory(size_t pi_MemorySize) override;
    virtual void        FreeMemory(Byte* pi_MemPtr, size_t pi_MemorySize) override;

    // New method FreeAll() Frees all memory under the control of the memory manager.
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

    IMAGEPP_EXPORT HPMMemoryMgrReuseAlreadyAllocatedBlocksWithAlignment(size_t numberOfAllowedFreeBlocks, size_t pi_alignmentSize);
    IMAGEPP_EXPORT ~HPMMemoryMgrReuseAlreadyAllocatedBlocksWithAlignment();

    IMAGEPP_EXPORT virtual Byte*        AllocMemoryExt(size_t pi_MemorySize, size_t& po_ActualMemory) override;
    IMAGEPP_EXPORT virtual void         FreeMemory(Byte* pi_MemPtr, size_t pi_MemorySize) override;
private:
    size_t m_alignment;

    };

END_IMAGEPP_NAMESPACE
#include "HPMPool.hpp"

