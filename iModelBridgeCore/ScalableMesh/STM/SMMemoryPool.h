//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/SMMemoryPool.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#pragma once

#include <limits>   

#include <ImagePP\all\h\HFCPtr.h>
#include <ImagePP\all\h\HPMDataStore.h>
#include <ImagePP/h/HIterators.h>

using namespace std;

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE
    
enum class SMPoolDataTypeDesc
    {
    Point = 0,
    TriPtIndices,    
    UVIndices, 
    DiffSet, 
    Graph,
    Texture,
    Unknown, 
    };

inline SMPoolDataTypeDesc GetDataType(const type_info& typeInfo)
    {
    /*
    if (typeid(DPoint3d) == typeInfo)
        return SMPoolDataTypeDesc::Point3d;
    else if (typeid(int32_t) == typeInfo)
        return SMPoolDataTypeDesc::Int32;
    else if (typeid(Byte) == typeInfo)
        return SMPoolDataTypeDesc::Byte;
    else if (typeid(DPoint2d) == typeInfo)
        return SMPoolDataTypeDesc::Point2d;
    else    
        return SMPoolDataTypeDesc::Unknown;

    /*
    switch (typeInfo)
        {
        case typeid(DPoint3d):
            return SMPoolDataTypeDesc::Point3d;
            break; 
        case typeid(int32_t):
            return SMPoolDataTypeDesc::Int32;
            break; 
        case typeid(Byte):
            return SMPoolDataTypeDesc::Byte;
            break; 
        case typeid(DPoint2d):
            return SMPoolDataTypeDesc::Point2d;
            break; 
        default : 
            return SMPoolDataTypeDesc::Unknown;
            break; 
        }  
        */

    return SMPoolDataTypeDesc::Unknown;
    }

    
/*
enum class ContainerType
    {
    PoolVector = 0
    }
    */

template <typename DataType> class SMMemoryPoolVectorItem;
template <typename DataType> class SMMemoryPoolItem;


