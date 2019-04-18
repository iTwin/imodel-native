//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImageppInternal.h>


#include <ImagePP/all/h/HMDLayersWMS.h>

//-----------------------------------------------------------------------------
// Public
// Constructor
//-----------------------------------------------------------------------------
HMDLayersWMS::HMDLayersWMS()
    : HMDLayers()
    {
    }

//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HMDLayersWMS::~HMDLayersWMS()
    {
    }

//-----------------------------------------------------------------------------
// Public
// Get a WMS layer
//-----------------------------------------------------------------------------
const HMDLayerInfoWMS* HMDLayersWMS::GetLayer(uint16_t pi_Index) const
    {
    HPRECONDITION(pi_Index < GetNbLayers());
    HPRECONDITION(HMDLayers::GetLayer(pi_Index)->IsCompatibleWith(HMDLayerInfoWMS::CLASS_ID));

    return (const HMDLayerInfoWMS*)HMDLayers::GetLayer(pi_Index);
    }
