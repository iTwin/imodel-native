//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hcd/src/HCDException.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// methods for HCD exception
//-----------------------------------------------------------------------------
#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HCDException.h>

HFC_IMPLEMENT_COMMON_EXCEPTION_FNC(HCDException)
HFC_IMPLEMENT_COMMON_EXCEPTION_FNC(HCDIJLErrorException)

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HCDException::HCDException()
    : HFCException(HCD_EXCEPTION)
    {
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HCDException::HCDException(ExceptionID        pi_ExceptionID)
    : HFCException(pi_ExceptionID)
    {
    HPRECONDITION((pi_ExceptionID >= HCD_BASE) && (pi_ExceptionID < HCD_SEPARATOR_ID));
    }


//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
HCDException::HCDException(const HCDException& pi_rObj)
    : HFCException(pi_rObj)
    {
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCDException::~HCDException()
    {
    }

//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------
void HCDException::FormatExceptionMessage(WString& pio_rMessage) const
    {
    }

//-----------------------------------------------------------------------------
// public
// operator=
//-----------------------------------------------------------------------------
HCDException& HCDException::operator=(const HCDException& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        HFCException::operator=(pi_rObj);
        }

    return *this;
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HCDIJLErrorException::HCDIJLErrorException(short pi_IJLErrorCode)
    : HCDException(HCD_IJL_ERROR_EXCEPTION)
    {
    m_pInfo = new HCDIJLErrorExInfo;
    ((HCDIJLErrorExInfo*)m_pInfo)->m_IJLErrorCode = pi_IJLErrorCode;
    }

//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
HCDIJLErrorException::HCDIJLErrorException(const HCDIJLErrorException& pi_rObj)
    : HCDException(pi_rObj)
    {
    COPY_EXCEPTION_INFO(m_pInfo, pi_rObj.m_pInfo, HCDIJLErrorExInfo)
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCDIJLErrorException::~HCDIJLErrorException()
    {
    }

//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------

void HCDIJLErrorException::FormatExceptionMessage(WString& pio_rMessage) const
    {
    }

//-----------------------------------------------------------------------------
// public
// operator=
//-----------------------------------------------------------------------------
HCDIJLErrorException& HCDIJLErrorException::operator=(const HCDIJLErrorException& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        HCDException::operator=(pi_rObj);
        COPY_EXCEPTION_INFO(m_pInfo, pi_rObj.m_pInfo, HCDIJLErrorExInfo)
        }

    return *this;
    }