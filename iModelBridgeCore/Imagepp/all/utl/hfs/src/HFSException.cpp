//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfs/src/HFSException.cpp $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HFSException.h>

//:>-----------------------------------------------------------------------------
//:> methods for HFS exception classes.
//:>-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// public
// Implementation of functions common to all exception classes
//-----------------------------------------------------------------------------
HFC_IMPLEMENT_COMMON_EXCEPTION_FNC(HFSException)
HFC_IMPLEMENT_COMMON_EXCEPTION_FNC(HFSInvalidPathException)
HFC_IMPLEMENT_COMMON_EXCEPTION_FNC(HFSHIBPInvalidResponseException)
HFC_IMPLEMENT_COMMON_EXCEPTION_FNC(HFSHIBPErrorException)

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HFSException::HFSException()
    : HFCException(HFS_EXCEPTION)
    {
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HFSException::HFSException(ExceptionID    pi_ExceptionID)
    : HFCException(pi_ExceptionID)
    {
    HPRECONDITION((pi_ExceptionID >= HFS_BASE) && (pi_ExceptionID < HFS_SEPARATOR_ID));
    }

//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
HFSException::HFSException(const HFSException& pi_rObj)
    : HFCException(pi_rObj)
    {

    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HFSException::~HFSException()
    {
    }

//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------
void HFSException::FormatExceptionMessage(WString& pio_rMessage) const
    {
    }

//-----------------------------------------------------------------------------
// public
// operator=
//-----------------------------------------------------------------------------
HFSException& HFSException::operator=(const HFSException& pi_rObj)
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
HFSInvalidPathException::HFSInvalidPathException(const WString& pi_rPath)
    : HFSException(HFS_INVALID_PATH_EXCEPTION)
    {
    m_pInfo = new HFSInvalidPathExInfo();
    ((HFSInvalidPathExInfo*)m_pInfo)->m_Path = pi_rPath;

    GENERATE_FORMATTED_EXCEPTION_MSG()
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HFSInvalidPathException::HFSInvalidPathException(ExceptionID     pi_ExceptionID,
                                                        const WString&  pi_rPath)
    : HFSException(pi_ExceptionID)
    {
    m_pInfo = new HFSInvalidPathExInfo();
    ((HFSInvalidPathExInfo*)m_pInfo)->m_Path = pi_rPath;

    GENERATE_FORMATTED_EXCEPTION_MSG()
    }

//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
HFSInvalidPathException::HFSInvalidPathException(const HFSInvalidPathException& pi_rObj)
    : HFSException(pi_rObj)
    {
    COPY_EXCEPTION_INFO(m_pInfo, pi_rObj.m_pInfo, HFSInvalidPathExInfo)
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HFSInvalidPathException::~HFSInvalidPathException()
    {
    }

//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------
void HFSInvalidPathException::FormatExceptionMessage(WString& pio_rMessage) const
    {
    HPRECONDITION(m_pInfo != 0);
    FORMAT_EXCEPTION_MSG(pio_rMessage, ((HFSInvalidPathExInfo*)m_pInfo)->m_Path.c_str())
    }

//-----------------------------------------------------------------------------
// public
// operator=
//-----------------------------------------------------------------------------
HFSInvalidPathException& HFSInvalidPathException::operator=(const HFSInvalidPathException& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        HFCException::operator=(pi_rObj);
        COPY_EXCEPTION_INFO(m_pInfo, pi_rObj.m_pInfo, HFSInvalidPathExInfo)
        }

    return *this;
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HFSHIBPInvalidResponseException::HFSHIBPInvalidResponseException(const WString&    pi_rRequest)
    : HFSException(HFS_IBP_INVALID_RESPONSE_EXCEPTION)
    {
    m_pInfo = new HFSHIBPInvalidResponseExInfo();
    ((HFSHIBPInvalidResponseExInfo*)m_pInfo)->m_Request = pi_rRequest;
    }


//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
HFSHIBPInvalidResponseException::HFSHIBPInvalidResponseException(const HFSHIBPInvalidResponseException& pi_rObj)
    : HFSException(pi_rObj)
    {
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HFSHIBPInvalidResponseException::~HFSHIBPInvalidResponseException()
    {
    }

//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------
void HFSHIBPInvalidResponseException::FormatExceptionMessage(WString& pio_rMessage) const
    {
    }

//-----------------------------------------------------------------------------
// public
// operator=
//-----------------------------------------------------------------------------
HFSHIBPInvalidResponseException& HFSHIBPInvalidResponseException::operator=(const HFSHIBPInvalidResponseException& pi_rObj)
    {
    if (this != &pi_rObj)
        HFSHIBPInvalidResponseException::operator=(pi_rObj);

    return *this;
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HFSHIBPErrorException::HFSHIBPErrorException(const WString&    pi_rErrorMsg)
    : HFSException(HFS_IBP_ERROR_EXCEPTION)
    {
    m_pInfo = new HFSHIBPErrorExInfo();
    ((HFSHIBPErrorExInfo*)m_pInfo)->m_ErrorMessage = pi_rErrorMsg;

    GENERATE_FORMATTED_EXCEPTION_MSG()
    }


//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
HFSHIBPErrorException::HFSHIBPErrorException(const HFSHIBPErrorException& pi_rObj)
    : HFSException(pi_rObj)
    {
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HFSHIBPErrorException::~HFSHIBPErrorException()
    {
    }

//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------
void HFSHIBPErrorException::FormatExceptionMessage(WString& pio_rMessage) const
    {
    HPRECONDITION(m_pInfo != 0);
    FORMAT_EXCEPTION_MSG(pio_rMessage, ((HFSHIBPErrorExInfo*)m_pInfo)->m_ErrorMessage.c_str())
    }

//-----------------------------------------------------------------------------
// public
// operator=
//-----------------------------------------------------------------------------
HFSHIBPErrorException& HFSHIBPErrorException::operator=(const HFSHIBPErrorException& pi_rObj)
    {
    if (this != &pi_rObj)
        HFSHIBPErrorException::operator=(pi_rObj);

    return *this;
    }