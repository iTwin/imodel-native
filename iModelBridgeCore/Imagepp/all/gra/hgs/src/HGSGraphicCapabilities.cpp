//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgs/src/HGSGraphicCapabilities.cpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
// Class HGSGraphicCapabilities
//---------------------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HGSGraphicCapabilities.h>

//-----------------------------------------------------------------------------
// public
// Default Constructor
//-----------------------------------------------------------------------------
HGSGraphicCapabilities::HGSGraphicCapabilities()
    {
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HGSGraphicCapabilities::HGSGraphicCapabilities(const HGSGraphicCapabilities& pi_rObj)
    {
    // copy the list of Capabilities
    Capabilities::iterator Itr;
    for (Itr = (const_cast<HGSGraphicCapabilities&> (pi_rObj)).m_Capabilities.begin(); Itr != (const_cast<HGSGraphicCapabilities&> (pi_rObj)).m_Capabilities.end(); Itr++)
        Add(*Itr);
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HGSGraphicCapabilities::HGSGraphicCapabilities(const HGSGraphicCapability* pi_pCapability)
    {
    // add the Capability in the Capabilities
    Add(pi_pCapability);
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HGSGraphicCapabilities::~HGSGraphicCapabilities()
    {
    }
