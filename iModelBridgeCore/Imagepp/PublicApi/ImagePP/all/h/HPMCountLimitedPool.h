//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HPMCountLimitedPool.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#pragma once

#include "HPMPool.h"

BEGIN_IMAGEPP_NAMESPACE
template <typename DataType> class HPMIndirectCountLimitedPool;


#if (0)

//***********************************************************************************************************************
// HPMCountLimitedPool - Count Limited Memory Pool
// Manages a list of pool items that share a limited amount of instances of a given data type.
// The count limited pool limited management system is used to administer a shared amount of a given data type.
// The pool performs management through "count limited pool items" each of which contain a block of
// some number of bytes. The Data Type should usually be fixed size to result in effective memory management.
//
// The pool allocates a specified amount of bytes when requested. This amount may not be greater than
// the limit specified. If the pool has already allocated more objects so the new allocation will result in
// exceeding the limit, the pool will call managed "count limited pool items" discard operation
// based on the Least Recently Used criterium
// till allocating the new block will not result in exceeding limit. In order for the Least Recently Used criterium be applied,
// every pool item managed is responsible of notifying the pool of its use using the NotifyAccess() method.
// It is possible to Reallocate a block of memory by calling the Reallocate() method. This method
// will reallocate a new memory block for the new count and copy the content of the former data to this new block.
//
// The pool can use of an allocator or memory manager to allocate or free memory if one is provided. Since the
// memory manager does not currently support typed allocation the bytes must not require explicit construction or
// destruction when allocated or deallocated when a memory manager is used. If no memory manager is provided then
// construction and destruction will occur correctly.
//***********************************************************************************************************************
class HPMCountLimitedPoolItem;

