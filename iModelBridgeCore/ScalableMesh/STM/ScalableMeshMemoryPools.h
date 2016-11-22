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
//#include <ImagePP/all/h/HPMIndirectCountLimitedPool.h>
#include "Edits/DifferenceSet.h"
#include "SMMemoryPool.h"

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE
template <typename POINT> class ScalableMeshMemoryPools
    {
    private:        
                        
        size_t                  m_genericPoolSize;                
        SMMemoryPoolPtr         m_genericPool;
        size_t                  m_videoPoolSize;
        SMMemoryPoolPtr         m_videoPool;

        ScalableMeshMemoryPools();
        static ScalableMeshMemoryPools* m_instance;

    public:
        static ScalableMeshMemoryPools* Get();                        
        SMMemoryPoolPtr& GetGenericPool();
        SMMemoryPoolPtr& GetVideoPool();
    };

template <typename POINT> ScalableMeshMemoryPools<POINT>::ScalableMeshMemoryPools()
    {    
    m_genericPoolSize = 2000000000;                 
    m_genericPool = SMMemoryPool::GetInstance();
    bool result = m_genericPool->SetMaxSize(m_genericPoolSize);
    assert(result == true);

    m_videoPoolSize = 500000000; 
    m_videoPool = SMMemoryPool::GetInstanceVideo();
    m_videoPool->SetMaxSize(m_videoPoolSize);
    }

template <typename POINT> ScalableMeshMemoryPools<POINT>*  ScalableMeshMemoryPools<POINT>::Get()
    {
    if (m_instance == nullptr)
        {
        m_instance = new ScalableMeshMemoryPools();
        }
    return m_instance;
    }

template <typename POINT> SMMemoryPoolPtr& ScalableMeshMemoryPools<POINT>::GetGenericPool()
    {
    return m_genericPool;
    }

template <typename POINT> SMMemoryPoolPtr& ScalableMeshMemoryPools<POINT>::GetVideoPool()
    {
    return m_videoPool;
    }

template <typename POINT> ScalableMeshMemoryPools<POINT>* ScalableMeshMemoryPools<POINT>::m_instance = nullptr;

END_BENTLEY_SCALABLEMESH_NAMESPACE