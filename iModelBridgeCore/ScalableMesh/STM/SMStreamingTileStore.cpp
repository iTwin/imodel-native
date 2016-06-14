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
bool s_stream_from_grouped_store = false;
bool s_is_virtual_grouping = true;
uint32_t s_max_number_nodes_in_group = 100;
size_t s_max_group_size = 256 << 10; // 256 KB
size_t s_max_group_depth = 5;
size_t s_max_group_common_ancestor = 2;

template class SMStreamingPointTaggedTileStore<DPoint3d, DRange3d>;
template class SMStreamingPointTaggedTileStore<int32_t, DRange3d>;