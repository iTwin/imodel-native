/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshMemoryPools.h $
|    $RCSfile: ScalableMeshMemoryPools.h,v $
|   $Revision: 1.0 $
|       $Date: 2015/07/14 09:22:17 $
|     $Author: Elenie.Godzaridis $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <ImagePP/all/h/HPMIndirectCountLimitedPool.h>

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE
template <typename POINT> class ScalableMeshMemoryPools
    {
    private:
        size_t m_pointPoolSize;
        size_t m_graphPoolSize;
        size_t m_featurePoolSize;
        HFCPtr<HPMCountLimitedPool<POINT>> m_pointPool;
        HFCPtr<HPMIndirectCountLimitedPool<MTGGraph>> m_graphPool;
        HPMMemoryMgrReuseAlreadyAllocatedBlocksWithAlignment * m_myMemMgr;
        HFCPtr<HPMCountLimitedPool<int32_t>> m_featurePool;
        ScalableMeshMemoryPools();
        static ScalableMeshMemoryPools* m_instance;

    public:
        static ScalableMeshMemoryPools* Get();
        HFCPtr<HPMCountLimitedPool<POINT>> GetPointPool();
        HFCPtr<HPMIndirectCountLimitedPool<MTGGraph>> GetGraphPool();
        HFCPtr<HPMCountLimitedPool<int32_t>> GetFeaturePool();
    };

template <typename POINT> ScalableMeshMemoryPools<POINT>::ScalableMeshMemoryPools()
    {
    m_myMemMgr = new HPMMemoryMgrReuseAlreadyAllocatedBlocksWithAlignment(100, 2000 * sizeof(POINT));
    m_pointPoolSize = 20000000;
    m_featurePoolSize = 10000000;
    m_graphPoolSize = 600000000;
    m_pointPool = new HPMCountLimitedPool<POINT>(m_myMemMgr, m_pointPoolSize);
    m_graphPool = new HPMIndirectCountLimitedPool<MTGGraph>(m_graphPoolSize);
    m_featurePool = new HPMCountLimitedPool<int32_t>(m_featurePoolSize);
    }

template <typename POINT> ScalableMeshMemoryPools<POINT>*  ScalableMeshMemoryPools<POINT>::Get()
    {
    if (m_instance == nullptr)
        {
        m_instance = new ScalableMeshMemoryPools();
        }
    return m_instance;
    }

template <typename POINT> HFCPtr<HPMCountLimitedPool<POINT>>  ScalableMeshMemoryPools<POINT>::GetPointPool()
    {
    return m_pointPool;
    }

template <typename POINT> HFCPtr<HPMIndirectCountLimitedPool<MTGGraph>>  ScalableMeshMemoryPools<POINT>::GetGraphPool()
    {
    return m_graphPool;
    }

template <typename POINT> HFCPtr<HPMCountLimitedPool<int32_t>>   ScalableMeshMemoryPools<POINT>::GetFeaturePool()
    {
    return m_featurePool;
    }

template <typename POINT> ScalableMeshMemoryPools<POINT>* ScalableMeshMemoryPools<POINT>::m_instance = nullptr;

END_BENTLEY_SCALABLEMESH_NAMESPACE