class HPMCountLimitedPool: public HPMPool // public HFCShareableObject<HPMCountLimitedPool>
    {
public:

    /**----------------------------------------------------------------------------
     Constructor.
     The pool count limit is provided at contruction. The pool will limit the total
     amount of bytes managed below this limit.
    -----------------------------------------------------------------------------*/
    HPMCountLimitedPool(size_t countLimit)
        {
        m_countLimit = countLimit;
        m_totalUsedCount = 0;
        m_memoryManager = NULL;
        }

    /**----------------------------------------------------------------------------
     Constructor.
     The pool count limit is provided at contruction. The pool will limit the total
     amount of bytes managed below this limit.
    -----------------------------------------------------------------------------*/
    HPMCountLimitedPool(IHPMMemoryManager* memoryManager, size_t countLimit)
        {
        m_countLimit = countLimit;
        m_totalUsedCount = 0;
        m_memoryManager = memoryManager;
        }

    /**----------------------------------------------------------------------------
     Destructor
     Normally upon destruction the pool should not be managing any objects !
    -----------------------------------------------------------------------------*/
    virtual ~HPMCountLimitedPool()
        {
        HASSERT(m_totalUsedCount == 0);
        }


    /**----------------------------------------------------------------------------
     Returns the count limit of bytes.
    -----------------------------------------------------------------------------*/
    size_t GetLimit() const
        {
        return m_countLimit;
        }


    /**----------------------------------------------------------------------------
     Returns the current actual count of bytes currently managed by pool.
    -----------------------------------------------------------------------------*/
    size_t GetActualCount() const
        {
        return m_totalUsedCount;
        }

    /**----------------------------------------------------------------------------
     Allows to change the limit of instances that can exists and be managed by
     pool. If the new limit is smaller than previous limit and the actual count
     is greater than this new limit then the required instances will be discarded.
    -----------------------------------------------------------------------------*/
    virtual void ChangeLimit(size_t pi_NewPoolLimit)
        {
        HPRECONDITION (pi_NewPoolLimit > 1);

        // Change the limit
        m_countLimit = pi_NewPoolLimit;

        // Check if memory limit exceeded
        while (m_totalUsedCount > m_countLimit)
            {
            // Not enough memory ... some other must be released
            // Request the least used uncompressed FastCompressiblePooledVector
            // to compress
            if (m_Pool.empty() || !m_Pool.back()->Discard())
                return false; // Something happened
            }

        }

    /**----------------------------------------------------------------------------
        Allocates a number of datatypes and assigns to provided pool item.
        The pool item is them managed by the pool manager.
    -----------------------------------------------------------------------------*/
    virtual bool Allocate (size_t Count, const HPMCountLimitedPoolItem* poolItem)
        {
        HPRECONDITION (!poolItem->GetPoolManaged());

        // Check if memory limit attained
        while (m_totalUsedCount + Count > m_countLimit)
            {
            // Not enough memory ... some other must be released
            // Request the least used uncompressed FastCompressiblePooledVector
            // to compress
            if (m_Pool.empty() || !m_Pool.back()->Discard())
                return false; // Something happened
            }

        Byte* pMemory;
        size_t actualAllocatedCount = Count;
        size_t actualAllocatedMemory = 0;
        if (m_memoryManager != NULL)
            {
            pMemory = m_memoryManager->AllocMemoryExt (Count, actualAllocatedMemory);
            actualAllocatedCount = actualAllocatedMemory; // Trunction of remainder is intended
            HASSERT (actualAllocatedCount >= Count);
            }
        else
            pMemory = new Byte[Count];

        HASSERT(pMemory != NULL);

        poolItem->SetMemory(pMemory, actualAllocatedCount);
        m_totalUsedCount += actualAllocatedCount;

        m_Pool.push_front(poolItem);
        poolItem->SetPoolIterator (m_Pool.begin());
        poolItem->SetPoolManaged(true);
        return true;
        }

    /**----------------------------------------------------------------------------
        Reallocates a number of datatypes and reassigns to provided pool item.
        The pool item must have been previously managed by the pool manager but simply
        change its allocation count (increase or decrease).
    -----------------------------------------------------------------------------*/
    virtual bool Reallocate (size_t NewCount, const HPMCountLimitedPoolItem* poolItem)
        {
        // This call will bring item to front and insure item is not discarded during the process
        NotifyAccess (poolItem);

        // Check if memory limit attained
        while (m_totalUsedCount + NewCount - poolItem->GetMemoryCount() > m_countLimit)
            {
            // Not enough memory ... some other must be released
            // Request the least used uncompressed FastCompressiblePooledVector
            // to compress
            if (m_Pool.empty() || !m_Pool.back()->Discard())
                return false; // Some error occured
            }

        Byte* newMemory;
        size_t actualAllocatedCount = NewCount;
        size_t actualAllocatedMemory = 0;

        if (m_memoryManager != NULL)
            {
            newMemory = m_memoryManager->AllocMemoryExt (NewCount, actualAllocatedMemory);
            actualAllocatedCount = actualAllocatedMemory; // Trunction of remainder is intended
            HASSERT (actualAllocatedCount >= NewCount);
            }
        else
            newMemory = new Byte[NewCount];

        memcpy(newMemory, poolItem->GetMemory(), MIN (NewCount, poolItem->GetMemoryCount()));
        m_totalUsedCount -= poolItem->GetMemoryCount();

        if (m_memoryManager != NULL)
            m_memoryManager->FreeMemory (reinterpret_cast<HPMCountLimitedPool*>(poolItem->GetMemory()), poolItem->GetMemoryCount());
        else
            delete [] poolItem->GetMemory();

        poolItem->SetMemory (newMemory, actualAllocatedCount);
        m_totalUsedCount += actualAllocatedCount;
        return true;
        }


    /**----------------------------------------------------------------------------
        Deallocates the memory used by the pool item and removed the pool item from
        pool management.
    -----------------------------------------------------------------------------*/
    virtual bool Free (const HPMCountLimitedPoolItem* poolItem)
        {
        if (poolItem->GetPoolManaged())
            {
// Critical section
            m_totalUsedCount -= poolItem->GetMemoryCount();
            if (m_memoryManager != NULL)
                m_memoryManager->FreeMemory (reinterpret_cast<Byte*>(poolItem->GetMemory()), poolItem->GetMemoryCount());
            else
                delete [] poolItem->GetMemory();
            poolItem->SetMemory(NULL, 0);
            m_Pool.erase (poolItem->GetPoolIterator());
            poolItem->SetPoolManaged (false);
// end critical section
            }

        return true;
        }

    /**----------------------------------------------------------------------------
        Removes the object from pool management. The memory used is NOT freed and
        must be freed by the object himself.
        IMPORTANT: If the pool uses a memory manager then this memory manager
        MUST be used to free the memory by the object. To make sure deallocation
        occurs correctly ALWAYS call FreeRemovedItemMemory upon this pool for such
        memory.
    -----------------------------------------------------------------------------*/
    virtual bool RemoveItem (const HPMCountLimitedPoolItem* poolItem)
        {
        if (poolItem->GetPoolManaged())
            {
// Critical section
            m_totalUsedCount -= poolItem->GetMemoryCount();

            // Notive that the memory is NOT release ... it becomes up to the pool item to deal with releasing
            // memory.

            m_Pool.erase (poolItem->GetPoolIterator());
            poolItem->SetPoolManaged (false);
// end critical section
            }

        return true;
        }

    /**----------------------------------------------------------------------------
    This method is called by objects that used the pool RemoveItem() function to
    prevent deallocation of the item while removing temporarily its memory from
    pool management. Typically this occurs only for Pinned objects that require
    memory to be maintained while removing it from pool management if pool decides
    that it should be discarded.
    -----------------------------------------------------------------------------*/
    virtual bool FreeRemovedItemMemory (const HPMCountLimitedPoolItem* poolItem)
        {
        // For this specific operation we do not expect the item to be part of the
        // pool management
        HASSERT (!poolItem->GetPoolManaged());

        if (m_memoryManager != NULL)
            m_memoryManager->FreeMemory (poolItem->GetMemory(), poolItem->GetMemoryCount());
        else
            delete [] poolItem->GetMemory();

        return true;
        }

    /**----------------------------------------------------------------------------
        Indicates to the pool that the object has been accessed and must thus be
        indicated as recently used in the MRU list maintained by the pool
    -----------------------------------------------------------------------------*/
    virtual void NotifyAccess (const HPMCountLimitedPoolItem* poolItem)
        {
        HPRECONDITION (poolItem->GetPoolManaged());

        // Check if the item is the one on front
        if (*m_Pool.begin() != *poolItem->GetPoolIterator())
            {
            // Not the one on front ...
// Critical section ...
            m_Pool.erase (poolItem->GetPoolIterator());
            m_Pool.push_front(poolItem);
            poolItem->SetPoolIterator(m_Pool.begin());
// End critical section
            }
        }

private:
    list<const HPMCountLimitedPoolItem*> m_Pool;
    size_t m_countLimit;
    size_t m_totalUsedCount;

    IHPMMemoryManager* m_memoryManager;

    };

