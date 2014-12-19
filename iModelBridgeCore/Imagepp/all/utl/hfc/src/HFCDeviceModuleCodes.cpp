//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCDeviceModuleCodes.cpp $
//:>
//:>  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HFCDeviceModuleCodes.h>
#include <Imagepp/all/h/HFCDeviceModule.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HFCResourceLoader.h>
#include <Imagepp/all/h/ImagePPMessages.xliff.h>


//-----------------------------------------------------------------------------
// Implement the constructors, destructors and code IDs here.
//-----------------------------------------------------------------------------
HFC_ERRORCODE_IMPLEMENT_CLASS2(HFCDeviceExceptionBuilder,\
                               HFCErrorCodeBuilder,\
                               HFCErrorCodeHandler,\
                               HFC_DEVICE_EXCEPTION)

HFC_ERRORCODE_IMPLEMENT_CLASS (HFCNoDiskSpaceLeftExceptionBuilder,\
                               HFCDeviceExceptionBuilder,\
                               HFC_NO_DISK_SPACE_LEFT_EXCEPTION)

HFC_ERRORCODE_IMPLEMENT_CLASS (HFCDeviceAbortExceptionBuilder,\
                               HFCDeviceExceptionBuilder,\
                               HFC_DEVICE_ABORT_EXCEPTION)


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCErrorCode* HFCDeviceExceptionBuilder::BuildFromException(const HFCException& pi_rException) const
    {
    HPRECONDITION(pi_rException.GetID() == HFC_DEVICE_EXCEPTION);

    const HFCDeviceExInfo* pDeviceInfo = (const HFCDeviceExInfo*)pi_rException.GetInfo();

    HAutoPtr<HFCErrorCode> pCode(new HFCErrorCode);

    // Set the fatal flag and the module ID
    pCode->SetFlags(0x1);
    pCode->SetModuleID(HFCDeviceModule::s_ModuleID);

    // Set the module specific error code
    pCode->SetSpecificCode(GetCode());

    // Set the parameter
    pCode->AddParameter(pDeviceInfo->m_DeviceName);

    // Set the message
    pCode->SetMessageText(L"HFCDeviceException");

    return (pCode.release());
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCErrorCode* HFCNoDiskSpaceLeftExceptionBuilder::BuildFromException(const HFCException& pi_rException) const
    {
    HAutoPtr<HFCErrorCode>
    pCode(HFCDeviceExceptionBuilder::BuildFromException(pi_rException));

    // Override the specific code
    pCode->SetSpecificCode(GetCode());

    // Override the message
    pCode->SetMessageText(L"HFCNoDiskSpaceLeftException");

    return (pCode.release());
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCErrorCode* HFCDeviceAbortExceptionBuilder::BuildFromException(const HFCException& pi_rException) const
    {
    HAutoPtr<HFCErrorCode>
    pCode(HFCDeviceExceptionBuilder::BuildFromException(pi_rException));

    // Override the specific code
    pCode->SetSpecificCode(GetCode());

    // Override the message
    pCode->SetMessageText(L"HFCDeviceAbortException");

    return (pCode.release());
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HSTATUS HFCDeviceExceptionBuilder::Handle(const HFCErrorCode& pi_rCode) const
    {
    HPRECONDITION(pi_rCode.GetParameters().size() == 1);
    HFCResourceLoader* stringLoader = HFCResourceLoader::GetInstance();

    // there must be 1 parameter
    if (pi_rCode.GetParameters().size() == 0)
        throw HFCDeviceException(stringLoader->GetString(IDS_MODULE_NoDeviceName)); // No Device Name
    else
        throw HFCDeviceException(pi_rCode.GetParameters().front());

    // never called
    // return (H_ERROR);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HSTATUS HFCNoDiskSpaceLeftExceptionBuilder::Handle(const HFCErrorCode& pi_rCode) const
    {
    HPRECONDITION(pi_rCode.GetParameters().size() == 1);
    HFCResourceLoader* stringLoader = HFCResourceLoader::GetInstance();

    // there must be 1 parameter
    if (pi_rCode.GetParameters().size() == 0)
        throw HFCDeviceException(HFC_NO_DISK_SPACE_LEFT_EXCEPTION, stringLoader->GetString(IDS_MODULE_NoDeviceName)); // No Device Name
    else
        throw HFCDeviceException(HFC_NO_DISK_SPACE_LEFT_EXCEPTION, pi_rCode.GetParameters().front());

    // never called
    // return (H_ERROR);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HSTATUS HFCDeviceAbortExceptionBuilder::Handle(const HFCErrorCode& pi_rCode) const
    {
    HPRECONDITION(pi_rCode.GetParameters().size() == 1);
    HFCResourceLoader* stringLoader = HFCResourceLoader::GetInstance();
    // there must be 1 parameter
    if (pi_rCode.GetParameters().size() == 0)
        throw HFCDeviceException(HFC_DEVICE_ABORT_EXCEPTION, stringLoader->GetString(IDS_MODULE_NoDeviceName)); // No Device Name
    else
        throw HFCDeviceException(HFC_DEVICE_ABORT_EXCEPTION, pi_rCode.GetParameters().front());

    // never called
    // return (H_ERROR);
    }