class SMMemoryPoolItemBase : public RefCountedBase
    {
    protected : 

        Byte*         m_data;
        uint64_t      m_size;
        uint64_t      m_nodeId;
        SMPoolDataTypeDesc  m_dataType;
        bool          m_dirty;
        //ContainerType m_containerType;

        //type_info m_dataType;
        
    public :

        SMMemoryPoolItemBase()
            {
            m_dataType = SMPoolDataTypeDesc::Unknown;
            m_dirty = false;
            m_nodeId = std::numeric_limits<uint64_t>::max();
            }

        SMMemoryPoolItemBase(Byte* data, uint64_t size, uint64_t nodeId, SMPoolDataTypeDesc& dataType)
            {
            m_data = data;
            m_size = size;
            m_nodeId = nodeId;
            m_dataType = dataType;
            m_data = data;
            m_dirty = true;
            }

        template<typename T>
        RefCountedPtr<SMMemoryPoolVectorItem<T>> GetAsPoolVector()
            {            
            return dynamic_cast<SMMemoryPoolVectorItem<T>*>(this);
            }

        template<typename T>
        RefCountedPtr<SMMemoryPoolItem<T>> GetAsTypedPoolItem()
            {/*
            if (GetDataType(typeid(T)) != m_dataType)
               return 0;
               */

            return dynamic_cast<SMMemoryPoolItem<T>*>(this);
            }
        
        virtual ~SMMemoryPoolItemBase()
            {
            delete [] m_data;
            }

        void* GetItem();

        uint64_t GetSize()
            {
            return m_size;
            }        

        bool IsCorrect(uint64_t nodeId, SMPoolDataTypeDesc& dataType)
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

template <typename DataType> class SMMemoryPoolItem : public SMMemoryPoolItemBase
    {
    protected : 

    public : 
        
        SMMemoryPoolItem(size_t size, uint64_t nodeId, SMPoolDataTypeDesc dataType)
            {            
            m_size = size;
            m_nodeId = nodeId;
            m_data = (Byte*)new Byte[m_size];
            memset(m_data, 1, m_size);                               
            m_dataType = dataType;
            }

        virtual ~SMMemoryPoolItem()
            {
            }
    };



template <typename DataType> class SMMemoryPoolVectorItem : public SMMemoryPoolItemBase
    {

private:

protected: 

        size_t    m_nbItems;

public:
    
    template <class T> class iteratorBase : public RandomAccessIteratorWithAutoReverseConst<iteratorBase<T>, iteratorBase<typename ReverseConstTrait<T>::type >, T>
        {
    public:
        static const size_t npos = -1;

        iteratorBase() {
            m_vector = NULL;
            m_index = iteratorBase::npos;
            }
        iteratorBase(SMMemoryPoolVectorItem<DataType>* vector, size_t initialPosition) {
            m_vector = vector;
            m_index = initialPosition; /* if (m_vector != NULL) m_vector->Pin();*/
            }
        const_iterator_t    ConvertToConst () const
            {
            return const_iterator_t(m_vector, m_index);
            }

        const_reference     Dereference    () const {
            return m_vector->operator[](m_index);
            }
        reference           Dereference    () {
            return m_vector->operator[](m_index);
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
        friend SMMemoryPoolVectorItem<DataType>;
        iteratorBase(const SMMemoryPoolVectorItem<DataType>* vector, size_t index) {
            m_vector = const_cast<SMMemoryPoolVectorItem<DataType>*>(vector);
            m_index = index;
            }

        
        RefCountedPtr<SMMemoryPoolVectorItem<DataType>> m_vector;
        size_t m_index;
        };

    typedef iteratorBase<DataType> iterator;
    typedef iteratorBase<const DataType> const_iterator ;

    /** -----------------------------------------------------------------------------

        Creates a pooled vector. Before using to its full extent the pool must be set using
        the SetPool() method.

        -----------------------------------------------------------------------------
    */

    SMMemoryPoolVectorItem(size_t nbItems, uint64_t nodeId, SMPoolDataTypeDesc dataType)                        
            {
            m_nbItems = nbItems;
            m_size = nbItems * sizeof(DataType);
            m_nodeId = nodeId;
            m_data = (Byte*)new DataType[nbItems];
            memset(m_data, 1, m_size);                                               
            m_dataType = dataType;
            }
    /*
    SMMemoryPoolVectorItem(uint64_t nodeId)                        
            {
            m_nbItems = 0;
            m_size = nbItems * sizeof(DataType);
            m_nodeId = nodeId;

            if (nbItems > 0)
                m_data = (Byte*)new DataType[nbItems];
                                               
            m_dataType = GetDataType(typeid(DataType));
            }
            */
    /*
    SMMemoryPoolVectorItem ()
        : PoolItem<DataType>::Type()
        {
        m_allocatedCount = 0;
        m_count = 0;
        m_memory = NULL;

        m_accessCount=0;
        }*/

    /** -----------------------------------------------------------------------------

        Creates a pooled vector. The pool used for the memory count management is
        provided.

        -----------------------------------------------------------------------------
    */
    /*
    SMMemoryPoolVectorItem(HFCPtr< typename PoolItem<DataType>::PoolType > pool)
        : PoolItem<DataType>::Type(pool)
        {
        m_allocatedCount = 0;
        m_count = 0;
        m_memory = NULL;


        m_accessCount=0;
        }*/
    /** -----------------------------------------------------------------------------

        Destroyer. If the object is pool managed then it is removed from the pool
        during destruction.

        -----------------------------------------------------------------------------
    */

    virtual ~SMMemoryPoolVectorItem()
        {
        /*
        if (m_pool != NULL)
            if (!GetPoolManaged())
                m_pool->Free(this);
                */
        }


    virtual bool reserve(size_t newCount)
        {
        assert(!"MST TBD");
        /*MST TBD
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
            */
        return true;
        }

    size_t capacity() const
        {
        assert(!"MST TBD");
        /*
        if (Discarded())
            Inflate();

        HASSERT(!Discarded());

        return m_allocatedCount;
        */
        return 0;
        }

    bool push_back(const DataType& newObject)
        {
        assert(!"MST TBD");

        
#if 0
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
#endif
        return true;
        }
    /** -----------------------------------------------------------------------------

        This push_back overload does not exist in std::vector but is provided for
        performance reason. It copies the provided pooled vector to self.

        -----------------------------------------------------------------------------
    */
#if 0
    bool push_back(const SMMemoryPoolVectorItem<DataType>* source)
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
#endif
    /** -----------------------------------------------------------------------------

        This push_back overload does not exist in std::vector but is provided for
        performance reason. It copies the provided pooled vector to self from given
        index to specified end index included.

        -----------------------------------------------------------------------------
    */

#if 0
    bool push_back(const SMMemoryPoolVectorItem<DataType>* source, size_t start, size_t end)
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
#endif

    /** -----------------------------------------------------------------------------

        This push_back overload does not exist in std::vector but is provided for
        performance reason. It copies into the vector many DataTypes in a single call.

        -----------------------------------------------------------------------------
    */
    bool push_back(const DataType* newObjects, size_t count)
        {
        assert(!"MST TBD");

        if (count == 0)
            return false;
/*
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
        */
        }


    /** -----------------------------------------------------------------------------

        This get method does not exist in std::vector but is provided for
        performance reason. It copies into the array the content of the vector in a single call.

        It returns the number of data type returned
        -----------------------------------------------------------------------------
    */
#if 0
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
#endif

    // This used to be a reference returned but this caused a problem when the ref was maintained and the
    // tile discarded...
    virtual const DataType& operator[](size_t index) const
        {             
        HPRECONDITION(index < m_nbItems);        

        /*
        m_accessCount++;        
              
        if ((m_accessCount > 100) && (m_pool != NULL) && !Pinned())
            {            
            m_pool->NotifyAccess(this);
            m_accessCount = 0;
            }
            */
        
        return ((DataType*)m_data)[index];
        }
    
    void modify(size_t          index,
                const DataType& newDataValue)
        {
        assert(!"MST TBD");
        /*
        m_memory[index] = newDataValue;
        SetDirty (true);
        */
        }

    virtual size_t size() const
        {
        return m_nbItems;
        }

    iterator erase (iterator itr)
        {
        assert(!"MST TBD");

        /*
        if (itr.m_vector != this)
            return iterator();
        m_itemMutex.lock();
        this->erase(itr.m_index);
        m_itemMutex.unlock();
        if (itr.m_index == 0)
            return iterator(this, iterator::npos);

        if (itr.m_index > m_count)
            return iterator(this, iterator::npos);
            */

        return iterator(this, itr.m_index - 1);
        }

    void erase (size_t index)
        {
        assert(!"MST TBD");

        /*
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
        */
        }

    virtual void clearFrom(size_t indexToClearFrom)
        {
        assert(!"MST TBD");

        /*
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
            */
        }

    virtual void clear()
        {
        assert(!"MST TBD");
        /*
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
        */
        }

    iterator begin()
        {
        if (m_nbItems > 0)
            return iterator(this, (size_t)0);
        return iterator(this, iterator::npos);

        }
    const_iterator begin() const
        {
        if (m_nbItems > 0)
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
        assert(!"MST TBD");
        /*

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
        */

        }

    };

    
template <typename DataType> class SMStoredMemoryPoolVectorItem : public SMMemoryPoolVectorItem<DataType>
    {
    private: 

        IHPMDataStore<DataType>* m_store;
        
    public:        
                        
        SMStoredMemoryPoolVectorItem(uint64_t nodeId, IHPMDataStore<DataType>* store, SMPoolDataTypeDesc dataType)
            : SMMemoryPoolVectorItem(store->GetBlockDataCount(HPMBlockID(nodeId)), nodeId, dataType)
            {                                    
            m_store = store;            

            /*
            HPMBlockID blockID(m_nodeId);
            
            size_t m_nbItems = m_store->GetBlockDataCount(blockID);
            */

            if (m_nbItems > 0)
                {                
                HPMBlockID blockID(m_nodeId);
                size_t nbBytesLoaded = m_store->LoadBlock ((DataType*)m_data, m_nbItems, blockID);
                assert(nbBytesLoaded == sizeof(DataType) * m_nbItems);
                }           
            }

        ~SMStoredMemoryPoolVectorItem()
            {
            if (m_dirty)
                {
                }
            }    
    };

typedef RefCountedPtr<SMMemoryPoolItemBase> SMMemoryPoolItemBasePtr;

//First impl - dead lock
static clock_t s_timeDiff = CLOCKS_PER_SEC * 120;
static double s_maxMemBeforeFlushing = 1.2;

typedef uint64_t SMMemoryPoolItemId; 
static uint64_t s_initSize = 100;

class SMMemoryPool : public RefCountedBase
    {
    private : 

        uint64_t                         m_maxPoolSizeInBytes;
        atomic<uint64_t>                 m_currentPoolSizeInBytes;
        bvector<SMMemoryPoolItemBasePtr> m_memPoolItems;
        bvector<std::mutex*>             m_memPoolItemMutex;
        bvector<clock_t>                 m_lastAccessTime;
        
        //std::mutex                 m_poolItemMutex;

        /*
        atomic<bool>               m_isFull;
        atomic<bool>               m_lastAvailableInd;
        */

    public : 

        static SMMemoryPoolItemId s_UndefinedPoolItemId;

        SMMemoryPool(uint64_t maxPoolSizeInBytes)                        
            {
            m_currentPoolSizeInBytes = 0;
            m_maxPoolSizeInBytes = maxPoolSizeInBytes;            
            m_memPoolItems.resize(s_initSize);
            m_lastAccessTime.resize(s_initSize);
            m_memPoolItemMutex.resize(s_initSize);

            for (size_t itemId = 0; itemId < m_memPoolItemMutex.size(); itemId++)
                {
                m_memPoolItemMutex[itemId] = new mutex;                
                }
            }

        virtual ~SMMemoryPool()
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


        template<typename T>
        bool GetItem(RefCountedPtr<SMMemoryPoolVectorItem<T>>& poolMemVectorItemPtr, SMMemoryPoolItemId id, uint64_t nodeId, SMPoolDataTypeDesc dataType)
            {
            if (id == SMMemoryPool::s_UndefinedPoolItemId)
                return false; 

            assert(!poolMemVectorItemPtr.IsValid());
            SMMemoryPoolItemBasePtr memItemPtr;
            
            if (GetItem(memItemPtr, id) && memItemPtr->IsCorrect(nodeId, dataType))
                {
                poolMemVectorItemPtr = memItemPtr->GetAsPoolVector<T>();                
                }

            return poolMemVectorItemPtr.IsValid();
            }                
               
        bool GetItem(SMMemoryPoolItemBasePtr& memItemPtr, SMMemoryPoolItemId id)
            {     
            if (id == SMMemoryPool::s_UndefinedPoolItemId)
                return false; 

            std::lock_guard<std::mutex> lock(*m_memPoolItemMutex[id]);
            m_lastAccessTime[id] = clock();
            memItemPtr = m_memPoolItems[id];
            return memItemPtr.IsValid();
            }

        bool RemoveItem(SMMemoryPoolItemId id, uint64_t nodeId, SMPoolDataTypeDesc dataType)
            {     
            if (id == SMMemoryPool::s_UndefinedPoolItemId)
                return false; 

            std::lock_guard<std::mutex> lock(*m_memPoolItemMutex[id]);            
            SMMemoryPoolItemBasePtr memItemPtr(m_memPoolItems[id]);
            if (memItemPtr.IsValid() && memItemPtr->IsCorrect(nodeId, dataType))
                {
                m_memPoolItems[id] = 0;
                return true;
                }

            return false;
            }

        SMMemoryPoolItemId AddItem(SMMemoryPoolItemBasePtr& poolItem)
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
                    m_lastAccessTime.resize(m_memPoolItems.size());

                    size_t oldItemCount = m_memPoolItemMutex.size();
                    m_memPoolItemMutex.resize(m_memPoolItems.size());

                    for (size_t itemId = oldItemCount; itemId < m_memPoolItemMutex.size(); itemId++)
                        {
                        m_memPoolItemMutex[itemId] = new mutex;                
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

typedef RefCountedPtr<SMMemoryPool> SMMemoryPoolPtr;

END_BENTLEY_SCALABLEMESH_NAMESPACE