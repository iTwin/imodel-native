//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCException.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for exception classes.
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HFCResourceLoader.h>
#include <Imagepp/all/h/ImagePPMessages.xliff.h>
#include <Imagepp/all/h/HFCMacros.h>
#include <Imagepp/all/h/HFCErrorCodeException.h>
#include <Imagepp/all/h/HPANode.h>

HFC_IMPLEMENT_SINGLETON(HFCExceptionATP)

//-----------------------------------------------------------------------------
// public
// Add an exception to be tested by ExceptionMsgTester
//-----------------------------------------------------------------------------
void HFCExceptionATP::AddException(const WString& pi_rLabelStr,
                                   HFCException*  pi_pException)
    {
    HFCExceptionInfoATP ExceptionInfo;

    ExceptionInfo.m_ExceptionLabel = pi_rLabelStr;
    ExceptionInfo.m_pException     = pi_pException;

    HASSERT(m_ToBeTestedExceptions.find(pi_pException->GetID()) == m_ToBeTestedExceptions.end());

    m_ToBeTestedExceptions.insert(HFCExceptionATPMap::value_type(pi_pException->GetID(), ExceptionInfo));
    }

//-----------------------------------------------------------------------------
// public
// Get an iterator initialized at the beginning of the exception list
//-----------------------------------------------------------------------------
HFCExceptionATPMap::const_iterator HFCExceptionATP::Begin()
    {
    return m_ToBeTestedExceptions.begin();
    }

//-----------------------------------------------------------------------------
// public
// Get an iterator pointing at the end of the exception list
//-----------------------------------------------------------------------------
HFCExceptionATPMap::const_iterator HFCExceptionATP::End()
    {
    return m_ToBeTestedExceptions.end();
    }

//-----------------------------------------------------------------------------
// public
// Default Constructor
//-----------------------------------------------------------------------------
HFCExceptionATP::HFCExceptionATP()
    {
    //Ensure that the exception counts in HmrError.h are valid
    HASSERT(HFC_EXCEPTION_COUNT      == (HFC_SEPARATOR_ID - HFC_BASE));
    HASSERT(HCD_EXCEPTION_COUNT      == (HCD_SEPARATOR_ID - HCD_BASE));
    HASSERT(HFS_EXCEPTION_COUNT      == (HFS_SEPARATOR_ID - HFS_BASE));
    HASSERT(HGF_EXCEPTION_COUNT      == (HGF_SEPARATOR_ID - HGF_BASE));
    HASSERT(HPA_EXCEPTION_COUNT      == (HPA_SEPARATOR_ID - HPA_BASE));
    HASSERT(HPS_EXCEPTION_COUNT      == (HPS_SEPARATOR_ID - HPS_BASE));
    HASSERT(HRFII_EXCEPTION_COUNT == (HRFII_SEPARATOR_ID - HRFII_BASE));
    HASSERT(HRF_EXCEPTION_COUNT   == (HRF_SEPARATOR_ID - HRF_BASE));
    HASSERT(HCS_EXCEPTION_COUNT   == (HCS_SEPARATOR_ID - HCS_BASE));
    HASSERT(HCP_EXCEPTION_COUNT   == (HCP_SEPARATOR_ID - HCP_BASE));
    HASSERT(HVE_EXCEPTION_COUNT   == (HVE_SEPARATOR_ID - HVE_BASE));
    }

