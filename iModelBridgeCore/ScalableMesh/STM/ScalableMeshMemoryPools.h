/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshMemoryPools.h $
|    $RCSfile: ScalableMeshMemoryPools.h,v $
|   $Revision: 1.0 $
|       $Date: 2015/07/14 09:22:17 $
|     $Author: Elenie.Godzaridis $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//#include <ImagePP/all/h/HPMIndirectCountLimitedPool.h>
#include "Edits/DifferenceSet.h"
#include "SMMemoryPool.h"

#ifndef VANCOUVER_API
    #include <Bentley\BeSystemInfo.h>
#endif

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

#ifdef VANCOUVER_API	

inline uint64_t GetAmountOfPhysicalMemory()
    {
    MEMORYSTATUSEX  memoryStatus;
    memoryStatus.dwLength = sizeof(memoryStatus);
    GlobalMemoryStatusEx(&memoryStatus);
    return memoryStatus.ullTotalPhys;
    }

#endif


template <typename POINT> ScalableMeshMemoryPools<POINT>::ScalableMeshMemoryPools()
    {    
	static double S_DEFAULT_PHYSICAL_MEM_USED_RATIO = 0.3;

#ifdef VANCOUVER_API	
	uint64_t totalPhysicalSize = GetAmountOfPhysicalMemory();
#else
    uint64_t totalPhysicalSize = BeSystemInfo::GetAmountOfPhysicalMemory();
#endif
	
	//32 bits usable system memory is limited. 
	if (sizeof(void*) == 32)
		{ 
		totalPhysicalSize = std::min((uint64_t)4000000000, totalPhysicalSize);
		}	

    m_genericPoolSize = (totalPhysicalSize * S_DEFAULT_PHYSICAL_MEM_USED_RATIO);
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