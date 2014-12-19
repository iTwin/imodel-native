//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCFileModule.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HFCFileModule.h>
#include <Imagepp/all/h/HFCFileModuleCodes.h>
#include <Imagepp/all/h/HFCErrorCodeReceiver.h>
#include <Imagepp/all/h/HFCErrorCodeGenerator.h>
#include <Imagepp/all/h/HFCException.h>

//-----------------------------------------------------------------------------
// Global Variable initialization
//-----------------------------------------------------------------------------

const uint32_t HFCFileModule::s_ModuleID = 0x011;


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCFileModule::HFCFileModule()
    : HFCErrorCodeModule(HFCFileModule::s_ModuleID)
    {
    HAutoPtr<HFCFileExceptionBuilder> pBuilder;
    HFCErrorCode ErrorCode;
    ErrorCode.SetModuleID(s_ModuleID);

    // add HFCFileExceptionBuilder
    ErrorCode.SetSpecificCode(HFCFileExceptionBuilder::GetCode());
    pBuilder = new HFCFileExceptionBuilder;
    AddHandler(ErrorCode, pBuilder);
    AddBuilderForException(HFC_FILE_EXCEPTION, pBuilder);
    m_Builders.push_back(pBuilder.release());

    // add HFCCannotLockFileException
    ErrorCode.SetSpecificCode(HFCCannotLockFileExceptionBuilder::GetCode());
    pBuilder = new HFCCannotLockFileExceptionBuilder;
    AddHandler(ErrorCode, pBuilder);
    AddBuilderForException(HFC_CANNOT_LOCK_FILE_EXCEPTION, pBuilder);
    m_Builders.push_back(pBuilder.release());

    // add HFCFileCorruptedException
    ErrorCode.SetSpecificCode(HFCFileCorruptedExceptionBuilder::GetCode());
    pBuilder = new HFCFileCorruptedExceptionBuilder;
    AddHandler(ErrorCode, pBuilder);
    AddBuilderForException(HFC_CORRUPTED_FILE_EXCEPTION, pBuilder);
    m_Builders.push_back(pBuilder.release());

    // add HFCFileNotFoundException
    ErrorCode.SetSpecificCode(HFCFileNotFoundExceptionBuilder::GetCode());
    pBuilder = new HFCFileNotFoundExceptionBuilder;
    AddHandler(ErrorCode, pBuilder);
    AddBuilderForException(HFC_FILE_NOT_FOUND_EXCEPTION, pBuilder);
    m_Builders.push_back(pBuilder.release());

    // add HFC_FILE_EXIST_EXCEPTION
    ErrorCode.SetSpecificCode(HFCFileExistExceptionBuilder::GetCode());
    pBuilder = new HFCFileExistExceptionBuilder;
    AddHandler(ErrorCode, pBuilder);
    AddBuilderForException(HFC_FILE_EXIST_EXCEPTION, pBuilder);
    m_Builders.push_back(pBuilder.release());

    // add HFC_FILE_READ_ONLY_EXCEPTION
    ErrorCode.SetSpecificCode(HFCFileReadOnlyExceptionBuilder::GetCode());
    pBuilder = new HFCFileReadOnlyExceptionBuilder;
    AddHandler(ErrorCode, pBuilder);
    AddBuilderForException(HFC_FILE_READ_ONLY_EXCEPTION, pBuilder);
    m_Builders.push_back(pBuilder.release());

    // add HFCFileNotCreatedException
    ErrorCode.SetSpecificCode(HFCFileNotCreatedExceptionBuilder::GetCode());
    pBuilder = new HFCFileNotCreatedExceptionBuilder;
    AddHandler(ErrorCode, pBuilder);
    AddBuilderForException(HFC_FILE_NOT_CREATED_EXCEPTION, pBuilder);
    m_Builders.push_back(pBuilder.release());

    // add HFC_FILE_NOT_SUPPORTED_EXCEPTION
    ErrorCode.SetSpecificCode(HFCFileNotSupportedExceptionBuilder::GetCode());
    pBuilder = new HFCFileNotSupportedExceptionBuilder;
    AddHandler(ErrorCode, pBuilder);
    AddBuilderForException(HFC_FILE_NOT_SUPPORTED_EXCEPTION, pBuilder);
    m_Builders.push_back(pBuilder.release());

    // add HFC_FILE_OUT_OF_RANGE_EXCEPTION
    ErrorCode.SetSpecificCode(HFCFileOutOfRangeExceptionBuilder::GetCode());
    pBuilder = new HFCFileOutOfRangeExceptionBuilder;
    AddHandler(ErrorCode, pBuilder);
    AddBuilderForException(HFC_FILE_OUT_OF_RANGE_EXCEPTION, pBuilder);
    m_Builders.push_back(pBuilder.release());

    // add HFC_FILE_PERMISSION_DENIED_EXCEPTION
    ErrorCode.SetSpecificCode(HFCFilePermissionDeniedExceptionBuilder::GetCode());
    pBuilder = new HFCFilePermissionDeniedExceptionBuilder;
    AddHandler(ErrorCode, pBuilder);
    AddBuilderForException(HFC_FILE_PERMISSION_DENIED_EXCEPTION, pBuilder);
    m_Builders.push_back(pBuilder.release());

    // register the module into the generator and handler
    HFCErrorCodeReceiver::GetInstance().AddModule(this);
    HFCErrorCodeGenerator::GetInstance().AddModule(this);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCFileModule::~HFCFileModule()
    {
    for (Builders::iterator Itr = m_Builders.begin(); Itr != m_Builders.end(); Itr++)
        delete (*Itr);
    }