//-----------------------------------------------------------------------------
// public
// Get the total number of exceptions to test
//-----------------------------------------------------------------------------
uint32_t HFCExceptionATP::GetNbExceptions()
    {
    return (uint32_t)m_ToBeTestedExceptions.size();
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HFCExceptionATP::~HFCExceptionATP()
    {
    HFCExceptionATPMap::const_iterator ExceptionIter = m_ToBeTestedExceptions.begin();
    HFCExceptionATPMap::const_iterator ExceptionIterEnd = m_ToBeTestedExceptions.end();

    while (ExceptionIter != ExceptionIterEnd)
        {
        delete (*ExceptionIter).second.m_pException;
        ExceptionIter++;
        }
    }

//-----------------------------------------------------------------------------
// public
// Implementation of functions common to all exception classes
//-----------------------------------------------------------------------------
HFC_IMPLEMENT_COMMON_EXCEPTION_FNC(HFCException)
HFC_IMPLEMENT_COMMON_EXCEPTION_FNC(HFCFileException)
HFC_IMPLEMENT_COMMON_EXCEPTION_FNC(HFCDeviceException)
HFC_IMPLEMENT_COMMON_EXCEPTION_FNC(HFCInternetConnectionException)
HFC_IMPLEMENT_COMMON_EXCEPTION_FNC(HFCCannotCreateSynchroObjException)

//-----------------------------------------------------------------------------
// public
// Default Constructor
//-----------------------------------------------------------------------------
HFCException::HFCException()
    : m_pInfo(0),
      m_pFormattedErrMsg(0)
    {
    m_ExceptionID = HFC_EXCEPTION;
    }


//-----------------------------------------------------------------------------
// Static Private
// Validate if Exception ID is in the expected range
//-----------------------------------------------------------------------------
inline static void ValidateExceptionIDInRange(ExceptionID pi_ExceptionID)
    {
    HPRECONDITION((pi_ExceptionID != NO_EXCEPTION) &&
                  ((pi_ExceptionID == UNKNOWN_EXCEPTION) ||
                   ((pi_ExceptionID >= HFC_BASE) && (pi_ExceptionID < HFC_SEPARATOR_ID)) ||
                   ((pi_ExceptionID >= HCD_BASE) && (pi_ExceptionID < HCD_SEPARATOR_ID)) ||
                   ((pi_ExceptionID >= HFS_BASE) && (pi_ExceptionID < HFS_SEPARATOR_ID)) ||
                   ((pi_ExceptionID >= HGF_BASE) && (pi_ExceptionID < HGF_SEPARATOR_ID)) ||
                   ((pi_ExceptionID >= HPA_BASE) && (pi_ExceptionID < HPA_SEPARATOR_ID)) ||
                   ((pi_ExceptionID >= HPS_BASE) && (pi_ExceptionID < HPS_SEPARATOR_ID)) ||
                   ((pi_ExceptionID >= HRFII_BASE) && (pi_ExceptionID < HRFII_SEPARATOR_ID)) ||
                   ((pi_ExceptionID >= HRF_BASE) && (pi_ExceptionID < HRF_SEPARATOR_ID)) ||
                   ((pi_ExceptionID >= HCS_BASE) && (pi_ExceptionID < HCS_SEPARATOR_ID)) ||
                   ((pi_ExceptionID >= HCP_BASE) && (pi_ExceptionID < HCP_SEPARATOR_ID)) ||
                   ((pi_ExceptionID >= HVE_BASE) && (pi_ExceptionID < HVE_SEPARATOR_ID))));
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HFCException::HFCException(ExceptionID pi_ExceptionID)
    :   m_ExceptionID       (pi_ExceptionID),
        m_pInfo             (0),
        m_pFormattedErrMsg  (0)
    {
    ValidateExceptionIDInRange(pi_ExceptionID);
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HFCException::~HFCException()
    {
    if (m_pInfo != 0)
        {
        delete m_pInfo;
        }

    if (m_pFormattedErrMsg != 0)
        {
        delete m_pFormattedErrMsg;
        }
    }

//-----------------------------------------------------------------------------
// public
// Get the exception's unique identifier
//-----------------------------------------------------------------------------
ExceptionID HFCException::GetID() const
    {
    return m_ExceptionID;
    }

//-----------------------------------------------------------------------------
// public
// Get the exception information, if any.
//-----------------------------------------------------------------------------
const HFCExInfo* HFCException::GetInfo() const
    {
    return (const HFCExInfo*)m_pInfo;
    }

//-----------------------------------------------------------------------------
// public
// Get the exception message
//-----------------------------------------------------------------------------
WString HFCException::GetExceptionMessage() const
    {
    WString ExceptionMsg;

    //Some formatting was required and was done at the creation time of
    //the derived exception class.
    if (m_pFormattedErrMsg != 0)
        {
        ExceptionMsg = *m_pFormattedErrMsg;
        }
    else
        {
        ExceptionMsg = GetExceptionMessageFromResource();
        }

    //Append the exception ID at the end of the exception message
    WChar TempBuffer[50];
    ExceptionID exceptionID(m_ExceptionID);
    BeStringUtilities::Snwprintf(TempBuffer, L" - [%hu]", exceptionID);
    ExceptionMsg += WString(TempBuffer);

    return ExceptionMsg;
    }

//-----------------------------------------------------------------------------
// public
// Get the exception message
//-----------------------------------------------------------------------------
void HFCException::FormatExceptionMessage(WString& pio_rMessage) const
    {
    //No formatting is required by default.
    }

//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
HFCException::HFCException(const HFCException& pi_rObj)
    : m_pInfo(0),
      m_pFormattedErrMsg(0)
    {
    operator=(pi_rObj);
    }

//-----------------------------------------------------------------------------
// public
// operator=
//-----------------------------------------------------------------------------
HFCException& HFCException::operator=(const HFCException& pi_rObj)
    {
    m_ExceptionID = pi_rObj.m_ExceptionID;

    if (pi_rObj.m_pFormattedErrMsg != 0)
        {
        if (m_pFormattedErrMsg == 0)
            {
            m_pFormattedErrMsg = new WString;
            }

        *m_pFormattedErrMsg = *pi_rObj.m_pFormattedErrMsg;
        }
    else
        {
        if (m_pFormattedErrMsg != 0)
            {
            delete m_pFormattedErrMsg;
            }

        m_pFormattedErrMsg = 0;
        }

    return *this;
    }

//-----------------------------------------------------------------------------
// protected
// Fetch the exception message from the resource
//-----------------------------------------------------------------------------
WString HFCException::GetExceptionMessageFromResource() const
    {
    WString ExceptionMsg = HFCResourceLoader::GetInstance()->GetExceptionString(m_ExceptionID);
  
    if (ExceptionMsg.empty())
        {
        HASSERT(!"Exception Message Not Found In Localized Resource");

        HFCResourceLoader::GetInstance()->GetExceptionString(UNKNOWN_EXCEPTION);
        }

    return ExceptionMsg;
    }

//-----------------------------------------------------------------------------
// public
// Constructor for derived classes
//-----------------------------------------------------------------------------
HFCFileException::HFCFileException(ExceptionID        pi_ExceptionID,
                                   const WString&    pi_rFileName,
                                   bool            pi_CreateExInfo)
    :HFCException(pi_ExceptionID)
    {
    if (pi_CreateExInfo)
        {
        m_pInfo = new HFCFileExInfo();
        ((HFCFileExInfo*)m_pInfo)->m_FileName = pi_rFileName;

        if ((GetID() == HFC_DLL_NOT_FOUND_EXCEPTION) ||
            (GetID() == HFC_CORRUPTED_DLL_EXCEPTION) ||
            (GetID() == HFC_DIRECTORY_READ_ONLY_EXCEPTION) ||
            (GetID() == HFC_SISTER_FILE_NOT_CREATED_EXCEPTION) ||
            (GetID() == HFC_DIRECTORY_NOT_CREATED_EXCEPTION))
            {
            GENERATE_FORMATTED_EXCEPTION_MSG()
            }
        }
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HFCFileException::~HFCFileException()
    {
    }

//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------
void HFCFileException::FormatExceptionMessage(WString& pio_rMessage) const
    {
    HPRECONDITION(m_pInfo != 0);

    //For some exception of this type, the file name correspond to something
    //different than the current raster file.
    if ((GetID() == HFC_DLL_NOT_FOUND_EXCEPTION) ||
        (GetID() == HFC_CORRUPTED_DLL_EXCEPTION) ||
        (GetID() == HFC_DIRECTORY_READ_ONLY_EXCEPTION) ||
        (GetID() == HFC_SISTER_FILE_NOT_CREATED_EXCEPTION) ||
        (GetID() == HFC_DIRECTORY_NOT_CREATED_EXCEPTION))
        {
        FORMAT_EXCEPTION_MSG(pio_rMessage, ((HFCFileExInfo*)m_pInfo)->m_FileName.c_str())
        }
    }


//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
HFCFileException::HFCFileException(const HFCFileException& pi_rObj)
    : HFCException(pi_rObj)
    {
    COPY_EXCEPTION_INFO(m_pInfo, pi_rObj.m_pInfo, HFCFileExInfo)
    }

//-----------------------------------------------------------------------------
// public
// operator=
//-----------------------------------------------------------------------------
HFCFileException& HFCFileException::operator=(const HFCFileException& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        HFCException::operator=(pi_rObj);
        COPY_EXCEPTION_INFO(m_pInfo, pi_rObj.m_pInfo, HFCFileExInfo)
        }

    return *this;
    }

//-----------------------------------------------------------------------------
// public
// Default Constructor
//-----------------------------------------------------------------------------
HFCDeviceException::HFCDeviceException(const WString& pi_rDeviceName,
                                       bool          pi_CreateExceptionInfo)
    :HFCException(HFC_DEVICE_EXCEPTION)
    {
    m_pInfo = new HFCDeviceExInfo();
    ((HFCDeviceExInfo*)m_pInfo)->m_DeviceName = pi_rDeviceName;

    GENERATE_FORMATTED_EXCEPTION_MSG()
    }

//-----------------------------------------------------------------------------
// public
// Constructor for derived classes
//-----------------------------------------------------------------------------
HFCDeviceException::HFCDeviceException(ExceptionID        pi_ExceptionID,
                                       const WString&    pi_rDeviceName,
                                       bool            pi_CreateExceptionInfo)
    :HFCException(pi_ExceptionID)
    {
    if (pi_CreateExceptionInfo == true)
        {
        m_pInfo = new HFCDeviceExInfo();
        ((HFCDeviceExInfo*)m_pInfo)->m_DeviceName = pi_rDeviceName;

        if (GetID() != HFC_NO_DISK_SPACE_LEFT_EXCEPTION)
            {
            GENERATE_FORMATTED_EXCEPTION_MSG()
            }
        }
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HFCDeviceException::~HFCDeviceException()
    {
    }

//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------
void HFCDeviceException::FormatExceptionMessage(WString& pio_rMessage) const
    {
    HPRECONDITION(m_pInfo != 0);

    if (GetID() != HFC_NO_DISK_SPACE_LEFT_EXCEPTION)
        {
        FORMAT_EXCEPTION_MSG(pio_rMessage, ((HFCDeviceExInfo*)m_pInfo)->m_DeviceName.c_str())
        }
    }

//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
HFCDeviceException::HFCDeviceException(const HFCDeviceException& pi_rObj)
    : HFCException(pi_rObj)
    {
    COPY_EXCEPTION_INFO(m_pInfo, pi_rObj.m_pInfo, HFCDeviceExInfo)
    }

//-----------------------------------------------------------------------------
// public
// operator=
//-----------------------------------------------------------------------------
HFCDeviceException& HFCDeviceException::operator=(const HFCDeviceException& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        HFCException::operator=(pi_rObj);
        COPY_EXCEPTION_INFO(m_pInfo, pi_rObj.m_pInfo, HFCDeviceExInfo)
        }

    return *this;
    }

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HFCInternetConnectionException::HFCInternetConnectionException(const WString&    pi_rDeviceName,
                                                               ErrorType        pi_ErrorType)
    :HFCDeviceException(HFC_INTERNET_CONNECTION_EXCEPTION, pi_rDeviceName, false)
    {
    m_pInfo = new HFCInternetConnectionExInfo();
    ((HFCInternetConnectionExInfo*)m_pInfo)->m_DeviceName = pi_rDeviceName;
    ((HFCInternetConnectionExInfo*)m_pInfo)->m_ErrorType  = (short)pi_ErrorType;

    GENERATE_FORMATTED_EXCEPTION_MSG()
    }

//-----------------------------------------------------------------------------
// public
// Constructor for derived classes
//-----------------------------------------------------------------------------
HFCInternetConnectionException::HFCInternetConnectionException(ExceptionID        pi_ExceptionID,
                                                               const WString&    pi_rDeviceName,
                                                               ErrorType        pi_ErrorType)
    :HFCDeviceException(pi_ExceptionID, pi_rDeviceName, false)
    {
    HASSERT(pi_ExceptionID == HFC_INTERNET_CONNECTION_EXCEPTION);

    m_pInfo = new HFCInternetConnectionExInfo();
    ((HFCInternetConnectionExInfo*)m_pInfo)->m_DeviceName = pi_rDeviceName;
    ((HFCInternetConnectionExInfo*)m_pInfo)->m_ErrorType  = (short)pi_ErrorType;

    GENERATE_FORMATTED_EXCEPTION_MSG()
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HFCInternetConnectionException::~HFCInternetConnectionException()
    {
    }

//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------
void HFCInternetConnectionException::FormatExceptionMessage(WString& pio_rMessage) const
    {
    HPRECONDITION(m_pInfo != 0);
    FORMAT_EXCEPTION_MSG(pio_rMessage,
                         ((HFCInternetConnectionExInfo*)m_pInfo)->m_DeviceName.c_str(),
                         ((HFCInternetConnectionExInfo*)m_pInfo)->m_ErrorType)
    }

//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
HFCInternetConnectionException::HFCInternetConnectionException(const HFCInternetConnectionException& pi_rObj)
    :HFCDeviceException((ExceptionID)pi_rObj.GetID(), L"", false)
    {
    COPY_EXCEPTION_INFO(m_pInfo, pi_rObj.m_pInfo, HFCInternetConnectionExInfo)
    COPY_FORMATTED_ERR_MSG(m_pFormattedErrMsg, pi_rObj.m_pFormattedErrMsg)
    }

//-----------------------------------------------------------------------------
// public
// operator=
//-----------------------------------------------------------------------------
HFCInternetConnectionException& HFCInternetConnectionException::operator=(const HFCInternetConnectionException& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        HFCException::operator=(pi_rObj);
        COPY_EXCEPTION_INFO(m_pInfo, pi_rObj.m_pInfo, HFCInternetConnectionExInfo)
        }

    return *this;
    }

//-----------------------------------------------------------------------------
// Public
// Get the error type
//-----------------------------------------------------------------------------
HFCInternetConnectionException::ErrorType HFCInternetConnectionException::GetErrorType() const
    {
    HPRECONDITION(m_pInfo != 0);
    return (HFCInternetConnectionException::ErrorType)
           ((HFCInternetConnectionExInfo*)m_pInfo)->m_ErrorType;
    }

//-----------------------------------------------------------------------------
// public
// Constructor for derived classes
//-----------------------------------------------------------------------------
HFCCannotCreateSynchroObjException::HFCCannotCreateSynchroObjException(SynchroObject pi_SynchroObject)
    :HFCException(HFC_CANNOT_CREATE_SYNCHRO_OBJ_EXCEPTION)
    {
    m_pInfo = new HFCCannotCreateSynchroObjExInfo;
    ((HFCCannotCreateSynchroObjExInfo*)m_pInfo)->m_SynchroObject = (unsigned short)pi_SynchroObject;

    GENERATE_FORMATTED_EXCEPTION_MSG()
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HFCCannotCreateSynchroObjException::~HFCCannotCreateSynchroObjException()
    {
    }

//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------
void HFCCannotCreateSynchroObjException::FormatExceptionMessage(WString& pio_rMessage) const
    {
    HPRECONDITION(m_pInfo != 0);
    FORMAT_EXCEPTION_MSG(pio_rMessage, ((HFCCannotCreateSynchroObjExInfo*)m_pInfo)->m_SynchroObject)
    }

//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
HFCCannotCreateSynchroObjException::HFCCannotCreateSynchroObjException(const HFCCannotCreateSynchroObjException& pi_rObj)
    : HFCException(pi_rObj)
    {
    COPY_EXCEPTION_INFO(m_pInfo, pi_rObj.m_pInfo, HFCCannotCreateSynchroObjExInfo)
    }

//-----------------------------------------------------------------------------
// public
// operator=
//-----------------------------------------------------------------------------
HFCCannotCreateSynchroObjException& HFCCannotCreateSynchroObjException::operator=(const HFCCannotCreateSynchroObjException& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        HFCException::operator=(pi_rObj);
        COPY_EXCEPTION_INFO(m_pInfo, pi_rObj.m_pInfo, HFCCannotCreateSynchroObjExInfo)
        }

    return *this;
    }

//-----------------------------------------------------------------------------
// public
// Implementation of functions common to all exception classes
//-----------------------------------------------------------------------------
HFC_IMPLEMENT_COMMON_EXCEPTION_FNC(HFCErrorCodeException)

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HFCErrorCodeException::HFCErrorCodeException(const HFCErrorCode&    pi_rErrorCode)
    : HFCException(HFC_ERROR_CODE_EXCEPTION)
    {
    m_pInfo = new HFCErrorCodeExInfo();
    ((HFCErrorCodeExInfo*)m_pInfo)->m_ErrorCode = pi_rErrorCode;
    }


//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
HFCErrorCodeException::HFCErrorCodeException(const HFCErrorCodeException& pi_rObj)
    : HFCException(pi_rObj)
    {
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HFCErrorCodeException::~HFCErrorCodeException()
    {
    }

//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------
void HFCErrorCodeException::FormatExceptionMessage(WString& pio_rMessage) const
    {
    }

//-----------------------------------------------------------------------------
// public
// operator=
//-----------------------------------------------------------------------------
HFCErrorCodeException& HFCErrorCodeException::operator=(const HFCErrorCodeException& pi_rObj)
    {
    if (this != &pi_rObj)
        HFCErrorCodeException::operator=(pi_rObj);

    return *this;
    }

//-----------------------------------------------------------------------------
// public
//-----------------------------------------------------------------------------
HFCErrorCodeExInfo::~HFCErrorCodeExInfo() {};

//-----------------------------------------------------------------------------
// public
//-----------------------------------------------------------------------------
HPAExInfo::HPAExInfo() {};
HPAExInfo::~HPAExInfo() {};