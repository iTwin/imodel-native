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

bool s_stream_from_disk = false;
bool s_stream_from_file_server = false;
bool s_stream_from_grouped_store = true;
bool s_stream_enable_caching = false;
bool s_is_virtual_grouping = true;


template class SMStreamingStore<DRange3d>;

template class SMStreamingNodeDataStore<DPoint3d, DRange3d>;
