//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCException.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Implementation of the exception classes.  The exception hierarchy look
// like this:
//
//  HFCException                            (Info struct : -)
//      HFCFileException                    (Info struct : HFCFileExInfo)
//      HFCDeviceException                    (Info struct : HFCDeviceExInfo)
//            HFCInternetConnectionException    (Info struct : HFCInternetConnectionExInfo)
//      HFCCannotCreateSynchroObjException  (Info struct : HFCCannotCreateSynchroObjExInfo)
//      HFCErrorCodeException               (Info struct : HFCErrorCodeExInfo)
//----------------------------------------------------------------------------

#pragma once


#include "HFCMacros.h"
#include "HFCErrorCode.h"

class HPANode;

/**---------------------------------------------------------------------------

These are the macros to use when a new class is created into the
HFCException class hierarchy.  Using these macros shorten the
development time, by implementing a part of the interface defined by the
HFCException class, avoiding the need to program the methods Clone and
ThrowCopy for each new class of exceptio n signaler.

The macro HFC_DECLARE_EXCEPTION is designed to be used into the header
file of the new class, inside the class declaration.  Place a call to
this macro just after the enclosing bracket that marks the beginning of
the declatation of the members.

The macro HFC_IMPLEMENT_EXCEPTION is designed to be used into the
implementation file of the class (the file with .cpp extension), which
is compiled only once for a given class (having no inline method).

@param pi_ExceptionType The name of the new exception signaler class.

----------------------------------------------------------------------------*/
class HFCException;


//#if defined(__HMR_DEBUG) || defined(__ATP)
struct HFCExceptionInfoATP
    {
    WString       m_ExceptionLabel;
    HFCException* m_pException;
    };

typedef map<ExceptionID, HFCExceptionInfoATP> HFCExceptionATPMap;

class HFCExceptionATP
    {
    HFC_DECLARE_SINGLETON_DLL(_HDLLu, HFCExceptionATP)

public :
    HFCExceptionATP();
    ~HFCExceptionATP();

    void                               AddException(const WString&    pi_rLabelStr,
                                                    HFCException*    pi_pException);
    HFCExceptionATPMap::const_iterator Begin();
    HFCExceptionATPMap::const_iterator End();
    uint32_t                           GetNbExceptions();

private :

    HFCExceptionATPMap m_ToBeTestedExceptions;
    };

//#endif


//----------------------------------------------------------------------------------
//Exception information struct for HFCException
//----------------------------------------------------------------------------------
struct HFCExInfo
    {
             HFCExInfo() {};
    virtual ~HFCExInfo() {};
    };

//----------------------------------------------------------------------------------
//Exception information struct for HFCFileException
//----------------------------------------------------------------------------------
struct HFCFileExInfo : public HFCExInfo
    {
    WString         m_FileName;

    HFCFileExInfo() {};
    HFCFileExInfo(const WString& pi_FileName) : m_FileName(pi_FileName) {};
    virtual ~HFCFileExInfo() {};
    };

//----------------------------------------------------------------------------------
//Exception information struct for HFCDeviceException
//----------------------------------------------------------------------------------
struct HFCDeviceExInfo : public HFCExInfo
    {
    WString            m_DeviceName;

    virtual ~HFCDeviceExInfo() {};
    };

//----------------------------------------------------------------------------------
//Exception information struct for HFCInternetConnectionException
//----------------------------------------------------------------------------------
struct HFCInternetConnectionExInfo : public HFCDeviceExInfo
    {
    short m_ErrorType; //see HFCInternetConnectionException::ErrorType

    virtual ~HFCInternetConnectionExInfo() {};
    };

//----------------------------------------------------------------------------------
//Exception information struct for HFCErrorCodeException
//----------------------------------------------------------------------------------
struct HFCErrorCodeExInfo : public HFCExInfo
    {
    HFCErrorCode    m_ErrorCode;

    virtual ~HFCErrorCodeExInfo();
    };

//----------------------------------------------------------------------------------
//Exception information struct for HFCCannotCreateSynchroObjException
//----------------------------------------------------------------------------------
struct HFCCannotCreateSynchroObjExInfo : public HFCExInfo
    {
    unsigned short m_SynchroObject; //HFCCannotCreateSynchroObjExInfo::SynchroObject

    virtual ~HFCCannotCreateSynchroObjExInfo() {};
    };

//----------------------------------------------------------------------------------
//Exception information struct for HCDIJLErrorException
//----------------------------------------------------------------------------------
struct HCDIJLErrorExInfo : public HFCExInfo
    {
    short m_IJLErrorCode;

    virtual ~HCDIJLErrorExInfo() {};
    };

