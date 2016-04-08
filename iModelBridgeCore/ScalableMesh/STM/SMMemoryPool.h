//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/SMMemoryPool.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#pragma once


#include "HPMCountLimitedPool.h"
#include "HPMIndirectCountLimitedPool.h"
#include "HPMDataStore.h"
#include <ImagePP/h/HIterators.h>


BEGIN_IMAGEPP_NAMESPACE


    

enum class DataTypeDesc
    {
    Point3d = 0,
    Int32,
    Byte,
    Point2d, 
    DiffSet, 
    Graph,
    Texture,
    Unknown, 
    };

DataTypeDesc GetDataType(const type_info& typeInfo)
    {
    if (typeid(DPoint3d) == typeInfo)
        return DataTypeDesc::Point3d;
    else if (typeid(int32_t) == typeInfo)
        return DataTypeDesc::Int32;
    else if (typeid(Byte) == typeInfo)
        return DataTypeDesc::Byte;
    else if (typeid(DPoint2d) == typeInfo)
        return DataTypeDesc::Point2d;
    else    
        return DataTypeDesc::Unknown;

    /*
    switch (typeInfo)
        {
        case typeid(DPoint3d):
            return DataTypeDesc::Point3d;
            break; 
        case typeid(int32_t):
            return DataTypeDesc::Int32;
            break; 
        case typeid(Byte):
            return DataTypeDesc::Byte;
            break; 
        case typeid(DPoint2d):
            return DataTypeDesc::Point2d;
            break; 
        default : 
            return DataTypeDesc::Unknown;
            break; 
        }  
        */
    }

/*
enum class ContainerType
    {
    PoolVector = 0
    }
    */

template <typename DataType> class HPMTypedPooledVector;
template <typename DataType> class HPMTypedPoolItem;


class MemoryPoolItem : public RefCountedBase
    {
    protected : 

        Byte*         m_data;
        uint64_t      m_size;
        uint64_t      m_nodeId;
        DataTypeDesc  m_dataType;
        bool          m_dirty;
        //ContainerType m_containerType;

        //type_info m_dataType;
        
    public :

        MemoryPoolItem()
            {
            m_dataType = DataTypeDesc::Unknown;
            m_dirty = false;
            m_nodeId = numeric_limits<uint64_t>::max();
            }

        MemoryPoolItem(Byte* data, uint64_t size, uint64_t nodeId, DataTypeDesc& dataType)
            {
            m_data = data;
            m_size = size;
            m_nodeId = nodeId;
            m_dataType = dataType;
            m_data = data;
            m_dirty = true;
            }

        template<typename T>
        RefCountedPtr<HPMTypedPooledVector<T>> GetAsPoolVector()
            {
            if (GetDataType(typeid(T)) != m_dataType)
                return 0;

            return dynamic_cast<HPMTypedPooledVector<T>*>(this);
            }

        template<typename T>
        RefCountedPtr<HPMTypedPoolItem<T>> GetAsTypedPoolItem()
            {/*
            if (GetDataType(typeid(T)) != m_dataType)
               return 0;
               */

            return dynamic_cast<HPMTypedPoolItem<T>*>(this);
            }
        
        virtual ~MemoryPoolItem()
            {
            delete [] m_data;
            }

        void* GetItem();

        uint64_t GetSize()
            {
            return m_size;
            }        

        bool IsCorrect(uint64_t nodeId, DataTypeDesc& dataType)
            {
            if (nodeId == m_nodeId && dataType == m_dataType)
                return true;

            return false;
            }
    };


/*
class CustomTypedPoolItemCreator
    {
    public : 
                
        virtual Byte* AllocateData() = 0;

        virtual uint64_t GetSize() = 0;
    };

class TextureTypedPoolItemCreator : public CustomTypedPoolItemCreator
    {
    TextureTypedPoolItemCreator(size_t sizeX, size_t sizeY)
        {
        }

    virtual ~TextureTypedPoolItemCreator()
        {
        }

    }
    */

template <typename DataType> class HPMTypedPoolItem : public MemoryPoolItem
    {
    protected : 

    public : 
        
        HPMTypedPoolItem(size_t size, uint64_t nodeId, DataTypeDesc dataType)
            {            
            m_size = size;
            m_nodeId = nodeId;
            m_data = (Byte*)new Byte[m_size];
            memset(m_data, 1, m_size);                               
            m_dataType = dataType;
            }

        virtual ~HPMTypedPoolItem()
            {
            }
    };

