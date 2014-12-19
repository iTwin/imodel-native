//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCErrorCodeModule.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HFCErrorCodeID.h"

class HFCErrorCode;
class HFCErrorCodeHandler;
class HFCErrorCodeBuilder;
class HFCException;

class HFCErrorCodeModule
    {
public:
    //--------------------------------------
    // Types
    //--------------------------------------

    // Map of handlers for error codes
    typedef map<uint32_t, const HFCErrorCodeHandler*>
    HandlerMap;

    // Map of builders for Image++ exceptions
    typedef map<HCLASS_ID, const HFCErrorCodeBuilder*>
    ExceptionBuilderMap;

    // Map of builders for OS exceptions
    typedef map<uint32_t, const HFCErrorCodeBuilder*>
    OSExceptionBuilderMap;

    // Map of builders for Image++ Status Code
    typedef map<HSTATUS, const HFCErrorCodeBuilder*>
    StatusBuilderMap;


    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    HFCErrorCodeModule(const HFCErrorCodeID& pi_rID);
    virtual         ~HFCErrorCodeModule();


    //--------------------------------------
    // Methods
    //--------------------------------------

    const HFCErrorCodeID&
    GetID() const;


    //--------------------------------------
    // Handler Methods
    //--------------------------------------

    // Adds a handler for a particular error code.
    _HDLLu virtual void    AddHandler(const HFCErrorCode&        pi_rCode,
                                      const HFCErrorCodeHandler* pi_pHandler);

    // Finds a handler for a particular error code
    _HDLLu virtual const HFCErrorCodeHandler*
    FindHandler(const HFCErrorCode& pi_rCode) const;


    //--------------------------------------
    // Builder Methods
    //--------------------------------------

    // Adds a handler for a particular error code.
    _HDLLu virtual void    AddBuilderForException  (unsigned short            pi_ID,
                                                    const HFCErrorCodeBuilder* pi_pHandler);
    _HDLLu virtual void    AddBuilderForOSException(uint32_t                 pi_ID,
                                                    const HFCErrorCodeBuilder* pi_pHandler);
    _HDLLu virtual void    AddBuilderForStatus     (HSTATUS                    pi_ID,
                                                    const HFCErrorCodeBuilder* pi_pHandler);

    // Finds a handler for a particular error code
    _HDLLu virtual const HFCErrorCodeBuilder*
    FindBuilderForException  (const HFCException& pi_rException) const;
    _HDLLu virtual const HFCErrorCodeBuilder*
    FindBuilderForOSException(uint32_t           pi_ID) const;
    _HDLLu virtual const HFCErrorCodeBuilder*
    FindBuilderForStatus     (HSTATUS             pi_ID) const;

    // Returns all the builders for a particular type of error
    const ExceptionBuilderMap&
    GetBuildersForException() const;
    const OSExceptionBuilderMap&
    GetBuildersForOSException() const;
    const StatusBuilderMap&
    GetBuildersForStatus() const;


private:
    //--------------------------------------
    // Attributes
    //--------------------------------------

    // The ID of the module
    HFCErrorCodeID          m_ID;

    // The handlers available for this module
    HandlerMap              m_Handlers;

    // The builders available for I++ exceptions
    ExceptionBuilderMap     m_ExceptionBuilders;
    OSExceptionBuilderMap   m_OSExceptionBuilders;
    StatusBuilderMap        m_StatusBuilders;


    //--------------------------------------
    // Disabled
    //--------------------------------------

    HFCErrorCodeModule(const HFCErrorCodeModule& pi_rObj);
    void            operator=(const HFCErrorCodeModule& pi_rObj);

    };