//----------------------------------------------------------------------------------
//Exception information struct for HGFmzGCoordException
//----------------------------------------------------------------------------------
struct HGFmzGCoordExInfo : public HFCExInfo
    {
    int32_t m_StatusCode;

    virtual ~HGFmzGCoordExInfo() {};
    };

//----------------------------------------------------------------------------------
//Exception information struct for HPAException
//----------------------------------------------------------------------------------
struct HPAExInfo : public HFCExInfo
    {
    HFCPtr<HPANode> m_pOffendingNode;

    _HDLLu          HPAExInfo();
    _HDLLu virtual ~HPAExInfo();
    };

//----------------------------------------------------------------------------------
//Exception information struct for HPAGenericException
//----------------------------------------------------------------------------------
struct HPAGenericExInfo : public HPAExInfo
    {
    WString            m_Message;

    virtual ~HPAGenericExInfo() {};
    };

//----------------------------------------------------------------------------------
//Exception information struct for HPSTypeMismatchException
//----------------------------------------------------------------------------------
struct HPSTypeMismatchExInfo : public HPAExInfo
    {
    unsigned short    m_ExpectedType;    //see HPSTypeMismatchExInfo::ExpectedType

    virtual ~HPSTypeMismatchExInfo() {};
    };

//----------------------------------------------------------------------------------
//Exception information struct for HPSOutOfRangeException
//----------------------------------------------------------------------------------
struct HPSOutOfRangeExInfo : public HPAExInfo
    {
    double         m_Lower;
    double         m_Upper;

    virtual ~HPSOutOfRangeExInfo() {};
    };

//----------------------------------------------------------------------------------
//Exception information struct for HPSAlreadyDefinedException
//----------------------------------------------------------------------------------
struct HPSAlreadyDefinedExInfo : public HPAExInfo
    {
    WString         m_Name;

    virtual ~HPSAlreadyDefinedExInfo() {};
    };

//----------------------------------------------------------------------------------
//Exception information struct for HPSInvalidPathException
//----------------------------------------------------------------------------------
struct HFSInvalidPathExInfo : public HFCExInfo
    {
    WString            m_Path;

    virtual ~HFSInvalidPathExInfo() {};
    };

//----------------------------------------------------------------------------------
//Exception information struct for HFSHIBPInvalidResponseException
//----------------------------------------------------------------------------------
struct HFSHIBPInvalidResponseExInfo : public HFCExInfo
    {
    WString            m_Request;

    virtual ~HFSHIBPInvalidResponseExInfo() {};
    };

//----------------------------------------------------------------------------------
//Exception information struct for HFSHIBPUnsupportProtocolVersionException
//----------------------------------------------------------------------------------
struct HFSHIBPUnsupportProtocolVerExInfo : public HFCExInfo
    {
    double            m_Version;

    virtual ~HFSHIBPUnsupportProtocolVerExInfo() {};
    };

//----------------------------------------------------------------------------------
//Exception information struct for HFSHIBPErrorException
//----------------------------------------------------------------------------------
struct HFSHIBPErrorExInfo : public HFCExInfo
    {
    WString            m_ErrorMessage;

    virtual ~HFSHIBPErrorExInfo() {};
    };

//----------------------------------------------------------------------------------
//Exception information struct for HRFFileParameterException
//----------------------------------------------------------------------------------
struct HRFFileParameterExInfo : public HFCFileExInfo
    {
    WString            m_ParameterName;

    virtual ~HRFFileParameterExInfo() {};
    };


//----------------------------------------------------------------------------------
//Exception information struct for HRFChildFileException
//----------------------------------------------------------------------------------
struct HRFChildFileExInfo : public HFCFileExInfo
    {
    WString            m_ChildFileName;

    virtual ~HRFChildFileExInfo() {};
    };

//----------------------------------------------------------------------------------
//Exception information struct for HRFChildFileParameterException
//----------------------------------------------------------------------------------
struct HRFChildFileParamExInfo : public HRFChildFileExInfo
    {
    WString            m_ParameterName;

    virtual ~HRFChildFileParamExInfo() {};
    };

//----------------------------------------------------------------------------------
//Exception information struct for HRFGdalErrorException
//----------------------------------------------------------------------------------
struct HRFGdalErrorExInfo : public HFCFileExInfo
    {
    int32_t          m_GdalErrorID;

    virtual ~HRFGdalErrorExInfo() {};
    };

