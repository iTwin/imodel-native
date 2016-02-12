/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/Threading/LightThreadPool.h $
|    $RCSfile: LightThreadPool.h,v $
|   $Revision: 1.0 $
|       $Date: 2015/10/29 15:38:13 $
|     $Author: Elenie.Godzaridis $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <ScalableMesh/IScalableMesh.h>
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE


extern std::thread s_threads[8];
extern std::atomic<bool> s_areThreadsBusy[8];
extern std::recursive_mutex s_nodeMapLock;
extern std::map<void*, std::atomic<unsigned int>> s_nodeMap;
bool TryReserveNodes(std::map<void*, std::atomic<unsigned int>>& map, void** reservedNodes, size_t nNodesToReserve, unsigned int id);
void SetThreadAvailableAsync(size_t threadId);
void RunOnNextAvailableThread(std::function<void(size_t threadId)> lambda);
void WaitForThreadStop();

END_BENTLEY_SCALABLEMESH_NAMESPACE