//***********************************************************************************************************************
// Count Limited Pool Item
//
// This template class implements the basic functionality for managing blocks of a given fixed
// size Datatype. The container will inherit from this Count Limited Pool Item and implement
// the appropriate Discard() function. The Discard() implementation MUST call the pool Free() method
// to deallocate its memory otherwise the pool memory management administration will be incorrect.
// A pool item cannot refuse to discard. If for some reason, the memory allocated cannot be
// released, then the pool item may call RemoveFromManagement() method. This pool method will remove the
// memory of the item from management without deallocating it. In such case the pool item is responsible
// of releasing the memory used by itself
//
// The memory will allways be allocated by the pool and set internally in the Pool Item through the
// SetMemory() function only available to the Pool (through friendship)
//
// NOTE: In order to fucntion properly, the pool must be advised whenever a pool item is accessed
// Thus a call to NotifyAccess() method of the pool should be performed.
//***********************************************************************************************************************
class HPMCountLimitedPoolItem
    {


public:

    /**----------------------------------------------------------------------------
     Default Constructor.
     The pool must be set afterwards using SetPool() otherwise the pool item
     cannot operate
    -----------------------------------------------------------------------------*/
    HPMCountLimitedPoolItem ()
        {
        m_allocatedCount = 0;
        m_count = 0;
        HDEBUGCODE(m_countLoaded = false;);

        m_memory = NULL;

        m_pool = NULL;
        m_poolIteratorPtr = NULL;
        m_poolManaged = false;
        m_pinned = false;
        m_discarded = false;
        m_dirty = true; // This is the default value ... after construction use SetDirty() to unset
        }

    HPMCountLimitedPoolItem (HFCPtr<HPMCountLimitedPool>  pool)
        {
        m_allocatedCount = 0;
        m_count = 0;
        HDEBUGCODE(m_countLoaded = false;);

        m_memory = NULL;

        m_pool = pool;
        m_poolIteratorPtr = NULL;
        m_poolManaged = false;
        m_pinned = false;
        m_discarded = false;

        m_dirty = true; // This is the default value ... after construction use SetDirty() to unset
        }

    ~HPMCountLimitedPoolItem()
        {
        if (Pinned())
            {
            UnPin();
            }
        if (m_poolIteratorPtr != NULL)
            delete m_poolIteratorPtr;
        if (m_poolManaged)
            m_pool->Free(this);
        }


    /**----------------------------------------------------------------------------
     Sets the pool. It can only be set if it has not already been set after using
     the default constructor.
    -----------------------------------------------------------------------------*/
    void SetPool(HPMCountLimitedPool* pool) const
        {
        // Only allowed if the pool is not currently set
        HASSERT(m_pool == NULL);
        m_pool = pool;
        }

    /**----------------------------------------------------------------------------
     Returns the pool
    -----------------------------------------------------------------------------*/
    HPMCountLimitedPool* GetPool() const
        {
        return m_pool;
        }

    /**----------------------------------------------------------------------------
     Returns true if the item is dirty and false otherwise.
    -----------------------------------------------------------------------------*/
    bool IsDirty() const
        {
        return m_dirty;
        }

    /**----------------------------------------------------------------------------
     Sets the item dirty flag.
    -----------------------------------------------------------------------------*/
    virtual void SetDirty (bool dirty) const // Intentionaly const ... only mutable field changed
        {
        m_dirty = dirty;
        }


    /**----------------------------------------------------------------------------
     Tells that the item should be discarded and thus removed from pool management.
    -----------------------------------------------------------------------------*/
    virtual bool Discard() const = 0; // Intentionaly const ... only mutable members are modified

    /**----------------------------------------------------------------------------
     Returns true if the item is currently discarded
    -----------------------------------------------------------------------------*/
    bool Discarded() const
        {
        return m_discarded;
        }

    /**----------------------------------------------------------------------------
     Indicates that the item should be inflated and given over to pool management
    -----------------------------------------------------------------------------*/
    virtual bool Inflate() const = 0; // Intentionaly const ... only mutable members are modified

    /**----------------------------------------------------------------------------
     Returns true if the item is currently pool managed.
    -----------------------------------------------------------------------------*/
    bool GetPoolManaged()const
        {
        return m_poolManaged;
        }

    /**----------------------------------------------------------------------------
     Manually indicates that the item has been accesssed. This method allows to 
     force reset of item in the MRU (or other strategy) stack
    -----------------------------------------------------------------------------*/
    void NotifyAccess() const
        {
        if (GetPoolManaged())
            GetPool()->NotifyAccess(this);
        }


protected:

    /**----------------------------------------------------------------------------
     The Pin method inflates object into memory if required and prevents it from being
     discarded in actuality (the memory s not freed till unpinned).
    -----------------------------------------------------------------------------*/
    void Pin() const // Intentionaly const as only mutable members get modified
        {

        if (Discarded())
            Inflate();

        m_pinned = true;
        }

    /**----------------------------------------------------------------------------
     The Pinned method indicates if the item is pinned
    -----------------------------------------------------------------------------*/
    bool Pinned() const
        {
        return m_pinned;
        }

    /**----------------------------------------------------------------------------
     Unpins the item ... The memory will be immediately deleted if the object has been
     discarded while pinned.
    -----------------------------------------------------------------------------*/
    void UnPin() const
        {
        if (Pinned())
            {
            m_pinned = false;
            // Delayed discard
            if (Discarded())
                {
                if (m_memory != NULL)
                    delete [] m_memory;
                }
            }
        }

    /**----------------------------------------------------------------------------
     Indicates if the object is discarded or not.
    -----------------------------------------------------------------------------*/
    void SetDiscarded (bool discarded) const // intentionaly const at it only changes mutable members.
        {
        m_discarded = discarded;
        }

    /**----------------------------------------------------------------------------
     Returns pointer to memory used by pool item
    -----------------------------------------------------------------------------*/
    Byte* GetMemory() const
        {
        return m_memory;
        }


    /**----------------------------------------------------------------------------
     Returns the allocated count as the number of bytes used by memory
    -----------------------------------------------------------------------------*/
    size_t GetMemoryCount() const
        {
        return m_allocatedCount;
        }


    // Protected members TBD ... place as private members
    mutable bool m_pinned;
    mutable Byte* m_memory;
    mutable size_t m_count;
    HDEBUGCODE(mutable bool m_countLoaded);

    mutable size_t m_allocatedCount;
    mutable HFCPtr<HPMCountLimitedPool> m_pool;
    mutable bool m_discarded;

private:

    // Allow access to the pool of private methods below.
    friend HPMCountLimitedPool;

    // Sets the memory used by object ... called by pool (friend)
    void SetMemory(Byte* newMemory, size_t memoryCount) const // Intentionaly const ... only mutable members are modified
        {
        HASSERT (!Pinned());
        m_memory = newMemory;
        m_allocatedCount = memoryCount;
        }


    // Returns the pool iterator assigned to item
    typename std::list<const HPMCountLimitedPoolItem* >::iterator GetPoolIterator() const
        {
        if (m_poolIteratorPtr != NULL)
            return *m_poolIteratorPtr;
        else
            {
            std::list<const HPMCountLimitedPoolItem* >::iterator dummyItr;
            return dummyItr;
            }
        }

    // Sets the pool iterator ... called by the pool
    void SetPoolIterator(typename std::list<const HPMCountLimitedPoolItem* >::iterator poolIterator) const // Intentionaly const ... only mutable members are modified
        {
        if (m_poolIteratorPtr != NULL)
            delete m_poolIteratorPtr;

        m_poolIteratorPtr = new std::list<const HPMCountLimitedPoolItem* >::iterator(poolIterator);
        }

    // Sets if the pool is managed ... the item may be removed from pool management by the pool itself
    void SetPoolManaged(bool poolManaged) const // Intentionaly const ... only mutable members are modified
        {
        m_poolManaged = poolManaged;
        }

    mutable typename std::list<const HPMCountLimitedPoolItem* >::iterator* m_poolIteratorPtr;
    mutable bool m_poolManaged;
    mutable bool m_dirty;

    };