//----------------------------------------------------------------------------------
//Exception information struct for HRFInvalidNewFileDimExInfo
//----------------------------------------------------------------------------------
struct HRFInvalidNewFileDimExInfo : public HFCFileExInfo
    {
    uint64_t       m_WidthLimit;
    uint64_t       m_HeightLimit;

    virtual ~HRFInvalidNewFileDimExInfo() {};
    };

//----------------------------------------------------------------------------------
//Exception information struct for HCPException
//----------------------------------------------------------------------------------
struct HCPGCoordExInfo : public HFCExInfo
    {
    int32_t         m_Code;

    virtual ~HCPGCoordExInfo() {};
    };

//----------------------------------------------------------------------------------
//Macro for catching one particular exception. Note that this macro cannot be
//used within a stack of catches.
//----------------------------------------------------------------------------------
#define BEGIN_HFC_CATCH(pi_ExceptionID) \
catch(HFCException& rException) \
{ \
    if (rException.GetID() != pi_ExceptionID) \
    throw;

#define END_HFC_CATCH }

//----------------------------------------------------------------------------------
//Macro for copying the pre-formatted message, if any
//----------------------------------------------------------------------------------
#define COPY_FORMATTED_ERR_MSG(pi_pFormattedErrMsg, pi_pCopiedFormattedErrMsg) \
    if (pi_pCopiedFormattedErrMsg != 0) \
    { \
        if (pi_pFormattedErrMsg == 0) \
        { \
            pi_pFormattedErrMsg = new WString(*pi_pCopiedFormattedErrMsg); \
        } \
        else \
        { \
            *pi_pFormattedErrMsg = *pi_pCopiedFormattedErrMsg; \
        } \
    } \
    else \
    { \
        if (pi_pFormattedErrMsg != 0) \
        { \
            delete pi_pFormattedErrMsg; \
            pi_pFormattedErrMsg = 0; \
        } \
        HASSERT(pi_pFormattedErrMsg == 0); \
    } \
 
//----------------------------------------------------------------------------------
//Macro for copying an exception information structure
//----------------------------------------------------------------------------------
#define COPY_EXCEPTION_INFO(pi_pInfo, pi_pCopiedInfo, pi_InfoStructName) \
if (pi_pCopiedInfo != 0) \
{ \
    if (pi_pInfo == 0) \
    { \
        pi_pInfo = new pi_InfoStructName; \
    } \
\
    *((pi_InfoStructName*)pi_pInfo) = *((pi_InfoStructName*)pi_pCopiedInfo); \
} \
else \
{ \
    if (pi_pInfo != 0) \
    { \
        delete pi_pInfo; \
    } \
    pi_pInfo = 0; \
} \
 
//----------------------------------------------------------------------------------
//Macro for creating an exception common exception function
//----------------------------------------------------------------------------------
#define HFC_DECLARE_COMMON_EXCEPTION_FNC() \
    public: \
    virtual HFCException*   Clone() const; \
    virtual void            ThrowCopy() const; \
    virtual void            FormatExceptionMessage(WString& pio_rMessage) const;

#define HFC_IMPLEMENT_COMMON_EXCEPTION_FNC(pi_ExceptionType) \
    HFCException* pi_ExceptionType::Clone() const \
{   return new pi_ExceptionType(*this);    } \
    void pi_ExceptionType::ThrowCopy() const \
{   throw pi_ExceptionType(*this);         }

//----------------------------------------------------------------------------------
//Macro for formatting parameterized exception message
//----------------------------------------------------------------------------------
#define FORMAT_EXCEPTION_MSG(pio_rMessage, ...) \
{ \
    WChar TempBuffer[2048]; \
    BeStringUtilities::Snwprintf(TempBuffer, pio_rMessage.c_str(), __VA_ARGS__); \
    pio_rMessage = WString(TempBuffer); \
}

//----------------------------------------------------------------------------------
//Macro for formatting an exception message at the construction of the
//exception object. The message is saved in the base class for being able to access
//it into a catch.
//----------------------------------------------------------------------------------
#define GENERATE_FORMATTED_EXCEPTION_MSG() \
{ \
    m_pFormattedErrMsg = new WString; \
    *m_pFormattedErrMsg = GetExceptionMessageFromResource(); \
    FormatExceptionMessage(*m_pFormattedErrMsg); \
}

//----------------------------------------------------------------------------
// Class HFCException
//----------------------------------------------------------------------------
class _HDLLu HFCException
    {
    HDECLARE_BASECLASS_ID(6000)
    HFC_DECLARE_COMMON_EXCEPTION_FNC()
public :


    HFCException                   ();
    HFCException                   (ExceptionID             pi_ExceptionID);
    virtual                     ~HFCException                  ();

    HFCException                   (const HFCException&     pi_rObj);
    HFCException&               operator=                      (const HFCException&     pi_rObj);

    virtual ExceptionID         GetID                          () const;
    virtual const HFCExInfo*    GetInfo                        () const;
    virtual WString             GetExceptionMessage            () const;

protected :

    WString                     GetExceptionMessageFromResource() const;

    HFCExInfo*                  m_pInfo; //Can be set only by a derived class
    WString*                    m_pFormattedErrMsg;

private :

    ExceptionID                 m_ExceptionID;
    };

//----------------------------------------------------------------------------
// Class HFCFileException
//----------------------------------------------------------------------------
class _HDLLu HFCFileException : public HFCException
    {
    HDECLARE_CLASS_ID(6001, HFCException)
    HFC_DECLARE_COMMON_EXCEPTION_FNC()

public :
    HFCFileException                   (ExceptionID                 pi_ExceptionID,
                                        const WString&              pi_rFileName,
                                        bool                       pi_CreateExInfo = true);
    virtual                 ~HFCFileException                  ();

    HFCFileException                   (const HFCFileException&     pi_rObj);
    HFCFileException&       operator=                          (const HFCFileException&     pi_rObj);
    };

//----------------------------------------------------------------------------
// Class HFCDeviceException
//----------------------------------------------------------------------------
class _HDLLu HFCDeviceException : public HFCException
    {
    HDECLARE_CLASS_ID(6002, HFCException)
    HFC_DECLARE_COMMON_EXCEPTION_FNC()

public :
    HFCDeviceException(const WString& pi_rDeviceName,
                       bool          pi_CreateExceptionInfo = true);

    HFCDeviceException(ExceptionID    pi_ExceptionID,
                       const WString& pi_rDeviceName,
                       bool          pi_CreateExceptionInfo = true);
    virtual                 ~HFCDeviceException();

    HFCDeviceException(const HFCDeviceException& pi_rObj);
    HFCDeviceException&     operator=(const HFCDeviceException& pi_rObj);
    };

//-----------------------------------------------------------------------------
// HFCInternetConnectionException class
//-----------------------------------------------------------------------------
class _HDLLu HFCInternetConnectionException : public HFCDeviceException
    {
    HDECLARE_CLASS_ID(6003, HFCDeviceException)
    HFC_DECLARE_COMMON_EXCEPTION_FNC()

public :

    enum ErrorType
        {
        UNKNOWN = 0,
        CANNOT_CONNECT,
        CANNOT_BIND,
        CANNOT_SELECT,
        CANNOT_LISTEN,
        CANNOT_CREATE,
        CANNOT_SEND,
        CANNOT_READ,
        CANNOT_PEEK,
        CANNOT_SEPARATE_DATA,
        CONNECTION_LOST,
        CONNECTION_CLOSED_BY_CLIENT,
        INVALID_DOMAIN_NAME,
        CANNOT_GET_PEER_NAME,
        PERMISSION_DENIED,
        PROXY_PERMISSION_DENIED
        };

    HFCInternetConnectionException(const WString&    pi_rDeviceName,
                                   ErrorType        pi_ErrorType);
    HFCInternetConnectionException(ExceptionID        pi_ExceptionID,
                                   const WString&    pi_rDeviceName,
                                   ErrorType        pi_ErrorType);
    virtual         ~HFCInternetConnectionException();

    HFCInternetConnectionException(const HFCInternetConnectionException& pi_rObj);
    HFCInternetConnectionException&   operator=(const HFCInternetConnectionException& pi_rObj);

    ErrorType     GetErrorType() const;
    };

//-----------------------------------------------------------------------------
// HFCCannotCreateSynchroObjException class
//-----------------------------------------------------------------------------
class HFCCannotCreateSynchroObjException : public HFCException
    {
    HDECLARE_CLASS_ID(6005, HFCException)
    HFC_DECLARE_COMMON_EXCEPTION_FNC()

public :

    enum SynchroObject
        {
        MUTEX = 0,
        SEMAPHORE,
        EVENT
        };

    HFCCannotCreateSynchroObjException(HFCCannotCreateSynchroObjException::SynchroObject pi_SynchroObj);
    virtual         ~HFCCannotCreateSynchroObjException();

    HFCCannotCreateSynchroObjException(const HFCCannotCreateSynchroObjException& pi_rObj);
    HFCCannotCreateSynchroObjException&   operator=(const HFCCannotCreateSynchroObjException& pi_rObj);
    };


