/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/Threading/LightThreadPool.h $
|    $RCSfile: LightThreadPool.h,v $
|   $Revision: 1.0 $
|       $Date: 2015/10/29 15:38:13 $
|     $Author: Elenie.Godzaridis $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <ScalableMesh/IScalableMesh.h>
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE


class LightThreadPool
    {
    public: 
    
        int                                        m_nbThreads;
        std::thread*                               m_threads;
        std::atomic<bool>*                         m_areThreadsBusy;
        std::recursive_mutex                       m_nodeMapLock;
        std::map<void*, std::atomic<unsigned int>> m_nodeMap;

        LightThreadPool();

        virtual ~LightThreadPool();
        
        static LightThreadPool* GetInstance();            

    private:

        static LightThreadPool* s_pool;

    };



bool TryReserveNodes(std::map<void*, std::atomic<unsigned int>>& map, void** reservedNodes, size_t nNodesToReserve, unsigned int id);
void SetThreadAvailableAsync(size_t threadId);
void RunOnNextAvailableThread(std::function<void(size_t threadId)> lambda);
void WaitForThreadStop();

END_BENTLEY_SCALABLEMESH_NAMESPACE