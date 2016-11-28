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
#include <Bentley\BeSystemInfo.h>

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE
template <typename POINT> class ScalableMeshMemoryPools
    {
    private:        
                        
        size_t                  m_genericPoolSize;                
        SMMemoryPoolPtr         m_genericPool;

        ScalableMeshMemoryPools();
        static ScalableMeshMemoryPools* m_instance;

    public:
        static ScalableMeshMemoryPools* Get();                        
        SMMemoryPoolPtr& GetGenericPool();
    };

template <typename POINT> ScalableMeshMemoryPools<POINT>::ScalableMeshMemoryPools()
    {    
	static double S_DEFAULT_PHYSICAL_MEM_USED_RATIO = 0.3;
	
	uint64_t totalPhysicalSize = BeSystemInfo::GetAmountOfPhysicalMemory();
	
	//32 bits usable system memory is limited. 
	if (sizeof(void*) == 32)
		{ 
		totalPhysicalSize = std::min((uint64_t)4000000000, totalPhysicalSize);
		}	

    m_genericPoolSize = (totalPhysicalSize * S_DEFAULT_PHYSICAL_MEM_USED_RATIO);
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

template <typename POINT> SMMemoryPoolPtr& ScalableMeshMemoryPools<POINT>::GetGenericPool()
    {
    return m_genericPool;
    }

template <typename POINT> ScalableMeshMemoryPools<POINT>* ScalableMeshMemoryPools<POINT>::m_instance = nullptr;

END_BENTLEY_SCALABLEMESH_NAMESPACE