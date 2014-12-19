//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCMemoryModuleCodes.cpp $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HFCMemoryModuleCodes.h>
#include <Imagepp/all/h/HFCMemoryModule.h>
#include <Imagepp/all/h/HFCException.h>


//-----------------------------------------------------------------------------
// Implement the constructors, destructors and code IDs here.
//-----------------------------------------------------------------------------
HFC_ERRORCODE_IMPLEMENT_CLASS2(HFCMemoryExceptionBuilder,\
                               HFCErrorCodeBuilder,\
                               HFCErrorCodeHandler,\
                               HFC_MEMORY_EXCEPTION)

HFC_ERRORCODE_IMPLEMENT_CLASS (HFCOutOfMemoryExceptionBuilder,\
                               HFCMemoryExceptionBuilder,\
                               HFC_OUT_OF_MEMORY_EXCEPTION)


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCErrorCode* HFCMemoryExceptionBuilder::BuildFromException(const HFCException& pi_rException) const
    {
    HAutoPtr<HFCErrorCode> pCode(new HFCErrorCode);

    // Set the fatal flag and the module ID
    pCode->SetFlags(0x1);
    pCode->SetModuleID(HFCMemoryModule::s_ModuleID);

    // Set the module specific error code
    pCode->SetSpecificCode(GetCode());

    // Set the message
    pCode->SetMessageText(L"HFCMemoryException");

    return (pCode.release());
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCErrorCode* HFCOutOfMemoryExceptionBuilder::BuildFromException(const HFCException& pi_rException) const
    {
    HAutoPtr<HFCErrorCode>
    pCode(HFCMemoryExceptionBuilder::BuildFromException(pi_rException));

    // Override the specific code
    pCode->SetSpecificCode(GetCode());

    // Override the message
    pCode->SetMessageText(L"HFCOutOfMemoryException");

    return (pCode.release());
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HSTATUS HFCMemoryExceptionBuilder::Handle(const HFCErrorCode& pi_rCode) const
    {
    HPRECONDITION(pi_rCode.GetParameters().size() == 0);

    // The the exception.  There is no parameters
    throw HFCException(HFC_MEMORY_EXCEPTION);

    // never called
    // return (H_ERROR);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HSTATUS HFCOutOfMemoryExceptionBuilder::Handle(const HFCErrorCode& pi_rCode) const
    {
    HPRECONDITION(pi_rCode.GetParameters().size() == 0);

    // The the exception.  There is no parameters
    throw HFCException(HFC_OUT_OF_MEMORY_EXCEPTION);

    // never called
    // return (H_ERROR);
    }
