//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HPMPooledVector.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#pragma once


#include "HPMCountLimitedPool.h"
#include "HPMIndirectCountLimitedPool.h"
#include "HPMDataStore.h"
#include <ImagePP/h/HIterators.h>

BEGIN_IMAGEPP_NAMESPACE

//***********************************************************************************************************************
// This general class mimics the STL std::vector class yet is very so slightly faster but not so general
//***********************************************************************************************************************
template <typename DataType> class FastVector
    {
public:

    FastVector ()
        {
        m_allocatedCount = 0;
        m_count = 0;
        m_memory = NULL;
        }
    ~FastVector()
        {
        if (m_allocatedCount > 0)
            delete m_memory;
        }
    bool reserve(size_t newCount)
        {
        if (newCount > m_allocatedCount)
            {
            void* newMemory = new DataType[newCount];
            if (newMemory == NULL)
                throw std::bad_alloc();
            memcpy (newMemory, m_memory, sizeof(DataType)*m_count);
            delete [] m_memory;
            m_memory = reinterpret_cast<DataType*>(newMemory);
            m_allocatedCount = newCount;
            }
        return true;
        }
    size_t capacity() const
        {
        return m_allocatedCount;
        }
    bool push_back(const DataType& newObject)
        {
        if((m_allocatedCount <= m_count) && !reserve (m_count + 1))
            throw std::bad_alloc();

        m_memory[m_count] = newObject;
        m_count++;
        return true;
        }
    bool push_back(DataType* newObjects, size_t count)
        {
        íf (m_allocatedCount <= (m_count + count))
        if (!reserve (m_count + count))
                throw std::bad_alloc();

        memcpy (&(m_memory[m_count]), newObjects, sizeof(DataType) * count);
        return true;
        }
    DataType& operator[](size_t index) const
        {
        HPRECONDITION(index < m_count);
        return m_memory[index];
        }
    size_t size() const
        {
        return m_count;
        }
    void erase (size_t index)
        {
        if (index < m_count - 1)
            memcpy(&(m_memory[index]), &(m_memory[index+1]), sizeof(DataType) * (m_count - 1 - index));
        m_count--;
        }
    void clear()
        {
        if (m_allocatedCount > 0)
            delete [] m_memory;
        m_count = 0;
        m_allocatedCount = 0;
        }
private:
    DataType* m_memory;
    size_t m_count;
    size_t m_allocatedCount;

    };


