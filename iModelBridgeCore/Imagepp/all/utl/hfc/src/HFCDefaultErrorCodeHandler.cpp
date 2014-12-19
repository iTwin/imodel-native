//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCDefaultErrorCodeHandler.cpp $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HFCDefaultErrorCodeHandler.h>
#include <Imagepp/all/h/HFCErrorCodeException.h>


//-----------------------------------------------------------------------------
// Public
// Constructor
//-----------------------------------------------------------------------------
HFCDefaultErrorCodeHandler::HFCDefaultErrorCodeHandler()
    {
    }


//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HFCDefaultErrorCodeHandler::~HFCDefaultErrorCodeHandler()
    {
    }


//-----------------------------------------------------------------------------
// Public
// Default error code handler.  Throw a generic error code exception if the
// error is fatal.  Otherwise, it returns the H_ERROR status code.
//-----------------------------------------------------------------------------
HSTATUS HFCDefaultErrorCodeHandler::Handle(const HFCErrorCode& pi_rCode) const
    {
    if (pi_rCode.GetFlags().IsFatal())
        throw HFCErrorCodeException(pi_rCode);

    return (H_ERROR);
    }
