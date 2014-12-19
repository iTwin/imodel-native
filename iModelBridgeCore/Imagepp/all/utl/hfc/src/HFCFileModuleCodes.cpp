//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCFileModuleCodes.cpp $
//:>
//:>  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HFCFileModuleCodes.h>
#include <Imagepp/all/h/HFCFileModule.h>
#include <Imagepp/all/h/HFCException.h>

#include <Imagepp/all/h/HFCResourceLoader.h>
#include <Imagepp/all/h/ImagePPMessages.xliff.h>


//-----------------------------------------------------------------------------
// Implement the constructors, destructors and code IDs here.
//-----------------------------------------------------------------------------
HFC_ERRORCODE_IMPLEMENT_CLASS2(HFCFileExceptionBuilder,\
                               HFCErrorCodeBuilder,\
                               HFCErrorCodeHandler,\
                               HFC_FILE_EXCEPTION)

HFC_ERRORCODE_IMPLEMENT_CLASS(HFCCannotLockFileExceptionBuilder,\
                              HFCFileExceptionBuilder,\
                              HFC_CANNOT_LOCK_FILE_EXCEPTION)

HFC_ERRORCODE_IMPLEMENT_CLASS(HFCFileCorruptedExceptionBuilder,\
                              HFCFileExceptionBuilder,\
                              HFC_CORRUPTED_FILE_EXCEPTION)

HFC_ERRORCODE_IMPLEMENT_CLASS(HFCFileNotFoundExceptionBuilder,\
                              HFCFileExceptionBuilder,\
                              HFC_FILE_NOT_FOUND_EXCEPTION)

HFC_ERRORCODE_IMPLEMENT_CLASS(HFCFileExistExceptionBuilder,\
                              HFCFileExceptionBuilder,\
                              HFC_FILE_EXIST_EXCEPTION)

HFC_ERRORCODE_IMPLEMENT_CLASS(HFCFileReadOnlyExceptionBuilder,\
                              HFCFileExceptionBuilder,\
                              HFC_FILE_READ_ONLY_EXCEPTION)

HFC_ERRORCODE_IMPLEMENT_CLASS(HFCFileNotCreatedExceptionBuilder,\
                              HFCFileExceptionBuilder,\
                              HFC_FILE_NOT_CREATED_EXCEPTION)

HFC_ERRORCODE_IMPLEMENT_CLASS(HFCFileNotSupportedExceptionBuilder,\
                              HFCFileExceptionBuilder,\
                              HFC_FILE_NOT_SUPPORTED_EXCEPTION)

HFC_ERRORCODE_IMPLEMENT_CLASS(HFCFileOutOfRangeExceptionBuilder,\
                              HFCFileExceptionBuilder,\
                              HFC_FILE_OUT_OF_RANGE_EXCEPTION)

