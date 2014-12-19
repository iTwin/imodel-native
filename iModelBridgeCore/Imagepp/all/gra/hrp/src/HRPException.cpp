//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPException.cpp $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// methods for HRP exception
//-----------------------------------------------------------------------------
#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HRPException.h>

//-----------------------------------------------------------------------------
// public
// Implementation of functions common to all exception classes
//-----------------------------------------------------------------------------
HFC_IMPLEMENT_COMMON_EXCEPTION_FNC(HRPException)


//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HRPException::HRPException()
    : HFCException(HRP_EXCEPTION)
    {
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HRPException::HRPException(ExceptionID pi_ExceptionID)
    : HFCException(pi_ExceptionID)
    {
    HPRECONDITION((pi_ExceptionID >= HRP_BASE) && (pi_ExceptionID < HRP_SEPARATOR_ID));
    }

//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
HRPException::HRPException(const HRPException& pi_rObj)
    : HFCException(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRPException::~HRPException()
    {
    }

//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------
void HRPException::FormatExceptionMessage(WString& pio_rMessage) const
    {
    }

//-----------------------------------------------------------------------------
// public
// operator=
//-----------------------------------------------------------------------------
HRPException& HRPException::operator=(const HRPException& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        HFCException::operator=(pi_rObj);
        }

    return *this;
    }