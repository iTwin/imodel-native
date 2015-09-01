//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCException.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for exception classes.
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/ImagePPMessages.xliff.h>
#include <Imagepp/h/ImagePPExceptionMessages.xliff.h>
#include <Imagepp/all/h/HFCMacros.h>

//-----------------------------------------------------------------------------
// public
// Default Constructor
//-----------------------------------------------------------------------------
HFCException::HFCException() 
    {
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HFCException::~HFCException()
    {

    }

//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
HFCException::HFCException(const HFCException& pi_rObj)
    {
    operator=(pi_rObj);
    }

//-----------------------------------------------------------------------------
// public
// operator=
//-----------------------------------------------------------------------------
HFCException& HFCException::operator=(const HFCException& pi_rObj)
    {
    return *this;
    }

//-----------------------------------------------------------------------------
// protected
// Fetch the exception message from the resource 
//-----------------------------------------------------------------------------
WString HFCException::GetRawMessageFromResource(const ImagePPExceptions::StringId& pi_ID) const
{
    WString ExceptionMsg = ImagePPExceptions::GetStringW(pi_ID);
    if (ExceptionMsg.empty())
        {
        // *** Not now because HFCResourceLoader is not thread safe so it cannot load string in working thread.
        // Should be fix when we rework exceptions or change Rsc manager. i.e. GetMessage should occur when required and not at construction.
        //HASSERT(!"Exception Message Not Found In Localized Resource");

        WString ExceptionMsg = ImagePPExceptions::GetStringW(ImagePPExceptions::Unknown());
        }
    return ExceptionMsg;
}

//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------
WString HFCException::_BuildMessage(const ImagePPExceptions::StringId& pi_rsID) const
{
    WString exceptionName(pi_rsID.m_str, true/*isUtf8*/);
    WPrintfString message(L"%ls - [%ls]", GetRawMessageFromResource(pi_rsID).c_str(), exceptionName.c_str());
    return message;
}

//-----------------------------------------------------------------------------
// public
// Constructor for derived classes
//-----------------------------------------------------------------------------
HFCFileException::HFCFileException(const WString&    pi_rFileName)
    :HFCException()
    {
        m_FileName = pi_rFileName;
    }
//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
 HFCFileException::HFCFileException(const HFCFileException&     pi_rObj) : HFCException(pi_rObj)
 {
     m_FileName = pi_rObj.m_FileName;
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
WString HFCFileException::_BuildMessage(const ImagePPExceptions::StringId& pi_rsID) const
    {
    //For some exception of this type, the file name correspond to something
    //different than the current raster file.
    WPrintfString rawMessage(GetRawMessageFromResource(pi_rsID).c_str(), m_FileName.c_str());
    WString exceptionName(pi_rsID.m_str, true/*isUtf8*/);
    WPrintfString message(L"%ls - [%ls]", rawMessage.c_str(), exceptionName.c_str());
    return message;
    }

//-----------------------------------------------------------------------------
// public
// Get the exception filename
//-----------------------------------------------------------------------------
WStringCR HFCFileException::GetFileName() const
    {
    return m_FileName;
    }
bool HFCFileException::IsInvalidAccess() const
    {
        return false;
    }
//-----------------------------------------------------------------------------
// public
// Default Constructor
//-----------------------------------------------------------------------------
HFCDeviceException::HFCDeviceException(const WString& pi_rDeviceName) :HFCException()
    {
    m_DeviceName = pi_rDeviceName; 
    }
//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
 HFCDeviceException::HFCDeviceException(const HFCDeviceException&     pi_rObj) : HFCException(pi_rObj)
 {
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
WString HFCDeviceException::_BuildMessage(const ImagePPExceptions::StringId& pi_rsID) const
    {
    WPrintfString rawMessage(GetRawMessageFromResource(pi_rsID).c_str(), m_DeviceName.c_str());
    WString exceptionName(pi_rsID.m_str, true/*isUtf8*/);
    WPrintfString message(L"%ls - [%ls]", rawMessage.c_str(), exceptionName.c_str());
    return message;
    }
//-----------------------------------------------------------------------------
// public
// Get the exception device name
//-----------------------------------------------------------------------------
WStringCR HFCDeviceException::GetDeviceName() const
    {
    return m_DeviceName;
    }

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HFCInternetConnectionException::HFCInternetConnectionException(const WString&    pi_rDeviceName,
                                                               ErrorType        pi_ErrorType)
    :HFCDeviceException(pi_rDeviceName)
    {
    m_ErrorType  = pi_ErrorType;   
    }

//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
 HFCInternetConnectionException::HFCInternetConnectionException(const HFCInternetConnectionException&     pi_rObj) : HFCDeviceException(pi_rObj)
 {
     m_ErrorType = pi_rObj.m_ErrorType;
 }
//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HFCInternetConnectionException::~HFCInternetConnectionException()
    {
    }

 /*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Julien.Rossignol 07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HFCException* HFCInternetConnectionException::Clone() const
    {
    return new HFCInternetConnectionException(*this);;
    }

//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------
WString HFCInternetConnectionException::GetExceptionMessage() const
    {
    WPrintfString rawMessage(GetRawMessageFromResource(ImagePPExceptions::HFCInternetConnection()).c_str(), m_DeviceName.c_str(), m_ErrorType);
    WString exceptionName(ImagePPExceptions::HFCInternetConnection().m_str, true/*isUtf8*/);
    WPrintfString message(L"%ls - [%ls]", rawMessage.c_str(), exceptionName.c_str());
    return message;
    }

//-----------------------------------------------------------------------------
// Public
// Get the error type
//-----------------------------------------------------------------------------
const HFCInternetConnectionException::ErrorType HFCInternetConnectionException::GetErrorType() const
    {
    return m_ErrorType;
    }

//-----------------------------------------------------------------------------
// public
// Constructor for derived classes
//-----------------------------------------------------------------------------
HFCCannotCreateSynchroObjException::HFCCannotCreateSynchroObjException(SynchroObject pi_SynchroObject)
    :HFCException()
    {
    m_SynchroObject = pi_SynchroObject;
    }
//-----------------------------------------------------------------------------
// public
// Copy Constructor
//-----------------------------------------------------------------------------
 HFCCannotCreateSynchroObjException::HFCCannotCreateSynchroObjException(const HFCCannotCreateSynchroObjException&     pi_rObj) : HFCException(pi_rObj)
 {
     m_SynchroObject = pi_rObj.m_SynchroObject;
 }
//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HFCCannotCreateSynchroObjException::~HFCCannotCreateSynchroObjException()
    {
    }
 /*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Julien.Rossignol 07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HFCException* HFCCannotCreateSynchroObjException::Clone() const
    {
    return new HFCCannotCreateSynchroObjException(*this);
    }

//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------
WString HFCCannotCreateSynchroObjException::GetExceptionMessage() const
    {
    WPrintfString rawMessage(GetRawMessageFromResource(ImagePPExceptions::HFCCannotCreateSynchroObj()).c_str(), m_SynchroObject);
    WString exceptionName(ImagePPExceptions::HFCCannotCreateSynchroObj().m_str, true/*isUtf8*/);
    WPrintfString message(L"%ls - [%ls]", rawMessage.c_str(), exceptionName.c_str());
    return message;
    }
//-----------------------------------------------------------------------------
// public
// Return the exception synchro object type
//-----------------------------------------------------------------------------
const HFCCannotCreateSynchroObjException::SynchroObject HFCCannotCreateSynchroObjException::GetSynchroObject() const
{
    return m_SynchroObject;
}

