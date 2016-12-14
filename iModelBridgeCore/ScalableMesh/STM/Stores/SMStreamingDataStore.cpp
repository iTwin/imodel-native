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
bool s_stream_from_wsg = true;
bool s_stream_from_azure_using_curl = false;
bool s_stream_using_cesium_3d_tiles_format = true;
bool s_stream_using_curl = false;
bool s_stream_enable_caching = false;
bool s_is_virtual_grouping = false;
bool s_is_legacy_dataset = false;
bool s_is_legacy_master_header = true;
bool s_use_azure_sandbox = true;
bool s_use_qa_azure = false;


template class SMStreamingStore<DRange3d>;

template class SMStreamingNodeDataStore<DPoint3d, DRange3d>;
