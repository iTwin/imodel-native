//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCException.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Implementation of the exception classes.  The exception hierarchy look
// like this:
//
//  HFCException ABSTRACT                   
//      HFCFileException ABSTRACT 
//            HFCGenericException
//            HFCCannotLockFileException
//            HFCFileLockViolationException
//            HFCCorruptedFileException
//            HFCFileNotFoundException
//            HFCFileExistException
//            HFCFileReadOnlyException
//            HFCFileNotCreatedException
//            HFCFileNotSupportedException
//            HFCFileOutOfRangeException
//            HFCFilePermissionDeniedException
//            HFCDllNotFoundException
//            HFCInvalidFileNameException
//            HFCSharingViolationException
//            HFCCannotUnlockException
//            HFCCannotOpenFileException
//            HFCCannotConnectToDBException
//            HFCInvalidUrlForSisterFileException
//            HFCDirectoryReadOnlyException
//            HFCSisterFileNotCreatedException
//            HFCDirectoryNotCreatedException
//            HFCCannotCloseFileException
//            HFCWriteFaultException
//            HFCReadFaultException
//            HFCFileIOErrorException
//            HFCFileNotSupportedInWriteModeException
//      HFCDeviceException ABSTRACT                 
//            HFCInternetConnectionException 
//            HFCUndefinedDeviceException
//            HFCNoDiskSpaceLeftException
//            HFCDeviceAbortException
//      HFCCannotCreateSynchroObjException  
//      HFCErrorCodeException
//      HFCUnknownException
//      HFCUndefinedException
//      HFCMemoryException
//      HFCOutOfMemoryException
//      HFCObjectNotInFactoryException
//      HFCHttpRequestStringException
//      HFCLoginInformationNotAvailableException
//----------------------------------------------------------------------------

#pragma once


#include <Imagepp/all/h/HFCMacros.h>
#include <Imagepp/h/ImagePPExceptionMessages.xliff.h>

BEGIN_IMAGEPP_NAMESPACE

class HPANode;
class HFCException;

