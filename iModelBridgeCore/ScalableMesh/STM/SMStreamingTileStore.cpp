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

bool s_stream_from_disk = false;
bool s_stream_from_file_server = false;
bool s_stream_from_grouped_store = true;
bool s_is_virtual_grouping = true;

template class SMStreamingPointTaggedTileStore<DPoint3d, DRange3d>;
template class SMStreamingPointTaggedTileStore<int32_t, DRange3d>;