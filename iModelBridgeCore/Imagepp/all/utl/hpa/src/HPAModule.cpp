//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hpa/src/HPAModule.cpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HPAModule.h>
#include <Imagepp/all/h/HPAModuleCodes.h>
#include <Imagepp/all/h/HFCErrorCodeReceiver.h>
#include <Imagepp/all/h/HFCErrorCodeGenerator.h>
#include <Imagepp/all/h/HPAException.h>

//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HPAModule::HPAModule(const HFCErrorCodeID& pi_rID)
    : HFCErrorCodeModule(pi_rID)
    {
    // add the handler for all
    HFCErrorCode ErrorCode;
    ErrorCode.SetSpecificCode(HPAExceptionBuilder::GetCode());
    AddHandler(ErrorCode, &m_Builder);

    // add the builders.  Used the same builder for all the exceptions.
    AddBuilderForException(HPA_EXCEPTION, &m_Builder);
    AddBuilderForException(HPA_NO_TOKEN_EXCEPTION, &m_Builder);
    AddBuilderForException(HPA_GENERIC_EXCEPTION, &m_Builder);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HPAModule::~HPAModule()
    {
    }
