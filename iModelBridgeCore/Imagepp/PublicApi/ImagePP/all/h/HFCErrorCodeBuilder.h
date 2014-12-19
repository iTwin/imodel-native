//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCErrorCodeBuilder.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

class HFCException;
class HFCErrorCode;

#include "HFCErrorCodeID.h"

class HFCErrorCodeBuilder
    {
public:
    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    HFCErrorCodeBuilder();
    HFCErrorCodeBuilder(const HFCErrorCodeID& pi_rID);
    virtual         ~HFCErrorCodeBuilder() = 0;


    //--------------------------------------
    // Information about the owning module
    //--------------------------------------

    const HFCErrorCodeID&
    GetModuleID() const;
    _HDLLu void            SetModuleID(const HFCErrorCodeID& pi_rID);


    //--------------------------------------
    // Handling method
    //--------------------------------------

    // At least one must be overridden by a derived builder class.
    virtual HFCErrorCode* BuildFromException  (const HFCException& pi_rException) const;
    virtual HFCErrorCode* BuildFromOSException(uint32_t             pi_Exception) const;
    virtual HFCErrorCode* BuildFromStatus     (HSTATUS             pi_Status) const;


protected:
    //--------------------------------------
    // Attributes
    //--------------------------------------

    HFCErrorCodeID  m_ModuleID;
    };


