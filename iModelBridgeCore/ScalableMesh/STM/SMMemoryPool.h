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
#include "ComputeMemoryUseTraits.h"
#include <iostream>
using namespace std;

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE
    
enum class SMPoolDataTypeDesc
    {
    Points = 0,
    TriPtIndices,    
    TriUvIndices, 
    UvCoords, 
    DiffSet, 
    Graph,
    Texture,
    Display, //Use to represents data created for display purpose, like QV element. 
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
template <typename DataType> class SMMemoryPoolGenericVectorItem;
template <typename DataType> class SMMemoryPoolBlobItem;
template <typename DataType> class SMMemoryPoolGenericBlobItem;

class SMMemoryPool;
typedef RefCountedPtr<SMMemoryPool> SMMemoryPoolPtr;

typedef uint64_t SMMemoryPoolItemId;


class SMMemoryPoolItemBase : public RefCountedBase
    {
    private : 

        SMMemoryPoolItemId        m_poolItemId;         

    protected : 

        Byte*                     m_data;
        uint64_t                  m_size;
        uint64_t                  m_nodeId;
        uint64_t                  m_smId;
        SMPoolDataTypeDesc        m_dataType;
        bool                      m_dirty;   
                
        //ContainerType m_containerType;

        //type_info m_dataType;
        
        void NotifySizeChangePoolItem(int64_t sizeDelta);
        
    public :

        SMMemoryPoolItemBase();
            
        SMMemoryPoolItemBase(Byte* data, uint64_t size, uint64_t nodeId, SMPoolDataTypeDesc& dataType, uint64_t smId);
                    
        template<typename T>
        RefCountedPtr<SMMemoryPoolVectorItem<T>> GetAsPoolVector()
            {            
            return dynamic_cast<SMMemoryPoolVectorItem<T>*>(this);
            }

        template<typename T>
        RefCountedPtr<SMMemoryPoolGenericVectorItem<T>> GetAsGenericPoolVector()
            {
            return dynamic_cast<SMMemoryPoolGenericVectorItem<T>*>(this);
            }

        template<typename T>
        RefCountedPtr<SMMemoryPoolBlobItem<T>> GetAsBlobPoolItem()
            {            
            return dynamic_cast<SMMemoryPoolBlobItem<T>*>(this);
            }

        template<typename T>
        RefCountedPtr<SMMemoryPoolGenericBlobItem<T>> GetAsGenericBlobPoolItem()
            {            
            return dynamic_cast<SMMemoryPoolGenericBlobItem<T>*>(this);
            }        
        
        virtual ~SMMemoryPoolItemBase();
                    
        uint64_t GetSize();
        
        bool IsCorrect(uint64_t nodeId, SMPoolDataTypeDesc& dataType);

        bool IsCorrect(uint64_t nodeId, SMPoolDataTypeDesc& dataType, uint64_t smId);
            
        SMMemoryPoolItemId GetPoolItemId() const;
            
        void SetPoolItemId(SMMemoryPoolItemId poolItemId);

        void SetDirty() { m_dirty = true; }
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

template <typename DataType> class SMMemoryPoolBlobItem : public SMMemoryPoolItemBase
    {
    protected :         

    public : 
        
        SMMemoryPoolBlobItem(size_t size, uint64_t nodeId, SMPoolDataTypeDesc dataType, uint64_t smId)
            {            
            m_size = size;
            m_nodeId = nodeId;
            m_smId = smId;
            if (m_size > 0)
                m_data = (Byte*)new Byte[m_size];
            else
                m_data = 0;

            m_dataType = dataType;
            }

        virtual ~SMMemoryPoolBlobItem()
            {
            }

        const DataType* GetData()
            {
            return (const DataType*)m_data;
            }
    };


template <typename DataType> class SMMemoryPoolGenericBlobItem : public SMMemoryPoolItemBase
    {
    protected :         

    public : 
        
        SMMemoryPoolGenericBlobItem(DataType* data, size_t size, uint64_t nodeId, SMPoolDataTypeDesc dataType, uint64_t smId)
            {               
            m_size = size;
            m_nodeId = nodeId;
            m_data = (Byte*)data;            
            m_dataType = dataType;
            m_smId = smId;
            }

        virtual ~SMMemoryPoolGenericBlobItem()
            {
            if (m_data != nullptr)
                delete (DataType*)m_data;
            m_data = 0;
            }

        const DataType* GetData()
            {
            return (const DataType*)m_data;
            }

        DataType* EditData()
            {
            return (DataType*)m_data;
            }

        void SetData(DataType* data)
            {
            if (m_data != nullptr && m_size > 0)
                delete (DataType*)m_data;
            m_data = (Byte*)data;
            if (GetPoolItemId() != SMMemoryPool::s_UndefinedPoolItemId) NotifySizeChangePoolItem(GetSizeInMemory(data) - m_size);
            m_size = GetSizeInMemory(data);
            }
    };


template <typename DataType> class SMStoredMemoryPoolGenericBlobItem : public SMMemoryPoolGenericBlobItem<DataType>
    {
    private:

        IHPMDataStore<DataType>* m_store;

    public:

        SMStoredMemoryPoolGenericBlobItem(uint64_t nodeId, IHPMDataStore<DataType>* store, SMPoolDataTypeDesc dataType, uint64_t smId)
            : SMMemoryPoolGenericBlobItem(nullptr,store->GetBlockDataCount(HPMBlockID(nodeId)), nodeId, dataType, smId)
            {
            m_store = store;

            if (m_size > 0)
                {
                m_data = (Byte*)new DataType();
                HPMBlockID blockID(m_nodeId);
                size_t nbBytesLoaded = m_store->LoadBlock((DataType*)m_data, m_size, blockID);
                m_size = nbBytesLoaded;
                }
            }

        virtual ~SMStoredMemoryPoolGenericBlobItem()
            {
            if (m_dirty)
                {
                HPMBlockID blockID(m_nodeId);
                m_store->StoreBlock((DataType*)m_data, 1, blockID);
                }
            }
    };


template <typename DataType> class SMStoredMemoryPoolBlobItem : public SMMemoryPoolBlobItem<DataType>
    {
    protected : 

        IHPMDataStore<Byte>* m_store;

    public : 
        
         SMStoredMemoryPoolBlobItem(uint64_t nodeId, IHPMDataStore<Byte>* store, SMPoolDataTypeDesc dataType, uint64_t smId)
            : SMMemoryPoolBlobItem(store->GetBlockDataCount(HPMBlockID(nodeId)) * sizeof(DataType), nodeId, dataType, smId)
            {                                    
            m_store = store;            
            
            if (m_size > 0)
                {                
                HPMBlockID blockID(m_nodeId);
                size_t nbBytesLoaded = m_store->LoadBlock ((DataType*)m_data, m_size, blockID);
                assert(nbBytesLoaded == m_size);
                }           
            }


         SMStoredMemoryPoolBlobItem(uint64_t nodeId, IHPMDataStore<DataType>* store, const DataType* data, uint64_t dataSize, SMPoolDataTypeDesc dataType, uint64_t smId)
            : SMMemoryPoolBlobItem(dataSize, nodeId, dataType, smId)
            {                                    
            m_store = store;                        

            if (m_size > 0)
                {                
                memcpy((DataType*)m_data, data, dataSize);
                m_dirty = true;
                }           
            }

        virtual ~SMStoredMemoryPoolBlobItem()
            {
            if (m_dirty)
                {
                HPMBlockID blockID(m_nodeId);
                m_store->StoreBlock(m_data, m_size, blockID);                
                }
            }    
    };



template <typename DataType> class SMMemoryPoolVectorItem : public SMMemoryPoolItemBase
    {

private:

protected: 

        size_t m_nbItems;
        size_t m_allocatedSize;

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
            m_index = initialPosition; 
            }
        const_iterator_t    ConvertToConst () const
            {
            return const_iterator_t(m_vector, m_index);
            }

        const_reference     Dereference    () const {
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

    SMMemoryPoolVectorItem(size_t nbItems, uint64_t nodeId, SMPoolDataTypeDesc dataType, uint64_t smId)                        
            {
            m_nbItems = nbItems;
            m_size = nbItems * sizeof(DataType);
            m_allocatedSize = m_size;
            m_nodeId = nodeId;
            m_smId = smId;
            
            if (m_nbItems > 0)
                m_data = (Byte*)new DataType[nbItems];
            else
                m_data = 0;
            
            m_dataType = dataType;
            }
      
    /** -----------------------------------------------------------------------------

        Destroyer. If the object is pool managed then it is removed from the pool
        during destruction.

        -----------------------------------------------------------------------------
    */

    virtual ~SMMemoryPoolVectorItem()
        { 
        if (m_data != 0)
            {
            delete[](DataType*)m_data;
            m_data = 0;
            }
        }

    virtual bool reserve(size_t newCount)
        {   
        assert(newCount > 0);
        
        size_t newSize = newCount * sizeof(DataType);
                                                
        if (newSize > m_allocatedSize)
            {            
            Byte* newMemory = (Byte*)new DataType[newCount];
            memcpy(newMemory, m_data, m_nbItems*sizeof(DataType));
            delete [] (DataType*)m_data;
            m_data = newMemory;                
            m_allocatedSize = newSize;
            }

        return true;
        }

    size_t capacity() const
        {
        return m_allocatedSize / sizeof(DataType);               
        }

    virtual bool push_back(const DataType& newObject)
        {        
        if (m_allocatedSize < (m_nbItems + 1) * sizeof(DataType))
            {
            reserve(ceil((m_nbItems + 1) * 1.5));
            }
        ((DataType*)m_data)[m_nbItems] = newObject;
        m_nbItems++;
        
        m_size = m_nbItems * sizeof(DataType);
        m_dirty = true;        

        NotifySizeChangePoolItem(sizeof(DataType));

        assert(m_size <= m_allocatedSize);

        return true;
        }
    
    virtual bool push_back(const DataType* newObjects, size_t count)
        {        
        assert(count != 0);      

        if ((m_nbItems + count) * sizeof(DataType) > m_allocatedSize)
            {
            reserve(ceil((m_nbItems + count) * 1.5));                   
            }

        memcpy(&((DataType*)m_data)[m_nbItems], newObjects, count * sizeof(DataType));
                      
        m_nbItems += count;    
        m_size = m_nbItems * sizeof(DataType);
        m_dirty = true;

        NotifySizeChangePoolItem(count * sizeof(DataType));

        assert(m_size <= m_allocatedSize);
       
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
        assert(maxCount == m_nbItems);
        memcpy(objects, m_data, maxCount * sizeof(DataType));        
        return maxCount;
        }

    // This used to be a reference returned but this caused a problem when the ref was maintained and the
    // tile discarded...
    virtual const DataType& operator[](size_t index) const
        {                     
        HPRECONDITION(index < m_nbItems);        
              
        return ((DataType*)m_data)[index];
        }
    
#if 0 
    void modify(size_t          index,
                const DataType& newDataValue)
        {
        assert(!"MST TBD");
        /*
        m_memory[index] = newDataValue;
        SetDirty (true);
        */
        }
#endif

    virtual size_t size() const
        {
        return m_nbItems;
        }
    
    virtual void erase (const bvector<size_t>& indexes)
        {
        assert(indexes.size() <= m_nbItems && indexes.size() > 0);        
                
        bvector<bool> toEraseItems(m_nbItems, false);
                               
        for (auto index : indexes)
            {
            assert(index < toEraseItems.size());
            toEraseItems[index] = true;            
            }
        
        DataType* currentData = (DataType*)m_data;
        //NEEDS_WORK_POOL //should we diminish pool size if the size is too large after erase 
        //size_t newSize = m_nbItems - indexes.size();
        //DataType* newData = new DataType[newSize];        
        DataType* newData = (DataType*)m_data;
        size_t newDataInd = 0;

        for (size_t ind = 0; ind < toEraseItems.size(); ind++)
            {
            if (!toEraseItems[ind])
                {
                newData[newDataInd] = currentData[ind];
                newDataInd++;
                }            
            }

        m_nbItems -= indexes.size();        
        m_size = m_nbItems * sizeof(DataType);
        m_dirty = true;

        NotifySizeChangePoolItem(-1 * indexes.size() * sizeof(DataType));
        }

     virtual void erase (size_t index)
        {
        memcpy(&m_data[index], &m_data[index + 1], m_nbItems - index - 1 * sizeof(DataType));
        m_nbItems -= 1;      
        m_size = m_nbItems * sizeof(DataType);
        m_dirty = true;  

        NotifySizeChangePoolItem(-1 * (int64_t)sizeof(DataType));
        }

    virtual void clearFrom(size_t indexToClearFrom)
        {        
        assert(indexToClearFrom < m_nbItems);

        if (indexToClearFrom == 0)
            clear();
        else
            {
            NotifySizeChangePoolItem((indexToClearFrom - m_nbItems) * sizeof(DataType));
            m_nbItems = indexToClearFrom; 
            m_size = m_nbItems * sizeof(DataType);
            m_dirty = true;
            }            
        }

    virtual void clear()
        {   
        if (m_data != 0)
            {
            NotifySizeChangePoolItem(-(int64_t)m_nbItems * sizeof(DataType));        
            delete [] m_data;                        
            m_nbItems = 0;
            m_data = 0;  
            m_size = 0;
            m_allocatedSize = 0;
            m_dirty = true;            
            }
        }
    
    const_iterator begin() const
        {
        if (m_nbItems > 0)
            return const_iterator(this, (size_t)0);
        return const_iterator(this, iterator::npos);

        }
    
    const_iterator end() const
        {
        return const_iterator(this, iterator::npos);
        }

    void random_shuffle()
        {                                        
        if (m_nbItems > 0)
            std::random_shuffle(m_data, &(m_data[m_nbItems - 1]));

        m_dirty = true;        
        }
    };

template <typename DataType> class SMMemoryPoolGenericVectorItem : public SMMemoryPoolVectorItem<DataType>
    {
    public:
    SMMemoryPoolGenericVectorItem(size_t nbItems, uint64_t nodeId, SMPoolDataTypeDesc dataType, uint64_t smId)
        : SMMemoryPoolVectorItem<DataType>(nbItems, nodeId, dataType, smId)
        {

        }

    virtual ~SMMemoryPoolGenericVectorItem()
        {
        if (m_data != 0)
            {
            delete[] (DataType*)m_data;
            m_data = 0;
            }
        }

    virtual bool reserve(size_t newCount)
        {
        assert(newCount > 0);

        size_t newSize = newCount * sizeof(DataType);

        if (newSize > m_allocatedSize)
            {
            Byte* newMemory = (Byte*)new DataType[newCount];
            DataType* newMemoryTyped = (DataType*)newMemory;
            for (size_t i = 0; i < m_nbItems; ++i)
                {
                newMemoryTyped[i] = ((DataType*)m_data)[i];
                }
            delete[](DataType*)m_data;
            m_data = newMemory;
            m_allocatedSize = newSize;
            }

        return true;
        }

    virtual void clear()
        {
        if (m_data != 0)
            {
            delete[] (DataType*)m_data;
            m_nbItems = 0;
            m_data = 0;
            m_size = 0;
            m_allocatedSize = 0;
            m_dirty = true;
            }
        }

    virtual void clearFrom(size_t indexToClearFrom)
        {
        assert(indexToClearFrom < m_nbItems);

        if (indexToClearFrom == 0)
            clear();
        else
            {
            size_t clearedDataSize = 0;
            for (size_t i = indexToClearFrom; i < m_nbItems; ++i)
                clearedDataSize += GetSizeInMemory((DataType*)m_data + i);
            NotifySizeChangePoolItem(-(int64_t)clearedDataSize);
            m_nbItems = indexToClearFrom;
            m_dirty = true;
            }
        }

    virtual void erase(const bvector<size_t>& indexes)
        {
        assert(indexes.size() <= m_nbItems && indexes.size() > 0);

        bvector<bool> toEraseItems(m_nbItems, false);

        for (auto index : indexes)
            {
            assert(index < toEraseItems.size());
            toEraseItems[index] = true;
            }

        DataType* currentData = (DataType*)m_data;
        //NEEDS_WORK_POOL //should we diminish pool size if the size is too large after erase 
        //size_t newSize = m_nbItems - indexes.size();
        //DataType* newData = new DataType[newSize];        
        DataType* newData = (DataType*)m_data;
        size_t newDataInd = 0;
        size_t erasedSize = 0;
        for (size_t ind = 0; ind < toEraseItems.size(); ind++)
            {
            if (!toEraseItems[ind])
                {
                newData[newDataInd] = currentData[ind];
                newDataInd++;
                }
            else
                {
                erasedSize += GetSizeInMemory(currentData + ind);
                }
            }

        m_nbItems -= indexes.size();
        m_size -= erasedSize;
        m_dirty = true;

        NotifySizeChangePoolItem(-1 * erasedSize);
        }

    virtual void Replace(size_t index, const DataType& val)
        {
        size_t oldSize = GetSizeInMemory((DataType*)m_data + index);
        size_t newSize = GetSizeInMemory((DataType*)&val);
        *((DataType*)m_data + index) = val;
        m_size += newSize - oldSize;
        NotifySizeChangePoolItem(newSize - oldSize);
        }

    virtual void erase(size_t index)
        {
        size_t erasedSize = GetSizeInMemory((DataType*)m_data + index);
        memcpy(&m_data[index], &m_data[index + 1], m_nbItems - index - 1 * sizeof(DataType));
        m_nbItems -= 1;
        m_size -= erasedSize;
        m_dirty = true;

        NotifySizeChangePoolItem(-1 * (int64_t)erasedSize);
        }

    virtual bool push_back(const DataType& newObject)
        {
        if (m_allocatedSize < (m_nbItems + 1) * sizeof(DataType))
            {
            reserve(ceil((m_nbItems + 1) * 1.5));
            }

        ((DataType*)m_data)[m_nbItems] = newObject;
        m_nbItems++;
        size_t addedSize = GetSizeInMemory(((DataType*)m_data) + m_nbItems - 1);
        m_size += addedSize;
        m_dirty = true;

        NotifySizeChangePoolItem(addedSize);

        assert(m_nbItems*sizeof(DataType) <= m_allocatedSize);

        return true;
        }

    virtual bool push_back(const DataType* newObjects, size_t count)
        {
        assert(count != 0);

        if ((m_nbItems + count) * sizeof(DataType) > m_allocatedSize)
            {
            reserve(ceil((m_nbItems + count) * 1.5));
            }
        size_t addedSize = 0;
        for (size_t i = 0; i < count; ++i)
            addedSize += GetSizeInMemory(newObjects + i);
        memcpy(&((DataType*)m_data)[m_nbItems], newObjects, count * sizeof(DataType));

        m_nbItems += count;
        m_size += addedSize;
        m_dirty = true;

        NotifySizeChangePoolItem(addedSize);

        assert(m_nbItems*sizeof(DataType) <= m_allocatedSize);

        return true;
        }
    };

    
template <typename DataType> class SMStoredMemoryPoolGenericVectorItem : public SMMemoryPoolGenericVectorItem<DataType>
    {
    private: 

        IHPMDataStore<DataType>* m_store;
        
    public:        
                        
        SMStoredMemoryPoolGenericVectorItem(uint64_t nodeId, IHPMDataStore<DataType>* store, SMPoolDataTypeDesc dataType, uint64_t smId)
            : SMMemoryPoolGenericVectorItem(store->GetBlockDataCount(HPMBlockID(nodeId)), nodeId, dataType, smId)
            {                                    
            m_store = store;            
            
            if (m_nbItems > 0)
                {                
                HPMBlockID blockID(m_nodeId);
                size_t nbBytesLoaded = m_store->LoadBlock ((DataType*)m_data, m_nbItems, blockID);
                m_size = nbBytesLoaded;
                }           
            }

        virtual ~SMStoredMemoryPoolGenericVectorItem()
            {
            if (m_dirty)
                {
                HPMBlockID blockID(m_nodeId);
                m_store->StoreBlock((DataType*)m_data, m_nbItems, blockID);                
                }
            }    
    };

    template <typename DataType> class SMStoredMemoryPoolVectorItem : public SMMemoryPoolVectorItem<DataType>
        {
        private:

            IHPMDataStore<DataType>* m_store;

        public:

            SMStoredMemoryPoolVectorItem(uint64_t nodeId, IHPMDataStore<DataType>* store, SMPoolDataTypeDesc dataType, uint64_t smId)
                : SMMemoryPoolVectorItem(store->GetBlockDataCount(HPMBlockID(nodeId)), nodeId, dataType, smId)
                {
                m_store = store;

                if (m_nbItems > 0)
                    {
                    HPMBlockID blockID(m_nodeId);
                    size_t nbBytesLoaded = m_store->LoadBlock((DataType*)m_data, m_nbItems, blockID);
                    assert(nbBytesLoaded == sizeof(DataType) * m_nbItems);
                    }
                }

            virtual ~SMStoredMemoryPoolVectorItem()
                {
                if (m_dirty)
                    {
                    HPMBlockID blockID(m_nodeId);
                    m_store->StoreBlock((DataType*)m_data, m_nbItems, blockID);
                    }
                }
        };


typedef RefCountedPtr<SMMemoryPoolItemBase> SMMemoryPoolItemBasePtr;

//First impl - dead lock
static clock_t s_timeDiff = CLOCKS_PER_SEC * 120;
static double s_maxMemBeforeFlushing = 1.2;

class SMMemoryPool : public RefCountedBase
    {
    private : 

        static SMMemoryPoolPtr           s_memoryPool; 

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

        SMMemoryPool();
            
        virtual ~SMMemoryPool();


    public : 

        static SMMemoryPoolItemId s_UndefinedPoolItemId;
        
            
        template<typename T>
        bool GetItem(RefCountedPtr<SMMemoryPoolVectorItem<T>>& poolMemVectorItemPtr, SMMemoryPoolItemId id, uint64_t nodeId, SMPoolDataTypeDesc dataType, uint64_t smId)
            {
            if (id == SMMemoryPool::s_UndefinedPoolItemId)
                return false; 

            assert(!poolMemVectorItemPtr.IsValid());
            SMMemoryPoolItemBasePtr memItemPtr;
            
            if (GetItem(memItemPtr, id) && memItemPtr->IsCorrect(nodeId, dataType,smId))
                {
                poolMemVectorItemPtr = memItemPtr->GetAsPoolVector<T>();                
                }

            return poolMemVectorItemPtr.IsValid();
            }  

        template<typename T>
        bool GetItem(RefCountedPtr<SMMemoryPoolGenericVectorItem<T>>& poolMemVectorItemPtr, SMMemoryPoolItemId id, uint64_t nodeId, SMPoolDataTypeDesc dataType, uint64_t smId)
            {
            if (id == SMMemoryPool::s_UndefinedPoolItemId)
                return false;

            assert(!poolMemVectorItemPtr.IsValid());
            SMMemoryPoolItemBasePtr memItemPtr;

            if (GetItem(memItemPtr, id) && memItemPtr->IsCorrect(nodeId, dataType, smId))
                {
                poolMemVectorItemPtr = memItemPtr->GetAsGenericPoolVector<T>();
                }

            return poolMemVectorItemPtr.IsValid();
            }

        template<typename T>
        bool GetItem(RefCountedPtr<SMMemoryPoolBlobItem<T>>& poolMemBlobItemPtr, SMMemoryPoolItemId id, uint64_t nodeId, SMPoolDataTypeDesc dataType, uint64_t smId)
            {
            if (id == SMMemoryPool::s_UndefinedPoolItemId)
                return false; 

            assert(!poolMemBlobItemPtr.IsValid());
            SMMemoryPoolItemBasePtr memItemPtr;
            
            if (GetItem(memItemPtr, id) && memItemPtr->IsCorrect(nodeId, dataType, smId))
                {
                poolMemBlobItemPtr = memItemPtr->GetAsBlobPoolItem<T>();                
                }

            return poolMemBlobItemPtr.IsValid();
            }  


        template<typename T>
        bool GetItem(RefCountedPtr<SMMemoryPoolGenericBlobItem<T>>& poolMemBlobItemPtr, SMMemoryPoolItemId id, uint64_t nodeId, SMPoolDataTypeDesc dataType, uint64_t smId)
            {
            if (id == SMMemoryPool::s_UndefinedPoolItemId)
                return false; 

            assert(!poolMemBlobItemPtr.IsValid());
            SMMemoryPoolItemBasePtr memItemPtr;
            
            if (GetItem(memItemPtr, id) && memItemPtr->IsCorrect(nodeId, dataType, smId))
                {
                poolMemBlobItemPtr = memItemPtr->GetAsGenericBlobPoolItem<T>();                
                }

            return poolMemBlobItemPtr.IsValid();
            }          

               
        bool GetItem(SMMemoryPoolItemBasePtr& memItemPtr, SMMemoryPoolItemId id);
            
        bool RemoveItem(SMMemoryPoolItemId id, uint64_t nodeId, SMPoolDataTypeDesc dataType, uint64_t smId);
            
        SMMemoryPoolItemId AddItem(SMMemoryPoolItemBasePtr& poolItem);

        void ReplaceItem(SMMemoryPoolItemBasePtr& poolItem, SMMemoryPoolItemId id, uint64_t nodeId, SMPoolDataTypeDesc dataType, uint64_t smId);

        void NotifySizeChangePoolItem(SMMemoryPoolItemBase* poolItem, int64_t sizeDelta);

        bool SetMaxSize(uint64_t maxSize)
            {
            if (m_currentPoolSizeInBytes > 0)
                return false;

            m_maxPoolSizeInBytes = maxSize;

            return true;
            }

        static SMMemoryPoolPtr GetInstance()
            {
            if (s_memoryPool == 0)
                s_memoryPool = new SMMemoryPool;     
            
            return s_memoryPool;
            }
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE