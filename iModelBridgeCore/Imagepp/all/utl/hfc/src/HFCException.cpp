//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for exception classes.
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>


#include <ImagePP/all/h/HFCException.h>
#include <ImagePP/all/h/ImagePPMessages.xliff.h>
#include <ImagePP/h/ImagePPExceptionMessages.xliff.h>
#include <ImagePP/all/h/HFCMacros.h>

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
Utf8String HFCException::GetRawMessageFromResource(const ImagePPExceptions::StringId& pi_ID) const
{
    Utf8String ExceptionMsg = ImagePPExceptions::GetString(pi_ID);
    if (ExceptionMsg.empty())
        {
        // *** Not now because HFCResourceLoader is not thread safe so it cannot load string in working thread.
        // Should be fix when we rework exceptions or change Rsc manager. i.e. GetMessage should occur when required and not at construction.
        //HASSERT(!"Exception Message Not Found In Localized Resource");

        Utf8String ExceptionMsg = ImagePPExceptions::GetString(ImagePPExceptions::Unknown());
        }
    return ExceptionMsg;
}

//-----------------------------------------------------------------------------
// public
// Return the message formatted with specific information on the exception
// that have occurred.
//-----------------------------------------------------------------------------
Utf8String HFCException::_BuildMessage(const ImagePPExceptions::StringId& pi_rsID) const
{
    Utf8String exceptionName(pi_rsID.m_str);
    Utf8PrintfString message("%s - [%s]", GetRawMessageFromResource(pi_rsID).c_str(), exceptionName.c_str());
    return message;
}

//-----------------------------------------------------------------------------
// public
// Constructor for derived classes
//-----------------------------------------------------------------------------
HFCFileException::HFCFileException(const Utf8String&    pi_rFileName)
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
Utf8String HFCFileException::_BuildMessage(const ImagePPExceptions::StringId& pi_rsID) const
    {
    //For some exception of this type, the file name correspond to something
    //different than the current raster file.
    Utf8PrintfString rawMessage(GetRawMessageFromResource(pi_rsID).c_str(), m_FileName.c_str());
    Utf8String exceptionName(pi_rsID.m_str);
    Utf8PrintfString message("%s - [%s]", rawMessage.c_str(), exceptionName.c_str());
    return message;
    }

//-----------------------------------------------------------------------------
// public
// Get the exception filename
//-----------------------------------------------------------------------------
Utf8StringCR HFCFileException::GetFileName() const
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
HFCDeviceException::HFCDeviceException(const Utf8String& pi_rDeviceName) :HFCException()
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
Utf8String HFCDeviceException::_BuildMessage(const ImagePPExceptions::StringId& pi_rsID) const
    {
    Utf8PrintfString rawMessage(GetRawMessageFromResource(pi_rsID).c_str(), m_DeviceName.c_str());
    Utf8String exceptionName(pi_rsID.m_str);
    Utf8PrintfString message("%s - [%s]", rawMessage.c_str(), exceptionName.c_str());
    return message;
    }
//-----------------------------------------------------------------------------
// public
// Get the exception device name
//-----------------------------------------------------------------------------
Utf8StringCR HFCDeviceException::GetDeviceName() const
    {
    return m_DeviceName;
    }

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HFCInternetConnectionException::HFCInternetConnectionException(const Utf8String&    pi_rDeviceName,
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
Utf8String HFCInternetConnectionException::GetExceptionMessage() const
    {
    Utf8PrintfString rawMessage(GetRawMessageFromResource(ImagePPExceptions::HFCInternetConnection()).c_str(), m_DeviceName.c_str(), m_ErrorType);
    Utf8String exceptionName(ImagePPExceptions::HFCInternetConnection().m_str);
    Utf8PrintfString message("%s - [%s]", rawMessage.c_str(), exceptionName.c_str());
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
Utf8String HFCCannotCreateSynchroObjException::GetExceptionMessage() const
    {
    Utf8PrintfString rawMessage(GetRawMessageFromResource(ImagePPExceptions::HFCCannotCreateSynchroObj()).c_str(), m_SynchroObject);
    Utf8String exceptionName(ImagePPExceptions::HFCCannotCreateSynchroObj().m_str);
    Utf8PrintfString message("%s - [%s]", rawMessage.c_str(), exceptionName.c_str());
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

