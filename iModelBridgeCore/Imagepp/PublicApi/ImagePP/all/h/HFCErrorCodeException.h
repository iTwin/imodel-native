//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCErrorCodeException.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include "HFCException.h"


class HFCErrorCodeException : public HFCException
    {
    HDECLARE_CLASS_ID(6008, HFCException)
    HFC_DECLARE_COMMON_EXCEPTION_FNC()

public:
    // Primary methods.
    // Contructor and destructor.
    HFCErrorCodeException(const HFCErrorCode&        pi_rErrorCode);
    virtual    ~HFCErrorCodeException();
    HFCErrorCodeException& operator=(const HFCErrorCodeException& pi_rObj);
    HFCErrorCodeException(const HFCErrorCodeException& pi_rObj);
    };


