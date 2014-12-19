//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCErrorCodeModule.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HFCErrorCodeModule.h>
#include <Imagepp/all/h/HFCErrorCode.h>
#include <Imagepp/all/h/HFCException.h>

//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HFCErrorCodeModule::AddHandler(const HFCErrorCode&        pi_rCode,
                                    const HFCErrorCodeHandler* pi_pHandler)
    {
    HPRECONDITION(m_Handlers.find(pi_rCode.GetSpecificCode()) == m_Handlers.end());

    m_Handlers.insert(HandlerMap::value_type(pi_rCode.GetSpecificCode(), pi_pHandler));
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
const HFCErrorCodeHandler*
HFCErrorCodeModule::FindHandler(const HFCErrorCode& pi_rCode) const
    {
    HandlerMap::const_iterator Itr = m_Handlers.find(pi_rCode.GetSpecificCode());

    if (Itr != m_Handlers.end())
        return (*Itr).second;
    else
        return 0;
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HFCErrorCodeModule::AddBuilderForException(unsigned short            pi_ID,
                                                const HFCErrorCodeBuilder* pi_pHandler)
    {
    HPRECONDITION(m_ExceptionBuilders.find(pi_ID) == m_ExceptionBuilders.end());

    m_ExceptionBuilders.insert(ExceptionBuilderMap::value_type(pi_ID, pi_pHandler));
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HFCErrorCodeModule::AddBuilderForOSException(uint32_t                  pi_ID,
                                                  const HFCErrorCodeBuilder* pi_pHandler)
    {
    HPRECONDITION(m_OSExceptionBuilders.find(pi_ID) == m_OSExceptionBuilders.end());

    m_OSExceptionBuilders.insert(OSExceptionBuilderMap::value_type(pi_ID, pi_pHandler));
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HFCErrorCodeModule::AddBuilderForStatus(HSTATUS                    pi_ID,
                                             const HFCErrorCodeBuilder* pi_pHandler)
    {
    HPRECONDITION(m_StatusBuilders.find(pi_ID) == m_StatusBuilders.end());

    m_StatusBuilders.insert(StatusBuilderMap::value_type(pi_ID, pi_pHandler));
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
const HFCErrorCodeBuilder*
HFCErrorCodeModule::FindBuilderForException  (const HFCException& pi_rException) const
    {
    ExceptionBuilderMap::const_iterator Itr = m_ExceptionBuilders.find(pi_rException.GetID());

    if (Itr != m_ExceptionBuilders.end())
        return (*Itr).second;
    else
        return 0;
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
const HFCErrorCodeBuilder*
HFCErrorCodeModule::FindBuilderForOSException(uint32_t pi_ID) const
    {
    OSExceptionBuilderMap::const_iterator Itr = m_OSExceptionBuilders.find(pi_ID);

    if (Itr != m_OSExceptionBuilders.end())
        return (*Itr).second;
    else
        return 0;
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
const HFCErrorCodeBuilder*
HFCErrorCodeModule::FindBuilderForStatus(HSTATUS pi_ID) const
    {
    StatusBuilderMap::const_iterator Itr = m_StatusBuilders.find(pi_ID);

    if (Itr != m_StatusBuilders.end())
        return (*Itr).second;
    else
        return 0;
    }

//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCErrorCodeModule::HFCErrorCodeModule(const HFCErrorCodeID& pi_rID)
    : m_ID(pi_rID)
    {
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCErrorCodeModule::~HFCErrorCodeModule()
    {
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
const HFCErrorCodeID& HFCErrorCodeModule::GetID() const
    {
    return (m_ID);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
const HFCErrorCodeModule::ExceptionBuilderMap&
    HFCErrorCodeModule::GetBuildersForException() const
    {
    return (m_ExceptionBuilders);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
const HFCErrorCodeModule::OSExceptionBuilderMap&
    HFCErrorCodeModule::GetBuildersForOSException() const
    {
    return (m_OSExceptionBuilders);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
const HFCErrorCodeModule::StatusBuilderMap&
    HFCErrorCodeModule::GetBuildersForStatus() const
    {
    return (m_StatusBuilders);
    }