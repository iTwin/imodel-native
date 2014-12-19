//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCMemoryModule.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HFCMemoryModule.h>
#include <Imagepp/all/h/HFCMemoryModuleCodes.h>
#include <Imagepp/all/h/HFCErrorCodeReceiver.h>
#include <Imagepp/all/h/HFCErrorCodeGenerator.h>
#include <Imagepp/all/h/HFCException.h>

//-----------------------------------------------------------------------------
// Global Variable initialization
//-----------------------------------------------------------------------------

const uint32_t HFCMemoryModule::s_ModuleID = 0x010;


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCMemoryModule::HFCMemoryModule()
    : HFCErrorCodeModule(HFCMemoryModule::s_ModuleID)
    {
    HAutoPtr<HFCMemoryExceptionBuilder> pBuilder;
    HFCErrorCode ErrorCode;
    ErrorCode.SetModuleID(s_ModuleID);

    // add HFCMemoryExceptionBuilder
    ErrorCode.SetSpecificCode(HFCMemoryExceptionBuilder::GetCode());
    pBuilder = new HFCMemoryExceptionBuilder;
    AddHandler(ErrorCode, pBuilder);
    AddBuilderForException(HFC_MEMORY_EXCEPTION, pBuilder);
    m_Builders.push_back(pBuilder.release());

    // add HFCOutOfMemoryExceptionBuilder
    ErrorCode.SetSpecificCode(HFCOutOfMemoryExceptionBuilder::GetCode());
    pBuilder = new HFCOutOfMemoryExceptionBuilder;
    AddHandler(ErrorCode, pBuilder);
    AddBuilderForException(HFC_OUT_OF_MEMORY_EXCEPTION, pBuilder);
    m_Builders.push_back(pBuilder.release());

    // register the module into the generator and handler
    HFCErrorCodeReceiver::GetInstance().AddModule(this);
    HFCErrorCodeGenerator::GetInstance().AddModule(this);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCMemoryModule::~HFCMemoryModule()
    {
    for (Builders::iterator Itr = m_Builders.begin(); Itr != m_Builders.end(); Itr++)
        delete (*Itr);
    }
