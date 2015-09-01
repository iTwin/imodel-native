//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hmd/src/HMDLayerInfoPDF.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HMDLayerInfoPDF.h>

//-----------------------------------------------------------------------------
// Public
// Constructor
//-----------------------------------------------------------------------------
HMDLayerInfoPDF::HMDLayerInfoPDF(const WString& pi_rKeyName,
                                        bool    pi_InitialVisibleState,
                                        const WString& pi_rLayerName)
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
const WString& HMDLayerInfoPDF::GetLayerName() const
    {
    return m_LayerName;
    }