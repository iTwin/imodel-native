//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCGeneralModuleCodes.cpp $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HFCGeneralModuleCodes.h>
#include <Imagepp/all/h/HFCGeneralModule.h>
#include <Imagepp/all/h/HFCException.h>


//-----------------------------------------------------------------------------
// Implement the constructors, destructors and code IDs here.
//-----------------------------------------------------------------------------
HFC_ERRORCODE_IMPLEMENT_CLASS2(HFCExceptionBuilder,\
                               HFCErrorCodeBuilder,\
                               HFCErrorCodeHandler,\
                               HFC_EXCEPTION)

HFC_ERRORCODE_IMPLEMENT_CLASS2(HFCObjectNotInFactoryExceptionBuilder,\
                               HFCErrorCodeBuilder,\
                               HFCErrorCodeHandler,\
                               HFC_OBJECT_NOT_IN_FACTORY_EXCEPTION)


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCErrorCode* HFCExceptionBuilder::BuildFromException(const HFCException& pi_rException) const
    {
    HAutoPtr<HFCErrorCode> pCode(new HFCErrorCode);

    // Set the fatal flag and the module ID
    pCode->SetFlags(0x1);
    pCode->SetModuleID(HFCGeneralModule::s_ModuleID);

    // Set the module specific error code
    pCode->SetSpecificCode(GetCode());

    // Set the message
    pCode->SetMessageText(L"HFCException");

    return (pCode.release());
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCErrorCode* HFCObjectNotInFactoryExceptionBuilder::BuildFromException(const HFCException& pi_rException) const
    {
    HAutoPtr<HFCErrorCode> pCode(new HFCErrorCode);

    // Set the fatal flag and the module ID
    pCode->SetFlags(0x1);
    pCode->SetModuleID(HFCGeneralModule::s_ModuleID);

    // Set the module specific error code
    pCode->SetSpecificCode(GetCode());

    // Set the message
    pCode->SetMessageText(L"HFCObjectNotInFactoryException");

    return (pCode.release());
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HSTATUS HFCExceptionBuilder::Handle(const HFCErrorCode& pi_rCode) const
    {
    HPRECONDITION(pi_rCode.GetParameters().size() == 0);

    HASSERT(!L"An more descriptive exception should be used instead");

    // The the exception.  There is no parameters
    throw HFCException();

    // never called
    //return (H_ERROR);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HSTATUS HFCObjectNotInFactoryExceptionBuilder::Handle(const HFCErrorCode& pi_rCode) const
    {
    HPRECONDITION(pi_rCode.GetParameters().size() == 0);

    // The the exception.  There is no parameters
    throw HFCException(HFC_OBJECT_NOT_IN_FACTORY_EXCEPTION);

    // never called
    //return (H_ERROR);
    }