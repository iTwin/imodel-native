//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFInternetImagingException.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFInternetImagingException
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HRFInternetImagingException.h>

HFC_IMPLEMENT_COMMON_EXCEPTION_FNC(HRFInternetImagingException)
HFC_IMPLEMENT_COMMON_EXCEPTION_FNC(HRFInternetImagingConnectionException)
HFC_IMPLEMENT_COMMON_EXCEPTION_FNC(HRFInternetImagingServerException)
HFC_IMPLEMENT_COMMON_EXCEPTION_FNC(HRFIIHandlerException)

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HRFInternetImagingException::HRFInternetImagingException(ExceptionID        pi_ExceptionID,
                                                                const WString&  pi_rFileName,
                                                                bool            pi_CreateInfoStruct)
    : HFCFileException(pi_ExceptionID, pi_rFileName, pi_CreateInfoStruct)
    {
    HPRECONDITION((pi_ExceptionID >= HRFII_BASE) && (pi_ExceptionID < HRFII_SEPARATOR_ID));
    }


//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
HRFInternetImagingException::HRFInternetImagingException(const HRFInternetImagingException& pi_rObj)
    : HFCFileException(pi_rObj)
    {
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRFInternetImagingException::~HRFInternetImagingException()
    {
    }

//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------
void HRFInternetImagingException::FormatExceptionMessage(WString& pio_rMessage) const
    {
    }

//-----------------------------------------------------------------------------
// public
// operator=
//-----------------------------------------------------------------------------
HRFInternetImagingException& HRFInternetImagingException::operator=(const HRFInternetImagingException& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        HFCFileException::operator=(pi_rObj);
        }

    return *this;
    }


//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HRFInternetImagingConnectionException::HRFInternetImagingConnectionException(const WString&  pi_rFileName,
        ErrorType       pi_ErrorType)
    : HRFInternetImagingException(HRFII_CONNECTION_EXCEPTION, pi_rFileName, false)
    {
    m_pInfo = new HRFInternetImagingConnectionExInfo;
    ((HRFInternetImagingConnectionExInfo*)m_pInfo)->m_FileName = pi_rFileName;
    ((HRFInternetImagingConnectionExInfo*)m_pInfo)->m_ErrorType = (unsigned short)pi_ErrorType;

    GENERATE_FORMATTED_EXCEPTION_MSG()
    }


