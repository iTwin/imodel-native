/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshMemoryPools.h $
|    $RCSfile: ScalableMeshMemoryPools.h,v $
|   $Revision: 1.0 $
|       $Date: 2015/07/14 09:22:17 $
|     $Author: Elenie.Godzaridis $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <ImagePP/all/h/HPMIndirectCountLimitedPool.h>
#include "Edits/DifferenceSet.h"
#include "SMMemoryPool.h"

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE
template <typename POINT> class ScalableMeshMemoryPools
    {
    private:        
        size_t m_graphPoolSize;                
        size_t m_featurePoolSize;
        size_t m_diffSetPoolSize;
        size_t m_genericPoolSize;
        HFCPtr<HPMCountLimitedPool<POINT>> m_pointPool;
        HFCPtr<HPMIndirectCountLimitedPool<MTGGraph>> m_graphPool;                        
        HPMMemoryMgrReuseAlreadyAllocatedBlocksWithAlignment * m_myMemMgr;
        HFCPtr<HPMCountLimitedPool<int32_t>> m_featurePool;
        HFCPtr<HPMIndirectCountLimitedPool<DifferenceSet>> m_diffSetPool;
        SMMemoryPoolPtr                                    m_genericPool;

        ScalableMeshMemoryPools();
        static ScalableMeshMemoryPools* m_instance;

    public:
        static ScalableMeshMemoryPools* Get();        
        HFCPtr<HPMIndirectCountLimitedPool<MTGGraph>> GetGraphPool();                
        HFCPtr<HPMCountLimitedPool<int32_t>> GetFeaturePool();
        HFCPtr<HPMIndirectCountLimitedPool<DifferenceSet>> GetDiffSetPool();
        SMMemoryPoolPtr& GetGenericPool();
    };

template <typename POINT> ScalableMeshMemoryPools<POINT>::ScalableMeshMemoryPools()
    {
    m_myMemMgr = new HPMMemoryMgrReuseAlreadyAllocatedBlocksWithAlignment(100, 2000 * sizeof(POINT));    
    m_featurePoolSize = 10000000;    
    m_diffSetPoolSize = 4000000;
    m_graphPoolSize = 600000000;        
    //m_genericPoolSize = 100000000;
    m_genericPoolSize = 1000000000;    
  //  m_graphPool = new HPMIndirectCountLimitedPool<MTGGraph>(m_graphPoolSize);        
    m_featurePool = new HPMCountLimitedPool<int32_t>(m_featurePoolSize);
   // m_diffSetPool = new HPMIndirectCountLimitedPool<DifferenceSet>(m_diffSetPoolSize);
    m_genericPool = SMMemoryPool::GetInstance();
    bool result = m_genericPool->SetMaxSize(m_genericPoolSize);
    assert(result == true);
    }

template <typename POINT> ScalableMeshMemoryPools<POINT>*  ScalableMeshMemoryPools<POINT>::Get()
    {
    if (m_instance == nullptr)
        {
        m_instance = new ScalableMeshMemoryPools();
        }
    return m_instance;
    }

template <typename POINT> HFCPtr<HPMIndirectCountLimitedPool<MTGGraph>>  ScalableMeshMemoryPools<POINT>::GetGraphPool()
    {
    return m_graphPool;
    }

template <typename POINT> HFCPtr<HPMCountLimitedPool<int32_t>>   ScalableMeshMemoryPools<POINT>::GetFeaturePool()
    {
    return m_featurePool;
    }

template <typename POINT> HFCPtr<HPMIndirectCountLimitedPool<DifferenceSet>>   ScalableMeshMemoryPools<POINT>::GetDiffSetPool()
    {
    return m_diffSetPool;
    }

template <typename POINT> SMMemoryPoolPtr& ScalableMeshMemoryPools<POINT>::GetGenericPool()
    {
    return m_genericPool;
    }


template <typename POINT> ScalableMeshMemoryPools<POINT>* ScalableMeshMemoryPools<POINT>::m_instance = nullptr;

END_BENTLEY_SCALABLEMESH_NAMESPACE