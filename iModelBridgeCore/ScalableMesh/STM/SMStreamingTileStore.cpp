//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/SMStreamingTileStore.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ScalableMeshPCH.h>
#include "ImagePPHeaders.h"
#include "SMStreamingTileStore.h"


template class SMStreamingPointTaggedTileStore<DPoint3d, DRange3d>;
template class SMStreamingPointTaggedTileStore<int32_t, DRange3d>;
