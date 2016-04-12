//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/SMMemoryPool.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include <ScalableMeshPCH.h>

#include <limits>   
#include "SMMemoryPool.h"


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE
    
SMMemoryPoolItemId SMMemoryPool::s_UndefinedPoolItemId = std::numeric_limits<SMMemoryPoolItemId>::max();

END_BENTLEY_SCALABLEMESH_NAMESPACE