template <typename DataType> class HPMTypedPooledVector : public MemoryPoolItem
    {
    protected:
        
        size_t    m_nbItems;

    public:
        typedef DataType value_type;
        
        HPMTypedPooledVector()
            {
            }

        HPMTypedPooledVector(size_t nbItems, uint64_t nodeId)                        
            {
            m_nbItems = nbItems;
            m_size = nbItems * sizeof(DataType);
            m_nodeId = nodeId;
            m_data = (Byte*)new DataType[nbItems];
            memset(m_data, 1, m_size);
                                   
            m_dataType = GetDataType(typeid(DataType));
            }

        ~HPMTypedPooledVector()
            {
            }    
    };



template <typename DataType> class HPMPooledVector : public PoolItem<DataType>::Type
    {
public:
    typedef DataType value_type;

    template <class T> class iteratorBase : public RandomAccessIteratorWithAutoReverseConst<iteratorBase<T>, iteratorBase<typename ReverseConstTrait<T>::type >, T>
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
            //HASSERT (m_index + increment > m_vector->size());

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
        m_itemMutex.lock();
        if (Discarded())
            Inflate();

        HASSERT(!Discarded());

        if((m_allocatedCount <= m_count) && !reserve ((m_count * 3)/2 + 2))
            throw std::bad_alloc();
   
        m_accessCount++;
        if ((m_accessCount > 100) && (m_pool != NULL) && !Pinned())
            {            
            /*
            HPMStoredPooledVector* poolVec = dynamic_cast<HPMStoredPooledVector*>(this);
            DataType dataType;
            PoolOperationLogger::GetLogger().LogAccess(poolVec->GetBlockID().m_integerID, m_count, &dataType);
            */
            
            m_pool->NotifyAccess(this);
            m_accessCount = 0;
            }

        m_memory[m_count] = newObject;
        m_itemMutex.unlock();
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
        if ((m_accessCount > 100) && (m_pool != NULL) && !Pinned())
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
        m_itemMutex.lock();
        // Pin the pooled vector given as source in case it gets discarded in the process
        source->Pin();

        if (Discarded())
            Inflate();

        HASSERT(!Discarded());

        if (m_allocatedCount <= (m_count + (end - start + 1)))
            if (!reserve (m_count + (end - start + 1)))
                throw std::bad_alloc();

        m_accessCount += (end - start + 1);
        if ((m_accessCount > 100) && (m_pool != NULL) && !Pinned())
            {
            m_pool->NotifyAccess(this);
            m_accessCount = 0;
            }

        memcpy (&(m_memory[m_count]), &(source->m_memory[start]), sizeof(DataType) * (end - start + 1));
        m_count += (end - start + 1);
        SetDirty(true);
        source->UnPin();
        m_itemMutex.unlock();
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
        if ((m_accessCount > 100) && (m_pool != NULL) && !Pinned())
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
        std::lock_guard<std::recursive_mutex> lock(m_itemMutex);
        if (maxCount == 0)
            return false;

        if (Discarded() && !Pinned())
            Inflate();
        else if (Pinned()) SetDiscarded(false);

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
    virtual const DataType& operator[](size_t index) const
        {
        if (Discarded())
            Inflate();

        HASSERT(!Discarded());

        HPRECONDITION(index < m_count);        
        m_accessCount++;        

        if (m_accessCount >= m_count || typeid(DataType) != typeid(DPoint3d))
            {
            const HPMStoredPooledVector<DataType>* poolVec = dynamic_cast<const HPMStoredPooledVector<DataType>*>(this);
            DataType dataType;
            PoolOperationLogger::GetLogger().LogAccess<DataType>(poolVec->GetBlockID().m_integerID, m_count, &dataType);

            if (Pinned())
                m_accessCount = 0;
            }
        
        if ((m_accessCount > 100) && (m_pool != NULL) && !Pinned())
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
        if ((m_accessCount > 100) && (m_pool != NULL) && !Pinned())
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
        m_itemMutex.lock();
        this->erase(itr.m_index);
        m_itemMutex.unlock();
        if (itr.m_index == 0)
            return iterator(this, iterator::npos);

        if (itr.m_index > m_count)
            return iterator(this, iterator::npos);

        return iterator(this, itr.m_index - 1);
        }

    void erase (size_t index)
        {
        m_itemMutex.lock();
        if (Discarded())
            Inflate();

        HASSERT(!Discarded());

        if (index < m_count - 1)
            memcpy(&(m_memory[index]), &(m_memory[index+1]), sizeof(DataType) * (m_count - 1 - index));
        m_count--;

        m_accessCount++;
        if ((m_accessCount > 100) && (m_pool != NULL) && !Pinned())
            {
            m_pool->NotifyAccess(this);
            m_accessCount = 0;
            }
        SetDirty (true);

        if ((m_allocatedCount > 0) && (m_count == 0))
            {
            if (m_pool != NULL)
                {
                if (GetPoolManaged())
                    {
                    bool freeSuccess = m_pool->Free(this);
                    freeSuccess = freeSuccess;
                    HASSERT(freeSuccess);
                    }
                }
            else
                {
                delete [] m_memory;
                m_memory = NULL;
                }
            }
        m_itemMutex.unlock();
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
        m_itemMutex.lock();
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
                    freeSuccess = freeSuccess;
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
        m_itemMutex.unlock();
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
        if ((m_accessCount > 100) && (m_pool != NULL) && !Pinned())
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


template <typename DataType> class HPMStoredTypedPooledVector : public HPMTypedPooledVector<DataType>
    {
    private: 

        IHPMDataStore<DataType>* m_store;
        bool                     m_isDirty;

    public:        
                
        HPMStoredTypedPooledVector(size_t nbItems, uint64_t nodeId, IHPMDataStore<DataType>* store)
            : HPMStoredTypedPooledVector(nbItems, nodeId)
            {            
            m_store = store;            
            m_store->LoadBlock (m_data, m_nbItems, m_nodeId);
            }

        ~HPMStoredTypedPooledVector()
            {
            if (m_dirty)
                {
                }
            }    
    };

typedef RefCountedPtr<MemoryPoolItem> MemoryPoolItemPtr;

//First impl - dead lock
static clock_t s_timeDiff = CLOCKS_PER_SEC * 120;
static double s_maxMemBeforeFlushing = 1.2;

class MemoryPool
    {
    private : 

        uint64_t                    m_maxPoolSizeInBytes;
        atomic<uint64_t>            m_currentPoolSizeInBytes;
        bvector<MemoryPoolItemPtr>  m_memPoolItems;
        bvector<std::mutex*>        m_memPoolItemMutex;
        bvector<clock_t>            m_lastAccessTime;
        
        //std::mutex                 m_poolItemMutex;

        /*
        atomic<bool>               m_isFull;
        atomic<bool>               m_lastAvailableInd;
        */

    public : 

        MemoryPool(uint64_t maxPoolSizeInBytes)            
            : m_memPoolItemMutex(500)
            {
            m_currentPoolSizeInBytes = 0;
            m_maxPoolSizeInBytes = maxPoolSizeInBytes;            
            m_memPoolItems.resize(500);
            m_lastAccessTime.resize(500);
            m_memPoolItemMutex.resize(500);

            for (size_t itemId = 0; itemId < m_memPoolItemMutex.size(); itemId++)
                {
                m_memPoolItemMutex[itemId] = new mutex;                
                }
            }

        virtual ~MemoryPool()
            {
            for (size_t itemId = 0; itemId < m_memPoolItemMutex.size(); itemId++)
                {                
                delete m_memPoolItemMutex[itemId];
                }            
                        
            /*
            m_lastAvailableInd = 0;
            m_isFull = false;
            */
            }


        
        bool GetItem(MemoryPoolItemPtr& memItemPtr, uint64_t id)
            {     
            std::lock_guard<std::mutex> lock(*m_memPoolItemMutex[id]);
            m_lastAccessTime[id] = clock();
            memItemPtr = m_memPoolItems[id];
            return memItemPtr.IsValid();
            }

        uint64_t AddItem(MemoryPoolItemPtr& poolItem)
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

            clock_t currentTime = clock(); 

            for (; itemInd < (uint64_t)m_memPoolItems.size(); itemInd++)
                {
                    {   
                    std::lock_guard<std::mutex> lock(*m_memPoolItemMutex[itemInd]);

                    if (!m_memPoolItems[itemInd].IsValid())
                        {
                        break;
                        }
                    }

                if ((needToFlush && (currentTime - m_lastAccessTime[itemInd] > s_timeDiff)))                
                    {
                    break;
                    }                

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

                    if (m_memPoolItems[itemIndToDelete].IsValid() && m_lastAccessTime[itemIndToDelete] < flushTimeThreshold)                    
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

                    if (m_memPoolItems[itemIndToDelete].IsValid())
                        {
                        m_currentPoolSizeInBytes -= m_memPoolItems[itemIndToDelete]->GetSize();                
                        m_memPoolItems[itemIndToDelete] = 0; 
                        itemInd = itemIndToDelete;
                        }                    
                    }
                }            

            if (itemInd == m_memPoolItems.size())
                {
                if (m_currentPoolSizeInBytes < m_maxPoolSizeInBytes)
                    {                
                    m_memPoolItems.resize((size_t)(m_memPoolItems.size() * 1.5));
                    m_lastAccessTime.resize((size_t)(m_lastAccessTime.size() * 1.5));
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

            HASSERT(!m_storeBlockID.IsValid() || m_storeBlockID.m_integerID < 1000000000);

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
//        HDEBUGCODE(size_t initialCount = m_count;);
     
        m_count = m_store->GetBlockDataCount (m_storeBlockID);
/*#ifdef __HMR_DEBUG
        if (m_countLoaded)
            HASSERT(initialCount == m_count);
#endif
*/
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