#else



//***********************************************************************************************************************
// HPMCountLimitedPool - Count Limited Memory Pool
// Manages a list of pool items that share a limited amount of instances of a given data type.
// The count limited pool limited management system is used to administer a shared amount of a given data type.
// The pool performs management through "count limited pool items" each of which contain a block of
// some number of DataType. The Data Type should usually be fixed size to result in effective memory management.
// Upon allocation, the normal C++ new operator is called so each
// of the individual instances of this DataType has its constructor called and the destroyer will likewise be called when
// deallocated using the delete [] operator.
//
// The pool allocates a specified amount of DataType objects when requested. This amount may not be greater than
// the limit specified. If the pool has already allocated more objects so the new allocation will result in
// exceeding the limit, the pool will call managed "count limited pool items" discard operation
// based on the Least Recently Used criterium
// till allocating the new block will not result in exceeding limit. In order for the Least Recently Used criterium be applied,
// every pool item managed is responsible of notifying the pool of its use using the NotifyAccess() method.
// It is possible to Reallocate a block of memory by calling the Reallocate() method. This method
// will reallocate a new memory block for the new count and copy the content of the former data to this new block.
//
// The pool can use of an allocator or memory manager to allocate or free memory if one is provided. Since the
// memory manager does not currently support typed allocation the DataType must not require explicit construction or
// destruction when allocated or deallocated when a memory manager is used. If no memory manager is provided then
// construction and destruction will occur correctly.
//***********************************************************************************************************************
template <typename DataType> class HPMCountLimitedPoolItem;

