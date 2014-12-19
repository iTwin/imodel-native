//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hps/src/HPSModule.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HPSModule.h>
#include <Imagepp/all/h/HFCErrorCodeReceiver.h>
#include <Imagepp/all/h/HFCErrorCodeGenerator.h>
#include <Imagepp/all/h/HPSException.h>

//-----------------------------------------------------------------------------
// Global Variable initialization
//-----------------------------------------------------------------------------

const uint32_t HPSModule::s_ModuleID = 0x021;


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HPSModule::HPSModule()
    : HPAModule(s_ModuleID)
    {
    // Change the module ID of the handler so that all the exceptions in
    // the parent class may have the same module ID as the ones here
    m_Builder.SetModuleID(s_ModuleID);

    // add the handler for all
    // Done by HPAModule ctor

    // add the builders.  Used the same builder for all the exceptions.
    AddBuilderForException(HPS_EXCEPTION, &m_Builder);
    AddBuilderForException(HPS_INCLUDE_NOT_FOUND_EXCEPTION, &m_Builder);
    AddBuilderForException(HPS_INVALID_OBJECT_EXCEPTION, &m_Builder);
    AddBuilderForException(HPS_TYPE_MISMATCH_EXCEPTION, &m_Builder);
    AddBuilderForException(HPS_IMAGE_EXPECTED_EXCEPTION, &m_Builder);
    AddBuilderForException(HPS_TRANSFO_EXPECTED_EXCEPTION, &m_Builder);
    AddBuilderForException(HPS_FILTER_EXPECTED_EXCEPTION, &m_Builder);
    AddBuilderForException(HPS_WORLD_EXPECTED_EXCEPTION, &m_Builder);
    AddBuilderForException(HPS_QUALIFIER_EXPECTED_EXCEPTION, &m_Builder);
    AddBuilderForException(HPS_EXPRESSION_EXPECTED_EXCEPTION, &m_Builder);
    AddBuilderForException(HPS_INVALID_NUMERIC_EXCEPTION, &m_Builder);
    AddBuilderForException(HPS_INVALID_URL_EXCEPTION, &m_Builder);
    AddBuilderForException(HPS_FILE_NOT_FOUND_EXCEPTION, &m_Builder);
    AddBuilderForException(HPS_NO_IMAGE_IN_FILE_EXCEPTION, &m_Builder);
    AddBuilderForException(HPS_OUT_OF_RANGE_EXCEPTION, &m_Builder);
    AddBuilderForException(HPS_ALPHA_PALETTE_NOT_SUPPORTED_EXCEPTION, &m_Builder);
    AddBuilderForException(HPS_INVALID_WORLD_EXCEPTION, &m_Builder);
    AddBuilderForException(HPS_TOO_FEW_PARAM_EXCEPTION, &m_Builder);
    AddBuilderForException(HPS_TOO_MANY_PARAM_EXCEPTION, &m_Builder);
    AddBuilderForException(HPS_ALREADY_DEFINED_EXCEPTION, &m_Builder);
    AddBuilderForException(HPS_TRANSFO_PARAMETERS_INVALID_EXCEPTION, &m_Builder);

    // register the module into the generator and handler
    HFCErrorCodeReceiver::GetInstance().AddModule(this);
    HFCErrorCodeGenerator::GetInstance().AddModule(this);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HPSModule::~HPSModule()
    {
    }
