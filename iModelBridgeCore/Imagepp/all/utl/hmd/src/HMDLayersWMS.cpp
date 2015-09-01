//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hmd/src/HMDLayersWMS.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HMDLayersWMS.h>

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
const HMDLayerInfoWMS* HMDLayersWMS::GetLayer(unsigned short pi_Index) const
    {
    HPRECONDITION(pi_Index < GetNbLayers());
    HPRECONDITION(HMDLayers::GetLayer(pi_Index)->IsCompatibleWith(HMDLayerInfoWMS::CLASS_ID));

    return (const HMDLayerInfoWMS*)HMDLayers::GetLayer(pi_Index);
    }