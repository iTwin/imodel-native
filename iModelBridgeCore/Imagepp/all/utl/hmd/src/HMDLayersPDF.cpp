//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hmd/src/HMDLayersPDF.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HMDLayersPDF.h>


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
const HMDLayerInfoPDF* HMDLayersPDF::GetLayer(unsigned short pi_Index) const
    {
    HPRECONDITION(pi_Index < GetNbLayers());
    HPRECONDITION(dynamic_cast<const HMDLayerInfoPDF*>(HMDLayers::GetLayer(pi_Index)) != 0);

    return (const HMDLayerInfoPDF*)HMDLayers::GetLayer(pi_Index);
    }