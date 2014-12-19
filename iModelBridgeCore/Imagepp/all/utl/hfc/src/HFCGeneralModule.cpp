//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCGeneralModule.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HFCGeneralModule.h>
#include <Imagepp/all/h/HFCGeneralModuleCodes.h>
#include <Imagepp/all/h/HFCErrorCodeReceiver.h>
#include <Imagepp/all/h/HFCErrorCodeGenerator.h>
#include <Imagepp/all/h/HFCException.h>

//-----------------------------------------------------------------------------
// Global Variable initialization
//-----------------------------------------------------------------------------

const uint32_t HFCGeneralModule::s_ModuleID = 0x000;


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCGeneralModule::HFCGeneralModule()
    : HFCErrorCodeModule(HFCGeneralModule::s_ModuleID)
    {
    // Prepare the dummy error code with the module ID
    HFCErrorCode ErrorCode;
    ErrorCode.SetModuleID(s_ModuleID);

    // add HFCException
    ErrorCode.SetSpecificCode(HFCExceptionBuilder::GetCode());
    AddHandler(ErrorCode, &m_ExceptionBuilder);
    AddBuilderForException(HFCException::CLASS_ID, &m_ExceptionBuilder);

    // add HFC_OBJECT_NOT_IN_FACTORY_EXCEPTION
    ErrorCode.SetSpecificCode(HFCObjectNotInFactoryExceptionBuilder::GetCode());
    AddHandler(ErrorCode, &m_FactoryBuilder);
    AddBuilderForException(HFC_OBJECT_NOT_IN_FACTORY_EXCEPTION, &m_FactoryBuilder);

    // register the module into the generator and handler
    HFCErrorCodeReceiver::GetInstance().AddModule(this);
    HFCErrorCodeGenerator::GetInstance().AddModule(this);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCGeneralModule::~HFCGeneralModule()
    {
    }
