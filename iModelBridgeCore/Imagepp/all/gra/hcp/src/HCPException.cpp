//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hcp/src/HCPException.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HCPException.h>

//-----------------------------------------------------------------------------
// Class HCPException
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// public
// Implementation of functions common to all exception classes
//-----------------------------------------------------------------------------
HFC_IMPLEMENT_COMMON_EXCEPTION_FNC(HCPException)
HFC_IMPLEMENT_COMMON_EXCEPTION_FNC(HCPGCoordException)

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HCPException::HCPException(ExceptionID pi_ExceptionID)
    : HFCException(pi_ExceptionID)
    {
    HPRECONDITION((pi_ExceptionID >= HCP_BASE) && (pi_ExceptionID < HCP_SEPARATOR_ID));
    }

//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
HCPException::HCPException(const HCPException& pi_rObj)
    : HFCException(pi_rObj)
    {
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCPException::~HCPException()
    {
    }

//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------
void HCPException::FormatExceptionMessage(WString& pio_rMessage) const
    {
    }

//-----------------------------------------------------------------------------
// public
// operator=
//-----------------------------------------------------------------------------
HCPException& HCPException::operator=(const HCPException& pi_rObj)
    {
    if (this != &pi_rObj)
        HFCException::operator=(pi_rObj);

    return *this;
    }


//-----------------------------------------------------------------------------
// Class HCPGCoordException
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HCPGCoordException::HCPGCoordException()
    : HCPException(HCP_GCOORD_EXCEPTION)
    {
    m_pInfo = new HCPGCoordExInfo;
    ((HCPGCoordExInfo*)m_pInfo)->m_Code = 0; //0 equals to SUCCESS defined in
    //Wrapper_GCoord.h
    GENERATE_FORMATTED_EXCEPTION_MSG()
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HCPGCoordException::HCPGCoordException(int32_t pi_Code)
    : HCPException(HCP_GCOORD_EXCEPTION)
    {
    m_pInfo = new HCPGCoordExInfo;
    ((HCPGCoordExInfo*)m_pInfo)->m_Code = pi_Code;

    GENERATE_FORMATTED_EXCEPTION_MSG()
    }


//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
HCPGCoordException::HCPGCoordException(const HCPGCoordException& pi_rObj)
    : HCPException(pi_rObj)
    {
    COPY_EXCEPTION_INFO(m_pInfo, pi_rObj.m_pInfo, HCPGCoordExInfo)

    this->operator=(pi_rObj);
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCPGCoordException::~HCPGCoordException()
    {
    }

//-----------------------------------------------------------------------------
// public
// Return the error code.
//-----------------------------------------------------------------------------
int32_t HCPGCoordException::GetErrorCode()
    {
    HPRECONDITION(m_pInfo != 0);

    return ((HCPGCoordExInfo*)m_pInfo)->m_Code;
    }

//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------
void HCPGCoordException::FormatExceptionMessage(WString& pio_rMessage) const
    {
    HPRECONDITION(m_pInfo != 0);
    FORMAT_EXCEPTION_MSG(pio_rMessage, ((HCPGCoordExInfo*)m_pInfo)->m_Code)
    }


//-----------------------------------------------------------------------------
// public
// operator=
//-----------------------------------------------------------------------------
HCPGCoordException& HCPGCoordException::operator=(const HCPGCoordException& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        HCPGCoordException::operator=(pi_rObj);
        COPY_EXCEPTION_INFO(m_pInfo, pi_rObj.m_pInfo, HCPGCoordExInfo)
        }

    return *this;
    }