//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------


#pragma once

#include <limits>   

#include <ImagePP/all/h/HFCPtr.h>
#include <ImagePP/all/h/HPMDataStore.h>
#include <ImagePP/h/HIterators.h>
#include "ComputeMemoryUseTraits.h"
#include <iostream>
#include <queue>

#include "Stores/ISMDataStore.h"
#include "Stores/SMStoreUtils.h"

#include "Tracer.h"

using namespace std;

template<typename T> inline size_t GetSizeInMemory(T* item)
{
    return sizeof(T);
}


template<> inline size_t GetSizeInMemory<MTGGraph>(MTGGraph* item)
{
    size_t count = 0;
    count += sizeof(*item);
    count += (sizeof(MTGLabelMask) + 2 * sizeof(int))*item->GetLabelCount();
    count += sizeof(MTG_Node)*item->GetNodeIdCount();
    count += sizeof(int)*item->GetNodeIdCount()* item->GetLabelCount();
    return count;
}

template<> inline size_t GetSizeInMemory<DifferenceSet>(DifferenceSet* item)
{
    size_t count = sizeof(item) + item->addedFaces.size() * sizeof(DPoint3d) + item->addedVertices.size() * sizeof(int32_t) +
        item->removedFaces.size() * sizeof(int32_t) + item->removedVertices.size() * sizeof(int32_t) + item->addedUvIndices.size() * sizeof(int32_t) +
        item->addedUvs.size() * sizeof(DPoint2d);
    return count;
}

template<> inline size_t GetSizeInMemory<BENTLEYTERRAINMODEL_NAMESPACE_NAME::BcDTMPtr>(BENTLEYTERRAINMODEL_NAMESPACE_NAME::BcDTMPtr* item)
{
    size_t count = (item->get() == nullptr ? 0 : ((*item)->GetTinHandle() == nullptr ? 0 : sizeof(BC_DTM_OBJ) + (*item)->GetPointCount() *(sizeof(DPoint3d) + sizeof(DTM_TIN_NODE) + sizeof(DTM_CIR_LIST) * 6)));
    return count;
}

USING_NAMESPACE_IMAGEPP
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE
    

class Spinlock
{
private:
    std::atomic_flag lockFlag; 
public:

    Spinlock()
    {
    lockFlag.clear();
    }

    void lock()
    {
        if (!lockFlag.test_and_set())
            return;
        while (lockFlag.test_and_set())
               std::this_thread::yield();
    }

    void unlock()
    {
        lockFlag.clear();
    }
};

//NEEDS_WORK_SM : Merge SMStoreDataType and SMStoreDataType together?
SMStoreDataType GetStoreDataType(SMStoreDataType poolDataType);
    

class SMMemoryPoolItemBase;
typedef RefCountedPtr<SMMemoryPoolItemBase> SMMemoryPoolItemBasePtr;


template <typename DataType> class SMMemoryPoolVectorItem;
template <typename DataType> class SMMemoryPoolGenericVectorItem;
template <typename DataType> class SMMemoryPoolBlobItem;
template <typename DataType> class SMMemoryPoolGenericBlobItem;
template <typename DataType> class SMStoredMemoryPoolMultiItems;


enum class SMMemoryPoolType
{
    CPU = 0,
    GPU = 1
};



class SMMemoryPool;
typedef RefCountedPtr<SMMemoryPool> SMMemoryPoolPtr;

typedef uint64_t SMMemoryPoolItemId;

void GetAllDataTypesInCompositeDataType(bvector<SMStoreDataType>& dataTypes, SMStoreDataType compositeDataType);

class SizeChangePoolItemListener
    {  
    public : 

        virtual void NotifyListener(SMMemoryPoolItemBase* poolMemoryItem, int64_t sizeDelta, int64_t nbItemDelta) = 0;
    };

class SMMemoryPoolItemBase : public RefCountedBase
    {
    template <typename DataType> friend class SMStoredMemoryPoolMultiItems;    

    private : 

        SMMemoryPoolItemId                   m_poolItemId;
        bvector<SizeChangePoolItemListener*> m_listeners; 
        std::mutex                           m_listenerMutex; 

    protected : 

        Byte*           m_data;
        uint64_t        m_size;
        uint64_t        m_nodeId;
        uint64_t        m_smId;
        SMStoreDataType m_dataType;
        bool            m_dirty;

                
        //ContainerType m_containerType;

        //type_info m_dataType;
        
        BENTLEY_SM_EXPORT virtual void NotifySizeChangePoolItem(int64_t sizeDelta, int64_t nbItemDelta);
        
    public :

        BENTLEY_SM_EXPORT virtual uint32_t AddRef() const;
        BENTLEY_SM_EXPORT virtual uint32_t Release() const;

        BENTLEY_SM_EXPORT SMMemoryPoolItemBase();
            
        BENTLEY_SM_EXPORT SMMemoryPoolItemBase(Byte* data, uint64_t size, uint64_t nodeId, SMStoreDataType& dataType, uint64_t smId);
                    
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
        
        BENTLEY_SM_EXPORT virtual ~SMMemoryPoolItemBase();

        BENTLEY_SM_EXPORT void AddListener(SizeChangePoolItemListener* listener)
            {
            std::lock_guard<std::mutex> lock(m_listenerMutex);            
            m_listeners.push_back(listener);
            }        

        BENTLEY_SM_EXPORT void RemoveListener(SizeChangePoolItemListener* toRemoveListener)
            {
            std::lock_guard<std::mutex> lock(m_listenerMutex);
            auto iter(m_listeners.begin());
            auto iterEnd(m_listeners.end());

            while (iter != iterEnd)
                {
                if (*iter == toRemoveListener)
                    {
                    m_listeners.erase(iter);
                    return;
                    }

                iter++;
                }

            assert(!"Listener not found");
            }

        BENTLEY_SM_EXPORT SMStoreDataType GetDataType() const { return m_dataType; }
                    
        BENTLEY_SM_EXPORT uint64_t GetSize();

        BENTLEY_SM_EXPORT bool IsCorrect(uint64_t nodeId, SMStoreDataType& dataType);

        BENTLEY_SM_EXPORT bool IsCorrect(uint64_t nodeId, SMStoreDataType& dataType, uint64_t smId);

        BENTLEY_SM_EXPORT virtual bool IsDirty () const { return this->m_dirty; }
            
        BENTLEY_SM_EXPORT SMMemoryPoolItemId GetPoolItemId() const;

        BENTLEY_SM_EXPORT uint64_t GetNodeId() { return m_nodeId;  }
            
        BENTLEY_SM_EXPORT  void SetPoolItemId(SMMemoryPoolItemId poolItemId);

        BENTLEY_SM_EXPORT void SetDirty(bool dirty = true) { this->m_dirty = dirty; }

        BENTLEY_SM_EXPORT bool IsCompressedType()
            {
            return m_dataType == SMStoreDataType::TextureCompressed;
            }
        SMMemoryPoolType m_poolId;

        virtual uint64_t GetSizeAllocated() { return GetSize(); }
    };


