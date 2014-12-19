//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCErrorCodeBuilder.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HFCErrorCodeBuilder.h>

//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCErrorCode* HFCErrorCodeBuilder::BuildFromException(const HFCException& pi_rException) const
    {
    HASSERT(false);
    return (0);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCErrorCode* HFCErrorCodeBuilder::BuildFromOSException(uint32_t pi_Exception) const
    {
    HASSERT(false);
    return (0);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCErrorCode* HFCErrorCodeBuilder::BuildFromStatus(HSTATUS pi_Status) const
    {
    HASSERT(false);
    return (0);
    }

//-----------------------------------------------------------------------------
// Public
// Constructor
//-----------------------------------------------------------------------------
HFCErrorCodeBuilder::HFCErrorCodeBuilder()
    {
    }


//-----------------------------------------------------------------------------
// Public
// Constructor
//-----------------------------------------------------------------------------
HFCErrorCodeBuilder::HFCErrorCodeBuilder(const HFCErrorCodeID& pi_rID)
    : m_ModuleID(pi_rID)
    {
    }


//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HFCErrorCodeBuilder::~HFCErrorCodeBuilder()
    {
    }


//-----------------------------------------------------------------------------
// Public
// gets the ID of the module to which this builder is associated
//-----------------------------------------------------------------------------
const HFCErrorCodeID& HFCErrorCodeBuilder::GetModuleID() const
    {
    return (m_ModuleID);
    }


//-----------------------------------------------------------------------------
// Public
// Changes the ID of the module to which this builder is associated
//-----------------------------------------------------------------------------
void HFCErrorCodeBuilder::SetModuleID(const HFCErrorCodeID& pi_rID)
    {
    m_ModuleID = pi_rID;
    }
