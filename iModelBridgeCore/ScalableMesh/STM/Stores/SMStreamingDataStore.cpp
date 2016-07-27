//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/Stores/SMStreamingDataStore.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ScalableMeshPCH.h>


#include "SMStreamingDataStore.h"
#include "SMStreamingDataStore.hpp"

template class SMStreamingStore<DRange3d>;

template class SMStreamingNodeDataStore<DPoint3d, DRange3d>;