class SMMemoryPoolMultiItemsBase : public SMMemoryPoolItemBase,
                                   public SizeChangePoolItemListener
    {   
    protected : 

        bvector<SMMemoryPoolItemBasePtr> m_items; 

    public : 

        SMMemoryPoolMultiItemsBase(uint64_t nodeId, SMStoreDataType& dataType, uint64_t smId)
        : SMMemoryPoolItemBase(0, 0, nodeId, dataType, smId)
            {            
            }

        virtual ~SMMemoryPoolMultiItemsBase()
            {            
            }

        template<typename T>
        bool GetItem(RefCountedPtr<SMMemoryPoolVectorItem<T>>& poolMemVectorItemPtr, SMStoreDataType dataType)
            {
            for (auto& item : m_items)
                {
                if (item->GetDataType() == dataType)
                    {
                    poolMemVectorItemPtr = item->GetAsPoolVector<T>();
                    return true;
                    }
                }

            assert(!"Should not occur");
            return false;
            }

        template<typename T>
        bool GetItem(RefCountedPtr<SMMemoryPoolBlobItem<T>>& poolMemBlobItemPtr, SMStoreDataType dataType)
            {
            for (auto& item : m_items)
                {
                if (item->GetDataType() == dataType)
                    {
                    poolMemBlobItemPtr = item->GetAsBlobPoolItem<T>();
                    return true;
                    }
                }

            assert(!"Should not occur");
            return false;
            }

        virtual bool IsDirty () const override
            {             
            for (auto& item : m_items)
                {
                if (item->IsDirty())
                    return true;                
                }

            return false;             
            }         
    };

typedef RefCountedPtr<SMMemoryPoolMultiItemsBase> SMMemoryPoolMultiItemsBasePtr;


template <typename DataType> class SMStoredMemoryPoolMultiItems : public SMMemoryPoolMultiItemsBase                                                                  
    { 
    private : 

        ISMNodeDataStoreTypePtr<DataType> m_dataStore;   
                        
        SMMemoryPoolItemBasePtr GetNodeDataStore(PointAndTriPtIndicesBase& multiData, size_t nbItems, uint64_t nodeId, SMStoreDataType dataType, uint64_t smId)
            {
            SMMemoryPoolItemBasePtr item; 

            if (dataType == SMStoreDataType::Points)
                {
                item = new SMMemoryPoolVectorItem<DPoint3d>(nbItems, nodeId, dataType, smId);            
                multiData.m_pointData = (DPoint3d*)item->m_data;
                }
            else
            if (dataType == SMStoreDataType::TriPtIndices)
                {
                item = new SMMemoryPoolVectorItem<int32_t>(nbItems, nodeId, dataType, smId);            
                multiData.m_indicesData = (int32_t*)item->m_data;        
                }   

            return item;
            }

        SMMemoryPoolItemBasePtr GetNodeDataStore(Cesium3DTilesBase& multiData, size_t nbItems, uint64_t nodeId, SMStoreDataType dataType, uint64_t smId)
            {
            SMMemoryPoolItemBasePtr item;
            switch (dataType)
                {
                case SMStoreDataType::Points:
                    {
                    item = new SMMemoryPoolVectorItem<DPoint3d>(nbItems, nodeId, dataType, smId);
                    multiData.m_pointData = (DPoint3d*)item->m_data;
                    break;
                    }
                case SMStoreDataType::TriPtIndices:
                    {
                    item = new SMMemoryPoolVectorItem<int32_t>(nbItems, nodeId, dataType, smId);
                    multiData.m_indicesData = (int32_t*)item->m_data;
                    multiData.m_uvIndicesData = multiData.m_indicesData;
                    break;
                    }
                case SMStoreDataType::UvCoords:
                    {
                    item = new SMMemoryPoolVectorItem<DPoint2d>(nbItems, nodeId, dataType, smId);
                    multiData.m_uvData = (DPoint2d*)item->m_data;
                    break;
                    }
                case SMStoreDataType::Texture:
                case SMStoreDataType::TextureCompressed:
                    {
                    item = new SMMemoryPoolBlobItem<Byte>(nbItems, nodeId, dataType, smId);
                    multiData.m_textureData = (Byte*)item->m_data;
                    break;
                    }
                default:
                    {
                    assert(!"Unhandled store data type");
                    break;
                    }
                }

            return item;
            }

    public : 
                
        SMStoredMemoryPoolMultiItems(ISMNodeDataStoreTypePtr<DataType>& dataStore, uint64_t nodeId, SMStoreDataType compositeDataType, uint64_t smId)
        : SMMemoryPoolMultiItemsBase(nodeId, compositeDataType, smId)
            {
            m_dataStore = dataStore;

            bvector<SMStoreDataType> dataTypes;
            DataType multiData; 

            GetAllDataTypesInCompositeDataType(dataTypes, compositeDataType);

            this->m_size = 0;                           
                        
            for (auto& dataType : dataTypes)
                {                
                m_items.push_back(GetNodeDataStore(multiData, dataStore->GetBlockDataCount(HPMBlockID(nodeId), dataType), nodeId, dataType, smId));                
                this->m_size += m_items.back()->GetSize();                
                m_items.back()->AddListener(this);
                }            
            
            if (this->m_size > 0)
                {
                HPMBlockID blockID(this->m_nodeId);
                size_t nbBytesLoaded = m_dataStore->LoadBlock(&multiData, this->m_size, blockID);
                assert(nbBytesLoaded <= this->m_size);
                }                                    
            } 


        virtual ~SMStoredMemoryPoolMultiItems()
            {            
            if (IsDirty ())
                {
                assert(!"Not implemented yet");
                /*
                DataType multiData; 

                HPMBlockID blockID(m_nodeId);               
                m_dataStore->StoreBlock(&m_multiData, 1, blockID);                                
                */
                }

            for (auto& item : m_items) 
                {
                item->RemoveListener(this);
                }                                    
            }

#ifdef VANCOUVER_API
        static SMStoredMemoryPoolMultiItems* CreateItem(ISMNodeDataStoreTypePtr<DataType>& dataStore, uint64_t nodeId, SMStoreDataType compositeDataType, uint64_t smId)
            {
            return new SMStoredMemoryPoolMultiItems(dataStore, nodeId, compositeDataType, smId);
            }
#endif

        virtual void NotifyListener(SMMemoryPoolItemBase* poolMemoryItem, int64_t sizeDelta, int64_t nbItemDelta) override
            {            
            HPMBlockID blockID(this->m_nodeId);

            m_dataStore->ModifyBlockDataCount(blockID, nbItemDelta, GetStoreDataType(poolMemoryItem->GetDataType()));            
            this->m_size += sizeDelta;
            }
    };