//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
HRFInternetImagingConnectionException::HRFInternetImagingConnectionException(const HRFInternetImagingConnectionException& pi_rObj)
    : HRFInternetImagingException((ExceptionID)pi_rObj.GetID(), L"", false)
    {
    COPY_EXCEPTION_INFO(m_pInfo, pi_rObj.m_pInfo, HRFInternetImagingConnectionExInfo)
    COPY_FORMATTED_ERR_MSG(m_pFormattedErrMsg, pi_rObj.m_pFormattedErrMsg)
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRFInternetImagingConnectionException::~HRFInternetImagingConnectionException()
    {
    }

//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------
void HRFInternetImagingConnectionException::FormatExceptionMessage(WString& pio_rMessage) const
    {
    HPRECONDITION(m_pInfo != 0);
    FORMAT_EXCEPTION_MSG(pio_rMessage, ((HRFInternetImagingConnectionExInfo*)m_pInfo)->m_ErrorType)
    }

//-----------------------------------------------------------------------------
// public
// Return the error type
//-----------------------------------------------------------------------------
HRFInternetImagingConnectionException::ErrorType HRFInternetImagingConnectionException::GetErrorType()
    {
    HPRECONDITION(m_pInfo != 0);

    return (HRFInternetImagingConnectionException::ErrorType)
           ((HRFInternetImagingConnectionExInfo*)m_pInfo)->m_ErrorType;
    }

//-----------------------------------------------------------------------------
// public
// operator=
//-----------------------------------------------------------------------------
HRFInternetImagingConnectionException& HRFInternetImagingConnectionException::operator=(const HRFInternetImagingConnectionException& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        HFCException::operator=(pi_rObj);
        COPY_EXCEPTION_INFO(m_pInfo, pi_rObj.m_pInfo, HRFInternetImagingConnectionExInfo)
        }

    return *this;
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HRFInternetImagingServerException::HRFInternetImagingServerException(const WString& pi_rFileName,
                                                                            uint32_t       pi_Class,
                                                                            uint32_t       pi_Number,
                                                                            const WString& pi_rObject,
                                                                            const WString& pi_rMessage)
    : HRFInternetImagingException(HRFII_SERVER_EXCEPTION, L"", false)
    {
    m_pInfo = new HRFInternetImagingServerExInfo();
    ((HRFInternetImagingServerExInfo*)m_pInfo)->m_FileName = pi_rFileName;
    ((HRFInternetImagingServerExInfo*)m_pInfo)->m_Class    = pi_Class;
    ((HRFInternetImagingServerExInfo*)m_pInfo)->m_Number   = pi_Number;
    ((HRFInternetImagingServerExInfo*)m_pInfo)->m_Object   = pi_rObject;
    ((HRFInternetImagingServerExInfo*)m_pInfo)->m_Message  = pi_rMessage;

    GENERATE_FORMATTED_EXCEPTION_MSG()
    }


//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
HRFInternetImagingServerException::HRFInternetImagingServerException(const HRFInternetImagingServerException& pi_rObj)
    : HRFInternetImagingException((ExceptionID)pi_rObj.GetID(), L"", false)
    {
    COPY_EXCEPTION_INFO(m_pInfo, pi_rObj.m_pInfo, HRFInternetImagingServerExInfo)
    COPY_FORMATTED_ERR_MSG(m_pFormattedErrMsg, pi_rObj.m_pFormattedErrMsg)
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRFInternetImagingServerException::~HRFInternetImagingServerException()
    {
    }

//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------
void HRFInternetImagingServerException::FormatExceptionMessage(WString& pio_rMessage) const
    {
    HPRECONDITION(m_pInfo != 0);
    FORMAT_EXCEPTION_MSG(pio_rMessage,
                         ((HRFInternetImagingServerExInfo*)m_pInfo)->m_Class,
                         ((HRFInternetImagingServerExInfo*)m_pInfo)->m_Number,
                         ((HRFInternetImagingServerExInfo*)m_pInfo)->m_Object.c_str(),
                         ((HRFInternetImagingServerExInfo*)m_pInfo)->m_Message.c_str())
    }


//-----------------------------------------------------------------------------
// public
// operator=
//-----------------------------------------------------------------------------
HRFInternetImagingServerException& HRFInternetImagingServerException::operator=(const HRFInternetImagingServerException& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        HFCException::operator=(pi_rObj);
        COPY_EXCEPTION_INFO(m_pInfo, pi_rObj.m_pInfo, HRFInternetImagingServerExInfo)
        }

    return *this;
    }


//-----------------------------------------------------------------------------
// public
// Return the exception class
//-----------------------------------------------------------------------------
uint32_t HRFInternetImagingServerException::GetClass()  const
    {
    HPRECONDITION(m_pInfo != 0);
    return ((HRFInternetImagingServerExInfo*)m_pInfo)->m_Class;
    }


//-----------------------------------------------------------------------------
// public
// Return the exception number
//-----------------------------------------------------------------------------
uint32_t HRFInternetImagingServerException::GetNumber() const
    {
    HPRECONDITION(m_pInfo != 0);
    return ((HRFInternetImagingServerExInfo*)m_pInfo)->m_Number;
    }

//-----------------------------------------------------------------------------
// public
// Return the exception object
//-----------------------------------------------------------------------------
WString HRFInternetImagingServerException::GetObject() const
    {
    HPRECONDITION(m_pInfo != 0);
    return ((HRFInternetImagingServerExInfo*)m_pInfo)->m_Object;
    }

//-----------------------------------------------------------------------------
// public
// Return the exception message
//-----------------------------------------------------------------------------
WString HRFInternetImagingServerException::GetMessageText() const
    {
    HPRECONDITION(m_pInfo != 0);
    return ((HRFInternetImagingServerExInfo*)m_pInfo)->m_Message;
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HRFIIHandlerException::HRFIIHandlerException(ExceptionID                      pi_ExceptionID,
                                                    const WString&                   pi_rFileName,
                                                    HRFIIHandlerException::HandlerID pi_HandlerID)
    : HRFInternetImagingException(pi_ExceptionID, pi_rFileName, false)
    {
    m_pInfo = new HRFIIHandlerExInfo;
    ((HRFIIHandlerExInfo*)m_pInfo)->m_FileName  = pi_rFileName;
    ((HRFIIHandlerExInfo*)m_pInfo)->m_HandlerID = (unsigned short)pi_HandlerID;

    GENERATE_FORMATTED_EXCEPTION_MSG()
    }


//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
HRFIIHandlerException::HRFIIHandlerException(const HRFIIHandlerException& pi_rObj)
    : HRFInternetImagingException((ExceptionID)pi_rObj.GetID(), L"", false)
    {
    COPY_EXCEPTION_INFO(m_pInfo, pi_rObj.m_pInfo, HRFIIHandlerExInfo)
    COPY_FORMATTED_ERR_MSG(m_pFormattedErrMsg, pi_rObj.m_pFormattedErrMsg)
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRFIIHandlerException::~HRFIIHandlerException()
    {
    }

//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------
void HRFIIHandlerException::FormatExceptionMessage(WString& pio_rMessage) const
    {
    HPRECONDITION(m_pInfo != 0);
    FORMAT_EXCEPTION_MSG(pio_rMessage, ((HRFIIHandlerExInfo*)m_pInfo)->m_HandlerID)
    }

//-----------------------------------------------------------------------------
// public
// operator=
//-----------------------------------------------------------------------------
HRFIIHandlerException& HRFIIHandlerException::operator=(const HRFIIHandlerException& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        HFCException::operator=(pi_rObj);
        COPY_EXCEPTION_INFO(m_pInfo, pi_rObj.m_pInfo, HRFIIHandlerExInfo)
        }

    return *this;
    }