HFC_ERRORCODE_IMPLEMENT_CLASS(HFCFilePermissionDeniedExceptionBuilder,\
                              HFCFileExceptionBuilder,\
                              HFC_FILE_PERMISSION_DENIED_EXCEPTION)


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCErrorCode* HFCFileExceptionBuilder::BuildFromException(const HFCException& pi_rException) const
    {
    HPRECONDITION(pi_rException.IsCompatibleWith(HFCFileException::CLASS_ID));
    const HFCFileExInfo* pFileExInfo = (const HFCFileExInfo*)pi_rException.GetInfo();

    HAutoPtr<HFCErrorCode> pCode(new HFCErrorCode);

    // Set the fatal flag and the module ID
    pCode->SetFlags(0x1);
    pCode->SetModuleID(HFCFileModule::s_ModuleID);

    // Set the module specific error code
    pCode->SetSpecificCode(GetCode());

    // Set the parameter
    pCode->AddParameter(pFileExInfo->m_FileName);

    // Set the message
    pCode->SetMessageText(L"HFCFileException");

    return (pCode.release());
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCErrorCode* HFCCannotLockFileExceptionBuilder::BuildFromException(const HFCException& pi_rException) const
    {
    HAutoPtr<HFCErrorCode>
    pCode(HFCFileExceptionBuilder::BuildFromException(pi_rException));

    // Override the specific code
    pCode->SetSpecificCode(GetCode());

    // Override the message
    pCode->SetMessageText(L"HFCCannotLockFileException");

    return (pCode.release());
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCErrorCode* HFCFileCorruptedExceptionBuilder::BuildFromException(const HFCException& pi_rException) const
    {
    HAutoPtr<HFCErrorCode>
    pCode(HFCFileExceptionBuilder::BuildFromException(pi_rException));

    // Override the specific code
    pCode->SetSpecificCode(GetCode());

    // Override the message
    pCode->SetMessageText(L"HFCFileCorruptedException");

    return (pCode.release());
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCErrorCode* HFCFileNotFoundExceptionBuilder::BuildFromException(const HFCException& pi_rException) const
    {
    HAutoPtr<HFCErrorCode>
    pCode(HFCFileExceptionBuilder::BuildFromException(pi_rException));

    // Override the specific code
    pCode->SetSpecificCode(GetCode());

    // Override the message
    pCode->SetMessageText(L"HFCFileNotFoundException");

    return (pCode.release());
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCErrorCode* HFCFileExistExceptionBuilder::BuildFromException(const HFCException& pi_rException) const
    {
    HAutoPtr<HFCErrorCode>
    pCode(HFCFileExceptionBuilder::BuildFromException(pi_rException));

    // Override the specific code
    pCode->SetSpecificCode(GetCode());

    // Override the message
    pCode->SetMessageText(L"HFCFileExistException");

    return (pCode.release());
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCErrorCode* HFCFileReadOnlyExceptionBuilder::BuildFromException(const HFCException& pi_rException) const
    {
    HAutoPtr<HFCErrorCode>
    pCode(HFCFileExceptionBuilder::BuildFromException(pi_rException));

    // Override the specific code
    pCode->SetSpecificCode(GetCode());

    // Override the message
    pCode->SetMessageText(L"HFCFileReadOnlyException");

    return (pCode.release());
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCErrorCode* HFCFileNotCreatedExceptionBuilder::BuildFromException(const HFCException& pi_rException) const
    {
    HAutoPtr<HFCErrorCode>
    pCode(HFCFileExceptionBuilder::BuildFromException(pi_rException));

    // Override the specific code
    pCode->SetSpecificCode(GetCode());

    // Override the message
    pCode->SetMessageText(L"HFCFileNotCreatedException");

    return (pCode.release());
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCErrorCode* HFCFileNotSupportedExceptionBuilder::BuildFromException(const HFCException& pi_rException) const
    {
    HAutoPtr<HFCErrorCode>
    pCode(HFCFileExceptionBuilder::BuildFromException(pi_rException));

    // Override the specific code
    pCode->SetSpecificCode(GetCode());

    // Override the message
    pCode->SetMessageText(L"HFCFileNotSupportedException");

    return (pCode.release());
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCErrorCode* HFCFileOutOfRangeExceptionBuilder::BuildFromException(const HFCException& pi_rException) const
    {
    HAutoPtr<HFCErrorCode>
    pCode(HFCFileExceptionBuilder::BuildFromException(pi_rException));

    // Override the specific code
    pCode->SetSpecificCode(GetCode());

    // Override the message
    pCode->SetMessageText(L"HFCFileOutOfRangeException");

    return (pCode.release());
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCErrorCode* HFCFilePermissionDeniedExceptionBuilder::BuildFromException(const HFCException& pi_rException) const
    {
    HAutoPtr<HFCErrorCode>
    pCode(HFCFileExceptionBuilder::BuildFromException(pi_rException));

    // Override the specific code
    pCode->SetSpecificCode(GetCode());

    // Override the message
    pCode->SetMessageText(L"HFCFilePermissionDeniedException");

    return (pCode.release());
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HSTATUS HFCFileExceptionBuilder::Handle(const HFCErrorCode& pi_rCode) const
    {
    HPRECONDITION(pi_rCode.GetParameters().size() == 1);
    //HFCResourceLoader* stringLoader = HFCResourceLoader::GetInstance();

    // there must be 1 parameter
    if (pi_rCode.GetParameters().size() == 0)
        throw HFCFileException(HFC_FILE_EXCEPTION, HFCResourceLoader::GetInstance()->GetString(IDS_MODULE_NoFile)); //No file
    else
        throw HFCFileException(HFC_FILE_EXCEPTION, pi_rCode.GetParameters().front());

    // never called
    // return (H_ERROR);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HSTATUS HFCCannotLockFileExceptionBuilder::Handle(const HFCErrorCode& pi_rCode) const
    {
    HPRECONDITION(pi_rCode.GetParameters().size() == 1);
    HFCResourceLoader* stringLoader = HFCResourceLoader::GetInstance();

    // there must be 1 parameter
    if (pi_rCode.GetParameters().size() == 0)
        throw HFCFileException(HFC_CANNOT_LOCK_FILE_EXCEPTION,
                               stringLoader->GetString(IDS_MODULE_NoFile)); // No file
    else
        throw HFCFileException(HFC_CANNOT_LOCK_FILE_EXCEPTION,
                               pi_rCode.GetParameters().front());

    // never called
    // return (H_ERROR);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HSTATUS HFCFileCorruptedExceptionBuilder::Handle(const HFCErrorCode& pi_rCode) const
    {
    HPRECONDITION(pi_rCode.GetParameters().size() == 1);
    HFCResourceLoader* stringLoader = HFCResourceLoader::GetInstance();

    // there must be 1 parameter
    if (pi_rCode.GetParameters().size() == 0)
        throw HFCFileException(HFC_CORRUPTED_FILE_EXCEPTION, stringLoader->GetString(IDS_MODULE_NoFile));  //No file
    else
        throw HFCFileException(HFC_CORRUPTED_FILE_EXCEPTION, pi_rCode.GetParameters().front());

    // never called
    // return (H_ERROR);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HSTATUS HFCFileNotFoundExceptionBuilder::Handle(const HFCErrorCode& pi_rCode) const
    {
    HPRECONDITION(pi_rCode.GetParameters().size() == 1);
    HFCResourceLoader* stringLoader = HFCResourceLoader::GetInstance();

    // there must be 1 parameter
    if (pi_rCode.GetParameters().size() == 0)
        throw HFCFileException(HFC_CORRUPTED_FILE_EXCEPTION, stringLoader->GetString(IDS_MODULE_NoFile)); // No file
    else
        throw HFCFileException(HFC_CORRUPTED_FILE_EXCEPTION, pi_rCode.GetParameters().front());

    // never called
    // return (H_ERROR);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HSTATUS HFCFileExistExceptionBuilder::Handle(const HFCErrorCode& pi_rCode) const
    {
    HPRECONDITION(pi_rCode.GetParameters().size() == 1);
    HFCResourceLoader* stringLoader = HFCResourceLoader::GetInstance();

    // there must be 1 parameter
    if (pi_rCode.GetParameters().size() == 0)
        throw HFCFileException(HFC_FILE_EXIST_EXCEPTION, stringLoader->GetString(IDS_MODULE_NoFile)); // No file
    else
        throw HFCFileException(HFC_FILE_EXIST_EXCEPTION, pi_rCode.GetParameters().front());

    // never called
    // return (H_ERROR);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HSTATUS HFCFileReadOnlyExceptionBuilder::Handle(const HFCErrorCode& pi_rCode) const
    {
    HPRECONDITION(pi_rCode.GetParameters().size() == 1);
    HFCResourceLoader* stringLoader = HFCResourceLoader::GetInstance();

    // there must be 1 parameter
    if (pi_rCode.GetParameters().size() == 0)
        throw HFCFileException(HFC_FILE_READ_ONLY_EXCEPTION, stringLoader->GetString(IDS_MODULE_NoFile)); //No file
    else
        throw HFCFileException(HFC_FILE_READ_ONLY_EXCEPTION, pi_rCode.GetParameters().front());

    // never called
    // return (H_ERROR);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HSTATUS HFCFileNotCreatedExceptionBuilder::Handle(const HFCErrorCode& pi_rCode) const
    {
    HPRECONDITION(pi_rCode.GetParameters().size() == 1);
    HFCResourceLoader* stringLoader = HFCResourceLoader::GetInstance();

    // there must be 1 parameter
    if (pi_rCode.GetParameters().size() == 0)
        throw HFCFileException(HFC_FILE_NOT_CREATED_EXCEPTION, stringLoader->GetString(IDS_MODULE_NoFile)); // No file
    else
        throw HFCFileException(HFC_FILE_NOT_CREATED_EXCEPTION, pi_rCode.GetParameters().front());

    // never called
    // return (H_ERROR);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HSTATUS HFCFileNotSupportedExceptionBuilder::Handle(const HFCErrorCode& pi_rCode) const
    {
    HPRECONDITION(pi_rCode.GetParameters().size() == 1);
    HFCResourceLoader* stringLoader = HFCResourceLoader::GetInstance();

    // there must be 1 parameter
    if (pi_rCode.GetParameters().size() == 0)
        throw HFCFileException(HFC_FILE_NOT_SUPPORTED_EXCEPTION, stringLoader->GetString(IDS_MODULE_NoFile)); // No file
    else
        throw HFCFileException(HFC_FILE_NOT_SUPPORTED_EXCEPTION, pi_rCode.GetParameters().front());

    // never called
    // return (H_ERROR);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HSTATUS HFCFileOutOfRangeExceptionBuilder::Handle(const HFCErrorCode& pi_rCode) const
    {
    HPRECONDITION(pi_rCode.GetParameters().size() == 1);
    HFCResourceLoader* stringLoader = HFCResourceLoader::GetInstance();

    // there must be 1 parameter
    if (pi_rCode.GetParameters().size() == 0)
        throw HFCFileException(HFC_FILE_OUT_OF_RANGE_EXCEPTION, stringLoader->GetString(IDS_MODULE_NoFile)); // No file
    else
        throw HFCFileException(HFC_FILE_OUT_OF_RANGE_EXCEPTION, pi_rCode.GetParameters().front());

    // never called
    // return (H_ERROR);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HSTATUS HFCFilePermissionDeniedExceptionBuilder::Handle(const HFCErrorCode& pi_rCode) const
    {
    HPRECONDITION(pi_rCode.GetParameters().size() == 1);
    HFCResourceLoader* stringLoader = HFCResourceLoader::GetInstance();

    // there must be 1 parameter
    if (pi_rCode.GetParameters().size() == 0)
        throw HFCFileException(HFC_FILE_PERMISSION_DENIED_EXCEPTION, stringLoader->GetString(IDS_MODULE_NoFile)); // No file
    else
        throw HFCFileException(HFC_FILE_PERMISSION_DENIED_EXCEPTION, pi_rCode.GetParameters().front());

    // never called
    // return (H_ERROR);
    }