template <typename DataType> class SMMemoryPoolBlobItem : public SMMemoryPoolItemBase
    {
    protected :         

    public : 
        
        SMMemoryPoolBlobItem(size_t size, uint64_t nodeId, SMStoreDataType dataType, uint64_t smId)
            {            
            this->m_size = size;
            m_nodeId = nodeId;
            m_smId = smId;
            if (this->m_size > 0)
                this->m_data = (Byte*)new Byte[m_size];
            else
                this->m_data = 0;

            m_dataType = dataType;
            }

        virtual ~SMMemoryPoolBlobItem()
            {
            }

        const DataType* GetData()
            {
            return (const DataType*)this->m_data;
            }

        void SetData(DataType* data, size_t sizeInBytes)
            {
            if (m_data != nullptr && this->m_size > 0)
                delete[] (DataType*)this->m_data;
            if (GetPoolItemId() != SMMemoryPool::s_UndefinedPoolItemId) 
                {
                TRACEPOINT(EventType::POOL_CHANGESIZEITEM, this->GetNodeId(), (uint64_t)-1, -1,
                    this->GetPoolItemId(), (uint64_t)data, 990)

                NotifySizeChangePoolItem((int64_t)sizeInBytes - (int64_t)this->m_size, (int64_t)sizeInBytes - (int64_t)this->m_size);
                }
            this->m_size = sizeInBytes;
            if (this->m_size > 0)
                {
                this->m_data = (Byte*)new Byte[m_size];
                memcpy(m_data, data, sizeInBytes);
                }
            else
                this->m_data = 0;
            }
    };


template <typename DataType> class SMMemoryPoolGenericBlobItem : public SMMemoryPoolItemBase
    {
    protected :         

    public : 
#ifdef VANCOUVER_API
        static SMMemoryPoolGenericBlobItem<DataType>* CreateItem(DataType* data, size_t size, uint64_t nodeId, SMStoreDataType dataType, uint64_t smId)
            {
            return new SMMemoryPoolGenericBlobItem<DataType>(data, size, nodeId, dataType, smId);
            }
#endif        
        SMMemoryPoolGenericBlobItem(DataType* data, size_t size, uint64_t nodeId, SMStoreDataType dataType, uint64_t smId)
            {               
            this->m_size = size;
            m_nodeId = nodeId;
            this->m_data = (Byte*)data;            
            m_dataType = dataType;
            m_smId = smId;
            }

        virtual ~SMMemoryPoolGenericBlobItem()
            {
            if (m_data != nullptr)
                delete (DataType*)this->m_data;
            this->m_data = 0;
            }

        const DataType* GetData()
            {
            return (const DataType*)this->m_data;
            }

        DataType* EditData()
            {
            return (DataType*)this->m_data;
            }

        void SetData(DataType* data)
            {
            if (m_data != nullptr && this->m_size > 0)
                delete (DataType*)this->m_data;
            this->m_data = (Byte*)data;
            if (GetPoolItemId() != SMMemoryPool::s_UndefinedPoolItemId)
                {
                TRACEPOINT(EventType::POOL_CHANGESIZEITEM, this->GetNodeId(), (uint64_t)-1, -1,
                    this->GetPoolItemId(), (uint64_t)data, 991)

                NotifySizeChangePoolItem((int64_t)GetSizeInMemory(data) - (int64_t)this->m_size, (int64_t)GetSizeInMemory(data) - (int64_t)this->m_size);
                }
            this->m_size = GetSizeInMemory(data);
            }

        void SetData(DataType* data, size_t sizeInBytes)
            {
            if (m_data != nullptr && this->m_size > 0)
                delete (DataType*)this->m_data;
            this->m_data = (Byte*)data;
            if (GetPoolItemId() != SMMemoryPool::s_UndefinedPoolItemId) 
            {
                TRACEPOINT(EventType::POOL_CHANGESIZEITEM, this->GetNodeId(), (uint64_t)-1, -1,
                    this->GetPoolItemId(), (uint64_t)data, 992)

                NotifySizeChangePoolItem((int64_t)sizeInBytes - (int64_t)this->m_size, (int64_t)sizeInBytes - (int64_t)this->m_size);
            }
            this->m_size = sizeInBytes;
            }
    };