template <typename DataType> class HPMCountLimitedPool: public HFCShareableObject<HPMCountLimitedPool<DataType> >
    {
public:

    /**----------------------------------------------------------------------------
     Constructor.
     The pool count limit is provided at contruction. The pool will limit the total
     amount of DataType instances managed below this limit.
    -----------------------------------------------------------------------------*/
    HPMCountLimitedPool(size_t countLimit)
        {
        m_countLimit = countLimit;
        m_totalUsedCount = 0;
        m_memoryManager = NULL;
        }

    /**----------------------------------------------------------------------------
     Constructor.
     The pool count limit is provided at contruction. The pool will limit the total
     amount of DataType instances managed below this limit.
    -----------------------------------------------------------------------------*/
    HPMCountLimitedPool(IHPMMemoryManager* memoryManager, size_t countLimit)
        {
        m_countLimit = countLimit;
        m_totalUsedCount = 0;
        m_memoryManager = memoryManager;
        }

    /**----------------------------------------------------------------------------
     Destructor
     Normally upon destruction the pool should not be managing any objects !
    -----------------------------------------------------------------------------*/
    virtual ~HPMCountLimitedPool()
        {
        HASSERT(m_totalUsedCount == 0);
        }


    /**----------------------------------------------------------------------------
     Returns the count limit of DataType objects.
    -----------------------------------------------------------------------------*/
    size_t GetLimit() const
        {
        return m_countLimit;
        }


    /**----------------------------------------------------------------------------
     Returns the current actual count of DataType instances currently managed by pool.
    -----------------------------------------------------------------------------*/
    size_t GetActualCount() const
        {
        return m_totalUsedCount;
        }

    /**----------------------------------------------------------------------------
     Allows to change the limit of DataType instances that can exists and be managed by
     pool. If the new limit is smaller than previous limit and the actual count
     is greater than this new limit then the required instances will be discarded.
    -----------------------------------------------------------------------------*/
    void ChangeLimit(size_t pi_NewPoolLimit)
        {
        HPRECONDITION (pi_NewPoolLimit > 1);

        // Change the limit
        m_countLimit = pi_NewPoolLimit;

        // Check if memory limit exceeded
        while (m_totalUsedCount > m_countLimit)
            {
            // Not enough memory ... some other must be released
            // Request the least used uncompressed FastCompressiblePooledVector
            // to compress
            if (m_Pool.empty() || !m_Pool.back()->Discard())
                return false; // Something happened
            }

        }

    /**----------------------------------------------------------------------------
        Allocates a number of datatypes and assigns to provided pool item.
        The pool item is them managed by the pool manager.
    -----------------------------------------------------------------------------*/
    virtual bool Allocate(size_t Count, const HPMCountLimitedPoolItem<DataType>* poolItem)
        {
        HPRECONDITION (!poolItem->GetPoolManaged());

        // Check if memory limit attained
        while (m_totalUsedCount + Count > m_countLimit)
            {
            // Not enough memory ... some other must be released
            // Request the least used uncompressed FastCompressiblePooledVector
            // to compress
            if (m_Pool.empty() || !m_Pool.back()->Discard())
                return false; // Something happened
            }

        DataType* pMemory;
        size_t actualAllocatedCount = Count;
        size_t actualAllocatedMemory = 0;
        if (m_memoryManager != NULL)
            {
            pMemory = reinterpret_cast<DataType*>(m_memoryManager->AllocMemoryExt (Count * sizeof (DataType), actualAllocatedMemory));
            actualAllocatedCount = actualAllocatedMemory / sizeof(DataType); // Trunction of remainder is intended
            HASSERT (actualAllocatedCount >= Count);
            }
        else
            pMemory = new DataType[Count];

        HASSERT(pMemory != NULL);

        poolItem->SetMemory(pMemory, actualAllocatedCount);
        m_totalUsedCount += actualAllocatedCount;

        m_Pool.push_front(poolItem);
        poolItem->SetPoolIterator (m_Pool.begin());
        poolItem->SetPoolManaged(true);
        return true;
        }

    /**----------------------------------------------------------------------------
        Reallocates a number of datatypes and reassigns to provided pool item.
        The pool item must have been previously managed by the pool manager but simply
        change its allocation count (increase or decrease).
    -----------------------------------------------------------------------------*/
    virtual  bool Reallocate(size_t NewCount, const HPMCountLimitedPoolItem<DataType>* poolItem)
        {
        // This call will bring item to front and insure item is not discarded during the process
        NotifyAccess (poolItem);

        // Check if memory limit attained
        while (m_totalUsedCount + NewCount - poolItem->GetMemoryCount() > m_countLimit)
            {
            // Not enough memory ... some other must be released
            // Request the least used uncompressed FastCompressiblePooledVector
            // to compress
            if (m_Pool.empty() || !m_Pool.back()->Discard())
                return false; // Some error occured
            }

        DataType* newMemory;
        size_t actualAllocatedCount = NewCount;
        size_t actualAllocatedMemory = 0;

        if (m_memoryManager != NULL)
            {
            newMemory = reinterpret_cast<DataType*>(m_memoryManager->AllocMemoryExt (NewCount * sizeof (DataType), actualAllocatedMemory));
            actualAllocatedCount = actualAllocatedMemory / sizeof(DataType); // Trunction of remainder is intended
            HASSERT (actualAllocatedCount >= NewCount);
            }
        else
            newMemory = new DataType[NewCount];

        memcpy(newMemory, poolItem->GetMemory(), MIN (NewCount, poolItem->GetMemoryCount()) *sizeof(DataType));
        m_totalUsedCount -= poolItem->GetMemoryCount();

        if (m_memoryManager != NULL)
            m_memoryManager->FreeMemory (reinterpret_cast<Byte*>(poolItem->GetMemory()), poolItem->GetMemoryCount() * sizeof (DataType));
        else
            delete [] poolItem->GetMemory();

        poolItem->SetMemory (newMemory, actualAllocatedCount);
        m_totalUsedCount += actualAllocatedCount;
        return true;
        }


    /**----------------------------------------------------------------------------
        Deallocates the memory used by the pool item and removed the pool item from
        pool management.
    -----------------------------------------------------------------------------*/
    virtual bool Free(const HPMCountLimitedPoolItem<DataType>* poolItem)
        {
        if (poolItem->GetPoolManaged())
            {
// Critical section
            m_totalUsedCount -= poolItem->GetMemoryCount();
            if (m_memoryManager != NULL)
                m_memoryManager->FreeMemory (reinterpret_cast<Byte*>(poolItem->GetMemory()), poolItem->GetMemoryCount() * sizeof (DataType));
            else
                delete [] poolItem->GetMemory();
            poolItem->SetMemory(NULL, 0);
            m_Pool.erase (poolItem->GetPoolIterator());
            poolItem->SetPoolManaged (false);
// end critical section
            }

        return true;
        }

    /**----------------------------------------------------------------------------
        Removes the object from pool management. The memory used is NOT freed and
        must be freed by the object himself.
        IMPORTANT: If the pool uses a memory manager then this memory manager
        MUST be used to free the memory by the object. To make sure deallocation
        occurs correctly ALWAYS call FreeRemovedItemMemory upon this pool for such
        memory.
    -----------------------------------------------------------------------------*/
    virtual bool RemoveItem(const HPMCountLimitedPoolItem<DataType>* poolItem)
        {
        if (poolItem->GetPoolManaged())
            {
// Critical section
            m_totalUsedCount -= poolItem->GetMemoryCount();

            // Notive that the memory is NOT release ... it becomes up to the pool item to deal with releasing
            // memory.

            m_Pool.erase (poolItem->GetPoolIterator());
            poolItem->SetPoolManaged (false);
// end critical section
            }

        return true;
        }

    /**----------------------------------------------------------------------------
    This method is called by objects that used the pool RemoveItem() function to
    prevent deallocation of the item while removing temporarily its memory from
    pool management. Typically this occurs only for Pinned objects that require
    memory to be maintained while removing it from pool management if pool decides
    that it should be discarded.
    -----------------------------------------------------------------------------*/
    bool FreeRemovedItemMemory (const HPMCountLimitedPoolItem<DataType>* poolItem)
        {
        // For this specific operation we do not expect the item to be part of the
        // pool management
        HASSERT (!poolItem->GetPoolManaged());

        if (m_memoryManager != NULL)
            m_memoryManager->FreeMemory (poolItem->GetMemory(), poolItem->GetMemoryCount() * sizeof (DataType));
        else
            delete [] poolItem->GetMemory();

        return true;
        }

    /**----------------------------------------------------------------------------
        Indicates to the pool that the object has been accessed and must thus be
        indicated as recently used in the MRU list maintained by the pool
    -----------------------------------------------------------------------------*/
    void NotifyAccess (const HPMCountLimitedPoolItem<DataType>* poolItem)
        {
        HPRECONDITION (poolItem->GetPoolManaged());

        // Check if the item is the one on front
        if (*m_Pool.begin() != *poolItem->GetPoolIterator())
            {
            // Not the one on front ...
// Critical section ...
            m_Pool.erase (poolItem->GetPoolIterator());
            m_Pool.push_front(poolItem);
            poolItem->SetPoolIterator(m_Pool.begin());
// End critical section
            }
        }

protected:
    list<const HPMCountLimitedPoolItem<DataType>*> m_Pool;
    size_t m_countLimit;
    size_t m_totalUsedCount;

    IHPMMemoryManager* m_memoryManager;

    };

