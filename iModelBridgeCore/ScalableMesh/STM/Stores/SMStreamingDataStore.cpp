//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/Stores/SMStreamingDataStore.cpp $
//:>
//:>  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ScalableMeshPCH.h>


#include "SMStreamingDataStore.h"
#include "SMStreamingDataStore.hpp"

bool s_stream_from_wsg = false;
bool s_stream_using_curl = true;
bool s_stream_enable_caching = true;
bool s_stream_from_grouped_store = true;
bool s_stream_using_cesium_3d_tiles_format = true;
bool s_import_from_bim_exported_cesium_3d_tiles = false;
bool s_is_virtual_grouping = false;
bool s_use_qa_azure = false;
   

template class SMStreamingStore<DRange3d>;

template class SMStreamingNodeDataStore<DPoint3d, DRange3d>;