template <typename DataType> class SMStoredMemoryPoolGenericBlobItem : public SMMemoryPoolGenericBlobItem<DataType>
    {
    private:
        
        ISMNodeDataStoreTypePtr<DataType> m_dataStore;

    public:
        
#ifdef VANCOUVER_API
        static SMStoredMemoryPoolGenericBlobItem<DataType>* CreateItem(uint64_t nodeId, ISMNodeDataStoreTypePtr<DataType>& store, SMStoreDataType dataType, uint64_t smId, bool loadBlock = true)
            {
            return new SMStoredMemoryPoolGenericBlobItem<DataType>(nodeId, store, dataType, smId, loadBlock);
            }
#endif

        SMStoredMemoryPoolGenericBlobItem(uint64_t nodeId, ISMNodeDataStoreTypePtr<DataType>& dataStore, SMStoreDataType dataType, uint64_t smId, bool loadBlock = true)
            : SMMemoryPoolGenericBlobItem<DataType>(nullptr,dataStore->GetBlockDataCount(HPMBlockID(nodeId)), nodeId, dataType, smId)
            {
            m_dataStore = dataStore;

            if (this->m_size > 0)
                {
                this->m_data = (Byte*)new DataType();

                if (loadBlock)
                    {
                    HPMBlockID blockID = HPMBlockID(this->m_nodeId);
                    size_t nbBytesLoaded = m_dataStore->LoadBlock((DataType*)this->m_data, this->m_size, blockID);
                    this->m_size = nbBytesLoaded;
                    }
                }
            }

        SMStoredMemoryPoolGenericBlobItem(uint64_t nodeId, IHPMDataStore<DataType>* store, const DataType* data, uint64_t dataSize, SMStoreDataType dataType, uint64_t smId)
            : SMMemoryPoolGenericBlobItem<DataType>((DataType*)new Byte[dataSize],dataSize, nodeId, dataType, smId)
            {
            this->m_store = store;

            if (this->m_size > 0)
                {
                memcpy((DataType*)this->m_data, data, dataSize);
                this->m_dirty = true;
                }
            }

        SMStoredMemoryPoolGenericBlobItem(uint64_t nodeId, ISMNodeDataStoreTypePtr<DataType>& store, const DataType* data, uint64_t dataSize, SMStoreDataType dataType, uint64_t smId)
            : SMMemoryPoolGenericBlobItem((DataType*)new Byte[dataSize], dataSize, nodeId, dataType, smId)
            {
            m_dataStore = store;

            if (this->m_size > 0)
                {
                memcpy((DataType*)this->m_data, data, dataSize);
                this->m_dirty = true;
                }
            }

        


        virtual ~SMStoredMemoryPoolGenericBlobItem()
            {            
            if (this->m_dirty)
                {
                HPMBlockID blockID = HPMBlockID(this->m_nodeId);                
                m_dataStore->StoreBlock((DataType*)this->m_data, 1, blockID);                
                }
            }
    };


