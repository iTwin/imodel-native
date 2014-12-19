//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/net/hcs/src/HCSException.cpp $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HCSException.h>

//-----------------------------------------------------------------------------
// Class HCSException
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// public
// Implementation of functions common to all exception classes
//-----------------------------------------------------------------------------
HFC_IMPLEMENT_COMMON_EXCEPTION_FNC(HCSException)

//-----------------------------------------------------------------------------
// public
// Default Constructor
//-----------------------------------------------------------------------------
HCSException::HCSException(const WString& pi_rDeviceName)
    : HFCDeviceException(HCS_EXCEPTION, pi_rDeviceName)
    {
    }


//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HCSException::HCSException(ExceptionID    pi_ExceptionID,
                                  const WString& pi_rDeviceName)
    : HFCDeviceException(pi_ExceptionID, pi_rDeviceName)
    {
    }

//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
HCSException::HCSException(const HCSException& pi_rObj)
    : HFCDeviceException(pi_rObj)
    {
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCSException::~HCSException()
    {
    }

//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------
void HCSException::FormatExceptionMessage(WString& pio_rMessage) const
    {
    if (GetID() != HCS_EXCEPTION)
        {
        HFCDeviceException::FormatExceptionMessage(pio_rMessage);
        }
    }

//-----------------------------------------------------------------------------
// public
// operator=
//-----------------------------------------------------------------------------
HCSException& HCSException::operator=(const HCSException& pi_rObj)
    {
    if (this != &pi_rObj)
        HFCDeviceException::operator=(pi_rObj);

    return *this;
    }


