//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImageppInternal.h>


#include <ImagePP/all/h/HMDLayersPDF.h>


//-----------------------------------------------------------------------------
// Public
// Constructor
//-----------------------------------------------------------------------------
HMDLayersPDF::HMDLayersPDF()
    : HMDLayers()
    {
    }

//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HMDLayersPDF::~HMDLayersPDF()
    {
    }

//-----------------------------------------------------------------------------
// Public
// Get a PDF layer
//-----------------------------------------------------------------------------
const HMDLayerInfoPDF* HMDLayersPDF::GetLayer(uint16_t pi_Index) const
    {
    HPRECONDITION(pi_Index < GetNbLayers());
    HPRECONDITION(dynamic_cast<const HMDLayerInfoPDF*>(HMDLayers::GetLayer(pi_Index)) != 0);

    return (const HMDLayerInfoPDF*)HMDLayers::GetLayer(pi_Index);
    }
