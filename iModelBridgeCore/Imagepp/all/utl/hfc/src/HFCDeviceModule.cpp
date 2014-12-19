//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCDeviceModule.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HFCDeviceModule.h>
#include <Imagepp/all/h/HFCDeviceModuleCodes.h>
#include <Imagepp/all/h/HFCErrorCodeReceiver.h>
#include <Imagepp/all/h/HFCErrorCodeGenerator.h>
#include <Imagepp/all/h/HFCException.h>

//-----------------------------------------------------------------------------
// Global Variable initialization
//-----------------------------------------------------------------------------

const uint32_t HFCDeviceModule::s_ModuleID = 0x012;


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCDeviceModule::HFCDeviceModule()
    : HFCErrorCodeModule(HFCDeviceModule::s_ModuleID)
    {
    HAutoPtr<HFCDeviceExceptionBuilder> pBuilder;
    HFCErrorCode ErrorCode;
    ErrorCode.SetModuleID(s_ModuleID);

    // add HFC_DEVICE_EXCEPTION
    ErrorCode.SetSpecificCode(HFCDeviceExceptionBuilder::GetCode());
    pBuilder = new HFCDeviceExceptionBuilder;
    AddHandler(ErrorCode, pBuilder);
    AddBuilderForException(HFC_DEVICE_EXCEPTION, pBuilder);
    m_Builders.push_back(pBuilder.release());

    // add HFC_NO_DISK_SPACE_LEFT_EXCEPTION
    ErrorCode.SetSpecificCode(HFCNoDiskSpaceLeftExceptionBuilder::GetCode());
    pBuilder = new HFCNoDiskSpaceLeftExceptionBuilder;
    AddHandler(ErrorCode, pBuilder);
    AddBuilderForException(HFC_NO_DISK_SPACE_LEFT_EXCEPTION, pBuilder);
    m_Builders.push_back(pBuilder.release());

    // add HFC_DEVICE_ABORT_EXCEPTION
    ErrorCode.SetSpecificCode(HFCDeviceAbortExceptionBuilder::GetCode());
    pBuilder = new HFCDeviceAbortExceptionBuilder;
    AddHandler(ErrorCode, pBuilder);
    AddBuilderForException(HFC_DEVICE_ABORT_EXCEPTION, pBuilder);
    m_Builders.push_back(pBuilder.release());

    // register the module into the generator and handler
    HFCErrorCodeReceiver::GetInstance().AddModule(this);
    HFCErrorCodeGenerator::GetInstance().AddModule(this);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCDeviceModule::~HFCDeviceModule()
    {
    for (Builders::iterator Itr = m_Builders.begin(); Itr != m_Builders.end(); Itr++)
        delete (*Itr);
    }
