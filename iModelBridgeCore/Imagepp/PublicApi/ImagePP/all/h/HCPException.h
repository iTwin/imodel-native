//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCPException.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Implementation of the HCS exception classes.  The exception hierarchy look
// like this:
//
//  HFCException
//        HFCDeviceException    (Info struct : HFCDeviceExInfo)
//            HCSException    (Info struct : HFCDeviceExInfo)
//----------------------------------------------------------------------------
#pragma once

#include "HFCException.h"

//----------------------------------------------------------------------------
// Class HCPException
//----------------------------------------------------------------------------
class HCPException : public HFCException
    {
    HDECLARE_CLASS_ID(6450, HFCException)
    HFC_DECLARE_COMMON_EXCEPTION_FNC()

public:
    // Primary methods.
    // Contructor and destructor.
    HCPException(ExceptionID pi_ExceptionID);
    virtual          ~HCPException();
    HCPException& operator=(const HCPException& pi_rObj);
    HCPException(const HCPException& pi_rObj);
    };


// ----------------------------------------------------------------------------
//  HCPGCoordException
// ----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Class HCPException
//----------------------------------------------------------------------------
class HCPGCoordException : public HCPException
    {
    HDECLARE_CLASS_ID(6451, HCPException)
    HFC_DECLARE_COMMON_EXCEPTION_FNC()

public:
    // Primary methods.
    // Contructor and destructor.
    HCPGCoordException();
    HCPGCoordException(int32_t pi_Code);
    virtual             ~HCPGCoordException();
    HCPGCoordException& operator=(const HCPGCoordException& pi_rObj);
    HCPGCoordException(const HCPGCoordException& pi_rObj);

    int32_t             GetErrorCode();
    };