//----------------------------------------------------------------------------
// Class HFCException ABSTRACT
//----------------------------------------------------------------------------
class HFCException
    {
public :
    IMAGEPP_EXPORT virtual ~HFCException();   
    virtual WString GetExceptionMessage() const = 0;
    virtual HFCException* Clone() const = 0;
    virtual void ThrowMyself() const = 0;
protected :
    //Those constructors are protected to make sure we always throw a specific exception and don't lose type information
    IMAGEPP_EXPORT HFCException();
    IMAGEPP_EXPORT HFCException(const HFCException& pi_rObj); 

    IMAGEPP_EXPORT WString GetRawMessageFromResource(const ImagePPExceptions::StringId& pi_ID) const;

    IMAGEPP_EXPORT virtual WString _BuildMessage(const ImagePPExceptions::StringId& pi_ID) const;
private:
    HFCException&               operator=                      (const HFCException&     pi_rObj);
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                   Julien.Rossignol 07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
template<ImagePPExceptions::StringId (*GetStringId)()>
class HFCException_T : public HFCException
    {
    public:
    HFCException_T() : HFCException(){}
    HFCException_T (const HFCException_T& pi_rObj) : HFCException(pi_rObj){}
    virtual HFCException* Clone() const override {return new HFCException_T(*this);}
    virtual void ThrowMyself() const override {throw *this;} 
    virtual WString GetExceptionMessage() const override
        {
        return HFCException::_BuildMessage(GetStringId());
        }
    };
typedef HFCException_T<ImagePPExceptions::Unknown> HFCUnknownException;
typedef HFCException_T<ImagePPExceptions::HFCUndefined> HFCUndefinedException;
typedef HFCException_T<ImagePPExceptions::HFCMemory> HFCMemoryException;
typedef HFCException_T<ImagePPExceptions::HFCOutOfMemory> HFCOutOfMemoryException;
typedef HFCException_T<ImagePPExceptions::HFCObjectNotInFactory> HFCObjectNotInFactoryException;
typedef HFCException_T<ImagePPExceptions::HFCHtppRequestString> HFCHttpRequestStringException;
typedef HFCException_T<ImagePPExceptions::HFCLoginInformationNotAvailable> HFCLoginInformationNotAvailableException;
typedef HFCException_T<ImagePPExceptions::HFCUnsupportedFileVersion> HFCUnsupportedFileVersionException;

//----------------------------------------------------------------------------
// Class HFCFileException ABSTRACT
//----------------------------------------------------------------------------
class HFCFileException : public HFCException
    {
public :
    IMAGEPP_EXPORT virtual ~HFCFileException ();
    IMAGEPP_EXPORT WStringCR GetFileName () const;
    IMAGEPP_EXPORT virtual bool IsInvalidAccess() const;
protected : 
    //Those constructors are protected to make sure we always throw a specific exception and don't lose type information
    IMAGEPP_EXPORT HFCFileException (WStringCR pi_rFileName);
    IMAGEPP_EXPORT HFCFileException (const HFCFileException&     pi_rObj); 
    WString m_FileName;
    IMAGEPP_EXPORT virtual WString _BuildMessage(const ImagePPExceptions::StringId& rsID) const override;
    };
/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                   Julien.Rossignol 07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
template<ImagePPExceptions::StringId (*GetStringId)(), bool IsInvalidAccessMode>
class HFCFileException_T : public HFCFileException
{
public:
    HFCFileException_T(WStringCR pi_rFileName) : HFCFileException(pi_rFileName){}
    HFCFileException_T (const HFCFileException_T& pi_rObj) : HFCFileException(pi_rObj){} 
    virtual HFCException* Clone() const override {return new HFCFileException_T(*this);}
    virtual void ThrowMyself() const override {throw *this;} 
    virtual bool IsInvalidAccess() const override {return IsInvalidAccessMode;} 
    virtual WString GetExceptionMessage() const override
        {
        return HFCFileException::_BuildMessage(GetStringId());
        }
};

typedef HFCFileException_T<ImagePPExceptions::HFCGenericFile, false> HFCGenericFileException;
typedef HFCFileException_T<ImagePPExceptions::HFCCannotLockFile, true> HFCCannotLockFileException;
typedef HFCFileException_T<ImagePPExceptions::HFCFileLockViolation, true> HFCFileLockViolationException;
typedef HFCFileException_T<ImagePPExceptions::HFCCorruptedFile, false> HFCCorruptedFileException;
typedef HFCFileException_T<ImagePPExceptions::HFCFileNotFound, false> HFCFileNotFoundException;
typedef HFCFileException_T<ImagePPExceptions::HFCFileExist, false> HFCFileExistException;
typedef HFCFileException_T<ImagePPExceptions::HFCFileReadOnly, true> HFCFileReadOnlyException;
typedef HFCFileException_T<ImagePPExceptions::HFCFileNotCreated, false> HFCFileNotCreatedException;
typedef HFCFileException_T<ImagePPExceptions::HFCFileNotSupported, false> HFCFileNotSupportedException;
typedef HFCFileException_T<ImagePPExceptions::HFCFileOutOfRange, false> HFCFileOutOfRangeException;
typedef HFCFileException_T<ImagePPExceptions::HFCFilePermissionDenied, true> HFCFilePermissionDeniedException;
typedef HFCFileException_T<ImagePPExceptions::HFCDllNotFound, false> HFCDllNotFoundException;
typedef HFCFileException_T<ImagePPExceptions::HFCInvalidFileName, false> HFCInvalidFileNameException;
typedef HFCFileException_T<ImagePPExceptions::HFCSharingViolation, true> HFCSharingViolationException;
typedef HFCFileException_T<ImagePPExceptions::HFCCannotUnlock, false> HFCCannotUnlockException;
typedef HFCFileException_T<ImagePPExceptions::HFCCannotOpenFile, false> HFCCannotOpenFileException;
typedef HFCFileException_T<ImagePPExceptions::HFCCannotConnectToDB, false> HFCCannotConnectToDBException;
typedef HFCFileException_T<ImagePPExceptions::HFCInvalidUrlForSisterFile, false> HFCInvalidUrlForSisterFileException;
typedef HFCFileException_T<ImagePPExceptions::HFCDirectoryReadOnly, false> HFCDirectoryReadOnlyException;
typedef HFCFileException_T<ImagePPExceptions::HFCSisterFileNotCreated, false> HFCSisterFileNotCreatedException;
typedef HFCFileException_T<ImagePPExceptions::HFCDirectoryNotCreated, false> HFCDirectoryNotCreatedException;
typedef HFCFileException_T<ImagePPExceptions::HFCCannotCloseFile, false> HFCCannotCloseFileException;
typedef HFCFileException_T<ImagePPExceptions::HFCWriteFault, false> HFCWriteFaultException;
typedef HFCFileException_T<ImagePPExceptions::HFCReadFault, false> HFCReadFaultException;
typedef HFCFileException_T<ImagePPExceptions::HFCFileIOError, false> HFCFileIOErrorException;
typedef HFCFileException_T<ImagePPExceptions::HFCFileNotSupportedInWriteMode, true> HFCFileNotSupportedInWriteModeException; 

//----------------------------------------------------------------------------
// Class HFCDeviceException ABSTRACT
//----------------------------------------------------------------------------
class HFCDeviceException : public HFCException 
{
public :
    IMAGEPP_EXPORT virtual ~HFCDeviceException();
    IMAGEPP_EXPORT WStringCR GetDeviceName() const;
protected :
    //Those constructors are protected to make sure we always throw a specific exception and don't lose type information
    IMAGEPP_EXPORT HFCDeviceException (const HFCDeviceException&     pi_rObj); 
    IMAGEPP_EXPORT HFCDeviceException(const WString& pi_rDeviceName);
    WString m_DeviceName;    
    IMAGEPP_EXPORT virtual WString _BuildMessage(const ImagePPExceptions::StringId& pi_ID) const;
};

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                   Julien.Rossignol 07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
template<ImagePPExceptions::StringId (*GetStringId)()>
class HFCDeviceException_T : public HFCDeviceException
{
public:
    HFCDeviceException_T(const WString& pi_rDeviceName) : HFCDeviceException(pi_rDeviceName){}
    HFCDeviceException_T (const HFCDeviceException_T& pi_rObj) : HFCDeviceException(pi_rObj){}
    virtual HFCException* Clone() const override {return new HFCDeviceException_T(*this);}
    virtual void ThrowMyself() const override {throw *this;} 
    virtual WString GetExceptionMessage() const override
        {
        return HFCDeviceException::_BuildMessage(GetStringId());
        }
};
typedef HFCDeviceException_T<ImagePPExceptions::HFCUndefinedDevice> HFCUndefinedDeviceException;
typedef HFCDeviceException_T<ImagePPExceptions::HFCNoDiskSpaceLeft> HFCNoDiskSpaceLeftException;
typedef HFCDeviceException_T<ImagePPExceptions::HFCDeviceAbort> HFCDeviceAbortException;

//-----------------------------------------------------------------------------
// HFCInternetConnectionException class
//-----------------------------------------------------------------------------
class HFCInternetConnectionException : public HFCDeviceException
    {
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
    IMAGEPP_EXPORT HFCInternetConnectionException(const WString&    pi_rDeviceName,
                    ErrorType        pi_ErrorType);
    IMAGEPP_EXPORT virtual         ~HFCInternetConnectionException();
    IMAGEPP_EXPORT const ErrorType     GetErrorType() const;
    IMAGEPP_EXPORT HFCInternetConnectionException                   (const HFCInternetConnectionException&     pi_rObj); 
    IMAGEPP_EXPORT virtual WString GetExceptionMessage() const override;
    virtual HFCException* Clone() const override;
    virtual void ThrowMyself() const override {throw *this;} 
protected:
    ErrorType            m_ErrorType;        
    };

//-----------------------------------------------------------------------------
// HFCCannotCreateSynchroObjException class
//-----------------------------------------------------------------------------
class HFCCannotCreateSynchroObjException : public HFCException
    {
public :

    enum SynchroObject
        {
        MUTEX = 0,
        SEMAPHORE,
        EVENT
        };
    
    HFCCannotCreateSynchroObjException(HFCCannotCreateSynchroObjException::SynchroObject pi_SynchroObj);
    virtual ~HFCCannotCreateSynchroObjException();
    const SynchroObject GetSynchroObject() const;
    HFCCannotCreateSynchroObjException(const HFCCannotCreateSynchroObjException&     pi_rObj); 
    virtual WString GetExceptionMessage() const override;
    virtual HFCException* Clone() const override;
    virtual void ThrowMyself() const override {throw *this;} 
protected:
    SynchroObject    m_SynchroObject;    
};

END_IMAGEPP_NAMESPACE