//***********************************************************************************************************************
// Count Limited Pool Item
//
// This template class implements the basic functionality for managing blocks of a given fixed
// size Datatype. The DataType container will inherit from this Count Limited Pool Item and implement
// the appropriate Discard() function. The Discard() implementation MUST call the pool Free() method
// to deallocate its memory otherwise the pool memory management administration will be incorrect.
// A pool item cannot refuse to discard. If for some reason, the memory allocated cannot be
// released, then the pool item may call RemoveFromManagement() method. This pool method will remove the
// memory of the item from management without deallocating it. In such case the pool item is responsible
// of releasing the memory used by itself
//
// The memory will allways be allocated by the pool and set internally in the Pool Item through the
// SetMemory() function only available to the Pool (through friendship)
//
// NOTE: In order to fucntion properly, the pool must be advised whenever a pool item is accessed
// Thus a call to NotifyAccess() method of the pool should be performed.
//***********************************************************************************************************************
template <typename DataType> class HPMCountLimitedPoolItem
    {


public:

    /**----------------------------------------------------------------------------
     Default Constructor.
     The pool must be set afterwards using SetPool() otherwise the pool item
     cannot operate
    -----------------------------------------------------------------------------*/
    HPMCountLimitedPoolItem ()
        {
        m_allocatedCount = 0;
        m_count = 0;
        HDEBUGCODE(m_countLoaded = false;);

        m_memory = NULL;

        m_pool = NULL;
        m_poolIteratorPtr = NULL;
        m_poolManaged = false;
        m_pinned = false;
        m_discarded = false;
        m_dirty = true; // This is the default value ... after construction use SetDirty() to unset
        }

    HPMCountLimitedPoolItem (HFCPtr<HPMCountLimitedPool<DataType> >  pool)
        {
        m_allocatedCount = 0;
        m_count = 0;
        HDEBUGCODE(m_countLoaded = false;);

        m_memory = NULL;

        m_pool = pool;
        m_poolIteratorPtr = NULL;
        m_poolManaged = false;
        m_pinned = false;
        m_discarded = false;

        m_dirty = true; // This is the default value ... after construction use SetDirty() to unset
        }

    ~HPMCountLimitedPoolItem()
        {
        if (Pinned())
            {
            UnPin();
            }
        if (m_poolIteratorPtr != NULL)
            delete m_poolIteratorPtr;
        if (m_poolManaged)
            m_pool->Free(this);
        }


    /**----------------------------------------------------------------------------
     Sets the pool. It can only be set if it has not already been set after using
     the default constructor.
    -----------------------------------------------------------------------------*/
    void SetPool(HPMCountLimitedPool<DataType>* pool) const
        {
        // Only allowed if the pool is not currently set
        HASSERT(m_pool == NULL);
        m_pool = pool;
        }

    /**----------------------------------------------------------------------------
     Returns the pool
    -----------------------------------------------------------------------------*/
    HPMCountLimitedPool<DataType>* GetPool() const
        {
        return m_pool;
        }

    /**----------------------------------------------------------------------------
     Returns true if the item is dirty and false otherwise.
    -----------------------------------------------------------------------------*/
    bool IsDirty() const
        {
        return m_dirty;
        }

    /**----------------------------------------------------------------------------
     Sets the item dirty flag.
    -----------------------------------------------------------------------------*/
#if (0)
    void SetDirty (bool dirty) const // Intentionaly const ... only mutable field changed
#else
    virtual void SetDirty (bool dirty) const // Intentionaly const ... only mutable field changed
#endif
        {
        m_dirty = dirty;
        }


    /**----------------------------------------------------------------------------
     Tells that the item should be discarded and thus removed from pool management.
    -----------------------------------------------------------------------------*/
    virtual bool Discard() const = 0; // Intentionaly const ... only mutable members are modified

    /**----------------------------------------------------------------------------
     Returns true if the item is currently discarded
    -----------------------------------------------------------------------------*/
    bool Discarded() const
        {
        return m_discarded;
        }

    /**----------------------------------------------------------------------------
     Indicates that the item should be inflated and given over to pool management
    -----------------------------------------------------------------------------*/
    virtual bool Inflate() const = 0; // Intentionaly const ... only mutable members are modified

    /**----------------------------------------------------------------------------
     Returns true if the item is currently pool managed.
    -----------------------------------------------------------------------------*/
    bool GetPoolManaged()const
        {
        return m_poolManaged;
        }

    /**----------------------------------------------------------------------------
    Indicates if the object is discarded or not.
    -----------------------------------------------------------------------------*/
    void SetDiscarded(bool discarded) const // intentionaly const at it only changes mutable members.
        {
        m_discarded = discarded;
        }


protected:

    /**----------------------------------------------------------------------------
     The Pin method inflates object into memory if required and prevents it from being
     discarded in actuality (the memory s not freed till unpinned).
    -----------------------------------------------------------------------------*/
    void Pin() const // Intentionaly const as only mutable members get modified
        {
        if (Discarded())
            Inflate();

        m_pinned = true;
        }

    /**----------------------------------------------------------------------------
     The Pinned method indicates if the item is pinned
    -----------------------------------------------------------------------------*/
    bool Pinned() const
        {
        return m_pinned;
        }

    /**----------------------------------------------------------------------------
     Unpins the item ... The memory will be immediately deleted if the object has been
     discarded while pinned.
    -----------------------------------------------------------------------------*/
    void UnPin() const
        {
        if (Pinned())
            {
            m_pinned = false;
            // Delayed discard
            if (Discarded())
                {
                if (m_memory != NULL)
                    delete [] m_memory;
                }
            }
        }

    /**----------------------------------------------------------------------------
     Returns pointer to memory used by pool item
    -----------------------------------------------------------------------------*/
    DataType* GetMemory() const
        {
        return m_memory;
        }


    /**----------------------------------------------------------------------------
     Returns the allocated count as the number of bytes used by memory
    -----------------------------------------------------------------------------*/
    size_t GetMemoryCount() const
        {
        return m_allocatedCount;
        }


    // Protected members TBD ... place as private members
    mutable bool m_pinned;
    mutable DataType* m_memory;
    mutable size_t m_count;
    HDEBUGCODE(mutable bool m_countLoaded);

    mutable size_t m_allocatedCount;
    mutable HFCPtr<HPMCountLimitedPool<DataType> > m_pool;
    mutable bool m_discarded;



    // Allow access to the pool of private methods below.
    friend HPMCountLimitedPool<DataType>;
    friend HPMIndirectCountLimitedPool<DataType>;

    // Sets the memory used by object ... called by pool
    void SetMemory(DataType* newMemory, size_t memoryCount) const // Intentionaly const ... only mutable members are modified
        {
        HASSERT (!Pinned());
        m_memory = newMemory;
        m_allocatedCount = memoryCount;
        }

    // Returns the pool iterator assigned to item
    typename std::list<const HPMCountLimitedPoolItem<DataType>* >::iterator GetPoolIterator() const
        {
#if (0)
        HPRECONDITION (m_poolManaged);
        return *m_poolIteratorPtr;
#else

        if (m_poolIteratorPtr != NULL)
            return *m_poolIteratorPtr;
        else
            {
            std::list<const HPMCountLimitedPoolItem<DataType>* >::iterator dummyItr;
            return dummyItr;
            }
#endif
        }

    // Sets the pool iterator ... called by the pool
    void SetPoolIterator(typename std::list<const HPMCountLimitedPoolItem<DataType>* >::iterator poolIterator) const // Intentionaly const ... only mutable members are modified
        {
        if (m_poolIteratorPtr != NULL)
            delete m_poolIteratorPtr;

        m_poolIteratorPtr = new std::list<const HPMCountLimitedPoolItem<DataType>* >::iterator(poolIterator);
        }



    // Sets if the pool is managed ... the item may be removed from pool management by the pool itself
    void SetPoolManaged(bool poolManaged) const // Intentionaly const ... only mutable members are modified
        {
        m_poolManaged = poolManaged;
        }

    mutable typename std::list<const HPMCountLimitedPoolItem<DataType>* >::iterator* m_poolIteratorPtr;
    mutable bool m_poolManaged;
    mutable bool m_dirty;

    };


#endif

END_IMAGEPP_NAMESPACE