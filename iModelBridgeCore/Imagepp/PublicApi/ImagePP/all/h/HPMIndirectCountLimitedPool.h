#pragma once

#include "HPMCountLimitedPool.h"
#include <Mtg/MtgStructs.h>

BEGIN_IMAGEPP_NAMESPACE

template <typename DataType> class HPMIndirectCountLimitedPoolItem;

template <typename DataType> class HPMIndirectCountLimitedPool : public HPMCountLimitedPool < DataType >
    {
    public:
        /*
        * Here countLimit is the size (in bytes) of all the objects in the pool.
        */
        HPMIndirectCountLimitedPool(size_t countLimit) : HPMCountLimitedPool(countLimit)
            {
         
            }

        HPMIndirectCountLimitedPool(IHPMMemoryManager* memoryManager, size_t countLimit) : HPMCountLimitedPool(memoryManager, countLimit)
            {
            
            }

        virtual ~HPMIndirectCountLimitedPool()
            {
            RecomputeTotalCount();
            HASSERT(m_totalUsedCount == 0);
            }

        virtual bool Allocate(size_t Count, const HPMCountLimitedPoolItem<DataType>* poolItem)
            {
            RecomputeTotalCount();
            bool retval = HPMCountLimitedPool<DataType>::Allocate(Count, poolItem);
            RecomputeTotalCount();
            return retval;
            }

        virtual bool Reallocate(size_t NewCount, const HPMCountLimitedPoolItem<DataType>* poolItem)
            {
            NotifyAccess(poolItem);
            const HPMIndirectCountLimitedPoolItem<DataType>* pPoolItem = dynamic_cast<const HPMIndirectCountLimitedPoolItem<DataType>*>(poolItem);
            // Check if memory limit attained
            while (m_totalUsedCount + NewCount*sizeof(DataType) - pPoolItem->GetDeepCount() > m_countLimit)
                {
                if (m_Pool.empty() || !m_Pool.back()->Discard())
                    return false; 
                }

            DataType* newMemory;
            size_t actualAllocatedCount = NewCount;
            size_t actualAllocatedMemory = 0;

            if (m_memoryManager != NULL)
                {
                newMemory = reinterpret_cast<DataType*>(m_memoryManager->AllocMemoryExt(NewCount * sizeof(DataType), actualAllocatedMemory));
                actualAllocatedCount = actualAllocatedMemory / sizeof(DataType); // Trunction of remainder is intended
                HASSERT(actualAllocatedCount >= NewCount);
                }
            else
                newMemory = new DataType[NewCount];

            //memcpy(newMemory, poolItem->GetMemory(), min(NewCount, poolItem->GetMemoryCount()) *sizeof(DataType));
            pPoolItem->DeepCopy(newMemory, NewCount);
            m_totalUsedCount -= pPoolItem->GetDeepCount();

            if (m_memoryManager != NULL)
                m_memoryManager->FreeMemory(reinterpret_cast<Byte*>(poolItem->GetMemory()), poolItem->GetMemoryCount() * sizeof(DataType));
            else
                delete[] poolItem->GetMemory();

            poolItem->SetMemory(newMemory, actualAllocatedCount);
            m_totalUsedCount += pPoolItem->GetDeepCount();
            return true;
            }

        virtual bool Free(const HPMCountLimitedPoolItem<DataType>* poolItem)
            {
            if (poolItem->GetPoolManaged())
                {
                // Critical section
                m_totalUsedCount -= dynamic_cast<const HPMIndirectCountLimitedPoolItem<DataType>*>(poolItem)->GetDeepCount();
                if (m_memoryManager != NULL)
                    {
                    //manually call destructor since this is not a delete call...
                    poolItem->GetMemory()->~DataType();
                    m_memoryManager->FreeMemory(reinterpret_cast<Byte*>(poolItem->GetMemory()), poolItem->GetMemoryCount() * sizeof(DataType));
                    }
                else
                    {
                    delete[] poolItem->GetMemory();
                    }
                poolItem->SetMemory(NULL, 0);
                m_Pool.erase(poolItem->GetPoolIterator());
                poolItem->m_poolIteratorPtr = NULL;
                poolItem->SetPoolManaged(false);
                // end critical section
                }

            return true;
            }

        virtual bool RemoveItem(const HPMCountLimitedPoolItem<DataType>* poolItem)
            {
            if (poolItem->GetPoolManaged())
                {
                // Critical section
                m_totalUsedCount -= dynamic_cast<const HPMIndirectCountLimitedPoolItem<DataType>*>(poolItem)->GetDeepCount();

                m_Pool.erase(poolItem->GetPoolIterator());
                poolItem->SetPoolManaged(false);
                // end critical section
                }

            return true;
            }
    private:
        void UpdateCountForItem(typename const std::list<const HPMIndirectCountLimitedPoolItem<DataType>* >::iterator poolItem)
            {
            m_totalUsedCount -= poolItem->GetDeepCount();
            poolItem->RecomputeCount();
            m_totalUsedCount += poolItem->GetDeepCount();
            }
        void RecomputeTotalCount()
            {
            m_totalUsedCount = 0;
            auto poolIterator = m_Pool.begin();
            for (; poolIterator != m_Pool.end(); ++poolIterator)
                {
                if (!(*poolIterator)->Discarded()) m_totalUsedCount += (static_cast<const HPMIndirectCountLimitedPoolItem<DataType>*>(*poolIterator))->GetDeepCount();
                }
            }
    };

    template <typename DataType> class HPMIndirectCountLimitedPoolItem : public HPMCountLimitedPoolItem<DataType>
    {


    public:

        HPMIndirectCountLimitedPoolItem() : HPMCountLimitedPoolItem<DataType>()
            {
            m_deepCount = 0;
            }

        HPMIndirectCountLimitedPoolItem(HFCPtr<HPMIndirectCountLimitedPool<DataType> >  pool) : HPMCountLimitedPoolItem<DataType>(&*pool)
            {
            m_deepCount = 0;
            }

        virtual ~HPMIndirectCountLimitedPoolItem()
            {
            /*if (Pinned())
                {
                UnPin();
                }
            if (m_poolIteratorPtr != NULL)
                delete m_poolIteratorPtr;
            if (m_poolManaged)
                m_pool->Free(this);*/
            }

         size_t GetDeepCount() const
            {
            if (m_deepCount == 0 && m_allocatedCount != 0) RecomputeCount();
            return m_deepCount;
            }

         /*
         * Target array memory needs to be allocated by caller.
         */
         void DeepCopy(DataType* targetArray, size_t count) const
             {
             for (size_t i = 0; i < count && i < m_allocatedCount; i++)
                 targetArray[i] = m_memory[i];
             }

         void RecomputeCount() const
             {
             HASSERT(false && "Need template specialization of RecomputeCount for non-POD types");
             }

        virtual bool Discard() const = 0; // Intentionaly const ... only mutable members are modified
        virtual bool Inflate() const = 0; // Intentionaly const ... only mutable members are modified

    protected:

    private:
        // Allow access to the pool of private methods below.
        friend HPMIndirectCountLimitedPool < DataType > ;
        mutable size_t m_deepCount;
    };

inline void HPMIndirectCountLimitedPoolItem<MTGGraph>::RecomputeCount() const
    {
    m_deepCount = 0;
    for (size_t i = 0; i < m_count; i++)
        {
        MTGGraph* g = m_memory + i;
        m_deepCount += sizeof(*g);
        m_deepCount += (sizeof(MTGLabelMask)+2*sizeof(int))*g->GetLabelCount();
        m_deepCount += sizeof(MTG_Node)*g->GetNodeIdCount();
        m_deepCount += sizeof(int)*g->GetNodeIdCount()* g->GetLabelCount();
        }
    }
END_IMAGEPP_NAMESPACE