//***********************************************************************************************************************
// Memory managed vector
// The present vector is simply a vector class (with limited interface) that uses a memory manager to allocate and deallocate
// memory.
//
// For the purpose of providing an additional service, the vector may be given an alignment. This alignment is the
// smallest fractional portion of memory that can be allocated. For example assuming an alignment of 1000 and a requested amount
// of 7777 objects, the actual amount of data that will be requested from the pool will be 8000. This simple feature
// allows to minimize somewhat memory fragmentation by diminishing the number of possible memory size allocation.
//
//
//***********************************************************************************************************************
template <typename DataType> class HPMMemoryManagedVector
    {
public:
    typedef DataType value_type;

    template <class T> class iteratorBase : public BentleyApi::ImagePP::RandomAccessIteratorWithAutoReverseConst<iteratorBase<T>, iteratorBase<typename BentleyApi::ImagePP::ReverseConstTrait<T>::type >, T>
        {
    public:
        static const size_t npos = -1;

        iteratorBase() {
            m_vector = NULL;
            m_index = iteratorBase::npos;
            }
        iteratorBase(HPMMemoryManagedVector<DataType>* vector, size_t initialPosition) {
            m_vector = vector;
            m_index = initialPosition; /* if (m_vector != NULL) m_vector->Pin();*/
            }
        const_iterator_t    ConvertToConst () const
            {
            return const_iterator_t(m_vector, m_index);
            }

        const_reference     Dereference    () const {
            return m_vector->PriviledgeAccessForIteratorOnly(m_index);
            }
        reference           Dereference    () {
            return m_vector->PriviledgeAccessForIteratorOnly(m_index);
            }
        void                Increment      ()
            {
            m_index++;
            if (m_index >= m_vector->size())
                m_index = npos;
            }

        void                Decrement      ()
            {
            if (m_index == 0)
                m_index = npos;
            else
                m_index--;
            }

        void                AdvanceOf        (difference_type increment)
            {
            HASSERT (m_index + increment > m_vector->size());

            m_index += increment;

            if (m_index >= m_vector->size())
                m_index = npos;
            }

        difference_type     DistanceFrom   (const iterator_t& otherItr) const
            {
            HASSERT (m_index != npos);
            HASSERT (otherItr.m_index != npos);

            HASSERT (m_vector == otherItr.m_vector);

            return m_index - otherItr.m_index;
            }

        bool                EqualTo        (const iterator_t& otherItr) const
            {
            HASSERT (m_vector == otherItr.m_vector);

            return ((m_vector == otherItr.m_vector) && (m_index == otherItr.m_index));

            }
        bool                LessThan       (const iterator_t&) const
            {
            HASSERT (m_vector == otherItr.m_vector);

            return m_index < otherItr.m_index;
            }

    private:
        friend HPMMemoryManagedVector<DataType>;
        iteratorBase(const HPMMemoryManagedVector<DataType>* vector, size_t index) {
            m_vector = const_cast<HPMMemoryManagedVector<DataType>*>(vector);
            m_index = index;
            }
        HPMMemoryManagedVector<DataType>* m_vector;
        size_t m_index;


        };

    typedef iteratorBase<DataType> iterator;
    typedef iteratorBase<const DataType> const_iterator ;

    /** -----------------------------------------------------------------------------

        Creates a pooled vector. Before using to its full extent the pool must be set using
        the SetPool() method.

        -----------------------------------------------------------------------------
    */
    HPMMemoryManagedVector (IHPMMemoryManager* memoryManager)
        {
        m_memoryManager = memoryManager;
        m_allocatedCount = 0;
        m_allocatedByteSize = 0;
        m_count = 0;
        m_memory = NULL;
        }

    /** -----------------------------------------------------------------------------

        Destroyer. If the object is pool managed then it is removed from the pool
        during destruction.

        -----------------------------------------------------------------------------
    */

    ~HPMMemoryManagedVector()
        {
        if (m_allocatedCount > 0)
            {
            m_memoryManager->FreeMemory((Byte*)m_memory, m_allocatedByteSize);
            m_allocatedCount = 0;
            m_allocatedByteSize = 0;
            }
        }


    virtual bool reserve(size_t newCount)
        {
        // We tolerate that the reserve count be 0 yet return immediately
        if (newCount == 0)
            return true;

        if (newCount > m_allocatedCount)
            {
            if (m_allocatedCount == 0)
                {
                m_memory = (DataType*)m_memoryManager->AllocMemoryExt (newCount * sizeof(DataType), m_allocatedByteSize);
                HASSERT (m_memory != NULL);

                // Integer division is required here
                m_allocatedCount = m_allocatedByteSize / sizeof(DataType);
                }
            else
                {
                void* temp = m_memory;
                size_t tempSize = m_allocatedByteSize;
                m_memory = (DataType*)m_memoryManager->AllocMemoryExt (newCount * sizeof(DataType), m_allocatedByteSize);
                HASSERT (m_memory != NULL);

                // Integer division is required here
                m_allocatedCount = m_allocatedByteSize / sizeof(DataType);
                memcpy(m_memory, temp, m_count * sizeof(DataType));

                m_memoryManager->FreeMemory((Byte*)temp, tempSize);
                }
            }
        return true;
        }

    size_t capacity() const
        {
        return m_allocatedCount;
        }
    bool push_back(const DataType& newObject)
        {
        if((m_allocatedCount <= m_count) && !reserve ((m_count * 3)/2 + 2))
            throw std::bad_alloc();


        m_memory[m_count] = newObject;
        m_count++;
        return true;
        }

    // This used to be a reference returned but this caused a problem when the ref was maintained and the
    // tile discarded...
    const DataType& operator[](size_t index) const
        {
        HPRECONDITION(index < m_count);

        return m_memory[index];
        }

    // This used to be a reference returned but this caused a problem when the ref was maintained and the
    // tile discarded...
    DataType& PriviledgeAccessForIteratorOnly(size_t index) const
        {
        HPRECONDITION(index < m_count);

        return m_memory[index];
        }


//    void modify(size_t          index,
//                const DataType& newDataValue)
//    {
//        m_memory[index] = newDataValue;
//    }

    size_t size() const
        {
        return m_count;
        }
    iterator erase (iterator itr)
        {
        if (itr.m_vector != this)
            return iterator();

        this->erase(itr.m_index);

        if (itr.m_index == 0)
            return iterator(this, iterator::npos);

        if (itr.m_index > m_count)
            return iterator(this, iterator::npos);

        return iterator(this, itr.m_index - 1);
        }

    void erase (size_t index)
        {
        if (index < m_count - 1)
            memcpy(&(m_memory[index]), &(m_memory[index+1]), sizeof(DataType) * (m_count - 1 - index));
        m_count--;


        if (m_allocatedCount > 0)
            {
            m_memoryManager->FreeMemory((Byte*)m_memory, m_allocatedByteSize);
            m_allocatedCount = 0;
            m_allocatedByteSize = 0;
            }
        }


    virtual void clear()
        {

        if (m_allocatedCount > 0)
            {
            m_memoryManager->FreeMemory((Byte*)m_memory, m_allocatedByteSize);

            m_allocatedCount = 0;
            m_allocatedByteSize = 0;
            }

        m_count = 0;
        }

    iterator begin()
        {
        if (m_count > 0)
            return iterator(this, (size_t)0);
        return iterator(this, iterator::npos);

        }
    const_iterator begin() const
        {
        if (m_count > 0)
            return const_iterator(this, (size_t)0);
        return const_iterator(this, iterator::npos);

        }

    iterator end()
        {
        return iterator(this, iterator::npos);
        }
    const_iterator end() const
        {
        return const_iterator(this, iterator::npos);
        }

private:

    IHPMMemoryManager* m_memoryManager;
    size_t m_count;
    mutable size_t m_allocatedCount; // Contains the number of DataType that can be stored in the allocated memory
    // this value may be different from the actual memory size allocated
    mutable size_t m_allocatedByteSize; // Contains the exact number of bytes allocated in memory
    DataType* m_memory;
    };

    template<typename DataType> struct PoolItem
        {
        typedef HPMCountLimitedPoolItem<DataType> Type;
        typedef HPMCountLimitedPool<DataType> PoolType;
        };

    template<> struct PoolItem<MTGGraph>
        {
        typedef HPMIndirectCountLimitedPoolItem<MTGGraph> Type;
        typedef HPMIndirectCountLimitedPool<MTGGraph> PoolType;
        };

//***********************************************************************************************************************
// The present vector is simply a vector class (with limited interface) that can discard
// its data according to memory availability. This memory availability maintenance is provided by a count
// limited  pool. It implements all mandatory behavior related to HPMCountLimitedPoolItem,
// by advising the pool of use and making sure it is discarded or not and requesting inflation() if data is to be accessed
// and is needed back. It does not however provide any implementation of Discard() and Inflate() methods since the class
// has absolutely no well-defined knowledge of the meaning of such operation. These Discard() and Inflate() could have
// equally been called MakeAvailable or MakeUnavailable() since from the point of view of the vector all it recognises
// is the fact that since pooled some of the data may not be readily available and the Inflate() operation must then be
// called. Here we say that the data is not readily available instead of immediately in order to underline that even though
// the data must be rendered available this operation can be performed immediately and the process will wait for the data
// to be in fact available. This definition should hint to the fact the Inflate/Discard mechanism is not meant to be used
// for slow data access such as data requested from a remote server accessed through Internet. Other mechanisms and
// vector interface will have to be devised to support such slow operation.
//
// The pooled vector makes use of a pool. The pool is responsible for managing the amount of objects readily available and
// from allocating/deallocating the memory to maintain these objects in memory (except when no pool is set in the pooled vector)
//
// For the purpose of providing an additional service, the pooled vector may be given an alignment. This alignment is the
// smallest fractional portion of memory that can be allocated. For example assuming an alignment of 1000 and a requested amount
// of 7777 objects, the actual amount of data that will be requested from the pool will be 8000. This simple feature
// allows to minimize somewhat memory fragmentation by diminishing the number of possible memory size allocation.
// Note that the allocation mechanism of the pool should normally be in charge of such alignment considerations but can be
// set nevertheless when the normal allocation mechanism (new / delete) is used.
//
// Although the class provides mainly the normal vector<> interface implementation it does provide additional interfaces
// that can be useful for purpose of optimisation. For example an array of objects can be added in a simple push_back call
// similarly a copy of another pooled vector can be made also using push_back. Similarly a copy of the array of objects
// can be extracted using the get() method.
//
// A randow_shuffle() method has been added for performance issues. Although the normal random_shuffle STL algorithm can
// operate on the pooled vector using the normal iterator interface provided, the random shuffling of the raw memory
// array of objects directly is far faster.
//
// Also for performance reasons a clearFrom method has also been added. It simply sets the count or size of the content
// thus eliminating the remaining objects. Note that in this case the destructor of these cleared objects will only be called
// when the memory will be freed.
//
//***********************************************************************************************************************
    template <typename DataType> class HPMPooledVector : public PoolItem<DataType>::Type
    {
public:
    typedef DataType value_type;

    template <class T> class iteratorBase : public BentleyApi::ImagePP::RandomAccessIteratorWithAutoReverseConst<iteratorBase<T>, iteratorBase<typename BentleyApi::ImagePP::ReverseConstTrait<T>::type >, T>
        {
    public:
        static const size_t npos = -1;

        iteratorBase() {
            m_vector = NULL;
            m_index = iteratorBase::npos;
            }
        iteratorBase(HPMPooledVector<DataType>* vector, size_t initialPosition) {
            m_vector = vector;
            m_index = initialPosition; /* if (m_vector != NULL) m_vector->Pin();*/
            }
        const_iterator_t    ConvertToConst () const
            {
            return const_iterator_t(m_vector, m_index);
            }

        const_reference     Dereference    () const {
            return m_vector->PriviledgeAccessForIteratorOnly(m_index);
            }
        reference           Dereference    () {
            return m_vector->PriviledgeAccessForIteratorOnly(m_index);
            }
        void                Increment      ()
            {
            m_index++;
            if (m_index >= m_vector->size())
                m_index = npos;
            }

        void                Decrement      ()
            {
            if (m_index == 0)
                m_index = npos;
            else
                m_index--;
            }

        void                AdvanceOf        (difference_type increment)
            {
            HASSERT (m_index + increment > m_vector->size());

            m_index += increment;

            if (m_index >= m_vector->size())
                m_index = npos;
            }

        difference_type     DistanceFrom   (const iterator_t& otherItr) const
            {
            HASSERT (m_index != npos);
            HASSERT (otherItr.m_index != npos);

            HASSERT (m_vector == otherItr.m_vector);

            return m_index - otherItr.m_index;
            }

        bool                EqualTo        (const iterator_t& otherItr) const
            {
            HASSERT (m_vector == otherItr.m_vector);

            return ((m_vector == otherItr.m_vector) && (m_index == otherItr.m_index));

            }
        bool                LessThan       (const iterator_t&) const
            {
            HASSERT (m_vector == otherItr.m_vector);

            return m_index < otherItr.m_index;
            }

    private:
        friend HPMPooledVector<DataType>;
        iteratorBase(const HPMPooledVector<DataType>* vector, size_t index) {
            m_vector = const_cast<HPMPooledVector<DataType>*>(vector);
            m_index = index;
            }
        HPMPooledVector<DataType>* m_vector;
        size_t m_index;


        };

    typedef iteratorBase<DataType> iterator;
    typedef iteratorBase<const DataType> const_iterator ;

    /** -----------------------------------------------------------------------------

        Creates a pooled vector. Before using to its full extent the pool must be set using
        the SetPool() method.

        -----------------------------------------------------------------------------
    */
    HPMPooledVector ()
        : PoolItem<DataType>::Type()
        {
        m_allocatedCount = 0;
        m_count = 0;
        m_memory = NULL;

        m_accessCount=0;
        }

    /** -----------------------------------------------------------------------------

        Creates a pooled vector. The pool used for the memory count management is
        provided.

        -----------------------------------------------------------------------------
    */
    HPMPooledVector(HFCPtr< typename PoolItem<DataType>::PoolType > pool)
        : PoolItem<DataType>::Type(pool)
        {
        m_allocatedCount = 0;
        m_count = 0;
        m_memory = NULL;


        m_accessCount=0;
        }
    /** -----------------------------------------------------------------------------

        Destroyer. If the object is pool managed then it is removed from the pool
        during destruction.

        -----------------------------------------------------------------------------
    */

    ~HPMPooledVector()
        {
        if (m_pool != NULL)
            if (!GetPoolManaged())
                m_pool->Free(this);
        }


    virtual bool reserve(size_t newCount)
        {
        if (Discarded())
            Inflate();

        HASSERT(!Discarded());

        // We tolerate that the reserve count be 0 yet return immediately
        if (newCount == 0)
            return true;

        if (newCount > m_allocatedCount)
            {
            if (m_pool != NULL)
                {
                bool allocationSuccess = true;
                if (!GetPoolManaged())
                    allocationSuccess = m_pool->Allocate (newCount, this);
                else
                    allocationSuccess = m_pool->Reallocate (newCount, this);

                if (!allocationSuccess)
                    throw std::bad_alloc();
                }
            else
                {
                void* newMemory = new DataType[newCount];
                if (newMemory == NULL)
                    throw std::bad_alloc();

                memcpy (newMemory, m_memory, sizeof(DataType)*m_count);
                delete [] m_memory;
                m_memory = reinterpret_cast<DataType*>(newMemory);
                m_allocatedCount = newCount;
                }
            SetDirty(true);
            }
        return true;
        }

    size_t capacity() const
        {
        if (Discarded())
            Inflate();

        HASSERT(!Discarded());

        return m_allocatedCount;
        }
    bool push_back(const DataType& newObject)
        {
        if (Discarded())
            Inflate();

        HASSERT(!Discarded());

        if((m_allocatedCount <= m_count) && !reserve ((m_count * 3)/2 + 2))
            throw std::bad_alloc();
   
        m_accessCount++;
        if ((m_accessCount > 100) && (m_pool != NULL))
            {
            m_pool->NotifyAccess(this);
            m_accessCount = 0;
            }

        m_memory[m_count] = newObject;
        m_count++;
        SetDirty(true);
        return true;
        }
    /** -----------------------------------------------------------------------------

        This push_back overload does not exist in std::vector but is provided for
        performance reason. It copies the provided pooled vector to self.

        -----------------------------------------------------------------------------
    */
    bool push_back(const HPMPooledVector<DataType>* source)
        {
        if (source->size() == 0)
            return false;

        // Pin the pooled vector given as source in case it gets discarded in the process
        source->Pin();

        if (Discarded())
            Inflate();

        HASSERT(!Discarded());

        if (m_allocatedCount <= (m_count + source->size()))
            if (!reserve (m_count + source->size()))
                throw std::bad_alloc();

        m_accessCount+=source->size();
        if ((m_accessCount > 100) && (m_pool != NULL))
            {
            m_pool->NotifyAccess(this);
            m_accessCount = 0;
            }

        memcpy (&(m_memory[m_count]), source->m_memory, sizeof(DataType) * source->size());
        m_count += source->size();
        SetDirty(true);
        source->UnPin();

        return true;
        }

    /** -----------------------------------------------------------------------------

        This push_back overload does not exist in std::vector but is provided for
        performance reason. It copies the provided pooled vector to self from given
        index to specified end index included.

        -----------------------------------------------------------------------------
    */
    bool push_back(const HPMPooledVector<DataType>* source, size_t start, size_t end)
        {
        HASSERT (end > start);
        HASSERT (end < source->size());

        if (source->size() == 0)
            return false;

        // Pin the pooled vector given as source in case it gets discarded in the process
        source->Pin();

        if (Discarded())
            Inflate();

        HASSERT(!Discarded());

        if (m_allocatedCount <= (m_count + (end - start + 1)))
            if (!reserve (m_count + (end - start + 1)))
                throw std::bad_alloc();

        m_accessCount += (end - start + 1);
        if ((m_accessCount > 100) && (m_pool != NULL))
            {
            m_pool->NotifyAccess(this);
            m_accessCount = 0;
            }

        memcpy (&(m_memory[m_count]), &(source->m_memory[start]), sizeof(DataType) * (end - start + 1));
        m_count += (end - start + 1);
        SetDirty(true);
        source->UnPin();

        return true;
        }

    /** -----------------------------------------------------------------------------

        This push_back overload does not exist in std::vector but is provided for
        performance reason. It copies into the vector many DataTypes in a single call.

        -----------------------------------------------------------------------------
    */
    bool push_back(const DataType* newObjects, size_t count)
        {
        if (count == 0)
            return false;

        if (Discarded())
            Inflate();

        HASSERT(!Discarded());

        if (m_allocatedCount <= (m_count + count))
            if (!reserve (m_count + count))
                throw std::bad_alloc();

        m_accessCount+=count;
        if ((m_accessCount > 100) && (m_pool != NULL))
            {
            m_pool->NotifyAccess(this);
            m_accessCount = 0;
            }

        memcpy (&(m_memory[m_count]), newObjects, sizeof(DataType) * count);
        m_count += count;
        SetDirty(true);

        return true;
        }


    /** -----------------------------------------------------------------------------

        This get method does not exist in std::vector but is provided for
        performance reason. It copies into the array the content of the vector in a single call.

        It returns the number of data type returned
        -----------------------------------------------------------------------------
    */
    size_t get(DataType* objects, size_t maxCount) const
        {
        if (maxCount == 0)
            return false;

        if (Discarded())
            Inflate();

        HASSERT(!Discarded());

        m_accessCount += MIN(m_count, maxCount);
        if ((m_accessCount > 100) && (m_pool != NULL))
            {
            m_pool->NotifyAccess(this);
            m_accessCount = 0;
            }

        memcpy (objects, m_memory, sizeof(DataType) * MIN (m_count, maxCount));

        return MIN(m_count, maxCount);
        }

    // This used to be a reference returned but this caused a problem when the ref was maintained and the
    // tile discarded...
    const DataType& operator[](size_t index) const
        {
        if (Discarded())
            Inflate();

        HASSERT(!Discarded());

        HPRECONDITION(index < m_count);

        m_accessCount++;
        if ((m_accessCount > 100) && (m_pool != NULL))
            {
            m_pool->NotifyAccess(this);
            m_accessCount = 0;
            }

        return m_memory[index];
        }

    // This used to be a reference returned but this caused a problem when the ref was maintained and the
    // tile discarded...
    DataType& PriviledgeAccessForIteratorOnly(size_t index) const
        {
        if (Discarded())
            Inflate();

        HASSERT(!Discarded());

        HPRECONDITION(index < m_count);

        m_accessCount++;
        if ((m_accessCount > 100) && (m_pool != NULL))
            {
            m_pool->NotifyAccess(this);
            m_accessCount = 0;
            }

        return m_memory[index];
        }


    void modify(size_t          index,
                const DataType& newDataValue)
        {
        m_memory[index] = newDataValue;
        SetDirty (true);
        }

    virtual size_t size() const
        {
        return m_count;
        }
    iterator erase (iterator itr)
        {
        if (itr.m_vector != this)
            return iterator();

        this->erase(itr.m_index);

        if (itr.m_index == 0)
            return iterator(this, iterator::npos);

        if (itr.m_index > m_count)
            return iterator(this, iterator::npos);

        return iterator(this, itr.m_index - 1);
        }

    void erase (size_t index)
        {
        if (Discarded())
            Inflate();

        HASSERT(!Discarded());

        if (index < m_count - 1)
            memcpy(&(m_memory[index]), &(m_memory[index+1]), sizeof(DataType) * (m_count - 1 - index));
        m_count--;

        m_accessCount++;
        if ((m_accessCount > 100) && (m_pool != NULL))
            {
            m_pool->NotifyAccess(this);
            m_accessCount = 0;
            }
        SetDirty (true);

        if (m_allocatedCount > 0)
            {
            if (m_pool != NULL)
                {
                if (GetPoolManaged())
                    {
                    bool freeSuccess = m_pool->Free(this);
                    HASSERT(freeSuccess);
                    }
                }
            else
                {
                delete [] m_memory;
                m_memory = NULL;
                }
            }
        }

    virtual void clearFrom(size_t indexToClearFrom)
        {
        if (Discarded())
            Inflate();

        HASSERT(!Discarded());

        HASSERT (indexToClearFrom < m_count);

        if (indexToClearFrom == 0)
            clear();
        else
            {
            m_count = indexToClearFrom;
            SetDirty (true);
            }
        }

    virtual void clear()
        {
        if (Discarded())
            Inflate();

        HASSERT(!Discarded());

        if (m_allocatedCount > 0)
            {
            if (m_pool != NULL)
                {
                if (GetPoolManaged())
                    {
                    bool freeSuccess = m_pool->Free(this);
                    HASSERT(freeSuccess);
                    }
                }
            else
                {
                delete [] m_memory;
                m_memory = NULL;
                }
            }

        m_count = 0;
        m_allocatedCount = 0;
        SetDirty (true);
        }

    iterator begin()
        {
        if (m_count > 0)
            return iterator(this, (size_t)0);
        return iterator(this, iterator::npos);

        }
    const_iterator begin() const
        {
        if (m_count > 0)
            return const_iterator(this, (size_t)0);
        return const_iterator(this, iterator::npos);

        }

    iterator end()
        {
        return iterator(this, iterator::npos);
        }
    const_iterator end() const
        {
        return const_iterator(this, iterator::npos);
        }

    void random_shuffle()
        {
        if (Discarded())
            Inflate();

        HASSERT(!Discarded());

        m_accessCount++;
        if ((m_accessCount > 100) && (m_pool != NULL))
            {
            m_pool->NotifyAccess(this);
            m_accessCount = 0;
            }

        if (m_count > 0)
            std::random_shuffle(m_memory, &(m_memory[m_count -1]));

        SetDirty(true);

        }



private:
    mutable size_t m_accessCount;

    };

//***********************************************************************************************************************
// The present vector is a simple implementation of a pooled vector that will compress data type memory and move it to
// another memory. It certainly does not provide strict memory management, but does provide a simple way
// to minimize the amount of memory without using some storage mechanism.
//
// Made for test purposes mainly
//***********************************************************************************************************************
template <typename DataType> class HPMCompressiblePooledVector : public HPMPooledVector<DataType>
    {
public:
    /** -----------------------------------------------------------------------------

        Creates a compressible vector. The pool must be set afterwards using SetPool()

        -----------------------------------------------------------------------------
    */
    HPMCompressiblePooledVector ()
        : HPMPooledVector()
        {
        m_compressedMemory = NULL;
        m_compressedAllocatedCount = 0;
        SetDiscarded(false);
        }


    /** -----------------------------------------------------------------------------

        Creates a compressible vector. The pool is provided

        -----------------------------------------------------------------------------
    */
    HPMCompressiblePooledVector (HFCPtr<HPMCountLimitedPool<DataType> > pool)
        : HPMPooledVector(pool)
        {
        m_compressedMemory = NULL;
        m_compressedAllocatedCount = 0;
        SetDiscarded(false);
        }
    ~HPMCompressiblePooledVector()
        {
        if (!Discarded() && IsDirty())
            {
            HASSERT(!"This is way too late to discard as we are during destruction");
            // This is way too late to discard as we are during destruction ... virtual overload of Discard is not accessible
            Discard();
            }
        }


    // HPMCountLimitedPoolItem implementation
    virtual bool Discard() const // Intentionaly const ... only mutable members are modified
        {
        HASSERT (!Discarded());

        // Compress memory
        Byte* compressedMemory = (Byte*)malloc (sizeof(DataType)*m_allocatedCount + 100);
        if (compressedMemory == NULL)
            throw std::bad_alloc();

        uint32_t OutLen = (uint32_t)sizeof(DataType)*m_allocatedCount + 100;
        int err;


        // Use level 5 compression. Varies from 1 to 9, 6 being the default.
        // There seems to be a big difference between 5 and 6 on execution time for
        // big images, with only a small size penalty.
        err = compress2(compressedMemory, &OutLen, (Byte*)m_memory, (uint32_t)((m_count+1)*sizeof(DataType)), 5);

        if(err != Z_OK)
            {
            free(compressedMemory);
            HASSERT(!"Memory compression error!");
            return false;
            }

        m_compressedMemory = (Byte*)malloc (OutLen+1);
        if (m_compressedMemory == NULL)
            {
            free(compressedMemory);
            throw bad_alloc();
            }
        m_compressedMemorySize = OutLen+1;
        m_compressedAllocatedCount = m_allocatedCount;

        memcpy (m_compressedMemory, compressedMemory, OutLen);

        free (compressedMemory);
        SetDiscarded(true);

        // Free memory
        bool freeSuccess = true;
        if (Pinned())
            freeSuccess = m_pool->RemoveItem(this);
        else
            freeSuccess = m_pool->Free (this);
        HASSERT(freeSuccess);

        SetDirty(false);

        return true;
        }

    bool Inflate() const // Intentionaly const ... only mutable members are modified
        {
        int err;

        HPRECONDITION(m_compressedMemory!= NULL);

        if (m_pool != NULL)
        {
            bool allocateSuccess = true;
            if (!GetPoolManaged())
                allocateSuccess = m_pool->Allocate (m_compressedAllocatedCount, this);
            else
                allocateSuccess = m_pool->Reallocate (m_compressedAllocatedCount, this);
            HASSERT(allocateSuccess);
        }
        else
            {
            DataType* newMemory = new DataType[m_compressedAllocatedCount];
            if (newMemory == NULL)
                throw std::bad_alloc();
            m_memory = reinterpret_cast<DataType*>(newMemory);
            m_allocatedCount = m_compressedAllocatedCount;
            }

        uint32_t OutLen = (uint32_t)m_allocatedCount*sizeof(DataType);
        err = uncompress((Byte*)m_memory, &OutLen, (Byte*)m_compressedMemory, (uint32_t)m_compressedMemorySize);

        if(err != Z_OK)
        {
            HASSERT(!"Memory decompression error");
            return false;
        }

        free(m_compressedMemory);
        SetDiscarded(false);
        m_compressedMemory = NULL;
        return true;

        }



private:
    mutable Byte* m_compressedMemory;
    mutable uint32_t m_compressedMemorySize;
    mutable size_t m_compressedAllocatedCount;


    };


//***********************************************************************************************************************
// For test purposes only ... implements exactly the same interface as HPMStoredPooledVector but is not stored but compressed
//***********************************************************************************************************************
template <typename DataType> class HPMNotStoredCompressiblePooledVector : public HPMCompressiblePooledVector<DataType>
    {
public:

    HPMNotStoredCompressiblePooledVector ()
        : HPMCompressiblePooledVector()
        {

        }


    HPMNotStoredCompressiblePooledVector (HPMCountLimitedPool<DataType>* pool)
        : HPMCompressiblePooledVector(pool)
        {

        }
    ~HPMNotStoredCompressiblePooledVector()
        {

        }

    IHPMDataStore<DataType>* GetStore() const
        {
        return NULL;
        }
    };

//***********************************************************************************************************************
// HPMStoredPooledVector
//
// This class implements the stored pooled vector full behavior. It inherits the full behavior of the HPMPooledVector
// class for count limited memory managements, but implements the discard and inflate operations through
// a data store provided.
//
// Although the StoredPooledVector instance requires a store to operate correctly, it may be omitted at creation then
// set one time only using the SetStore() method.
//***********************************************************************************************************************
template <typename DataType> class HPMStoredPooledVector : public HPMPooledVector<DataType>
    {
public:

    /** -----------------------------------------------------------------------------

        Default constructor. The store and the pool must be provided afterwards using
        the SetPool() and SetStore() methods.

        -----------------------------------------------------------------------------
    */

    HPMStoredPooledVector ()
        : HPMPooledVector()
        {
        m_store = NULL;
        SetDiscarded(false);
        }

    /** -----------------------------------------------------------------------------

        Constructor. The store and the pool are provided.

        -----------------------------------------------------------------------------
    */
    HPMStoredPooledVector (HFCPtr<typename PoolItem<DataType>::PoolType > pool, IHPMDataStore<DataType>* store)
        : HPMPooledVector(pool)
        {
        m_store = store;
        SetDiscarded(false);
        }

    /** -----------------------------------------------------------------------------

        Destroyer. It will store the vector if it can and remove the vector from pool
        management with memory release.
        the SetPool() and SetStore() methods.

        -----------------------------------------------------------------------------
    */
    ~HPMStoredPooledVector()
        {
        if (!Discarded() && IsDirty())
            {
            HASSERT(!"This is way too late to discard as we are during destruction");
            // This is way too late to discard as we are during destruction ... virtual overload of Discard is not accessible
            Discard();
            }
        }

    /**----------------------------------------------------------------------------
     Returns a pointer to the store.
     @return Pointer to data  store.
    -----------------------------------------------------------------------------*/
    IHPMDataStore<DataType>* GetStore() const
        {
        return m_store;
        }

    /**----------------------------------------------------------------------------
     Sets the store for the pooled vector. This operation can only be called once
     immediately after creation. Afterwards, it is still possible to change the store
     using the ChangeStore() method.

     @param newStore IN The new store for the pooled vector.

     @return true if operation succesfull
    -----------------------------------------------------------------------------*/
    virtual bool SetStore(IHPMDataStore<DataType>* newStore)
        {
        // Change the store
        m_store = newStore;

        // Sabotage block id ... this should be sufficient
        m_storeBlockID = HPMBlockID();

        return true;
        }

    /**----------------------------------------------------------------------------
     Changes the store. This operation will result in loading the vector in memory
     if it is currently discarded, then change the store. After this operation,
     the next discard or store operation will result in storage of the vector in the
     new store. The original stored vector of the old store is not destroyed.

     @param newStore IN The new store for the pooled vector.

     @return true if operation succesfull
    -----------------------------------------------------------------------------*/
    virtual bool ChangeStore(IHPMDataStore<DataType>* newStore)
        {
        // First load into memory
        if (m_storeBlockID.IsValid() && Discarded())
            Inflate();

        // Change the store
        m_store = newStore;

        // Sabotage block id ... this should be sufficient
        m_storeBlockID = HPMBlockID();

        return true;
        }


    /**----------------------------------------------------------------------------
     Reutrns the block ID for this pooled vector. The blockID can be non-initialised
     if the vector has not been pushed on the store yet.
     @return The block ID for this vector.
    -----------------------------------------------------------------------------*/
    HPMBlockID GetBlockID() const
        {
        return m_storeBlockID;
        }

    void SetBlockID(HPMBlockID id) const
        {
        m_storeBlockID = id;
        }

    // HPMCountLimitedPoolItem implementation
    virtual bool Discard() const // Intentionaly const ... only mutable members are modified
        {
        HASSERT (!Discarded());

        if (IsDirty() == true)
            {

            HDEBUGCODE (HPMBlockID initialBlockID(m_storeBlockID););
            m_storeBlockID = m_store->StoreBlock(m_memory, m_count, m_storeBlockID);

            HASSERT(m_storeBlockID.m_integerID < 1000000);

            HDEBUGCODE(m_countLoaded = true;);
            }

        if (!m_storeBlockID.IsValid())
        {
            HASSERT (!"Block could not be properly stored!");
            return false;
        }

        SetDiscarded(true);
        m_storedAllocatedCount = m_allocatedCount;

        // Free memory
        bool freeSuccess = true;
        if (Pinned())
            freeSuccess = m_pool->RemoveItem(this);
        else
            freeSuccess = m_pool->Free (this);

        HASSERT(freeSuccess);

        SetDirty (false);

        return true;
        }

    virtual bool Inflate() const // Intentionaly const ... only mutable members are modified
        {
        HPRECONDITION(Discarded());

        // Safegard to prevent count change upon loading when count has already been loaded
        // Yes this case did occur.
        HDEBUGCODE(size_t initialCount = m_count;);

        m_count = m_store->GetBlockDataCount (m_storeBlockID);
#ifdef __HMR_DEBUG
        if (m_countLoaded)
            HASSERT(initialCount == m_count);
#endif

        HDEBUGCODE(m_countLoaded = true;);


        if (m_count != 0)
            {
            size_t countToAllocate = m_count + 10;

            if (m_pool != NULL)
            {
                bool successfullAllocation = true;
                if (!GetPoolManaged())
                    successfullAllocation = m_pool->Allocate (countToAllocate, this);
                else
                    successfullAllocation = m_pool->Reallocate (countToAllocate, this);

                HASSERT(successfullAllocation);
            }
            else
                {
                DataType* newMemory = new DataType[countToAllocate];
                if (newMemory == NULL)
                    throw std::bad_alloc();
                m_memory = reinterpret_cast<DataType*>(newMemory);
                m_allocatedCount = countToAllocate;
                }

            m_store->LoadBlock (m_memory, m_allocatedCount, m_storeBlockID);
            }
        SetDiscarded(false);
        return true;

        }



protected:
    IHPMDataStore<DataType>* m_store;
    mutable size_t m_storedAllocatedCount;
    mutable HPMBlockID m_storeBlockID;


    };


END_IMAGEPP_NAMESPACE
