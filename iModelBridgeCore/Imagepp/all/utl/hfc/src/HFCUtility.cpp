//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCUtility.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for exception classes.
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HFCUtility.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HFCResourceLoader.h>
#include <Imagepp/all/h/HFCMacros.h>

#include <Bentley/BeFile.h>

#if !defined (_WIN32)
#error "platform is not supported"
#endif 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
HRESULT static translateBeFileStatusToHresult (BeFileStatus status)
    {
    switch (status)
        {
        case BeFileStatus::Success:
            return S_OK;

        case BeFileStatus::SharingViolationError:   
            return ERROR_SHARING_VIOLATION;

        case BeFileStatus::AccessViolationError:
            return ERROR_ACCESS_DENIED;

        case BeFileStatus::TooManyOpenFilesError:
            return ERROR_TOO_MANY_OPEN_FILES;

        case BeFileStatus::FileNotFoundError:
            return ERROR_FILE_NOT_FOUND;

        case BeFileStatus::NotLockedError:
            return ERROR_NOT_LOCKED;
        }

    return ERROR_OPEN_FAILED;
    }

void MapHFCFileExceptionFromBeFileStatus(BeFileStatus    pi_Status,
                                         const WString&  pi_rFileName,
                                         ExceptionID     pi_DefaultException,
                                         ExceptionID*    po_pMappedHFCException)
    {
    MapHFCFileExceptionFromGetLastError(translateBeFileStatusToHresult(pi_Status), pi_rFileName, pi_DefaultException, po_pMappedHFCException);
    }


void MapHFCFileExceptionFromGetLastError(uint32_t        pi_LastError,
                                         const WString&  pi_rFileName,
                                         ExceptionID     pi_DefaultException,
                                         ExceptionID*    po_pMappedHFCException)
    {
    ExceptionID fileExceptionID = pi_DefaultException;

    switch (pi_LastError)
        {
        case ERROR_SHARING_VIOLATION:
            fileExceptionID = HFC_SHARING_VIOLATION_EXCEPTION;
            break;
        case ERROR_READ_FAULT:
            fileExceptionID = HFC_READ_FAULT_EXCEPTION;
            break;

        case ERROR_LOCK_VIOLATION:
            fileExceptionID = HFC_LOCK_VIOLATION_FILE_EXCEPTION;
            break;

        case ERROR_ACCESS_DENIED:
            fileExceptionID = HFC_FILE_PERMISSION_DENIED_EXCEPTION;
            break;

        case ERROR_FILE_CORRUPT:
        case ERROR_FILE_INVALID:
            fileExceptionID = HFC_CORRUPTED_FILE_EXCEPTION;
            break;

        case ERROR_FILE_EXISTS:
            fileExceptionID = HFC_FILE_EXIST_EXCEPTION;
            break;

        case ERROR_INVALID_NAME:
        case ERROR_BAD_PATHNAME:   //?
            fileExceptionID = HFC_INVALID_FILE_NAME_EXCEPTION;
            break;

        case ERROR_PATH_NOT_FOUND:
        case ERROR_BAD_NETPATH:
        case ERROR_FILE_NOT_FOUND:
            fileExceptionID = HFC_FILE_NOT_FOUND_EXCEPTION;
            break;

        case ERROR_DISK_FULL:
            fileExceptionID = HFC_NO_DISK_SPACE_LEFT_EXCEPTION;
            break;

        default:
            fileExceptionID = pi_DefaultException;
            break;
        }


    if (po_pMappedHFCException == 0)
        {
        throw HFCFileException(fileExceptionID, pi_rFileName);
        }

    *po_pMappedHFCException = fileExceptionID;
    }