template <typename DataType> class SMStoredMemoryPoolBlobItem : public SMMemoryPoolBlobItem<DataType>
    {
    protected : 
        
        ISMNodeDataStoreTypePtr<DataType> m_dataStore;

    public : 
#ifdef VANCOUVER_API
        static SMStoredMemoryPoolBlobItem<DataType>* CreateItem(uint64_t nodeId, ISMNodeDataStoreTypePtr<DataType>& store, SMStoreDataType dataType, uint64_t smId)
            {
            return new SMStoredMemoryPoolBlobItem<DataType>(nodeId, store, dataType, smId);
            }
        static SMStoredMemoryPoolBlobItem<DataType>* CreateItem(uint64_t nodeId, ISMNodeDataStoreTypePtr<DataType>& store, const DataType* data, uint64_t dataSize, SMStoreDataType dataType, uint64_t smId)
            {
            return new SMStoredMemoryPoolBlobItem<DataType>(nodeId, store, data, dataSize, dataType, smId);
            }
#endif
        
        SMStoredMemoryPoolBlobItem(uint64_t nodeId, ISMNodeDataStoreTypePtr<DataType>& store, SMStoreDataType dataType, uint64_t smId)
            : SMMemoryPoolBlobItem<DataType>(store->GetBlockDataCount(HPMBlockID(nodeId)) * sizeof(DataType), nodeId, dataType, smId)
            {                                    
            m_dataStore = store;            
            
            if (this->m_size > 0)
                {                
                HPMBlockID blockID = HPMBlockID(this->m_nodeId);
                size_t nbBytesLoaded = m_dataStore->LoadBlock ((DataType*)this->m_data, this->m_size, blockID);
                assert(nbBytesLoaded == this->m_size);
                }           
            }
         
        SMStoredMemoryPoolBlobItem(uint64_t nodeId, ISMNodeDataStoreTypePtr<DataType>& store, const DataType* data, uint64_t dataSize, SMStoreDataType dataType, uint64_t smId)
            : SMMemoryPoolBlobItem<DataType>(dataSize, nodeId, dataType, smId)
            {                                    
            m_dataStore = store;                        

            if (this->m_size > 0)
                {                
                memcpy((DataType*)this->m_data, data, dataSize);
                this->m_dirty = true;
                }           
            }

        virtual ~SMStoredMemoryPoolBlobItem()
            {
            if (this->m_dirty)
                {                
                HPMBlockID blockID = HPMBlockID(this->m_nodeId);
                if (this->m_dataType == SMStoreDataType::TextureCompressed)
                {
                    m_dataStore->StoreCompressedBlock(this->m_data, this->m_size, blockID);
                }
                else
                m_dataStore->StoreBlock(this->m_data, this->m_size, blockID);                                
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
    
    template <class T> class iteratorBase : public RandomAccessIteratorWithAutoReverseConst<iteratorBase<T>, iteratorBase<typename ImagePP::ReverseConstTrait<T>::type >, T>
        {
typedef RandomAccessIteratorWithAutoReverseConst<iteratorBase<T>, iteratorBase<typename ImagePP::ReverseConstTrait<T>::type >, T> super_class;
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
        typename super_class::const_iterator_t    ConvertToConst () const
            {
            return typename super_class::const_iterator_t(m_vector, m_index);
            }

        typename super_class::const_reference     Dereference    () const {
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

        void                AdvanceOf        (typename super_class::difference_type increment)
            {
            //HASSERT (m_index + increment > m_vector->size());

            m_index += increment;

            if (m_index >= m_vector->size())
                m_index = npos;
            }

        typename super_class::difference_type     DistanceFrom   (const typename super_class::iterator_t& otherItr) const
            {
            HASSERT (m_index != npos);
            HASSERT (otherItr.m_index != npos);

            HASSERT (m_vector == otherItr.m_vector);

            return m_index - otherItr.m_index;
            }

        bool                EqualTo        (const typename super_class::iterator_t& otherItr) const
            {
            HASSERT (m_vector == otherItr.m_vector);

            return ((m_vector == otherItr.m_vector) && (m_index == otherItr.m_index));

            }
        bool                LessThan       (const typename super_class::iterator_t& otherItr) const
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

    SMMemoryPoolVectorItem(size_t nbItems, uint64_t nodeId, SMStoreDataType dataType, uint64_t smId)
            {
            this->m_nbItems = nbItems;
            this->m_size = nbItems * sizeof(DataType);
            m_allocatedSize = this->m_size;
            m_nodeId = nodeId;
            m_smId = smId;
            
            if (this->m_nbItems > 0)
                this->m_data = (Byte*)new DataType[nbItems];
            else
                this->m_data = 0;
            
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
            delete[](DataType*)this->m_data;
            this->m_data = 0;
            }
        }

    virtual bool reserve(size_t newCount)
        {   
        assert(newCount > 0);
        
        size_t newSize = newCount * sizeof(DataType);
                                                
        if (newSize > m_allocatedSize)
            {            
            Byte* newMemory = (Byte*)new DataType[newCount];
            memcpy(newMemory, m_data, this->m_nbItems*sizeof(DataType));
            delete [] (DataType*)this->m_data;
            this->m_data = newMemory;       

            // Advice the pool for the memory, the count will by advice in the push methods
            NotifySizeChangePoolItem(newSize - m_allocatedSize, 0);

            TRACEPOINT(EventType::POOL_CHANGESIZEITEM, this->GetNodeId(), (uint64_t)-1, -1,
                this->GetPoolItemId(), (uint64_t)newMemory, 993)

            m_allocatedSize = newSize;
            }

        return true;
        }

    size_t capacity() const
        {
        return m_allocatedSize / sizeof(DataType);               
        }

    virtual uint64_t GetSizeAllocated() { return m_allocatedSize; }

    virtual bool push_back(const DataType& newObject)
        {        
        if (m_allocatedSize < (this->m_nbItems + 1) * sizeof(DataType))
            {
            reserve((size_t)ceil((this->m_nbItems + 1) * 1.5));
            }
        ((DataType*)this->m_data)[m_nbItems] = newObject;
        this->m_nbItems++;
        
        this->m_size = this->m_nbItems * sizeof(DataType);
        this->m_dirty = true;        

        // The reserve method did the memory part, for that reason we pass 0 here.
        NotifySizeChangePoolItem(0, 1);

        BeAssert(this->m_size <= m_allocatedSize);
        return true;
        }
    
    virtual bool push_back(const DataType* newObjects, size_t count)
        {        
        BeAssert(count != 0);      

        if ((this->m_nbItems + count) * sizeof(DataType) > m_allocatedSize)
            {
            reserve((size_t)ceil((this->m_nbItems + count) * 1.5));                   
            }

        memcpy(&((DataType*)this->m_data)[m_nbItems], newObjects, count * sizeof(DataType));
                      
        this->m_nbItems += count;    
        this->m_size = this->m_nbItems * sizeof(DataType);
        this->m_dirty = true;

		// The reserve method did the memory part, for that reason we pass 0 here.
        NotifySizeChangePoolItem(0, count);
        BeAssert(this->m_size <= m_allocatedSize);
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
        assert(maxCount == this->m_nbItems);
        memcpy(objects, m_data, maxCount * sizeof(DataType));        
        return maxCount;
        }

    // This used to be a reference returned but this caused a problem when the ref was maintained and the
    // tile discarded...
    virtual const DataType& operator[](size_t index) const
        {                     
        HPRECONDITION(index < this->m_nbItems);        
              
        return ((DataType*)this->m_data)[index];
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
        return this->m_nbItems;
        }
    
    virtual void erase (const bvector<size_t>& indexes)
        {
        assert(indexes.size() <= this->m_nbItems && indexes.size() > 0);        
                
        bvector<bool> toEraseItems(this->m_nbItems, false);
                               
        for (auto index : indexes)
            {
            assert(index < toEraseItems.size());
            toEraseItems[index] = true;            
            }
        
        DataType* currentData = (DataType*)this->m_data;
        //NEEDS_WORK_POOL //should we diminish pool size if the size is too large after erase 
        //size_t newSize = this->m_nbItems - indexes.size();
        //DataType* newData = new DataType[newSize];        
        DataType* newData = (DataType*)this->m_data;
        size_t newDataInd = 0;

        for (size_t ind = 0; ind < toEraseItems.size(); ind++)
            {
            if (!toEraseItems[ind])
                {
                newData[newDataInd] = currentData[ind];
                newDataInd++;
                }            
            }
        
        this->m_nbItems -= indexes.size();        
        this->m_size = this->m_nbItems * sizeof(DataType);
        this->m_dirty = true;

        // No impact on the memory for the moment, see //NEEDS_WORK_POOL above.
        NotifySizeChangePoolItem(0, -1 * (int64_t)indexes.size());
        }

     virtual void erase (size_t index)
        { 
        memcpy(&m_data[index], &m_data[index + 1], this->m_nbItems - index - 1 * sizeof(DataType));
        this->m_nbItems -= 1;      
        this->m_size = this->m_nbItems * sizeof(DataType);
        this->m_dirty = true;  

		// No memory size modification
        NotifySizeChangePoolItem(0, -1);
        }

    virtual void clearFrom(size_t indexToClearFrom)
        {        
        BeAssert(indexToClearFrom < this->m_nbItems);

        if (indexToClearFrom == 0)
            clear();
        else
            { 
			// No memory size modification
            NotifySizeChangePoolItem(0, -1 * ((int64_t)indexToClearFrom - (int64_t)this->m_nbItems));
            this->m_nbItems = indexToClearFrom; 
            this->m_size = this->m_nbItems * sizeof(DataType);
            this->m_dirty = true;
            }            
        }

    virtual void clear()
        {   
        if (m_data != 0)
            {
            TRACEPOINT(EventType::POOL_CHANGESIZEITEM, this->GetNodeId(), (uint64_t)-1, -1,
                this->GetPoolItemId(), (uint64_t)m_data, 994)

            NotifySizeChangePoolItem(-1 * static_cast<int64_t>(m_allocatedSize), -1 * static_cast<int64_t>(m_nbItems));
            delete [] m_data;                        
            this->m_nbItems = 0;
            this->m_data = 0;  
            this->m_size = 0;
            m_allocatedSize = 0;
            this->m_dirty = true;            
            }
        }
    
    const_iterator begin() const
        {
        if (this->m_nbItems > 0)
            return const_iterator(this, (size_t)0);
        return const_iterator(this, iterator::npos);

        }
    
    const_iterator end() const
        {
        return const_iterator(this, iterator::npos);
        }

    void random_shuffle()
        {                                        
        if (this->m_nbItems > 0)
            std::random_shuffle(m_data, &(m_data[m_nbItems - 1]));

        this->m_dirty = true;        
        }
    };

template <typename DataType> class SMMemoryPoolGenericVectorItem : public SMMemoryPoolVectorItem<DataType>
    {
    public:
    SMMemoryPoolGenericVectorItem(size_t nbItems, uint64_t nodeId, SMStoreDataType dataType, uint64_t smId)
        : SMMemoryPoolVectorItem<DataType>(nbItems, nodeId, dataType, smId)
        {

        }
#ifdef VANCOUVER_API
        static SMMemoryPoolGenericVectorItem<DataType>* CreateItem(size_t nbItems, uint64_t nodeId, SMStoreDataType dataType, uint64_t smId)
            {
            return new SMMemoryPoolGenericVectorItem<DataType>(nbItems, nodeId, dataType, smId);
            }
#endif

    virtual ~SMMemoryPoolGenericVectorItem()
        {
        if (this->m_data != 0)
            {
            delete[] (DataType*)this->m_data;
            this->m_data = 0;
            }
        }

    virtual bool reserve(size_t newCount)
        {
        BeAssert(newCount > 0);

        size_t newSize = newCount * sizeof(DataType);

        if (newSize > this->m_allocatedSize)
            {
            Byte* newMemory = (Byte*)new DataType[newCount];
            DataType* newMemoryTyped = (DataType*)newMemory;
            for (size_t i = 0; i < this->m_nbItems; ++i)
                {
                newMemoryTyped[i] = ((DataType*) this->m_data)[i];
                }
            delete[](DataType*)this->m_data;
            this->m_data = newMemory;

            TRACEPOINT(EventType::POOL_CHANGESIZEITEM, this->GetNodeId(), (uint64_t)-1, -1,
                this->GetPoolItemId(), (uint64_t)newMemory, 995)
            // The count parameter will be modified by the push method.
            this->NotifySizeChangePoolItem(newSize - this->m_allocatedSize, 0);
            this->m_allocatedSize = newSize;
            }

        return true;
        }

    virtual void clear()
        {
        if (this->m_data != 0)
            {
            TRACEPOINT(EventType::POOL_CHANGESIZEITEM, this->GetNodeId(), (uint64_t)-1, -1,
                this->GetPoolItemId(), (uint64_t)m_data, 996)
            this->NotifySizeChangePoolItem(-1 * static_cast<int64_t>(this->m_allocatedSize), -1 * static_cast<int64_t>(this->m_nbItems));
            delete[] (DataType*)this->m_data;
            this->m_nbItems = 0;
            this->m_data = 0;
            this->m_size = 0;
            this->m_allocatedSize = 0;
            this->m_dirty = true;
            }
        }

    virtual void clearFrom(size_t indexToClearFrom)
        {
        BeAssert(indexToClearFrom < this->m_nbItems);

        if (indexToClearFrom == 0)
            clear();
        else
            {
            size_t clearedDataSize = 0;
            for (size_t i = indexToClearFrom; i < this->m_nbItems; ++i)
                clearedDataSize += GetSizeInMemory((DataType*) this->m_data + i);
				
			// No memory size modification
            this->NotifySizeChangePoolItem(0, -1 * ((int64_t)indexToClearFrom - (int64_t)this->m_nbItems));
            this->m_nbItems = indexToClearFrom;
            this->m_dirty = true;
            }
        }

    virtual void erase(const bvector<size_t>& indexes)
        {
        BeAssert(indexes.size() <= this->m_nbItems && indexes.size() > 0);

        bvector<bool> toEraseItems(this->m_nbItems, false);

        for (auto index : indexes)
            {
            BeAssert(index < toEraseItems.size());
            toEraseItems[index] = true;
            }

        DataType* currentData = (DataType*) this->m_data;
        //NEEDS_WORK_POOL //should we diminish pool size if the size is too large after erase 
        //size_t newSize = this->m_nbItems - indexes.size();
        //DataType* newData = new DataType[newSize];        
        DataType* newData = (DataType*) this->m_data;
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

        this->m_nbItems -= indexes.size();
        this->m_size -= erasedSize;
        this->m_dirty = true;

        this->NotifySizeChangePoolItem(0, -1 * static_cast<int64_t>(indexes.size()));
        }

    virtual void Replace(size_t index, const DataType& val)
        {
        *((DataType*) this->m_data + index) = val;
        this->m_dirty = true;
        }

    virtual void erase(size_t index)
        {
        size_t erasedSize = GetSizeInMemory((DataType*)this->m_data + index);
        memcpy(&this->m_data[index], &this->m_data[index + 1], this->m_nbItems - index - 1 * sizeof(DataType));
        this->m_nbItems -= 1;
        this->m_size -= erasedSize;
        this->m_dirty = true;

		// No memory size modification
        this->NotifySizeChangePoolItem(0, -1);
        }
   
    virtual bool push_back(const DataType& newObject)
        {
        if (this->m_allocatedSize < (this->m_nbItems + 1) * sizeof(DataType))
            {
            reserve((size_t)ceil((this->m_nbItems + 1) * 1.5));
            }

        ((DataType*)this->m_data)[this->m_nbItems] = newObject;
        this->m_nbItems++;
        size_t addedSize = GetSizeInMemory(((DataType*)this->m_data) + this->m_nbItems - 1);
        this->m_size += addedSize;
        this->m_dirty = true;

		// The reserve method did the memory part, for that reason we pass 0 here.
        this->NotifySizeChangePoolItem(0, 1);
        BeAssert(this->m_nbItems * sizeof(DataType) <= this->m_allocatedSize);
        return true;
        }

    virtual bool push_back(const DataType* newObjects, size_t count)
        {
        BeAssert(count != 0);

        if ((this->m_nbItems + count) * sizeof(DataType) > this->m_allocatedSize)
            {
            reserve((size_t)ceil((this->m_nbItems + count) * 1.5));
            }
        size_t addedSize = 0;
        for (size_t i = 0; i < count; ++i)
            addedSize += GetSizeInMemory(newObjects + i);
        memcpy(&((DataType*)this->m_data)[this->m_nbItems], newObjects, count * sizeof(DataType));

        this->m_nbItems += count;
        this->m_size += addedSize;
        this->m_dirty = true;

		// The reserve method did the memory part, for that reason we pass 0 here.
        this->NotifySizeChangePoolItem(0, count);
        BeAssert(this->m_nbItems * sizeof(DataType) <= this->m_allocatedSize);
        return true;
        }
    };

    
template <typename DataType> class SMStoredMemoryPoolGenericVectorItem : public SMMemoryPoolGenericVectorItem<DataType>
    {
    private: 
        
        ISMNodeDataStoreTypePtr<DataType> m_dataStore;
        
    public:        
#ifdef VANCOUVER_API
        static SMStoredMemoryPoolGenericVectorItem<DataType>* CreateItem(uint64_t nodeId, ISMNodeDataStoreTypePtr<DataType>& store, SMStoreDataType dataType, uint64_t smId)
            {
            return new SMStoredMemoryPoolGenericVectorItem<DataType>(nodeId, store, dataType, smId);
            }
#endif
                        
        SMStoredMemoryPoolGenericVectorItem(uint64_t nodeId, ISMNodeDataStoreTypePtr<DataType>& dataStore, SMStoreDataType dataType, uint64_t smId)
            : SMMemoryPoolGenericVectorItem<DataType>(dataStore->GetBlockDataCount(HPMBlockID(nodeId)), nodeId, dataType, smId)
            {                                    
            m_dataStore = dataStore;            
            
            if (this->m_nbItems > 0)
                {                
                HPMBlockID blockID = HPMBlockID(this->m_nodeId);
                size_t nbBytesLoaded = m_dataStore->LoadBlock ((DataType*)this->m_data, this->m_nbItems, blockID);
                this->m_size = nbBytesLoaded;
                }           
            }

        virtual ~SMStoredMemoryPoolGenericVectorItem()
            {
            if (this->m_dirty && this->m_nbItems > 0)
                {
                HPMBlockID blockID = HPMBlockID(this->m_nodeId);
                m_dataStore->StoreBlock((DataType*)this->m_data, this->m_nbItems, blockID);                                
                }
            }    
    };

    template <typename DataType> class SMStoredMemoryPoolVectorItem : public SMMemoryPoolVectorItem<DataType>
        {
        private:
                        
            ISMNodeDataStoreTypePtr<DataType> m_dataStore;

        public:
#ifdef VANCOUVER_API
            static SMStoredMemoryPoolVectorItem<DataType>* CreateItem(uint64_t nodeId, ISMNodeDataStoreTypePtr<DataType>& store, SMStoreDataType dataType, uint64_t smId)
                {
                return new SMStoredMemoryPoolVectorItem<DataType>(nodeId, store, dataType, smId);
                }
#endif
            SMStoredMemoryPoolVectorItem(uint64_t nodeId, ISMNodeDataStoreTypePtr<DataType>& dataStore, SMStoreDataType dataType, uint64_t smId)
                : SMMemoryPoolVectorItem<DataType>(dataStore->GetBlockDataCount(HPMBlockID(nodeId)), nodeId, dataType, smId)
                {                
                m_dataStore = dataStore;

                if (this->m_nbItems > 0)
                    {
                    HPMBlockID blockID(this->m_nodeId);
                    size_t nbBytesLoaded = m_dataStore->LoadBlock((DataType*)this->m_data, this->m_nbItems, blockID);
                    BeAssert(nbBytesLoaded == sizeof(DataType) * this->m_nbItems);
                    }
                }

            virtual ~SMStoredMemoryPoolVectorItem()
                {
                if (this->m_dirty)
                    {
                    HPMBlockID blockID(this->m_nodeId);                    
                    m_dataStore->StoreBlock((DataType*)this->m_data, this->m_nbItems, blockID);                
                    }
                }

            virtual void NotifySizeChangePoolItem(int64_t sizeDelta, int64_t nbItemDelta)
                {               
                HPMBlockID blockID(this->m_nodeId);
                m_dataStore->ModifyBlockDataCount(blockID, nbItemDelta);                

                SMMemoryPoolVectorItem<DataType>::NotifySizeChangePoolItem(sizeDelta, nbItemDelta);
                }
        };



//First impl - dead lock
static clock_t s_timeDiff = CLOCKS_PER_SEC * 120;
static double s_maxMemBeforeFlushing = 1.2;

class SMMemoryPool : public RefCountedBase
    {
    private : 

        static SMMemoryPoolPtr           s_memoryPool; 
        static SMMemoryPoolPtr           s_videoMemoryPool;

        uint64_t                                  m_maxPoolSizeInBytes;
        atomic<uint64_t>                          m_currentPoolSizeInBytes;
        bvector<bvector<SMMemoryPoolItemBasePtr>> m_memPoolItems;
        bvector<bvector<Spinlock*>>               m_memPoolItemMutex;
        bvector<bvector<clock_t>>                 m_lastAccessTime;
        uint64_t                                  m_nbBins;
        mutex                                     m_freeEntryMutex;
        std::queue<SMMemoryPoolItemId>            m_freeEntries;
        SMMemoryPoolItemId                        m_nextEntryAvailable;

        SMMemoryPoolType                          m_id;

        SMMemoryPool(SMMemoryPoolType type);
            
        virtual ~SMMemoryPool();


    public : 

        static BENTLEY_SM_EXPORT SMMemoryPoolItemId s_UndefinedPoolItemId;

        
            
        template<typename T>
        bool GetItem(RefCountedPtr<SMMemoryPoolVectorItem<T>>& poolMemVectorItemPtr, SMMemoryPoolItemId id, uint64_t nodeId, SMStoreDataType dataType, uint64_t smId)
            {
            if (id == SMMemoryPool::s_UndefinedPoolItemId)
                return false; 

            BeAssert(!poolMemVectorItemPtr.IsValid());
            SMMemoryPoolItemBasePtr memItemPtr;
            
            if (GetItem(memItemPtr, id) && memItemPtr->IsCorrect(nodeId, dataType,smId))
                {
                poolMemVectorItemPtr = memItemPtr->GetAsPoolVector<T>();                
                }

            return poolMemVectorItemPtr.IsValid();
            }  

        template<typename T>
        bool GetItem(RefCountedPtr<SMMemoryPoolGenericVectorItem<T>>& poolMemVectorItemPtr, SMMemoryPoolItemId id, uint64_t nodeId, SMStoreDataType dataType, uint64_t smId)
            {
            if (id == SMMemoryPool::s_UndefinedPoolItemId)
                return false;

            BeAssert(!poolMemVectorItemPtr.IsValid());
            SMMemoryPoolItemBasePtr memItemPtr;

            if (GetItem(memItemPtr, id) && memItemPtr->IsCorrect(nodeId, dataType, smId))
                {
                poolMemVectorItemPtr = memItemPtr->GetAsGenericPoolVector<T>();
                }

            return poolMemVectorItemPtr.IsValid();
            }

        template<typename T>
        bool GetItem(RefCountedPtr<SMMemoryPoolBlobItem<T>>& poolMemBlobItemPtr, SMMemoryPoolItemId id, uint64_t nodeId, SMStoreDataType dataType, uint64_t smId)
            {
            if (id == SMMemoryPool::s_UndefinedPoolItemId)
                return false; 

            BeAssert(!poolMemBlobItemPtr.IsValid());
            SMMemoryPoolItemBasePtr memItemPtr;
            
            if (GetItem(memItemPtr, id) && memItemPtr->IsCorrect(nodeId, dataType, smId))
                {
                poolMemBlobItemPtr = memItemPtr->GetAsBlobPoolItem<T>();                
                }

            return poolMemBlobItemPtr.IsValid();
            }  


        template<typename T>
        bool GetItem(RefCountedPtr<SMMemoryPoolGenericBlobItem<T>>& poolMemBlobItemPtr, SMMemoryPoolItemId id, uint64_t nodeId, SMStoreDataType dataType, uint64_t smId)
            {
            if (id == SMMemoryPool::s_UndefinedPoolItemId)
                return false; 

            BeAssert(!poolMemBlobItemPtr.IsValid());
            SMMemoryPoolItemBasePtr memItemPtr;
            
            if (GetItem(memItemPtr, id) && memItemPtr->IsCorrect(nodeId, dataType, smId))
                {
                poolMemBlobItemPtr = memItemPtr->GetAsGenericBlobPoolItem<T>();       

               TRACEPOINT(EventType::POOL_GETITEM, poolMemBlobItemPtr->GetNodeId(), (uint64_t)-1, poolMemBlobItemPtr->GetDataType() == SMStoreDataType::DisplayTexture ? poolMemBlobItemPtr->GetNodeId() : -1, 
                   memItemPtr->GetPoolItemId(), (uint64_t)&poolMemBlobItemPtr, poolMemBlobItemPtr->GetRefCount())

                }

            return poolMemBlobItemPtr.IsValid();
            }          
                               
        bool GetItem(SMMemoryPoolMultiItemsBasePtr& poolMemBlobItemPtr, SMMemoryPoolItemId id, uint64_t nodeId, SMStoreDataType dataType, uint64_t smId)
            {
            if (id == SMMemoryPool::s_UndefinedPoolItemId)
                return false; 

            BeAssert(!poolMemBlobItemPtr.IsValid());
            SMMemoryPoolItemBasePtr memItemPtr;
            
            if (GetItem(memItemPtr, id) && memItemPtr->IsCorrect(nodeId, dataType, smId))
                {
                poolMemBlobItemPtr = dynamic_cast<SMMemoryPoolMultiItemsBase*>(memItemPtr.get());
                BeAssert(poolMemBlobItemPtr.IsValid());
                }

            return poolMemBlobItemPtr.IsValid();
            }  

               
        BENTLEY_SM_EXPORT bool GetItem(SMMemoryPoolItemBasePtr& memItemPtr, SMMemoryPoolItemId id);
            
        BENTLEY_SM_EXPORT bool RemoveItem(SMMemoryPoolItemId id, uint64_t nodeId, SMStoreDataType dataType, uint64_t smId);

        uint64_t RemoveAllItemsOfType(SMStoreDataType dataType, uint64_t smId);
            
        BENTLEY_SM_EXPORT SMMemoryPoolItemId AddItem(SMMemoryPoolItemBasePtr& poolItem);

        BENTLEY_SM_EXPORT void ReplaceItem(SMMemoryPoolItemBasePtr& poolItem, SMMemoryPoolItemId id, uint64_t nodeId, SMStoreDataType dataType, uint64_t smId);

        BENTLEY_SM_EXPORT void NotifySizeChangePoolItem(SMMemoryPoolItemBase* poolItem, int64_t sizeDelta);

        size_t GetCurrentlyUsed()
            {
            return m_currentPoolSizeInBytes;
            }

        void IncreaseCurrentPool(uint64_t memSize)
            {
            m_currentPoolSizeInBytes += memSize;
            }

        void DecreaseCurrentPool(uint64_t memSize)
            {
            if (m_currentPoolSizeInBytes < memSize)
                {
                BeAssert(0);    // Error in the pool
                m_currentPoolSizeInBytes = 0;
                }
            else
                m_currentPoolSizeInBytes -= memSize;
            }

        size_t GetMaxSize()
        {
            return m_maxPoolSizeInBytes;
        }

        bool SetMaxSize(uint64_t maxSize)
            {

            m_maxPoolSizeInBytes = maxSize;

            return true;
            }

        BENTLEY_SM_EXPORT static SMMemoryPoolPtr GetInstance()
            {
            if (s_memoryPool == 0) 
                s_memoryPool = new SMMemoryPool(SMMemoryPoolType::CPU);     
            
            return s_memoryPool;
            }

        BENTLEY_SM_EXPORT static SMMemoryPoolPtr GetInstanceVideo()
        {
            if (s_videoMemoryPool == 0)
                s_videoMemoryPool = new SMMemoryPool(SMMemoryPoolType::GPU);

            return s_videoMemoryPool;
        }

        static void CleanVideoMemoryPool();
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE
