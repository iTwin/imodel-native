//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hmd/src/HMDLayerInfoPDF.cpp $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImageppInternal.h>


#include <ImagePP/all/h/HMDLayerInfoPDF.h>

//-----------------------------------------------------------------------------
// Public
// Constructor
//-----------------------------------------------------------------------------
HMDLayerInfoPDF::HMDLayerInfoPDF(const Utf8String& pi_rKeyName,
                                        bool    pi_InitialVisibleState,
                                        const Utf8String& pi_rLayerName)
    : HMDLayerInfo(pi_rKeyName, pi_InitialVisibleState)
    {
    m_LayerName = pi_rLayerName;
    }

//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HMDLayerInfoPDF::~HMDLayerInfoPDF()
    {
    }

//-----------------------------------------------------------------------------
// Public
// Get the layer's name
//-----------------------------------------------------------------------------
const Utf8String& HMDLayerInfoPDF::GetLayerName() const
    {
    return m_LayerName;
    }
